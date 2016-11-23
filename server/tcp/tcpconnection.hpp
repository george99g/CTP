#ifndef TCPCONNECTION_HPP
#define TCPCONNECTION_HPP

#include <QObject>
#include <QDebug>
#include <QTcpSocket>

class TcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnection(QObject* parent = 0);
    ~TcpConnection();
    virtual void setSocket(QTcpSocket* socket);
    QTcpSocket* getSocket();
protected:
    QTcpSocket* _socket;
public slots:
    virtual void connected();
    virtual void disconnected();
    virtual void readyRead();
    virtual void bytesWritten(qint64 bytes);
    virtual void stateChanged(QAbstractSocket::SocketState socketState);
    virtual void error(QAbstractSocket::SocketError socketError);
};

#endif // TCPCONNECTION_HPP
