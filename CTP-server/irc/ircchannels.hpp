#ifndef IRCCHANNELS_HPP
#define IRCCHANNELS_HPP

#include <QObject>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "ircchannel.hpp"

class IrcChannels : public QObject
{
    Q_OBJECT
public:
    explicit IrcChannels(QSqlDatabase* db, QObject *parent = 0);
    ~IrcChannels();
    bool channelExists(const QString &channel);
    void createChannel(const QString &channel);
    void sendMessage(const QString &channel, const QString &message, const QString &sender);
    void joinChannel(const QString &channel, const QString &username, QTcpSocket* socket);
    void rejoinChannels(const QString &username, QTcpSocket* socket);
    bool hasUser(const QString &channel, const QString &username);
    bool hasOfflineUser(const QString &channel, const QString &username);
    void removeChannel(const QString &channel);
    void clearChannel(const QString &channel);
    void clearUser(const QString &username);
    void partChannel(const QString &channel, const QString &username);
    bool isOnlyUser(const QString &channel, const QString &username);
    QString generateChannelList();
    QString generateUserList(const QString &channel);
    void loadChannelsFromDatabase();
    void insertChannelIntoDatabase(const QString &channel);
    void insertUserIntoChannelDatabase(const QString &channel, const QString &user);
    void removeChannelFromDatabase(const QString &channel);
    void removeUserFromChannelDatabase(const QString &channel, const QString &user);
signals:

public slots:

private:
    QMap<QString, IrcChannel*> _channels;
    QSqlDatabase* _db;
};

#endif // IRCCHANNELS_HPP
