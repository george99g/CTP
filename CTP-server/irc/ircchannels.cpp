#include "ircchannels.hpp"

IrcChannels::IrcChannels(QSqlDatabase *db, QObject *parent) : QObject(parent)
{
    _db = db;
}

IrcChannels::~IrcChannels()
{
    for(unsigned i = 0; i < (unsigned)_channels.keys().count(); i++)
        removeChannel(_channels.keys().at(i));
}

bool IrcChannels::channelExists(const QString &channel)
{
    if(_channels.keys().contains(channel))
        return true;
    else return false;
}

void IrcChannels::createChannel(const QString &channel)
{
    if(!channelExists(channel))
    {
        _channels.insert(channel, new IrcChannel(channel));
        qDebug()<<this<<"created channel "<<channel;
    }
    return;
}

void IrcChannels::sendMessage(const QString &channel, const QString &message, const QString &sender)
{
    if(_channels.value(channel)->hasUser(sender))
        _channels.value(channel)->sendMessage(sender, message);
    return;
}

void IrcChannels::joinChannel(const QString &channel, const QString &username, QTcpSocket* socket)
{
    if(channelExists(channel))
        _channels.value(channel)->addUser(username, socket);
    return;
}

bool IrcChannels::hasUser(const QString &channel, const QString &username)
{
    if(channelExists(channel))
    {
        if(_channels.value(channel)->hasUser(username)) return true;
        else return false;
    }
    else return false;
}

void IrcChannels::removeChannel(const QString &channel)
{
    IrcChannel* channelToRemove = _channels.value(channel);
    channelToRemove->deleteLater();
    _channels.remove(channel);
    qDebug()<<this<<"removed channel "<<channel;
    return;
}

void IrcChannels::partChannel(const QString &channel, const QString &username)
{
    _channels.value(channel)->removeUser(username);
    return;
}

bool IrcChannels::isOnlyUser(const QString &channel, const QString &username)
{
    if(_channels.value(channel)->isOnlyUser(username))
        return true;
    else return false;
}

QString IrcChannels::generateChannelList()
{
    QString list = "";
    for(unsigned i = 0; i < (unsigned)_channels.count(); i++)
    {
        if(i != 0)
            list += " ";
        list += _channels.keys().at(i);
    }
    return list;
}
