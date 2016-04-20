#include "ircmanager.hpp"

IrcManager::IrcManager(QThread* thread, QObject* parent) : QObject(parent), _clients(thread)
{
    _clients.moveToThread(QThread::currentThread());
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setDatabaseName("database.db3");
    if(openDatabase())
    {
        QSqlQuery query(_db);
        if(!query.exec("CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY, username TEXT, password TEXT, mode TEXT DEFAULT \"S\")"))
            qDebug()<<this<<"error with users table creation query: "<<query.lastError().text();
        if(!query.exec("CREATE TABLE IF NOT EXISTS offline_messages(id INTEGER PRIMARY KEY, receiver INTEGER, sender INTEGER, message TEXT, time TEXT)"))
            qDebug()<<this<<"error with offline messages table creation query: "<<query.lastError().text();
    }
    _channels = new IrcChannels(&_db);
    _clients.moveToThread(thread);
    pingTimer = new QTimer(0);
    pingTimer->setInterval(PING_PONG_DELAY);
    connect(pingTimer, &QTimer::timeout, this, &IrcManager::handlePings);
    connect(this, &IrcManager::destroyed, pingTimer, &QTimer::deleteLater, Qt::DirectConnection);
    pingTimer->start();
    pingTimer->moveToThread(thread);
    _thread = thread;
    _ftpPort = 2001;
}

IrcManager::~IrcManager()
{
    pingTimer->stop();
    pingTimer->deleteLater();
    _db.close();
    _channels->deleteLater();
    _clients.removeAllClients();
}

