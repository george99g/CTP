#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include "ircmode.hpp"

class IrcClient : public QObject
{
    Q_OBJECT
public:
    explicit IrcClient(QString username, QTcpSocket* socket, QObject* parent = 0);
    ~IrcClient();
    QString username();
    void setUsername(const QString username);
    QTcpSocket* socket();
    void setSocket(QTcpSocket* socket);
    IrcMode* mode();
    void setMode(const QString &mode);
signals:

public slots:

private:
    QString _username;
    QTcpSocket* _socket;
    IrcMode _mode;
};

#endif // IRCCLIENT_HPP
