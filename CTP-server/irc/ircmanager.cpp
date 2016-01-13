#include "ircmanager.hpp"

IrcManager::IrcManager(QObject *parent) : QObject(parent)
{

}

IrcManager::~IrcManager()
{

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
                    QString sendMessage = message.mid(message.indexOf(" ", 8));
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

void IrcManager::handleLogin(QTcpSocket* socket, const QString &message) // LOGIN <username> <password>
{
    qDebug()<<this<<"handling login from"<<socket;
    QStringList messageParameters = message.split(" ", QString::SkipEmptyParts);
    qDebug()<<messageParameters;
    if(messageParameters.count() == 3)
    {
        QString username = messageParameters.at(1);
        QString password = messageParameters.at(2);
        //TODO: implement passwords
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
            qDebug()<<socket<<"logged in with a username in use";
            socket->write("IN_USE\r\n");
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
