#ifndef IRCMANAGER_HPP
#define IRCMANAGER_HPP

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QTimer>
#include <QThread>
#include <qmath.h>
#include "../config.hpp"
#include "ircclients.hpp"
#include "ircchannels.hpp"
#include "../ftp/ftpmanager.hpp"

class IrcManager : public QObject
{
    Q_OBJECT
public:
    explicit IrcManager(QThread* thread, QObject* parent = 0);
    ~IrcManager();
    void handleMessage(QTcpSocket* socket, const QString &message);
    void handleLogin(QTcpSocket* socket, const QString &message);
    void handleRegister(QTcpSocket* socket, const QString &message);
    void handleLogout(QTcpSocket* socket);
    void handleConnection(QTcpSocket* socket);
    void handleDisconnection(QTcpSocket* socket);
    void setFtpPort(qint16 ftpPort);
    qint16 ftpPort();
    void ftpReceiveFileList(qint32 id, const QStringList &list);
    void ftpSendMessageToId(qint32 id, const QString &message);
signals:
    void ftpAddUsernameIdPair(const QString &username, qint32 id);
    void ftpRemoveRecord(qint32 id);
    void ftpGenerateHomeDirectoryForUser(QString username);
    void ftpRequestFileList(qint32 id, QString dir);
    void sendFileToId(qint32 id, QString file);
private:
    void setClientModeInDatabase(const QString &username, IrcMode* mode);
    QString getClientModeFromDatabase(const QString &username);
    void sendMessageToUsername(const QString &username, const QString &message);
    void sendOfflineMessageToUsername(const QString &username, const QString &senderUsername, const QString &message);
    void sendMissedMessages(QTcpSocket* socket);
    bool hasMissedMessages(QTcpSocket* socket);
    QString generateOfflineClientList();
    bool checkDatabaseForUsername(const QString &username);
    bool checkDatabaseForLogin(const QString &username, const QString &password);
    bool registerDatabaseLogin(const QString &username, const QString &password);
    bool openDatabase();
    void handlePings();
    QTcpSocket* getSocket(const QString &username);
    QString getUsername(QTcpSocket* socket);
    bool isLoggedIn(const QString &username);
    void broadcast(const QString &message);
    QTimer* pingTimer;
    IrcClients _clients;
    QSqlDatabase _db;
    QMap<qint32, IrcClient*> _clientIds;
    IrcChannels* _channels;
    QThread* _thread;
    qint16 _ftpPort;
};

#endif // IRCMANAGER_HPP
