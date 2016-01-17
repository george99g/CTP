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
        if(!query.exec("CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY, username TEXT, password TEXT)"))
        {
            qDebug()<<this<<"error with table creation query: "<<query.lastError().text();
        }
    }
}

IrcManager::~IrcManager()
{
    _db.close();
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
            QString output = "USERLIST";
            for(unsigned i = 0; i < (unsigned)_usernames.values().count(); i++)
            {
                output += " " + _usernames.values().at(i);
            }
            output += "\r\n";
            socket->write(output.toUtf8());
            socket->flush();
        }
        else if(messageParameters.at(0) == "PRIVMSG")
        {
            if(messageParameters.count() > 2)
            {
                QString receiverUsername = messageParameters.at(1);
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
                    socket->write("USER_IS_NOT_LOGGED_IN\r\n");
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
            if(!_usernames.values().contains(username))
            {
                qDebug()<<this<<"successfully logged in"<<socket<<" as"<<username;
                broadcast("CONNECT "+username+"\r\n");
                socket->write("AUTH\r\n");
                socket->flush();
                _usernames.insert(socket, username);
            }
            else
            {
                qDebug()<<socket<<"tried logging in with a username in use";
                socket->write("IN_USE\r\n");
                socket->flush();
            }
        }
        else
        {
            qDebug()<<socket<<"tried logging in with invalid login information";
            socket->write("LOGIN_FAIL\r\n");
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
            socket->write("REGISTERED\r\n");
            socket->flush();
        }
        else
        {
            qDebug()<<this<<"failed registration";
            socket->write("REGISTRATION_FAILED\r\n");
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
    if(!_usernames.contains(socket))
    {
        qDebug()<<socket<<"logged out without login";
        socket->close();
    }
    else
    {
        QString disconnectingUsername = _usernames.value(socket);
        _usernames.remove(socket);
        broadcast("DISCONNECT "+disconnectingUsername+"\r\n");
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
    if(_usernames.contains(socket))
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
    return _usernames.key(username);
}

QString IrcManager::getUsername(QTcpSocket *socket)
{
    return _usernames.value(socket, "");
}

bool IrcManager::isLoggedIn(const QString &username)
{
    if(_usernames.values().contains(username)) return true;
    else return false;
}

void IrcManager::broadcast(const QString &message)
{
    QTcpSocket* socket;
    for(unsigned i = 0; i < (unsigned)_usernames.keys().count(); i++)
    {
        socket = _usernames.keys().at(i);
        socket->write(message.toUtf8());
        socket->flush();
    }
    return;
}
