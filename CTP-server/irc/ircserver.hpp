#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <QObject>

class IrcServer : public QObject
{
    Q_OBJECT
public:
    explicit IrcServer(QObject *parent = 0);
    ~IrcServer();
signals:

public slots:
};

#endif // IRCSERVER_HPP
