#ifndef IRCCONNECTIONS_HPP
#define IRCCONNECTIONS_HPP

#include <QObject>
#include "../config.hpp"
#include "ircmanager.hpp"
#include "../tcp/tcpconnections.hpp"


class IrcConnections : public TcpConnections
{
    Q_OBJECT
public:
    explicit IrcConnections(QThread* thread, qint16 ftpPort, QObject* parent = 0);
    ~IrcConnections();
    IrcManager* manager();
public slots:
    virtual void accept(qintptr handle, TcpConnection* connection);
    virtual void disconnected();
    virtual void connected();
    virtual void error(QAbstractSocket::SocketError socketError);
protected slots:
    void readyRead();
private:
    IrcManager* _manager;
    QThread* _thread;
    qint16 _ftpPort;
};

#endif // IRCCONNECTIONS_HPP
