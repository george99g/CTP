#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <QObject>
#include "../tcp/tcpserver.hpp"
#include "ircconnections.hpp"

//Despite its name, these classes do not conform to the IRC protocol
//TODO: Rename these classes when re-writing, possibly implement them with .pri instead of a subfolder

class IrcServer : public TcpServer
{
    Q_OBJECT
public:
    explicit IrcServer(QObject *parent = 0);
    ~IrcServer();
    virtual bool listen(const QHostAddress &address, quint16 port);
protected:
    virtual void incomingConnection(qintptr/*Can be qint64, qHandle, qintptr or uint, depending on platform*/
                                            handle);
signals:

public slots:
    virtual void complete();
private:
    IrcConnections* _connections;
};

#endif // IRCSERVER_HPP
