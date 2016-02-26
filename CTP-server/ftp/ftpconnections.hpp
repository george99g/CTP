#ifndef FTPCONNECTIONS_HPP
#define FTPCONNECTIONS_HPP

#include "../tcp/tcpconnections.hpp"
#include <QObject>

class FtpConnections : public TcpConnections
{
    Q_OBJECT
public:
    explicit FtpConnections(QObject* parent = 0);
    ~FtpConnections();
};

#endif // FTPCONNECTIONS_HPP
