#include "ircchannels.hpp"

IrcChannels::IrcChannels(QSqlDatabase* db, QObject* parent) : QObject(parent)
{
    _db = db;
    openDatabase();
    if(_db->isOpen())
    {
        QSqlQuery query(*_db);
        if(!query.exec("CREATE TABLE IF NOT EXISTS channels(id INTEGER PRIMARY KEY, name TEXT, mode TEXT DEFAULT \"ST\")"))
            qDebug()<<this<<"error with channels table creation query: "<<query.lastError().text();
        if(!query.exec("CREATE TABLE IF NOT EXISTS userlists(id INTEGER, user TEXT, last_message INTEGER)"))
            qDebug()<<this<<"error with userlists table creation query: "<<query.lastError().text();
        if(!query.exec("CREATE TABLE IF NOT EXISTS offline_channel_messages(id INTEGER PRIMARY KEY, channel TEXT, sender TEXT, message TEXT, time TEXT)"))
            qDebug()<<this<<"error with offline channel messages table creation query: "<<query.lastError().text();
        qDebug()<<this<<"loading channels from database";
        loadChannelsFromDatabase();
    }
}

IrcChannels::~IrcChannels()
{
    for(unsigned i = 0; i < (unsigned)_channels.keys().count(); i++)
        clearChannel(_channels.keys().at(i));
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
        IrcChannel* channelptr = new IrcChannel(channel);
        _channels.insert(channel, channelptr);
        insertChannelIntoDatabase(channel);
        channelptr->mode()->fromString(getChannelModeFromDatabase(channel));
        qDebug()<<this<<"created channel "<<channel;
    }
    return;
}

void IrcChannels::sendMessage(const QString &channel, const QString &message, const QString &sender)
{
    if(_channels.value(channel)->hasUser(sender))
    {
        _channels.value(channel)->sendMessage(sender, message);
        QSqlQuery query(*_db);
        query.prepare("INSERT INTO offline_channel_messages(channel, sender, message, time) VALUES(:channel, :sender, :message, :time)");
        query.bindValue(":channel", channel);
        query.bindValue(":sender", sender);
        query.bindValue(":message", message);
        query.bindValue(":time", QString::number(QDateTime::currentDateTime().toTime_t()));
        if(!query.exec())
            qDebug()<<this<<"error with query: "<<query.lastError().text();
    }
    return;
}

void IrcChannels::joinChannel(const QString &channel, const QString &username, QTcpSocket* socket)
{
    if(channelExists(channel))
        _channels.value(channel)->addUser(username, socket);
    insertUserIntoChannelDatabase(channel, username);
    return;
}

void IrcChannels::rejoinChannels(const QString &username, QTcpSocket* socket)
{
    for(unsigned i = 0; i < (unsigned)_channels.values().count(); i++)
    {
        IrcChannel* channel = _channels.values().at(i);
        if(channel->hasOfflineUser(username) && !channel->hasUser(username))
            channel->rejoinUser(username, socket);
    }
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

bool IrcChannels::hasOfflineUser(const QString &channel, const QString &username)
{
    if(channelExists(channel))
    {
        if(_channels.value(channel)->hasOfflineUser(username)) return true;
        else return false;
    }
    else return false;
}

void IrcChannels::removeChannel(const QString &channel)
{
    IrcChannel* channelToRemove = _channels.value(channel);
    removeChannelFromDatabase(channel);
    channelToRemove->removeAllUsers();
    channelToRemove->deleteLater();
    _channels.remove(channel);
    qDebug()<<this<<"removed channel "<<channel;
    return;
}

void IrcChannels::clearChannel(const QString &channel)
{
    IrcChannel* channelToRemove = _channels.value(channel);
    channelToRemove->deleteLater();
    _channels.remove(channel);
    qDebug()<<this<<"cleared channel "<<channel;
    return;
}

void IrcChannels::clearUser(const QString &username)
{
    for(unsigned i = 0; i < (unsigned)_channels.values().count(); i++)
        _channels.values().at(i)->clearUser(username);
    return;
}

void IrcChannels::partChannel(const QString &channel, const QString &username)
{
    _channels.value(channel)->removeUser(username);
    removeUserFromChannelDatabase(channel, username);
    return;
}

bool IrcChannels::isOnlyUser(const QString &channel, const QString &username)
{
    if(_channels.value(channel)->isOnlyUser(username))
        return true;
    else return false;
}

IrcChannel* IrcChannels::channel(const QString &channel)
{
    if(channelExists(channel))
        return _channels.value(channel);
    else
        return (IrcChannel*)0;
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

QString IrcChannels::generateUserList(const QString &channel)
{
    QString list = "";
    QStringList* userlist = _channels.value(channel)->offlineUserlist();
    for(unsigned i = 0; i < (unsigned)userlist->count(); i++)
    {
        if(i != 0)
            list += " ";
        list += userlist->at(i);
    }
    return list;
}

QString IrcChannels::generateUserChannelList(const QString &username)
{
    if(!openDatabase())
        return "";
    QSqlQuery query(*_db);
    query.prepare("SELECT channels.name FROM channels WHERE channels.id IN ("
                  "SELECT userlists.id FROM userlists WHERE userlists.user = :username)");
    query.bindValue(":username", username);
    if(!query.exec())
    {
        qDebug()<<this<<"error with query: "<<query.lastError().text();
        return "";
    }
    if(!query.first())
        return "";
    QString list = "";
    do list += query.value(0).toString() + ' ';
    while(query.next());
    list.remove(list.length() - 1, 1);
    return list;
}

bool IrcChannels::openDatabase()
{
    if(!_db->isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db->open())
        {
            qDebug()<<this<<"database failed to open: "<<_db->lastError().text();
            return false;
        }
    }
    return true;
}

void IrcChannels::setChannelModeInDatabase(const QString &channel, IrcMode* mode)
{
    if(!openDatabase())
        return;
    QSqlQuery query(*_db);
    query.prepare("UPDATE channels SET mode = :mode WHERE name = :channelname");
    query.bindValue(":mode", mode->toString());
    query.bindValue(":channelname", channel);
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return;
    }
    return;
}

