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
    bool pinged();
    void setPinged(bool state);
    bool ponged();
    void setPonged(bool state);
private:
    bool _pinged;
    bool _ponged;
    QString _username;
    QTcpSocket* _socket;
    IrcMode _mode;
};

#endif // IRCCLIENT_HPP
