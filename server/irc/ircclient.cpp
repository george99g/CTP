#include "ircclient.hpp"

IrcClient::IrcClient(QString username, QTcpSocket* socket, QObject* parent) : QObject(parent)
{
    _username = username;
    _socket = socket;
    _pinged = false;
    _ponged = false;
    qDebug()<<"initialized"<<this<<"in"<<QThread::currentThread();
}

IrcClient::~IrcClient()
{

}

QString IrcClient::username()
{
    if(_username.length()>=1)
        return _username;
    else return "";
}

void IrcClient::setUsername(const QString username)
{
    _username = username;
    return;
}

QTcpSocket* IrcClient::socket()
{
    return _socket;
}

void IrcClient::setSocket(QTcpSocket* socket)
{
    _socket = socket;
    return;
}

IrcMode* IrcClient::mode()
{
    return &_mode;
}

void IrcClient::setMode(const QString &mode)
{
    _mode.fromString(mode);
    return;
}

bool IrcClient::pinged()
{
    return _pinged;
}

void IrcClient::setPinged(bool state)
{
    _pinged = state;
    return;
}

bool IrcClient::ponged()
{
    return _ponged;
}

void IrcClient::setPonged(bool state)
{
    _ponged = state;
}
