#include "ircmanager.hpp"

IrcManager::IrcManager(QObject *parent) : QObject(parent)
{

}

IrcManager::~IrcManager()
{

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
