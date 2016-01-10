#ifndef IRCCONNECTIONS_HPP
#define IRCCONNECTIONS_HPP

#include <QObject>
#include "ircconnection.hpp"
#include "../tcp/tcpconnections.hpp"

class IrcConnections : public TcpConnections
{
    Q_OBJECT
public:
    explicit IrcConnections(QObject *parent = 0);
    ~IrcConnections();
signals:

public slots:
    virtual void accept(qintptr handle, TcpConnection *connection);
    virtual void disconnected();
    virtual void connected();
protected slots:
    void readyRead();
protected:

private:

};

#endif // IRCCONNECTIONS_HPP
