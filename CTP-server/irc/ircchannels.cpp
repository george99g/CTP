#include "ircchannels.hpp"

IrcChannels::IrcChannels(QSqlDatabase *db, QObject *parent) : QObject(parent)
{
    _db = db;
    if(!_db->isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db->open())
            qDebug()<<this<<"database failed to open: "<<_db->lastError().text();
    }
    if(_db->isOpen())
    {
        QSqlQuery query(*_db);
        if(!query.exec("CREATE TABLE IF NOT EXISTS channels(id INTEGER PRIMARY KEY, name TEXT, mode TEXT)"))
            qDebug()<<this<<"error with channels table creation query: "<<query.lastError().text();
        if(!query.exec("CREATE TABLE IF NOT EXISTS userlists(id INTEGER, user INTEGER)"))
            qDebug()<<this<<"error with userlists table creating query: "<<query.lastError().text();
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
        _channels.insert(channel, new IrcChannel(channel));
        insertChannelIntoDatabase(channel);
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
    insertUserIntoChannelDatabase(channel, username);
    return;
}

void IrcChannels::rejoinChannels(const QString &username, QTcpSocket *socket)
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

void IrcChannels::loadChannelsFromDatabase()
{
    if(!_db->isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db->open())
        {
            qDebug()<<this<<"database failed to open: "<<_db->lastError().text();
            return;
        }
    }
    QSqlQuery query(*_db);
    query.prepare("SELECT channels.name FROM channels");

    return;
}

void IrcChannels::loadChannelFromDatabase(const QString &channel)
{

    return;
}

void IrcChannels::insertChannelIntoDatabase(const QString &channel)
{
    if(!_db->isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db->open())
        {
            qDebug()<<this<<"database failed to open: "<<_db->lastError().text();
            return;
        }
    }
    QSqlQuery query(*_db);
    query.prepare("INSERT INTO channels(name, mode) VALUES(:channelname, :mode)");
    query.bindValue(":channelname", channel);
    query.bindValue(":mode", "");
    if(!query.exec())
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
    return;
}

void IrcChannels::insertUserIntoChannelDatabase(const QString &channel, const QString &user)
{
    if(!_db->isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db->open())
        {
            qDebug()<<this<<"database failed to open: "<<_db->lastError().text();
            return;
        }
    }
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
    if(!_db->isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db->open())
        {
            qDebug()<<this<<"database failed to open: "<<_db->lastError().text();
            return;
        }
    }
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
    if(!_db->isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db->open())
        {
            qDebug()<<this<<"database failed to open: "<<_db->lastError().text();
            return;
        }
    }
    QSqlQuery query(*_db);
    query.prepare("DELETE FROM userlists WHERE userlists.id = (SELECT channels.id FROM channels WHERE channels.name = :channelname LIMIT 1) AND userlists.user = :username");
    query.bindValue(":channelname", channel);
    query.bindValue(":username", user);
    if(!query.exec())
        qDebug()<<this<<"failed to execute query: "<<query.lastError().text();
    return;
}