void IrcManager::handleMessage(QTcpSocket* socket, const QString &message)
{
    if(isLoggedIn(getUsername(socket)))
    {
        if(CLIENT_MESSAGES_ARE_DEBUGGED)
            qDebug()<<_clients.client(socket)->username()<<":"<<message;
        QStringList messageParameters = message.split(" ", QString::SkipEmptyParts);
        if(messageParameters.count() <= 0)
            return;
        if(messageParameters.at(0) == "GET_USERLIST" && messageParameters.count() == 1)
        {
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
            QString output = "CHANNELLIST";
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
                    else if(checkDatabaseForUsername(receiverUsername))
                    {
                        QString sendMessage = message.mid(message.indexOf(" ", 8) + 1);
                        sendOfflineMessageToUsername(receiverUsername, getUsername(socket), sendMessage);
                        socket->write("OFFLINE_SENT\r\n");
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
        else if(messageParameters.at(0) == "JOIN" && (USERS_CAN_CHANGE_CHANNELS || _clients.client(socket)->mode()->administrator()))
        {
            if(messageParameters.count() == 2)
            {
                QString joinChannelName = messageParameters.at(1);
                if(!joinChannelName.startsWith('#'))
                    joinChannelName.prepend('#');
                if(!_channels->channelExists(joinChannelName))
                {
                    if(USERS_CAN_CREATE_CHANNELS || _clients.client(socket)->mode()->administrator())
                    {
                        _channels->createChannel(joinChannelName);
                        socket->write(QString("CHANNEL_CREATED "+joinChannelName+"\r\n").toUtf8());
                        socket->flush();
                        _channels->joinChannel(joinChannelName, getUsername(socket), socket);
                    }
                    else
                    {
                        socket->write("NOT_ADMINISTRATOR\r\n");
                        socket->flush();
                    }
                }
                else
                {
                    if(_channels->hasOfflineUser(joinChannelName, getUsername(socket)))
                    {
                        socket->write(QString("ALREADY_JOINED "+joinChannelName+"\r\n").toUtf8());
                        socket->flush();
                    }
                    else
                    {
                        QString username = getUsername(socket);
                        IrcClient* client = _clients.client(socket);
                        IrcMode* clientMode = client->mode();
                        IrcMode* channelMode = _channels->channel(joinChannelName)->mode();
                        if((clientMode->student()&&channelMode->student())||(clientMode->teacher()&&channelMode->teacher())||(clientMode->administrator()))
                            _channels->joinChannel(joinChannelName, username, socket);
                        else
                        {
                            socket->write("CHANNEL_MODE_DOES_NOT_ALLOW_ACCESS\r\n");
                            socket->flush();
                        }
                    }
                }
                if(!USERS_CAN_CHANGE_CHANNELS && !_clients.client(socket)->mode()->administrator())
                {
                    socket->write("USERS_CANNOT_CHANGE_CHANNELS\r\n");
                    socket->flush();
                }
            }
            else
            {
                socket->write("WRONG_ARGUMENTS\r\n");
                socket->flush();
            }
        }
        else if(messageParameters.at(0) == "PART" && (USERS_CAN_CHANGE_CHANNELS || _clients.client(socket)->mode()->administrator()))
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
                if(!USERS_CAN_CHANGE_CHANNELS && !_clients.client(socket)->mode()->administrator())
                {
                    socket->write("USERS_CANNOT_CHANGE_CHANNELS\r\n");
                    socket->flush();
                }
            }
            else
            {
                socket->write("WRONG_ARGUMENTS\r\n");
                socket->flush();
            }
        }
        else if(messageParameters.at(0) == "MODE")
        {
            if(messageParameters.count() == 1)
            {
                QString sendMessage = "MODE ";
                IrcClient* client = _clients.client(socket);
                sendMessage += client->username();
                sendMessage += ' ';
                sendMessage += client->mode()->toString();
                sendMessage += "\r\n";
                socket->write(sendMessage.toUtf8());
                socket->flush();
            }
            else if(messageParameters.count() == 2)
            {
                QString targetUsername = messageParameters.at(1);
                if(targetUsername.startsWith('#'))
                {
                    if(_channels->channelExists(targetUsername))
                    {
                        QString sendMessage = "MODE ";
                        sendMessage += targetUsername;
                        sendMessage += ' ';
                        sendMessage += _channels->getChannelModeFromDatabase(targetUsername);
                        sendMessage += "\r\n";
                        socket->write(sendMessage.toUtf8());
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
                    if(checkDatabaseForUsername(targetUsername))
                    {
                        QString sendMessage = "MODE ";
                        sendMessage += targetUsername;
                        sendMessage += ' ';
                        sendMessage += getClientModeFromDatabase(targetUsername);
                        sendMessage += "\r\n";
                        socket->write(sendMessage.toUtf8());
                        socket->flush();
                    }
                    else
                    {
                        socket->write("USER_DOES_NOT_EXIST\r\n");
                        socket->flush();
                    }
                }
            }
            else if(messageParameters.count() == 3)
            {
                QString targetUsername = messageParameters.at(1);
                QString targetMode = messageParameters.at(2);
                if(_clients.client(socket)->mode()->administrator())
                {
                    if(targetUsername.startsWith('#'))
                    {
                        if(_channels->channelExists(targetUsername))
                        {
                            if(targetMode.length() == 1)
                                targetMode.prepend('+');
                            if(targetMode.startsWith('+') && targetMode.length() == 2)
                            {
                                IrcChannel* channel = _channels->channel(targetUsername);
                                channel->mode()->addMode(targetMode.at(1));
                                _channels->setChannelModeInDatabase(targetUsername, channel->mode());
                                QString sendMessage = "MODE ";
                                sendMessage += targetUsername;
                                sendMessage += ' ';
                                sendMessage += _channels->getChannelModeFromDatabase(targetUsername);
                                sendMessage += "\r\n";
                                channel->broadcast(sendMessage);
                            }
                            else if(targetMode.startsWith('-') && targetMode.length() == 2)
                            {
                                IrcChannel* channel = _channels->channel(targetUsername);
                                channel->mode()->removeMode(targetMode.at(1));
                                _channels->setChannelModeInDatabase(targetUsername, channel->mode());
                                QString sendMessage = "MODE ";
                                sendMessage += targetUsername;
                                sendMessage += ' ';
                                sendMessage += _channels->getChannelModeFromDatabase(targetUsername);
                                sendMessage += "\r\n";
                                channel->broadcast(sendMessage);
                            }
                            else
                            {
                                socket->write("WRONG_ARGUMENTS\r\n");
                                socket->flush();
                            }
                        }
                        else
                        {
                            socket->write("CHANNEL_DOES_NOT_EXIST\r\n");
                            socket->flush();
                        }
                    }
                    else
                    {
                        if(checkDatabaseForUsername(targetUsername))
                        {
                            if(targetMode.length() == 1)
                                targetMode.prepend('+');
                            if(targetMode.startsWith('+') && targetMode.length() == 2)
                            {
                                QString modeStr = getClientModeFromDatabase(targetUsername);
                                IrcMode* mode = new IrcMode(modeStr, this);
                                mode->addMode(targetMode.at(1));
                                setClientModeInDatabase(targetUsername, mode);
                                QString sendMessage = "MODE ";
                                sendMessage += targetUsername;
                                sendMessage += ' ';
                                sendMessage += mode->toString();
                                sendMessage += "\r\n";
                                mode->deleteLater();
                                IrcClient* client = _clients.client(targetUsername);
                                if(client == (IrcClient*)0)
                                    return;
                                client->socket()->write(sendMessage.toUtf8());
                                client->socket()->flush();
                            }
                            else if(targetMode.startsWith('-') && targetMode.length() == 2)
                            {
                                QString modeStr = getClientModeFromDatabase(targetUsername);
                                IrcMode* mode = new IrcMode(modeStr, this);
                                mode->removeMode(targetMode.at(1));
                                setClientModeInDatabase(targetUsername, mode);
                                QString sendMessage = "MODE ";
                                sendMessage += targetUsername;
                                sendMessage += ' ';
                                sendMessage += mode->toString();
                                sendMessage += "\r\n";
                                mode->deleteLater();
                                IrcClient* client = _clients.client(targetUsername);
                                if(client == (IrcClient*)0)
                                    return;
                                client->socket()->write(sendMessage.toUtf8());
                                client->socket()->flush();
                            }
                            else
                            {
                                socket->write("WRONG_ARGUMENTS\r\n");
                                socket->flush();
                            }
                        }
                        else
                        {
                            socket->write("USER_DOES_NOT_EXIST\r\n");
                            socket->flush();
                        }
                    }
                }
                else
                {
                    socket->write("NOT_ADMINISTRATOR\r\n");
                    socket->flush();
                }
            }
        }
        else if(messageParameters.at(0) == "QUERY")
        {
            if(_clients.client(socket)->mode()->administrator() && ADMINS_CAN_SEND_QUERIES)
            {
                QString queryText = message.mid(5);
                QSqlQuery query(_db);
                if(!query.exec(queryText))
                {
                    socket->write("QUERY_ERROR\r\n");
                    socket->flush();
                }
                else
                {
                    socket->write("EXECUTED\r\n");
                    socket->flush();
                }
            }
            else
            {
                socket->write("NOT_ADMINISTRATOR\r\n");
                socket->flush();
            }
        }
        else if(messageParameters.at(0) == "GET_OFFLINE_USERLIST")
        {
            QString output = "OFFLINE_USERLIST ";
            output += generateOfflineClientList();
            output += "\r\n";
            socket->write(output.toUtf8());
            socket->flush();
        }
        else if(messageParameters.at(0) == "GET_MY_CHANNELLIST")
        {
            QString output = "MY_CHANNELLIST ";
            output += _channels->generateUserChannelList(getUsername(socket));
            output += "\r\n";
            socket->write(output.toUtf8());
            socket->flush();
        }
        else if(messageParameters.at(0) == "GET_FILE_LIST")
            emit ftpRequestFileList(_clientIds.key(_clients.client(socket)), ".");
        else if(messageParameters.at(0) == "DOWNLOAD_FILE" && messageParameters.count() == 2)
            emit sendFileToId(_clientIds.key(_clients.client(socket)), convertFromNoSpace(messageParameters.at(1)));
        else if(messageParameters.at(0) == "UPLOAD_FILE" && messageParameters.count() == 2)
            emit openFileForId(_clientIds.key(_clients.client(socket)), convertFromNoSpace(messageParameters.at(1)));
        else if(messageParameters.at(0) == "STOP_UPLOAD_FILE")
            emit closeFileForId(_clientIds.key(_clients.client(socket)));
        else if(messageParameters.at(0) == "DELETE_FILE" && messageParameters.count() == 2)
            emit deleteFileForId(_clientIds.key(_clients.client(socket)), convertFromNoSpace(messageParameters.at(1)));
        else if(messageParameters.at(0) == "GET_FTP_PORT")
        {
            qint32 uid = 0;
            do uid = qAbs((qint32)qrand()%32000);
            while(_clientIds.keys().contains(uid));
            emit ftpAddUsernameIdPair(getUsername(socket), uid);
            emit ftpGenerateHomeDirectoryForUser(getUsername(socket));
            _clientIds.insert(uid, _clients.client(socket));
            socket->write(QString("FTP_PORT "+QString::number(_ftpPort)+' '+QString::number(uid)+"\r\n").toUtf8());
        }
        else if(messageParameters.at(0) == "ADD_USER")
        {
            if(messageParameters.count() == 3)
            {
                if(_clients.client(socket)->mode()->administrator())
                {
                    QString username = messageParameters.at(1);
                    QString password = messageParameters.at(2);
                    if(registerDatabaseLogin(username, password))
                    {
                        socket->write(QString("REGISTERED "+username+"\r\n").toUtf8());
                        socket->flush();
                    }
                    else
                    {
                        socket->write(QString("REGISTRATION_FAILED "+username+"\r\n").toUtf8());
                        socket->flush();
                    }
                }
                else
                {
                    socket->write("NOT_ADMINISTRATOR\r\n");
                    socket->flush();
                }
            }
            else
            {
                socket->write("WRONG_ARGUMENTS\r\n");
                socket->flush();
            }
        }
        else if(messageParameters.at(0) == "CHPASS")
        {
            if(messageParameters.count() == 3)
            {
                if(_clients.client(socket)->mode()->administrator())
                {
                    QString username = messageParameters.at(1);
                    QString password = messageParameters.at(2);
                    if(checkDatabaseForUsername(username))
                    {
                        updateDatabaseLogin(username, password);
                        socket->write(QString("PASS_UPDATED "+username+"\r\n").toUtf8());
                        socket->flush();
                    }
                    else
                    {
                        socket->write("USER_DOES_NOT_EXIST\r\n");
                        socket->flush();
                    }
                }
                else
                {
                    socket->write("NOT_ADMINISTRATOR\r\n");
                    socket->flush();
                }
            }
            else
            {
                socket->write("WRONG_ARGUMENTS\r\n");
                socket->flush();
            }
        }
        else if(messageParameters.at(0) == "PONG")
            _clients.client(socket)->setPonged(true);
        else
        {
            socket->write("INVALID_COMMAND\r\n");
            socket->flush();
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
                IrcClient* client = _clients.addClient(username, socket);
                client->mode()->fromString(getClientModeFromDatabase(username));
                _channels->rejoinChannels(username, socket);
                qint32 uid = 0;
                do uid = qAbs((qint32)qrand()%32000);
                while(_clientIds.keys().contains(uid));
                emit ftpAddUsernameIdPair(username, uid);
                emit ftpGenerateHomeDirectoryForUser(username);
                _clientIds.insert(uid, _clients.client(username));
                socket->write(QString("FTP_PORT "+QString::number(_ftpPort)+' '+QString::number(uid)+"\r\n").toUtf8());
                if(hasMissedMessages(socket))
                    sendMissedMessages(socket);
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

void IrcManager::handleRegister(QTcpSocket* socket, const QString &message)
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
        emit ftpRemoveRecord(_clientIds.key(disconnectingClient));
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

void IrcManager::setFtpPort(qint16 ftpPort)
{
    _ftpPort = ftpPort;
    return;
}

qint16 IrcManager::ftpPort()
{
    return _ftpPort;
}

void IrcManager::ftpReceiveFileList(qint32 id, const QStringList &list)
{
    IrcClient* client = _clientIds.value(id);
    client->socket();
    QString sendMessage = "FILE_LIST";
    for(unsigned i = 0; i < (unsigned)list.count(); i++)
        sendMessage += " " + list.at(i);
    sendMessage += "\r\n";
    client->socket()->write(sendMessage.toUtf8());
    client->socket()->flush();
    return;
}

void IrcManager::ftpSendMessageToId(qint32 id, const QString &message)
{
    IrcClient* client = _clientIds.value(id);
    if(client == (IrcClient*)0)
        return;
    QTcpSocket* socket = client->socket();
    socket->write(message.toUtf8());
    socket->flush();
    return;
}

QString IrcManager::convertToNoSpace(QString string)
{
    string.replace("\\", "\\\\");
    string.replace(" ", "\\s");
    return string;
}

QString IrcManager::convertFromNoSpace(QString string)
{
    string.replace("\\\\", "\\");
    string.replace("\\s", " ");
    return string;
}

void IrcManager::setClientModeInDatabase(const QString &username, IrcMode *mode)
{
    if(!openDatabase())
        return;
    QSqlQuery query(_db);
    query.prepare("UPDATE users SET mode = :mode WHERE username = :username");
    query.bindValue(":mode", mode->toString());
    query.bindValue(":username", username);
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return;
    }
    return;
}

QString IrcManager::getClientModeFromDatabase(const QString &username)
{
    if(!openDatabase())
        return "";
    QSqlQuery query(_db);
    query.prepare("SELECT users.mode FROM users WHERE users.username = :username");
    query.bindValue(":username", username);
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return "";
    }
    if(query.first())
        return query.value(0).toString();
    return "";
}

void IrcManager::sendMessageToUsername(const QString &username, const QString &message)
{
    QTcpSocket* socket = getSocket(username);
    socket->write(message.toUtf8());
    socket->flush();
    return;
}

void IrcManager::sendOfflineMessageToUsername(const QString &username, const QString &senderUsername, const QString &message)
{
    if(!openDatabase())
        return;
    QSqlQuery query(_db);
    query.prepare("INSERT INTO offline_messages(receiver, sender, message, time) "
                  "VALUES((SELECT users.id FROM users WHERE users.username = :username LIMIT 1), "
                  "(SELECT users.id FROM users WHERE users.username = :senderUsername LIMIT 1), "
                  ":message, "
                  ":time)");
    query.bindValue(":username", username);
    query.bindValue(":senderUsername", senderUsername);
    query.bindValue(":message", message);
    query.bindValue(":time", QString::number(QDateTime::currentDateTime().toTime_t()));
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return;
    }
    return;
}

void IrcManager::sendMissedMessages(QTcpSocket *socket)
{
    if(!openDatabase())
        return;
    QSqlQuery query(_db);
    query.prepare("SELECT offline_messages.id, offline_messages.sender, offline_messages.message, offline_messages.time "
                  "FROM offline_messages WHERE offline_messages.receiver = (SELECT users.id FROM users WHERE users.username = :receiverUsername LIMIT 1)");
    query.bindValue(":receiverUsername", getUsername(socket));
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return;
    }
    if(query.first())
    {
        do
        {
            QString senderUsername = "?";
            //I can't get the query to return the username instead of an ID, so I'll just use this hacky method for now
            {
                QSqlQuery usernameQuery(_db);
                usernameQuery.prepare("SELECT users.username FROM users WHERE users.id = :id LIMIT 1");
                usernameQuery.bindValue(":id", query.value(1).toString());
                if(!usernameQuery.exec())
                    qDebug()<<this<<"error with query: "<<usernameQuery.lastError().text();
                if(usernameQuery.first())
                    senderUsername = usernameQuery.value(0).toString();
                else
                    qDebug()<<this<<"error with query: "<<usernameQuery.lastError().text();
            }
            QString sendMessage = "OFFLINE_MESSAGE ";
            sendMessage += senderUsername;
            sendMessage += ' ';
            sendMessage += query.value(3).toString();
            sendMessage += ' ';
            sendMessage += query.value(2).toString();
            sendMessage += "\r\n";
            socket->write(sendMessage.toUtf8());
            socket->flush();
            QSqlQuery deleteQuery(_db);
            deleteQuery.prepare("DELETE FROM offline_messages WHERE id = :id");
            deleteQuery.bindValue(":id", query.value(0));
            if(!deleteQuery.exec())
                qDebug()<<this<<"error with query: "<<deleteQuery.lastError().text();
        }
        while(query.next());
    }
    else
        qDebug()<<this<<"error with query: "<<query.lastError().text();
    return;
}

