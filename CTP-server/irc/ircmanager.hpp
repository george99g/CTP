#ifndef IRCMANAGER_HPP
#define IRCMANAGER_HPP

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include "ircclients.hpp"
#include "ircchannels.hpp"

class IrcManager : public QObject
{
    Q_OBJECT
public:
    explicit IrcManager(QObject* parent = 0);
    ~IrcManager();
    void handleMessage(QTcpSocket* socket, const QString &message);
    void handleLogin(QTcpSocket* socket, const QString &message);
    void handleRegister(QTcpSocket* socket, const QString &message);
    void handleLogout(QTcpSocket* socket);
    void handleConnection(QTcpSocket* socket);
    void handleDisconnection(QTcpSocket* socket);
private:
    void setClientModeInDatabase(const QString &username, IrcMode* mode);
    QString getClientModeFromDatabase(const QString &username);
    void sendMessageToUsername(const QString &username, const QString &message);
    bool checkDatabaseForUsername(const QString &username);
    bool checkDatabaseForLogin(const QString &username, const QString &password);
    bool registerDatabaseLogin(const QString &username, const QString &password);
    bool openDatabase();
    QTcpSocket* getSocket(const QString &username);
    QString getUsername(QTcpSocket* socket);
    bool isLoggedIn(const QString &username);
    void broadcast(const QString &message);
    IrcClients _clients;
    QSqlDatabase _db;
    IrcChannels* _channels;
};

#endif // IRCMANAGER_HPP
