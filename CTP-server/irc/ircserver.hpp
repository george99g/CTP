#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <QObject>
#include "../tcp/tcpserver.hpp"
#include "ircconnection.hpp"

class IrcServer : public TcpServer
{
    Q_OBJECT
public:
    explicit IrcServer(QObject *parent = 0);
    ~IrcServer();
signals:

public slots:
};

#endif // IRCSERVER_HPP
