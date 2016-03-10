#include "ftpmanager.hpp"

FtpManager::FtpManager(QThread* thread, QObject* parent) : QObject(parent)
{
    _thread = thread;
}

FtpManager::~FtpManager()
{

}

QTcpSocket* FtpManager::getSocket(qint32 id)
{
    return _socketIdMap.value(id);
}

QTcpSocket* FtpManager::getSocket(const QString &username)
{
    return _socketIdMap.value(_usernameIdMap.key(username));
}

QString FtpManager::getUsername(qint32 id)
{
    return _usernameIdMap.value(id);
}

QString FtpManager::getUsername(QTcpSocket *socket)
{
    return _usernameIdMap.value(_socketIdMap.key(socket));
}

qint32 FtpManager::getId(const QString username)
{
    return _usernameIdMap.key(username);
}

qint32 FtpManager::getId(QTcpSocket *socket)
{
    return _socketIdMap.key(socket);
}

void FtpManager::addUsernameIdPair(const QString &username, qint32 id)
{
    _usernameIdMap.insert(id, username);
    return;
}

void FtpManager::addSocket(QTcpSocket *socket, qint32 id)
{
    _socketIdMap.insert(id, socket);
    return;
}

bool FtpManager::addSocket(QTcpSocket *socket, const QString &username)
{
    if(_usernameIdMap.values().contains(username))
        _socketIdMap.insert(_usernameIdMap.key(username), socket);
    else return false;
    return true;
}

void FtpManager::removeSocket(QTcpSocket *socket)
{
    _socketIdMap.remove(_socketIdMap.key(socket));
    return;
}

void FtpManager::removeRecord(const QString &username)
{
    if(_socketIdMap.keys().contains(_usernameIdMap.key(username)))
        _socketIdMap.remove(_usernameIdMap.key(username));
    _usernameIdMap.remove(_usernameIdMap.key(username));
    return;
}

void FtpManager::removeRecord(qint32 id)
{
    if(_socketIdMap.keys().contains(id))
        _socketIdMap.remove(id);
    _usernameIdMap.remove(id);
    return;
}
