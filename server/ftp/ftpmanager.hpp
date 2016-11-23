#ifndef FTPMANAGER_HPP
#define FTPMANAGER_HPP

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTcpSocket>
#include <QMap>
#include <QByteArray>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDataStream>
#include <QThread>

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
    void handleSocketReadyRead(QTcpSocket* socket);
    void openFileForId(qint32 id, QString file);
    void closeFileForId(qint32 id);
    void generateHomeDirectoryForUser(QString username);
    void sendFileToId(qint32 id, QString file);
    void requestFileList(qint32 id, QString dir);
    void deleteFileForId(qint32 id, QString file);
    static QString convertToNoSpace(QString string);
    static QString convertFromNoSpace(QString string);
signals:
    void sendFileList(qint32 id, const QStringList &list);
    void sendMessageToId(qint32 id, const QString &message);
private:
    QString getUserHomeDirectory(const QString &username);
    bool openDatabase();
    void createUserfilesDir();
    QMap<qint32, QString> _usernameIdMap;
    QMap<qint32, QTcpSocket*> _socketIdMap;
    QMap<qint32, QFile*> _socketFileMap;
    QThread* _thread;
    QSqlDatabase _db;
};

#endif // FTPMANAGER_HPP
