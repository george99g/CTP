#ifndef FTPCONNECTIONS_HPP
#define FTPCONNECTIONS_HPP

#include "../tcp/tcpconnections.hpp"
#include "../config.hpp"
#include <QObject>

class FtpConnections : public TcpConnections
{
    Q_OBJECT
public:
    explicit FtpConnections(QThread* thread, QObject* parent = 0);
    ~FtpConnections();
public slots:
    virtual void accept(qintptr handle, TcpConnection* connection);
    virtual void disconnected();
    virtual void connected();
    virtual void error(QAbstractSocket::SocketError socketError);
protected slots:
    void readyRead();
private:
    QThread* _thread;
};

#endif // FTPCONNECTIONS_HPP
