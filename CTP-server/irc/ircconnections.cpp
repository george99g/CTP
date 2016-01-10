#include "ircconnections.hpp"

IrcConnections::IrcConnections(QObject *parent) : TcpConnections(parent)
{

}

IrcConnections::~IrcConnections()
{

}

void IrcConnections::accept(qintptr handle, TcpConnection *connection)
{
    QTcpSocket* socket = new QTcpSocket(this);
    if(!socket->setSocketDescriptor(handle))
    {
        qWarning()<<this<<" could not accept socket with handle "<<handle;
        connection->deleteLater();
        return;
    }
    connect(socket, &QTcpSocket::disconnected, this, &IrcConnections::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &IrcConnections::readyRead);
    connect(socket, &QTcpSocket::connected, this, &IrcConnections::connected);
    connect(socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &IrcConnections::error); //More magic. Don't touch.
    connection->moveToThread(QThread::currentThread());
    connection->setSocket(socket);
    _connections.insert(socket, connection);
    qDebug()<<this<<" is now handling "<<count()<<" connections";
    emit socket->connected();
    return;
}

void IrcConnections::disconnected()
{
    if(sender())
    {
        QTcpSocket* socket = (QTcpSocket*)sender();
        if(socket)
            qDebug()<<socket<<"disconnected in"<<this;
    }
    TcpConnections::disconnected();
    return;
}

void IrcConnections::connected()
{
    if(!sender()) return;
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(!socket) return;
    qDebug()<<socket<<"connected in"<<this;
}

void IrcConnections::readyRead()
{
    if(!sender()) return;
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(!socket) return;
    qDebug()<<this<<"is ready to read"<<socket;
    qDebug()<<this<<"read:"<<socket->readAll();
    socket->write("Test\n");
    socket->flush();
    socket->close();
}
