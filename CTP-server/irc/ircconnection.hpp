#ifndef IRCCONNECTION_HPP
#define IRCCONNECTION_HPP

#include <QObject>

class IrcConnection : public QObject
{
    Q_OBJECT
public:
    explicit IrcConnection(QObject *parent = 0);
    ~IrcConnection();
signals:

public slots:
};

#endif // IRCCONNECTION_HPP
