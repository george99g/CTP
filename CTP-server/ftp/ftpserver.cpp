#include "ftpserver.hpp"

FtpServer::FtpServer(QObject *parent) : TcpServer(parent)
{
    qDebug()<<this<<"constructed";
    _thread = (QThread*)0;
}

FtpServer::~FtpServer()
{
    qDebug()<<this<<"destroyed";
    close();
}

bool FtpServer::listen(const QHostAddress &address, quint16 port)
{
    if(!QTcpServer::listen(address, port))
        return false;
    _thread = new QThread(this);
    _connections = new FtpConnections(_thread);
    connect(_thread, &QThread::started, _connections, &FtpConnections::start, Qt::QueuedConnection);
    connect(this, &FtpServer::accepting, _connections, &FtpConnections::accept, Qt::QueuedConnection);
    connect(this, &FtpServer::finished, _connections, &FtpConnections::quit, Qt::QueuedConnection);
    connect(_connections, &FtpConnections::finished, this, &FtpServer::complete, Qt::QueuedConnection);
    _connections->moveToThread(_thread);
    _thread->start();
    return true;
}

void FtpServer::complete()
{
    if(!_thread)
    {
        qWarning()<<this<<"completed without thread";
        return;
    }
    qDebug()<<this<<"complete called, destroying"<<_connections;
    delete _connections;
    qDebug()<<this<<"destroying thread"<<_thread;
    _thread->quit();
    _thread->wait();
    _thread->deleteLater();
    return;
}