QString IrcChannels::getChannelModeFromDatabase(const QString &channel)
{
    if(!openDatabase())
        return "";
    QSqlQuery query(*_db);
    query.prepare("SELECT channels.mode FROM channels WHERE channels.name = :channelname");
    query.bindValue(":channelname", channel);
    if(!query.exec())
    {
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
        return "";
    }
    if(query.first())
        return query.value(0).toString();
    return "";
}

void IrcChannels::loadChannelsFromDatabase()
{
    if(!openDatabase())
        return;
    QSqlQuery query(*_db);
    if(!query.exec("SELECT channels.name FROM channels"))
        qDebug()<<this<<"error with query: "<<query.lastError().text();
    if(query.first())
    {
        do
        {
            qDebug()<<this<<"loading channel";
            loadChannelFromDatabase(query.value(0).toString());
        }
        while(query.next());
    }
    else
        qDebug()<<this<<"error with query: "<<query.lastError().text()<<" This is probably due to a lack of saved channels";
    return;
}

void IrcChannels::loadChannelFromDatabase(const QString &channel)
{
    if(!channelExists(channel))
    {
        IrcChannel* channelptr = new IrcChannel(channel);
        _channels.insert(channel, channelptr);
        channelptr->mode()->fromString(getChannelModeFromDatabase(channel));
        qDebug()<<this<<"created channel "<<channel;
        if(!openDatabase())
            return;
        QSqlQuery query(*_db);
        query.prepare("SELECT userlists.user FROM channels, userlists WHERE userlists.id = (SELECT channels.id FROM channels WHERE channels.name = :channelname LIMIT 1)");
        query.bindValue(":channelname", channel);
        if(!query.exec())
            qDebug()<<this<<"error with query: "<<query.lastError().text();
        if(query.first())
        {
            do
                channelptr->offlineUserlist()->append(query.value(0).toString());
            while(query.next());
        }
        else
            qDebug()<<this<<"error with query: "<<query.lastError().text()<<" Channels exist yet there are no users in them. Is the database okay?";
    }
    return;
}

void IrcChannels::insertChannelIntoDatabase(const QString &channel)
{
    if(!openDatabase())
        return;
    QSqlQuery query(*_db);
    query.prepare("INSERT INTO channels(name) VALUES(:channelname)");
    query.bindValue(":channelname", channel);
    if(!query.exec())
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
    return;
}

void IrcChannels::insertUserIntoChannelDatabase(const QString &channel, const QString &user)
{
    if(!openDatabase())
        return;
    QSqlQuery query(*_db);
    query.prepare("INSERT INTO userlists(id, user) VALUES((SELECT channels.id FROM channels WHERE channels.name = :channelname LIMIT 1), :username)");
    query.bindValue(":channelname", channel);
    query.bindValue(":username", user);
    if(!query.exec())
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
    return;
}

void IrcChannels::removeChannelFromDatabase(const QString &channel)
{
    if(!openDatabase())
        return;
    QStringList* users = _channels.value(channel)->offlineUserlist();
    for(unsigned i = 0; i < (unsigned)users->count(); i++)
        removeUserFromChannelDatabase(channel, users->at(i));
    QSqlQuery query(*_db);
    query.prepare("DELETE FROM channels WHERE channels.name = :channelname");
    query.bindValue(":channelname", channel);
    if(!query.exec())
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
    return;
}

void IrcChannels::removeUserFromChannelDatabase(const QString &channel, const QString &user)
{
    if(!openDatabase())
        return;
    QSqlQuery query(*_db);
    query.prepare("DELETE FROM userlists WHERE userlists.id = (SELECT channels.id FROM channels WHERE channels.name = :channelname LIMIT 1) AND userlists.user = :username");
    query.bindValue(":channelname", channel);
    query.bindValue(":username", user);
    if(!query.exec())
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
    return;
}
