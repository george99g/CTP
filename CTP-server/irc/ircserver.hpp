#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <QObject>
#include "../tcp/tcpserver.hpp"
#include "ircconnections.hpp"

//Despite their name, these classes do not conform to the IRC protocol

class IrcServer : public TcpServer
{
    Q_OBJECT
public:
    explicit IrcServer(QObject* parent = 0);
    ~IrcServer();
    virtual bool listen(const QHostAddress &address, quint16 port);
    void setFtpPort(qint16 port);
    qint16 ftpPort();
    IrcManager* manager();
public slots:
    virtual void complete();
private:
    IrcConnections* _connections;
    qint16 _ftpPort;
};

#endif // IRCSERVER_HPP
