#ifndef IRCCONNECTION_HPP
#define IRCCONNECTION_HPP

#include <QObject>
#include "../tcp/tcpconnection.hpp"

class IrcConnection : public TcpConnection
{
    Q_OBJECT
public:
    explicit IrcConnection(QObject* parent = 0);
    ~IrcConnection();
signals:

public slots:

private:
};

#endif // IRCCONNECTION_HPP
