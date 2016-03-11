#include "ftpconnections.hpp"

FtpConnections::FtpConnections(QThread* thread, QObject* parent) : TcpConnections(parent), _manager(thread)
{
    qDebug()<<this<<"constructed";
    _thread = thread;
    _manager.moveToThread(thread);
}

FtpConnections::~FtpConnections()
{
    qDebug()<<this<<"destroyed";
}

void FtpConnections::accept(qintptr handle, TcpConnection *connection)
{
    QTcpSocket* socket = new QTcpSocket(0);
    if(!socket->setSocketDescriptor(handle))
    {
        qWarning()<<this<<"could not accept socket with handle "<<handle;
        connection->deleteLater();
        return;
    }
    connect(socket, &QTcpSocket::disconnected, this, &FtpConnections::disconnected, Qt::QueuedConnection);
    connect(socket, &QTcpSocket::readyRead, this, &FtpConnections::readyRead, Qt::QueuedConnection);
    connect(socket, &QTcpSocket::connected, this, &FtpConnections::connected, Qt::QueuedConnection);
    connect(socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &FtpConnections::error, Qt::QueuedConnection); //Even more magic. Don't touch.
    connection->moveToThread(QThread::currentThread());
    connection->setSocket(socket);
    _connections.insert(socket, connection);
    qDebug()<<this<<"is now handling "<<count()<<" connections";
    emit socket->connected();
    return;
}

void FtpConnections::disconnected()
{
    if(!sender()) return;
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(!socket) return;
    qDebug()<<socket<<"disconnected in"<<this;
    //Handle disconnection here
    TcpConnections::disconnected();
    return;
}

void FtpConnections::connected()
{
    if(!sender()) return;
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(!socket) return;
    qDebug()<<socket<<"connected in"<<this;
    //Handle connection here
    return;
}

void FtpConnections::error(QAbstractSocket::SocketError socketError)
{
    if(socketError == QAbstractSocket::RemoteHostClosedError)
    {
        if(!sender()) return;
        QTcpSocket* socket = (QTcpSocket*)sender();
        if(!socket) return;
        qDebug()<<socket<<"closed from remote host in"<<this;
        //Handle remote host closed here
    }
    TcpConnections::error(socketError);
    return;
}

FtpManager* FtpConnections::manager()
{
    return &_manager;
}

void FtpConnections::readyRead()
{
    if(!sender()) return;
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(!socket) return;
    qDebug()<<this<<"is ready to read"<<socket;
    _manager.handleSocketReadyRead(socket);
    return;
}
