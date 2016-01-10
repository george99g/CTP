#ifndef TCPCONNECTIONS_HPP
#define TCPCONNECTIONS_HPP

#include <QObject>
#include "tcpconnection.hpp"
#include <QThread>
#include <QMap>
#include <QReadWriteLock>

//Contains and manages TcpConnection objects after being started by TcpServer.

class TcpConnections : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnections(QObject* parent = 0);
    ~TcpConnections();
    virtual int count(); //Returns the connection count.

protected:
    QMap<QTcpSocket*, TcpConnection*> _connections; //Holds a map of sockets and connections. Useful for looking up connections by sockets.
    virtual void removeSocket(QTcpSocket* socket); //Remvoes a socket.
signals:
    void quitting(); //Emitted when the class is quitting
    void finished(); //Emitted when the class is finished  quitting
public slots:
    void start(); //Called when the class is started
    void quit(); //Called when the class needs to quit
    virtual void accept(qintptr handle, TcpConnection* connection); //Constructs a socket from the socket descriptor and connection sent by TcpServer
protected slots:
    virtual void disconnected(); //Handles a socket disonnection
    virtual void error(QAbstractSocket::SocketError socketError); //Handles a socket error
private:

};

#endif // TCPCONNECTIONS_HPP
