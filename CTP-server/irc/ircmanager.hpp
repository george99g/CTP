#ifndef IRCMANAGER_HPP
#define IRCMANAGER_HPP

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>

class IrcManager : public QObject
{
    Q_OBJECT
public:
    explicit IrcManager(QObject *parent = 0);
    ~IrcManager();
    void handleMessage(QTcpSocket* socket, const QString &message);
    void handleLogin(QTcpSocket* socket, const QString &message);
    void handleRegister(QTcpSocket* socket, const QString &message);
    void handleLogout(QTcpSocket* socket);
    void handleConnection(QTcpSocket* socket);
    void handleDisconnection(QTcpSocket* socket);
signals:

public slots:

private:
    void sendMessageToUsername(const QString &username, const QString &message);
    bool checkDatabaseForUsername(const QString &username);
    bool checkDatabaseForLogin(const QString &username, const QString &password);
    bool registerDatabaseLogin(const QString &username, const QString &password);
    QTcpSocket* getSocket(const QString &username);
    QString getUsername(QTcpSocket* socket);
    bool isLoggedIn(const QString &username);
    void broadcast(const QString &message);
    QMap<QTcpSocket*, QString> _usernames;
    QSqlDatabase _db;
};

#endif // IRCMANAGER_HPP
