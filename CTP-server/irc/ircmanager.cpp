#include "ircmanager.hpp"

IrcManager::IrcManager(QObject *parent) : QObject(parent)
{
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setDatabaseName("database.db3");
    if(!_db.open())
    {
        qDebug()<<this<<"failed opening database file: "<<_db.lastError().text();
    }
    else
    {
        QSqlQuery query(_db);
        if(!query.exec("CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY, username TEXT, password TEXT, mode TEXT DEFAULT \"s\")"))
        {
            qDebug()<<this<<"error with table creation query: "<<query.lastError().text();
        }
    }
    _channels = new IrcChannels(&_db);
}

IrcManager::~IrcManager()
{
    _db.close();
    _channels->deleteLater();
    _clients.removeAllClients();
}

void IrcManager::handleMessage(QTcpSocket *socket, const QString &message)
{
    qDebug()<<this<<"handling message "<<message<<" from"<<socket;
    if(isLoggedIn(getUsername(socket)))
    {
        QStringList messageParameters = message.split(" ", QString::SkipEmptyParts);
        if(messageParameters.at(0) == "GET_USERLIST" && messageParameters.count() == 1)
        {
            qDebug()<<this<<"sending userlist to"<<socket;
            QString output = "USERLIST ";
            output += _clients.generateClientList();
            output += "\r\n";
            socket->write(output.toUtf8());
            socket->flush();
        }
        else if(messageParameters.at(0) == "GET_USERLIST")
        {
            if(messageParameters.count() == 2)
            {
                QString channel = messageParameters.at(1);
                if(!channel.startsWith('#'))
                    channel.prepend('#');
                if(_channels->channelExists(channel))
                {
                    QString output = "CHANNEL_USERLIST ";
                    output += channel;
                    output += ' ';
                    output += _channels->generateUserList(channel);
                    output += "\r\n";
                    socket->write(output.toUtf8());
                    socket->flush();
                }
                else
                {
                    socket->write("CHANNEL_DOES_NOT_EXIST\r\n");
                    socket->flush();
                }
            }
            else
            {
                socket->write("WRONG_ARGUMENTS\r\n");
                socket->flush();
            }
        }
        else if(messageParameters.at(0) == "GET_CHANNELLIST" && messageParameters.count() == 1)
        {
            qDebug()<<this<<"sending channel list to"<<socket;
            QString output = "CHANELLIST";
            output += " " + _channels->generateChannelList();
            output += "\r\n";
            socket->write(output.toUtf8());
            socket->flush();
        }
        else if(messageParameters.at(0) == "PRIVMSG")
        {
            if(messageParameters.count() > 2)
            {
                QString receiverUsername = messageParameters.at(1);
                if(!receiverUsername.startsWith('#'))
                {
                    if(isLoggedIn(receiverUsername))
                    {
                        QString sendMessage = getUsername(socket) + " PRIVMSG " + receiverUsername + " ";
                        sendMessage += message.mid(message.indexOf(" ", 8) + 1);
                        sendMessage += "\r\n";
                        sendMessageToUsername(receiverUsername, sendMessage);
                        socket->write("SENT\r\n");
                        socket->flush();
                    }
                    else
                    {
                        socket->write(QString("USER_IS_NOT_LOGGED_IN "+receiverUsername+"\r\n").toUtf8());
                        socket->flush();
                    }
                }
                else
                {
                    if(_channels->hasUser(receiverUsername, getUsername(socket)))
                    {
                        QString sendMessage = message.mid(message.indexOf(" ", 8) + 1);
                        _channels->sendMessage(receiverUsername, sendMessage, getUsername(socket));
                    }
                    else
                    {
                        socket->write(QString("NOT_IN_CHANNEL "+receiverUsername+"\r\n").toUtf8());
                        socket->flush();
                    }
                }
            }
            else
            {
                socket->write("WRONG_ARGUMENTS\r\n");
                socket->flush();
            }
        }
        else if(messageParameters.at(0) == "JOIN")
        {
            if(messageParameters.count() == 2)
            {
                QString joinChannelName = messageParameters.at(1);
                if(!joinChannelName.startsWith('#'))
                    joinChannelName.prepend('#');
                if(!_channels->channelExists(joinChannelName))
                {
                    _channels->createChannel(joinChannelName);
                    _channels->joinChannel(joinChannelName, getUsername(socket), socket);
                    socket->write(QString("CHANNEL_CREATED "+joinChannelName+"\r\n").toUtf8());
                    socket->flush();
                }
                else
                {
                    if(!_channels->hasOfflineUser(joinChannelName, getUsername(socket)))
                        _channels->joinChannel(joinChannelName, getUsername(socket), socket);
                    else
                    {
                        socket->write(QString("ALREADY_JOINED "+joinChannelName+"\r\n").toUtf8());
                        socket->flush();
                    }
                }
            }
            else
            {
                socket->write("WRONG_ARGUMENTS\r\n");
                socket->flush();
            }
        }
        else if(messageParameters.at(0) == "PART")
        {
            if(messageParameters.count() == 2)
            {
                QString partChannelName = messageParameters.at(1);
                if(!partChannelName.startsWith('#'))
                    partChannelName.prepend('#');
                if(_channels->channelExists(partChannelName))
                {
                    if(_channels->hasUser(partChannelName, getUsername(socket)))
                    {
                        if(_channels->isOnlyUser(partChannelName, getUsername(socket)))
                        {
                            _channels->partChannel(partChannelName, getUsername(socket));
                            _channels->removeChannel(partChannelName);
                        }
                        else
                            _channels->partChannel(partChannelName, getUsername(socket));
                    }
                    else
                    {
                        socket->write(QString("NOT_IN_CHANNEL "+partChannelName+"\r\n").toUtf8());
                        socket->flush();
                    }
                }
                else
                {
                    socket->write(QString("NOT_IN_CHANNEL "+partChannelName+"\r\n").toUtf8());
                    socket->flush();
                }
            }
            else
            {
                socket->write("WRONG_ARGUMENTS\r\n");
                socket->flush();
            }
        }
    }
    else
    {
        socket->write("NOT_LOGGED_IN\r\n");
        socket->flush();
    }
    return;
}

