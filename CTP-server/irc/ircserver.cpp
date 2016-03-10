#include "ircserver.hpp"

IrcServer::IrcServer(QObject* parent) : TcpServer(parent)
{
    qDebug()<<this<<"constructed";
    _thread = (QThread*)0;
    _ftpPort = 2001;
}

IrcServer::~IrcServer()
{
    qDebug()<<this<<"destroyed";
    close();
}

bool IrcServer::listen(const QHostAddress &address, quint16 port)
{
    if(!QTcpServer::listen(address, port))
        return false;
    _thread = new QThread(this);
    _connections = new IrcConnections(_thread, _ftpPort);
    connect(_thread, &QThread::started, _connections, &IrcConnections::start, Qt::QueuedConnection);
    connect(this, &IrcServer::accepting, _connections, &IrcConnections::accept, Qt::QueuedConnection);
    connect(this, &IrcServer::finished, _connections, &IrcConnections::quit, Qt::QueuedConnection);
    connect(_connections, &IrcConnections::finished, this, &IrcServer::complete, Qt::QueuedConnection);
    _connections->moveToThread(_thread);
    _thread->start();
    return true;
}

void IrcServer::setFtpPort(qint16 port)
{
    _ftpPort = port;
    return;
}

qint16 IrcServer::ftpPort()
{
    return _ftpPort;
}

IrcManager* IrcServer::manager()
{
    return _connections->manager();
}

void IrcServer::complete()
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
