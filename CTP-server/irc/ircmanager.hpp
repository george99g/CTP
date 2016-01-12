#ifndef IRCMANAGER_HPP
#define IRCMANAGER_HPP

#include <QObject>
#include <QString>
#include <QTcpSocket>

class IrcManager : public QObject
{
    Q_OBJECT
public:
    explicit IrcManager(QObject *parent = 0);
    ~IrcManager();
    void handleLogin(QTcpSocket* socket, const QString &message);
    void handleLogout(QTcpSocket* socket);
    void handleConnection(QTcpSocket* socket);
    void handleDisconnection(QTcpSocket* socket);
signals:

public slots:

private:
    void broadcast(const QString &message);
    QMap<QTcpSocket*, QString> _usernames;
};

#endif // IRCMANAGER_HPP