void IrcManager::handleLogin(QTcpSocket* socket, const QString &message)
{
    qDebug()<<this<<"handling login from"<<socket;
    QStringList messageParameters = message.split(" ", QString::SkipEmptyParts);
    if(messageParameters.count() == 3)
    {
        QString username = messageParameters.at(1);
        QString password = messageParameters.at(2);
        if(checkDatabaseForLogin(username, password))
        {
            if(!_clients.hasClient(username))
            {
                qDebug()<<this<<"successfully logged in"<<socket<<" as"<<username;
                broadcast("CONNECT "+username+"\r\n");
                socket->write(QString("AUTH "+username+"\r\n").toUtf8());
                socket->flush();
                _clients.addClient(username, socket);
                _channels->rejoinChannels(username, socket);
            }
            else
            {
                qDebug()<<socket<<"tried logging in with a username in use";
                socket->write(QString("IN_USE "+username+"\r\n").toUtf8());
                socket->flush();
            }
        }
        else
        {
            qDebug()<<socket<<"tried logging in with invalid login information";
            socket->write(QString("LOGIN_FAIL "+username+"\r\n").toUtf8());
            socket->flush();
        }
    }
    else
    {
        qDebug()<<this<<"login denied";
        socket->write("WRONG_ARGUMENTS\r\n");
        socket->flush();
    }
    return;
}

void IrcManager::handleRegister(QTcpSocket *socket, const QString &message)
{
    qDebug()<<this<<"handling login from"<<socket;
    QStringList messageParameters = message.split(" ", QString::SkipEmptyParts);
    if(messageParameters.count() == 3)
    {
        QString username = messageParameters.at(1);
        QString password = messageParameters.at(2);
        if(registerDatabaseLogin(username, password))
        {
            qDebug()<<this<<"successfully registered "<<username;
            socket->write(QString("REGISTERED "+username+"\r\n").toUtf8());
            socket->flush();
        }
        else
        {
            qDebug()<<this<<"failed registration";
            socket->write(QString("REGISTRATION_FAILED "+username+"\r\n").toUtf8());
            socket->flush();
        }
    }
    else
    {
        qDebug()<<this<<"registration denied";
        socket->write("WRONG_ARGUMENTS\r\n");
        socket->flush();
    }
    return;
}

void IrcManager::handleLogout(QTcpSocket *socket)
{
    qDebug()<<this<<"handling logout from "<<socket;
    if(!_clients.hasClient(socket))
    {
        qDebug()<<socket<<"logged out without login";
        socket->close();
    }
    else
    {
        IrcClient* disconnectingClient = _clients.client(socket);
        _channels->clearUser(disconnectingClient->username());
        _clients.removeClient(disconnectingClient);
        broadcast("DISCONNECT "+disconnectingClient->username()+"\r\n");
        socket->close();
    }
    return;
}

void IrcManager::handleConnection(QTcpSocket* socket)
{
    qDebug()<<this<<"handling connection from"<<socket;
    return;
}

void IrcManager::handleDisconnection(QTcpSocket* socket)
{
    qDebug()<<this<<"handling disconnection from"<<socket;
    if(_clients.hasClient(socket))
        handleLogout(socket);
    qDebug()<<socket<<"disconnected without login.";
    return;
}

void IrcManager::sendMessageToUsername(const QString &username, const QString &message)
{
    QTcpSocket* socket = getSocket(username);
    socket->write(message.toUtf8());
    socket->flush();
    return;
}

bool IrcManager::checkDatabaseForUsername(const QString &username)
{
    if(!_db.isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db.open())
        {
            qDebug()<<this<<"database failed to open: "<<_db.lastError().text();
            return false;
        }
    }
    QSqlQuery query(_db);
    query.prepare("SELECT EXISTS(SELECT users.username FROM users WHERE users.username=:username LIMIT 1)");
    query.bindValue(":username", username);
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return false;
    }
    if(query.first())
        if(query.value(0).toBool()) return true;
    return false;
}

bool IrcManager::checkDatabaseForLogin(const QString &username, const QString &password)
{
    if(!_db.isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db.open())
        {
            qDebug()<<this<<"database failed to open: "<<_db.lastError().text();
            return false;
        }
    }
    QSqlQuery query(_db);
    query.prepare("SELECT EXISTS(SELECT users.id FROM users WHERE users.username=:username AND users.password=:password LIMIT 1)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return false;
    }
    if(query.first())
        if(query.value(0).toBool()) return true;
    return false;
}

bool IrcManager::registerDatabaseLogin(const QString &username, const QString &password)
{
    if(!_db.isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db.open())
        {
            qDebug()<<this<<"database failed to open: "<<_db.lastError().text();
            return false;
        }
    }
    if(checkDatabaseForUsername(username)) return false;
    QSqlQuery query(_db);
    query.prepare("INSERT INTO users(username, password) VALUES(:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return false;
    }
    return true;
}

QTcpSocket* IrcManager::getSocket(const QString &username)
{
    return _clients.client(username)->socket();
}

QString IrcManager::getUsername(QTcpSocket *socket)
{
    return _clients.client(socket)->username();
}

bool IrcManager::isLoggedIn(const QString &username)
{
    if(_clients.hasClient(username)) return true;
    else return false;
}

void IrcManager::broadcast(const QString &message)
{
    _clients.broadcast(message);
    return;
}
