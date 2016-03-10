#ifndef FTPMANAGER_HPP
#define FTPMANAGER_HPP

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTcpSocket>
#include <QMap>

class FtpManager : public QObject
{
    Q_OBJECT
public:
    explicit FtpManager(QThread* thread, QObject* parent = 0);
    ~FtpManager();
    QTcpSocket* getSocket(qint32 id);
    QTcpSocket* getSocket(const QString &username);
    QString getUsername(qint32 id);
    QString getUsername(QTcpSocket* socket);
    qint32 getId(const QString username);
    qint32 getId(QTcpSocket* socket);
    void addUsernameIdPair(const QString &username, qint32 id);
    void addSocket(QTcpSocket* socket, qint32 id);
    bool addSocket(QTcpSocket* socket, const QString &username);
    void removeSocket(QTcpSocket* socket);
    void removeRecord(const QString &username);
    void removeRecord(qint32 id);
private:
    QMap<qint32, QString> _usernameIdMap;
    QMap<qint32, QTcpSocket*> _socketIdMap;
    QThread* _thread;
};

#endif // FTPMANAGER_HPP