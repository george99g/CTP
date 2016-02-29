#include "tcpconnections.hpp"

TcpConnections::TcpConnections(QObject* parent) : QObject(parent)
{
    qDebug()<<"Constructed connections handler "<<this;
}

TcpConnections::~TcpConnections()
{
    qDebug()<<"Destroyed connections handler "<<this;
    quit();
}

int TcpConnections::count()
{
    QReadWriteLock lock;
    lock.lockForRead();
    int mapCount = _connections.count();
    lock.unlock();
    return mapCount;
}

void TcpConnections::removeSocket(QTcpSocket* socket)
{
    if(!socket) return;
    else if(!_connections.contains(socket)) return;
    else
    {
        qDebug()<<"Removing socket "<<socket<<" from connection handler "<<this;
        if(socket->isOpen())
        {
            qDebug()<<"Socket "<<socket<<" in connection handler "<<this<<" is open, closing";
            socket->disconnect();
            socket->close();
        }
        _connections.remove(socket);
        qDebug()<<socket<<" removed from connection handler "<<this;
        qDebug()<<"Setting socket "<<socket<<" for deletion";
        socket->deleteLater();
        qDebug()<<"Connection handler "<<this<<" is now handling "<<count()<<" connections";
    }
    return;
}

void TcpConnections::start()
{
    qDebug()<<"Connection handler "<<this<<" starting on thread "<<QThread::currentThread();
    return;
}

void TcpConnections::quit()
{
    if(!sender()) return;
    qDebug()<<"Removing all sockets on connection handler "<<this;
    for(uint i = 0; i < (uint)_connections.keys().count(); i++)
    {
        QTcpSocket* socket = _connections.keys().at(i);
        qDebug()<<"Calling socket removal for "<<socket<<" in connection handler "<<this;
        removeSocket(socket);
    }
    qDebug()<<"Connection handler "<<this<<" quitting on thread "<<QThread::currentThread();
    emit finished();
    return;
}

void TcpConnections::accept(qintptr handle, TcpConnection* connection)
{
    QTcpSocket* socket = new QTcpSocket(0);
    if(!socket->setSocketDescriptor(handle))
    {
        qWarning()<<"Connection handler "<<this<<" could not accept socket with handle "<<handle;
        connection->deleteLater();
        return;
    }
    connect(socket, &QTcpSocket::disconnected, this, &TcpConnections::disconnected, Qt::QueuedConnection);
    connect(socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &TcpConnections::error, Qt::QueuedConnection); //More magic. Don't touch.
    connection->moveToThread(QThread::currentThread());
    connection->setSocket(socket);
    _connections.insert(socket, connection);
    qDebug()<<"Connection handler "<<this<<" is now handling "<<count()<<" connections";
    emit socket->connected();
    return;
}

void TcpConnections::disconnected()
{
    if(!sender()) return;
    qDebug()<<"Connection handler "<<this<<" disconnecting socket "<<sender();
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    if(!socket) return;
    removeSocket(socket);
    return;
}

void TcpConnections::error(QAbstractSocket::SocketError socketError)
{
    if(!sender()) return;
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    if(!socket) return;
    qDebug()<<"Socket error "<<socketError<<" from socket "<<socket<<" on connection handler "<<this<<" on thread "<<QThread::currentThread();
    removeSocket(socket);
    return;
}
