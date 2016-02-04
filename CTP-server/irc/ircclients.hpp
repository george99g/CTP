#ifndef IRCCLIENTS_HPP
#define IRCCLIENTS_HPP

#include <QObject>
#include <QMap>
#include "ircclient.hpp"

class IrcClients : public QObject
{
    Q_OBJECT
public:
    explicit IrcClients(QObject* parent = 0);
    ~IrcClients();
    IrcClient* addClient(const QString &username, QTcpSocket* socket);
    void removeClient(const QString &username);
    void removeClient(IrcClient* client);
    void removeAllClients();
    IrcClient* client(const QString &username);
    IrcClient* client(QTcpSocket* socket);
    bool hasClient(const QString &username);
    bool hasClient(QTcpSocket* socket);
    void broadcast(const QString &message);
    QString generateClientList();
private:
    QMap<QString, IrcClient*> _clients;
};

#endif // IRCCLIENTS_HPP