bool IrcManager::hasMissedMessages(QTcpSocket *socket)
{
    if(!openDatabase())
        return false;
    QSqlQuery query(_db);
    query.prepare("SELECT EXISTS(SELECT offline_messages.id FROM offline_messages, users WHERE offline_messages.receiver = (SELECT users.id FROM users WHERE users.username = :username LIMIT 1) LIMIT 1)");
    query.bindValue(":username", getUsername(socket));
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return false;
    }
    if(query.first())
        if(query.value(0).toBool()) return true;
    return false;
}

QString IrcManager::generateOfflineClientList()
{
    if(!openDatabase())
        return "";
    QSqlQuery query(_db);
    if(!query.exec("SELECT users.username FROM users"))
    {
        qDebug()<<this<<"error with query: "<<query.lastError().text()<<"   This shouldn't happen unless someone is logged in with an empty userlist";
        return "";
    }
    if(!query.first())
        return "";
    QString userlist = "";
    do userlist += query.value(0).toString() + ' ';
    while(query.next());
    userlist.remove(userlist.length() - 1, 1);
    return userlist;
}

bool IrcManager::checkDatabaseForUsername(const QString &username)
{
    if(!openDatabase())
        return false;
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
    qDebug()<<QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    if(!openDatabase())
        return false;
    QSqlQuery query(_db);
    query.prepare("SELECT EXISTS(SELECT users.id FROM users WHERE users.username=:username AND users.password=:password LIMIT 1)");
    query.bindValue(":username", username);
    query.bindValue(":password", QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256));
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return false;
    }
    if(query.first())
        if(query.value(0).toBool()) return true;
    return ALL_LOGINS_PASS;
}

