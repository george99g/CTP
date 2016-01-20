#include "ircchannel.hpp"

IrcChannel::IrcChannel(QString name, QObject *parent) : QObject(parent)
{
    _name = name;
}

IrcChannel::~IrcChannel()
{
    for(unsigned i = 0; i < (unsigned)_userlist.keys().count(); i++)
    {
        _userlist.remove(_userlist.keys().at(i));
    }
}

void IrcChannel::addUser(const QString &username, QTcpSocket *socket)
{
    if(!_userlist.values().contains(username))
    {
        _userlist.insert(socket, username);
        socket->write(QString("CHANNEL_JOINED "+_name+"\r\n").toUtf8());
        socket->flush();
    }
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

void IrcChannel::removeUser(const QString &username)
{
    QTcpSocket* socket = _userlist.key(username);
    socket->write(QString("CHANNEL_PARTED "+_name+"\r\n").toUtf8());
    socket->flush();
    _userlist.remove(socket);
    return;
}

void IrcChannel::removeAllUsers()
{
    for(unsigned i = 0; i < (unsigned)_userlist.values().count(); i++)
        removeUser(_userlist.values().at(i));
}

bool IrcChannel::hasUser(const QString &username)
{
    if(_userlist.values().contains(username))
        return true;
    else return false;
}

bool IrcChannel::isOnlyUser(const QString &username)
{
    if(_userlist.count() == 1 && _userlist.values().contains(username))
        return true;
    else return false;
}

QMap<QTcpSocket*, QString> IrcChannel::userlist()
{
    return _userlist;
}
