#include "tcpconnection.hpp"

TcpConnection::TcpConnection(QObject* parent) : QObject(parent)
{
    qDebug()<<"Created connection wrapper "<<this;
}

TcpConnection::~TcpConnection()
{
    qDebug()<<"Destroyed connection wrapper "<<this;
}

void TcpConnection::setSocket(QTcpSocket* socket)
{
    _socket = socket;
    connect(_socket, &QTcpSocket::connected, this, &TcpConnection::connected);
    connect(_socket, &QTcpSocket::disconnected, this, &TcpConnection::disconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &TcpConnection::readyRead);
    connect(_socket, &QTcpSocket::bytesWritten, this, &TcpConnection::bytesWritten);
    connect(_socket, &QTcpSocket::stateChanged, this, &TcpConnection::stateChanged);
    connect(_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &TcpConnection::error); //Magic. Don't touch.
    return;
}

QTcpSocket *TcpConnection::getSocket()
{
    if(!sender()) return 0;
    else return static_cast<QTcpSocket*>(sender());
}

void TcpConnection::connected()
{
    if(!sender()) return;
    qDebug()<<this<<" connected "<<sender();
    return;
}

void TcpConnection::disconnected()
{
    if(!sender()) return;
    qDebug()<<this<<" disconnected "<<getSocket();
    return;
}

void TcpConnection::readyRead()
{
    if(!sender()) return;
    qDebug()<<this<<" is ready to read "<<getSocket();
    return;
}

void TcpConnection::bytesWritten(qint64 bytes)
{
    if(!sender()) return;
    qDebug()<<this<<" wrote "<<bytes<<" bytes in "<<getSocket();
    return;
}

void TcpConnection::stateChanged(QAbstractSocket::SocketState socketState)
{
    if(!sender()) return;
    qDebug()<<this<<" changed state to "<<socketState<<" in "<<getSocket();
    return;
}

void TcpConnection::error(QAbstractSocket::SocketError socketError)
{
    if(!sender()) return;
    qDebug()<<this<<" errored "<<socketError<<" in "<<getSocket();
    return;
}
