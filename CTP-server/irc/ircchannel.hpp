#ifndef IRCCHANNEL_HPP
#define IRCCHANNEL_HPP

#include <QObject>
#include <QTcpSocket>

class IrcChannel : public QObject
{
    Q_OBJECT
public:
    explicit IrcChannel(QString name, QObject *parent = 0);
    ~IrcChannel();
    void addUser(const QString &username, QTcpSocket* socket);
    void sendJoinMessage(const QString &sender);
    void sendPartMessage(const QString &sender);
    void sendMessage(const QString &sender, const QString &message);
    void removeUser(const QString &username);
    void clearUser(const QString &username);
    void removeAllUsers();
    void clearAllUsers();
    bool hasUser(const QString &username);
    bool hasOfflineUser(const QString &username);
    bool isOnlyUser(const QString &username);
    QMap<QTcpSocket*, QString>* userlist();
    QStringList* offlineUserlist();
signals:

public slots:

private:
    QString _name;
    QMap<QTcpSocket*, QString> _userlist;
    QStringList _offlineUserlist;
};

#endif // IRCCHANNEL_HPP
