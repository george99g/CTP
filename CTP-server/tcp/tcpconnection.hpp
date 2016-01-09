#ifndef TCPCONNECTION_HPP
#define TCPCONNECTION_HPP

#include <QObject>
#include <QDebug>
#include <QTcpSocket>

//Wraps a QTcpSocket

class TcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnection(QObject *parent = 0);
    ~TcpConnection();
    virtual void setSocket(QTcpSocket* socket); //Sets the socket and creates the connections

protected:
    QTcpSocket* _socket;
    QTcpSocket* getSocket(); //Returns the socket
signals:

public slots:
    virtual void connected();
    virtual void disconnected();
    virtual void readyRead();
    virtual void bytesWritten(qint64 bytes);
    virtual void stateChanged(QAbstractSocket::SocketState socketState);
    virtual void error(QAbstractSocket::SocketError socketError);
private:

};

#endif // TCPCONNECTION_HPP
