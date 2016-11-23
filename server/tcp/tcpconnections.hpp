#ifndef TCPCONNECTIONS_HPP
#define TCPCONNECTIONS_HPP

#include <QObject>
#include <QThread>
#include <QMap>
#include <QReadWriteLock>
#include "tcpconnection.hpp"

class TcpConnections : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnections(QObject* parent = 0);
    ~TcpConnections();
    virtual int count();
protected:
    QMap<QTcpSocket*, TcpConnection*> _connections;
    virtual void removeSocket(QTcpSocket* socket);
signals:
    void quitting();
    void finished();
public slots:
    void start();
    void quit();
    virtual void accept(qintptr handle, TcpConnection* connection);
protected slots:
    virtual void disconnected();
    virtual void error(QAbstractSocket::SocketError socketError);
};

#endif // TCPCONNECTIONS_HPP
