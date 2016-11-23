#ifndef FTPSERVER_HPP
#define FTPSERVER_HPP

#include "../tcp/tcpserver.hpp"
#include "ftpconnections.hpp"
#include <QObject>

class FtpServer : public TcpServer
{
    Q_OBJECT
public:
    explicit FtpServer(QObject *parent = 0);
    ~FtpServer();
    virtual bool listen(const QHostAddress &address, quint16 port);
    FtpManager* manager();
public slots:
    virtual void complete();
private:
    FtpConnections* _connections;
};

#endif // FTPSERVER_HPP
