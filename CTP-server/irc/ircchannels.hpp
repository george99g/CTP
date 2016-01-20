#ifndef IRCCHANNELS_HPP
#define IRCCHANNELS_HPP

#include <QObject>
#include <QMap>
#include <QSqlDatabase>
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
    bool hasUser(const QString &channel, const QString &username);
    void removeChannel(const QString &channel);
    void partChannel(const QString &channel, const QString &username);
    bool isOnlyUser(const QString &channel, const QString &username);
    QString generateChannelList();
signals:

public slots:

private:
    QMap<QString, IrcChannel*> _channels;
    QSqlDatabase* _db;
};

#endif // IRCCHANNELS_HPP
