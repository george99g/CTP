#include "ircconnections.hpp"

IrcConnections::IrcConnections(QThread* thread, qint16 ftpPort, QObject* parent) : TcpConnections(parent)
{
    _manager = new IrcManager(thread);
    _manager->setFtpPort(ftpPort);
    _manager->moveToThread(thread);
    _thread = thread;
    _ftpPort = ftpPort;
}

IrcConnections::~IrcConnections()
{
    _manager->deleteLater();
}

IrcManager* IrcConnections::manager()
{
    return _manager;
}

void IrcConnections::accept(qintptr handle, TcpConnection* connection)
{
    QTcpSocket* socket = new QTcpSocket(0);
    if(!socket->setSocketDescriptor(handle))
    {
        qWarning()<<this<<" could not accept socket with handle "<<handle;
        connection->deleteLater();
        return;
    }
    connect(socket, &QTcpSocket::disconnected, this, &IrcConnections::disconnected, Qt::QueuedConnection);
    connect(socket, &QTcpSocket::readyRead, this, &IrcConnections::readyRead, Qt::QueuedConnection);
    connect(socket, &QTcpSocket::connected, this, &IrcConnections::connected, Qt::QueuedConnection);
    connect(socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &IrcConnections::error, Qt::QueuedConnection);
    connection->moveToThread(QThread::currentThread());
    connection->setSocket(socket);
    _connections.insert(socket, connection);
    qDebug()<<this<<" is now handling "<<count()<<" connections";
    emit socket->connected();
    return;
}

void IrcConnections::disconnected()
{
    if(!sender()) return;
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(!socket) return;
    qDebug()<<socket<<"disconnected in"<<this;
    _manager->handleDisconnection(socket);
    TcpConnections::disconnected();
    return;
}

void IrcConnections::connected()
{
    if(!sender()) return;
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(!socket) return;
    qDebug()<<socket<<"connected in"<<this;
    _manager->handleConnection(socket);
    return;
}

void IrcConnections::error(QAbstractSocket::SocketError socketError)
{
    if(socketError == QAbstractSocket::RemoteHostClosedError)
    {
        if(!sender()) return;
        QTcpSocket* socket = (QTcpSocket*)sender();
        if(!socket) return;
        qDebug()<<socket<<"closed from remote host in"<<this;
        _manager->handleDisconnection(socket);
    }
    TcpConnections::error(socketError);
    return;
}

void IrcConnections::readyRead()
{
    if(!sender()) return;
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(!socket) return;
    qDebug()<<this<<"is ready to read"<<socket;
    while(socket->canReadLine())
    {
        QString line = QString::fromUtf8(socket->readLine().trimmed());
        if(CLIENT_MESSAGES_ARE_DEBUGGED)
            qDebug()<<line;
        if(line.mid(0, 5) == "LOGIN")
            _manager->handleLogin(socket, line);
        else if(line.mid(0, 8) == "REGISTER")
        {
            if(!ADMIN_ONLY_REGISTRATION)
                _manager->handleRegister(socket, line);
        }
        else if(line.mid(0, 6) == "LOGOUT")
            _manager->handleLogout(socket);
        else if(line.length() >= 1)
            _manager->handleMessage(socket, line);
    }
    return;
}