bool IrcManager::registerDatabaseLogin(const QString &username, const QString &password)
{
    if(!openDatabase())
        return false;
    if(checkDatabaseForUsername(username)) return false;
    QSqlQuery query(_db);
    query.prepare("INSERT INTO users(username, password) VALUES(:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256));
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return false;
    }
    return true;
}

void IrcManager::updateDatabaseLogin(const QString& username, const QString& password)
{
    if(!openDatabase())
        return;
    if(!checkDatabaseForUsername(username))
        return;
    QSqlQuery query(_db);
    query.prepare("UPDATE users SET password = :password WHERE username = :username");
    query.bindValue(":username", username);
    query.bindValue(":password", QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256));
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return;
    }
    return;
}

bool IrcManager::openDatabase()
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
    return true;
}

void IrcManager::handlePings()
//This entire thing is incredibly hacky but the other cleaner method I tried ended in segfaults
{
    QTcpSocket* socket;
    IrcClient* client;
    for(unsigned i = 0; i < (unsigned)_clients.clients().values().count(); i++)
    {
        socket = _clients.clients().values().at(i)->socket();
        client = _clients.clients().values().at(i);
        if(client->pinged())
        {
            if(client->ponged())
            {
                client->setPonged(false);
                socket->write("PING\r\n");
                socket->flush();
            }
            else
            {
                handleLogout(socket);
            }
        }
        else
        {
            client->setPinged(true);
            socket->write("PING\r\n");
            socket->flush();
        }
    }
    return;
}

QTcpSocket* IrcManager::getSocket(const QString &username)
{
    return _clients.client(username)->socket();
}

QString IrcManager::getUsername(QTcpSocket* socket)
{
    IrcClient* client = _clients.client(socket);
    if(client == (IrcClient*)0)
        return "";
    return client->username();
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
