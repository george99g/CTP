#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <QObject>
#include <QFile>
#include <QRect>
#include <QDebug>
#include <QDataStream>

class Configuration : public QObject
{
    Q_OBJECT
public:
    explicit Configuration(QObject *parent = 0);
    ~Configuration();
    void saveToFile();
    void loadFromFile();
    void setHostParameters(const QString &hostname, const qint64 &port);
    QString hostname();
    qint64 port();
    bool autoLogin();
    void setAutoLogin(bool autoLogin);
    QString username();
    QString password();
    void setLogin(const QString &username, const QString &password);
    int mainWindowX();
    int mainWindowY();
    void setMainWindowParameters(int x, int y);
private:
    bool _autoLogin;
    QString _hostname;
    qint64 _port;
    QString _username;
    QString _password;
    int _mainWindowX;
    int _mainWindowY;
    QFile* _saveFile;
};

#endif // CONFIGURATION_HPP
