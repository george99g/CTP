#include "ircclients.hpp"

IrcClients::IrcClients(QObject* parent) : QObject(parent)
{

}

IrcClients::~IrcClients()
{
    removeAllClients();
}

IrcClient* IrcClients::addClient(const QString &username, QTcpSocket* socket)
{
    IrcClient* client = new IrcClient(username, socket);
    _clients.insert(username, client);
    return client;
}

void IrcClients::removeClient(const QString &username)
{
    _clients.value(username)->deleteLater();
    _clients.remove(username);
    return;
}

void IrcClients::removeClient(IrcClient* client)
{
    client->deleteLater();
    _clients.remove(client->username());
    return;
}

void IrcClients::removeAllClients()
{
    for(unsigned i = 0; i < (unsigned)_clients.values().count(); i++)
        _clients.values().at(i)->deleteLater();
    _clients.clear();
    return;
}

IrcClient* IrcClients::client(const QString &username)
{
    return _clients.value(username);
}

IrcClient* IrcClients::client(QTcpSocket* socket)
{
    for(unsigned i = 0; i < (unsigned)_clients.values().count(); i++)
    {
        if(_clients.values().at(i)->socket() == socket)
            return  _clients.values().at(i);
    }
    return (IrcClient*)0;
}

bool IrcClients::hasClient(const QString &username)
{
    if(_clients.keys().contains(username))
        return true;
    else return false;
}

bool IrcClients::hasClient(QTcpSocket* socket)
{
    for(unsigned i = 0; i < (unsigned)_clients.values().count(); i++)
        if(_clients.values().at(i)->socket() == socket)
            return true;
    return false;
}

void IrcClients::broadcast(const QString &message)
{
    QTcpSocket* socket;
    for(unsigned i = 0; i < (unsigned)_clients.keys().count(); i++)
    {
        socket = client(_clients.keys().at(i))->socket();
        socket->write(message.toUtf8());
        socket->flush();
    }
    return;
}

QString IrcClients::generateClientList()
{
    QString list = "";
    for(unsigned i = 0; i < (unsigned)_clients.keys().count(); i++)
    {
        if(i != 0)
            list += ' ';
        list += _clients.keys().at(i);
    }
    return list;
}

bool IrcClients::hasPonged(const QString &username)
{
    return client(username)->ponged();
}

void IrcClients::setPonged(const QString &username, bool state)
{
    client(username)->setPonged(state);
    return;
}

bool IrcClients::hasPinged(const QString &username)
{
    return client(username)->pinged();
}

void IrcClients::setPinged(const QString &username, bool state)
{
    client(username)->setPinged(state);
    return;
}

QMap<QString, IrcClient *> &IrcClients::clients()
{
    return _clients;
}
