#include "ircchannel.hpp"

IrcChannel::IrcChannel(QString name, QObject *parent) : QObject(parent)
{
    _name = name;
}

IrcChannel::~IrcChannel()
{
    clearAllUsers();
}

void IrcChannel::addUser(const QString &username, QTcpSocket* socket)
{
    if(!_userlist.values().contains(username))
    {
        sendJoinMessage(username);
        _userlist.insert(socket, username);
        if(!_offlineUserlist.contains(username))
            _offlineUserlist.append(username);
        socket->write(QString("CHANNEL_JOINED "+_name+"\r\n").toUtf8());
        socket->flush();
    }
    return;
}

void IrcChannel::rejoinUser(const QString &username, QTcpSocket* socket)
{
    if(!_userlist.values().contains(username) && _offlineUserlist.contains(username))
        _userlist.insert(socket, username);
    return;
}

void IrcChannel::sendJoinMessage(const QString &sender)
{
    for(unsigned i = 0; i < (unsigned)_userlist.keys().count(); i++)
    {
        QTcpSocket* socket = _userlist.keys().at(i);
        QString message = sender;
        message += " JOIN ";
        message += _name;
        message += "\r\n";
        socket->write(message.toUtf8());
        socket->flush();
    }
    return;
}

void IrcChannel::sendPartMessage(const QString &sender)
{
    for(unsigned i = 0; i < (unsigned)_userlist.keys().count(); i++)
    {
        QTcpSocket* socket = _userlist.keys().at(i);
        QString message = sender;
        message += " PART ";
        message += _name;
        message += "\r\n";
        socket->write(message.toUtf8());
        socket->flush();
    }
    return;
    return;
}

void IrcChannel::sendMessage(const QString &sender, const QString &message)
{
    for(unsigned i = 0; i < (unsigned)_userlist.keys().count(); i++)
    {
        QTcpSocket* socket = _userlist.keys().at(i);
        if(_userlist.values().at(i) != sender)
        {
            socket->write(QString(sender + " PRIVMSG " + _name + " " + message + "\r\n").toUtf8());
            socket->flush();
        }
        else
        {
            socket->write("SENT\r\n");
            socket->flush();
        }
    }
}

void IrcChannel::broadcast(const QString &message)
{
    for(unsigned i = 0; i < (unsigned)_userlist.keys().count(); i++)
    {
        QTcpSocket* socket = _userlist.keys().at(i);
        socket->write(message.toUtf8());
        socket->flush();
    }
    return;
}

void IrcChannel::removeUser(const QString &username)
{
    if(_offlineUserlist.contains(username))
    {
        QTcpSocket* socket = _userlist.key(username);
        socket->write(QString("CHANNEL_PARTED "+_name+"\r\n").toUtf8());
        socket->flush();
        _userlist.remove(socket);
        _offlineUserlist.removeAll(username);
        sendPartMessage(username);
    }
    return;
}

void IrcChannel::clearUser(const QString &username)
{
    if(_userlist.values().contains(username))
    {
        QTcpSocket* socket = _userlist.key(username);
        _userlist.remove(socket);
    }
    return;
}

void IrcChannel::removeAllUsers()
{
    for(unsigned i = 0; i < (unsigned)_userlist.values().count(); i++)
        removeUser(_userlist.values().at(i));
    return;
}

void IrcChannel::clearAllUsers()
{
    for(unsigned i = 0; i < (unsigned)_userlist.keys().count(); i++)
        _userlist.remove(_userlist.keys().at(i));
}

bool IrcChannel::hasUser(const QString &username)
{
    if(_userlist.values().contains(username))
        return true;
    else return false;
}

bool IrcChannel::hasOfflineUser(const QString &username)
{
    if(_offlineUserlist.contains(username))
        return true;
    else return false;
}

bool IrcChannel::isOnlyUser(const QString &username)
{
    if(_offlineUserlist.count() == 1 && _offlineUserlist.contains(username))
        return true;
    else return false;
}

IrcMode *IrcChannel::mode()
{
    return &_mode;
}

QMap<QTcpSocket*, QString>* IrcChannel::userlist()
{
    return &_userlist;
}

QStringList *IrcChannel::offlineUserlist()
{
    return &_offlineUserlist;
}
