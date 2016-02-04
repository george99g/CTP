#include "ircclient.hpp"

IrcClient::IrcClient(QString username, QTcpSocket* socket, QObject* parent) : QObject(parent)
{
    _username = username;
    _socket = socket;
}

IrcClient::~IrcClient()
{

}

QString IrcClient::username()
{
    return _username;
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
