#include "configuration.hpp"

Configuration::Configuration(QObject *parent) : QObject(parent)
{
    //Make sure all of these are set
    //Everything will fall back to these if the read fails or the config file doesn't exist
    _saveFile = new QFile("config.dat");
    _mainWindowX = 1000;
    _mainWindowY = 500;
    _autoLogin = false;
    _username = "";
    _password = "";
    _hostname = "localhost";
    _port = 2000;
    _maximized = false;
    _splitterSizes = QList<int>();
    _pmSplitterSizes = QList<int>();
    _pmWindowX = 1000;
    _pmWindowY = 500;
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
    //For the sake of compatability, please only add values at the end.
    //Otherwise you'll need to re-create all config files to avoid issues
    out << _mainWindowX << _mainWindowY
        << _autoLogin << _username
        << _password << _hostname
        << _port << _maximized
        << _splitterSizes << _pmSplitterSizes
        << _pmWindowX << _pmWindowY;
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
    //Make sure this matches the above
    in >> _mainWindowX >> _mainWindowY
       >> _autoLogin >> _username
       >> _password >> _hostname
       >> _port >> _maximized
       >> _splitterSizes >> _pmSplitterSizes
       >> _pmWindowX >> _pmWindowY;
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

int Configuration::pmWindowX()
{
    return _pmWindowX;
}

int Configuration::pmWindowY()
{
    return _pmWindowY;
}

void Configuration::setPmWindowParameters(int x, int y)
{
    _pmWindowX = x;
    _pmWindowY = y;
    return;
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

QList<int> Configuration::pmSplitterSizes()
{
    return _pmSplitterSizes;
}

void Configuration::setPmSplitterSizes(const QList<int> &splitterSizes)
{
    _pmSplitterSizes = splitterSizes;
    return;
}

QList<int> Configuration::splitterSizes()
{
    return _splitterSizes;
}

void Configuration::setSplitterSizes(const QList<int> &splitterSizes)
{
    _splitterSizes = splitterSizes;
    return;
}
