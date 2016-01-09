#include "tcpserver.hpp"

TcpServer::TcpServer(QObject* parent) : QTcpServer(parent)
{
    qDebug()<<"TcpServer "<<this<<" constructed on thread "<<QThread::currentThread();
    _thread = (QThread*)0;
}

TcpServer::~TcpServer()
{
    qDebug()<<"TcpServer "<<this<<" destroyed";
    close();
}

bool TcpServer::listen(const QHostAddress &address, quint16 port)
{
    if(!QTcpServer::listen(address, port))
        return false;
    _thread = new QThread(this);
    _connections = new TcpConnections();
    connect(_thread, &QThread::started, _connections, &TcpConnections::start, Qt::QueuedConnection);
    connect(this, &TcpServer::accepting, _connections, &TcpConnections::accept, Qt::QueuedConnection);
    connect(this, &TcpServer::finished, _connections, &TcpConnections::quit, Qt::QueuedConnection);
    connect(_connections, &TcpConnections::finished, this, &TcpServer::complete, Qt::QueuedConnection);
    _connections->moveToThread(_thread);
    _thread->start();
    return true;
}

void TcpServer::close()
{
    qDebug()<<"TcpServer "<<this<<" closing";
    emit finished();
    QTcpServer::close();
}

qint64 TcpServer::port()
{
    if(isListening())
        return serverPort();
    else return 2000;
}

void TcpServer::incomingConnection(qintptr handle)
{
    qDebug()<<"TcpServer "<<this<<" attempting to accept connection with descriptor "<<handle;
    TcpConnection* connection = new TcpConnection();
    connection->moveToThread(_thread);
    emit accepting(handle, connection);
    return;
}

void TcpServer::complete()
{
    if(!_thread)
    {
        qWarning()<<"TcpServer "<<this<<" completed without thread";
        return;
    }
    qDebug()<<"TcpServer "<<this<<" complete called, destroying TcpConnections "<<_connections;
    delete _connections;
    qDebug()<<"TcpServer "<<this<<" destroying thread "<<_thread;
    _thread->quit();
    _thread->wait();
    _thread->deleteLater();
    return;
}
