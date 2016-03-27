#include "ftpmanager.hpp"

FtpManager::FtpManager(QThread* thread, QObject* parent) : QObject(parent)
{
    _thread = thread;
    createUserfilesDir();
    _db = QSqlDatabase::database();
    _db.setDatabaseName("database.db3");
    if(openDatabase())
    {
        QSqlQuery query(_db);
        if(!query.exec("CREATE TABLE IF NOT EXISTS userfolders(folder INTEGER PRIMARY KEY, username TEXT)"))
            qDebug()<<"Error with query: "<<query.lastError().text();
    }
}

FtpManager::~FtpManager()
{
    _db.close();
    for(unsigned i = 0; i < (unsigned)_socketFileMap.values().size(); i++)
    {
        QFile* file = _socketFileMap.values().at(i);
        file->deleteLater();
        _socketFileMap.remove(_socketFileMap.keys().at(i));
    }
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

QString FtpManager::getUsername(QTcpSocket* socket)
{
    return _usernameIdMap.value(_socketIdMap.key(socket));
}

qint32 FtpManager::getId(const QString username)
{
    return _usernameIdMap.key(username);
}

qint32 FtpManager::getId(QTcpSocket* socket)
{
    return _socketIdMap.key(socket);
}

void FtpManager::addUsernameIdPair(const QString &username, qint32 id)
{
    _usernameIdMap.insert(id, username);
    return;
}

void FtpManager::addSocket(QTcpSocket* socket, qint32 id)
{
    _socketIdMap.insert(id, socket);
    _socketFileMap.insert(id, new QFile());
    return;
}

bool FtpManager::addSocket(QTcpSocket* socket, const QString &username)
{
    if(_usernameIdMap.values().contains(username))
    {
        _socketIdMap.insert(_usernameIdMap.key(username), socket);
        _socketFileMap.insert(_usernameIdMap.key(username), new QFile());
    }
    else return false;
    return true;
}

void FtpManager::removeSocket(QTcpSocket* socket)
{
    QFile* file = _socketFileMap.value(_socketIdMap.key(socket));
    file->deleteLater();
    _socketFileMap.remove(_socketIdMap.key(socket));
    _socketIdMap.remove(_socketIdMap.key(socket));
    return;
}

void FtpManager::removeRecord(const QString &username)
{
    if(_socketIdMap.keys().contains(_usernameIdMap.key(username)))
    {
        QFile* file = _socketFileMap.value(_usernameIdMap.key(username));
        file->deleteLater();
        _socketFileMap.remove(_usernameIdMap.key(username));
        _socketIdMap.remove(_usernameIdMap.key(username));
    }
    _usernameIdMap.remove(_usernameIdMap.key(username));
    return;
}

void FtpManager::removeRecord(qint32 id)
{
    if(_socketIdMap.keys().contains(id))
    {
        QFile* file = _socketFileMap.value(id);
        file->deleteLater();
        _socketFileMap.remove(id);
        _socketIdMap.remove(id);
    }
    _usernameIdMap.remove(id);
    return;
}

void FtpManager::handleSocketReadyRead(QTcpSocket* socket)
{
    while(socket->canReadLine())
    {
        qint64 size = 0;
        qint32 uid = -1;
        QByteArray fileData;
        QDataStream dataStream(socket);
        dataStream >> size;
        if(size == 0)
        {
            qDebug()<<"id set";
            dataStream >> uid;
            QString temp;
            dataStream >> temp;
            if(uid == -1)
                continue;
            addSocket(socket, uid);
            continue;
        }
        else if(size == 1)
        {
            QByteArray data;
            dataStream >> data;
            data = data.trimmed();
            fileData = QByteArray::fromBase64(data);
            QFile* file = _socketFileMap.value(getId(socket));
            if(file == (QFile*)0)
                continue;
            if(!file->isOpen())
                continue;
            file->write(fileData);
            file->flush();
            file->waitForBytesWritten(1000);
        }
        else if(size == 2)
        {
            QString temp;
            dataStream >> temp;
            sendMessageToId(_socketIdMap.key(socket), "UPLOAD_FILE_RECV\r\n");
            continue;
        }
    }
    return;
}

void FtpManager::openFileForId(qint32 id, QString file)
{
    file.replace("..", ".");
    if(!file.startsWith('/'))
        file.prepend('/');
    file.prepend(QString("/" + getUserHomeDirectory(_usernameIdMap.value(id))));
    file.prepend("/userfiles");
    file.prepend(QApplication::applicationDirPath());
    QFile* fileObj;
    fileObj = _socketFileMap.value(id);
    if(fileObj == (QFile*)0)
        fileObj = new QFile();
    if(fileObj->isOpen())
        fileObj->close();
    fileObj->setFileName(file);
    if(!fileObj->open(QFile::Truncate|QFile::WriteOnly))
    {
        qDebug()<<this<<"failed to open file"<<fileObj->fileName()<<"for "<<id<<" for the following reason: "<<fileObj->errorString();
        fileObj->deleteLater();
        _socketFileMap.insert(id, new QFile());
        sendMessageToId(id, "FTP_OPEN_FILE_ERROR\r\n");
        return;
    }
    _socketFileMap.insert(id, fileObj);
    sendMessageToId(id, "UPLOAD_FILE_ACK\r\n");
    qDebug()<<this<<"successfully opened file"<<fileObj->fileName()<<"for "<<id;
    return;
}

void FtpManager::closeFileForId(qint32 id)
{
    QFile* file = _socketFileMap.value(id);
    if(file == (QFile*)0)
    {
        _socketFileMap.insert(id, new QFile());
        qDebug()<<this<<"successfully created new file pointer for "<<id;
    }
    if(file->isOpen())
        file->close();
    file->deleteLater();
    _socketFileMap.insert(id, new QFile());
    qDebug()<<this<<"successfully closed file for "<<id;
    return;
}

void FtpManager::generateHomeDirectoryForUser(QString username)
{
    if(!openDatabase())
        return;
    QSqlQuery query(_db);
    bool passed = false;
    int tries = 0;
    do
    {
        query.prepare("SELECT userfolders.folder FROM userfolders WHERE userfolders.username = :username LIMIT 1");
        query.bindValue(":username", username);
        if(!query.exec())
        {
            qDebug()<<"Error with query: "<<query.lastError().text();
            return;
        }
        if(query.first())
        {
            createUserfilesDir();
            QDir dir(QApplication::applicationDirPath() + "/userfiles");
            dir.mkdir(query.value(0).toString());
            qDebug()<<"Created directory for user "<<username;
            passed = true;
        }
        else
        {
            query.prepare("INSERT INTO userfolders(username) VALUES(:username)");
            query.bindValue(":username", username);
            if(!query.exec())
            {
                qDebug()<<"Error with query: "<<query.lastError().text();
                return;
            }
        }
        tries++;
    }
    while(!passed && tries < 5);
    return;
}

void FtpManager::sendFileToId(qint32 id, QString file)
{
    file.replace("..", ".");
    if(!file.startsWith('/'))
        file.prepend('/');
    file.prepend(QString("/" + getUserHomeDirectory(_usernameIdMap.value(id))));
    file.prepend("/userfiles");
    file.prepend(QApplication::applicationDirPath());
    QFile fileObj(file);
    if(!fileObj.open(QFile::ReadOnly))
    {
        emit sendMessageToId(id, "FTP_OPEN_FILE_ERROR\r\n");
        return;
    }
    QTcpSocket* socket = getSocket(id);
    if(socket == (QTcpSocket*)0)
        return;
    while(!fileObj.atEnd())
    {
        socket->write(fileObj.read(2048*8));
        socket->flush();
        qDebug()<<"send";
    }
    return;
}

void FtpManager::requestFileList(qint32 id, QString dir)
{
    dir.replace("..", ".");
    if(!dir.startsWith('/'))
        dir.prepend('/');
    dir.prepend(QString("/" + getUserHomeDirectory(_usernameIdMap.value(id))));
    dir.prepend("/userfiles");
    dir.prepend(QApplication::applicationDirPath());
    QFileInfo info(dir);
    if(!info.isDir())
        return;
    QDir dirObj(dir);
    dirObj.setFilter(QDir::Files);
    QStringList list = dirObj.entryList(QDir::NoFilter, QDir::NoSort);
    emit sendFileList(id, list);
    return;
}

QString FtpManager::getUserHomeDirectory(const QString &username)
{
    if(!openDatabase())
        return "";
    QSqlQuery query(_db);
    query.prepare("SELECT userfolders.folder FROM userfolders WHERE userfolders.username = :username LIMIT 1");
    query.bindValue(":username", username);
    query.exec();
    if(query.first())
        return query.value(0).toString();
    return "";
}

bool FtpManager::openDatabase()
{
    if(!_db.isOpen())
    {
        qDebug()<<this<<"database is not open, opening";
        if(!_db.open())
        {
            qDebug()<<this<<"database failed to open: "<<_db.lastError().text();
            return false;
        }
    }
    return true;
}

void FtpManager::createUserfilesDir()
{
    QDir dir(QApplication::applicationDirPath() + "/userfiles");
    if(!dir.exists())
    {
        dir.cd(QApplication::applicationDirPath());
        dir.mkdir("userfiles");
    }
    return;
}
