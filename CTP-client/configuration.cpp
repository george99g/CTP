#include "configuration.hpp"

Configuration::Configuration(QObject *parent) : QObject(parent)
{
    _saveFile = new QFile("config.dat");
    _mainWindowX = 1000;
    _mainWindowY = 500;
    _autoLogin = false;
    _username = "";
    _password = "";
    _hostname = "localhost";
    _port = 2000;
    _maximized = false;
}

Configuration::~Configuration()
{
    _saveFile->deleteLater();
}

void Configuration::saveToFile()
{
    if(!_saveFile->open(QFile::Truncate|QFile::WriteOnly))
    {
        qDebug()<<"Could not open config file";
        return;
    }
    QDataStream out(_saveFile);
    out << _mainWindowX << _mainWindowY
        << _autoLogin << _username
        << _password << _hostname
        << _port << _maximized;
    _saveFile->close();
    return;
}

void Configuration::loadFromFile()
{
    if(!_saveFile->open(QFile::ReadOnly))
    {
        qDebug()<<"Could not open config file";
        return;
    }
    QDataStream in(_saveFile);
    in >> _mainWindowX >> _mainWindowY
       >> _autoLogin >> _username
       >> _password >> _hostname
       >> _port >> _maximized;
    _saveFile->close();
    return;
}

void Configuration::setHostParameters(const QString &hostname, const qint64 &port)
{
    _hostname = hostname;
    _port = port;
}

QString Configuration::hostname()
{
    return _hostname;
}

qint64 Configuration::port()
{
    return _port;
}

bool Configuration::autoLogin()
{
    return _autoLogin;
}

void Configuration::setAutoLogin(bool autoLogin)
{
    _autoLogin = autoLogin;
    return;
}

QString Configuration::username()
{
    return _username;
}

QString Configuration::password()
{
    return _password;
}

void Configuration::setLogin(const QString &username, const QString &password)
{
    _username = username;
    _password = password;
    return;
}

int Configuration::mainWindowX()
{
    return _mainWindowX;
}

int Configuration::mainWindowY()
{
    return _mainWindowY;
}

bool Configuration::maximized()
{
    return _maximized;
}

void Configuration::setMaximized(bool maximized)
{
    _maximized = maximized;
    return;
}

void Configuration::setMainWindowParameters(int x, int y)
{
    _mainWindowX = x;
    _mainWindowY = y;
    return;
}
