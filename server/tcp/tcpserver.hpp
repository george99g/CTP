#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <QObject>
#include <QTcpServer>
#include <QThread>
#include "tcpconnections.hpp"

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject* parent = 0);
    ~TcpServer();
    virtual bool listen(const QHostAddress &address, quint16 port);
    virtual void close();
    virtual qint64 port();
protected:
    TcpConnections* _connections;
    QThread* _thread;
    virtual void incomingConnection(qintptr/*Can be qint64, qHandle, qintptr or uint, depending on platform*/
                                            handle);
signals:
    void accepting(qintptr handle, TcpConnection* connection);
    void finished();
public slots:
    virtual void complete();
};

#endif // TCPSERVER_HPP
