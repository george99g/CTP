#include "configuration.hpp"

Configuration::Configuration(QObject *parent) : QObject(parent)
{
    _saveFile = new QFile("config.dat");
    _mainWindowX = 1000;
    _mainWindowY = 500;
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
    out << _mainWindowX << _mainWindowY;
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
    in >> _mainWindowX >> _mainWindowY;
    _saveFile->close();
    return;
}

int Configuration::getMainWindowX()
{
    return _mainWindowX;
}

int Configuration::getMainWindowY()
{
    return _mainWindowY;
}

void Configuration::setMainWindowParameters(int x, int y)
{
    _mainWindowX = x;
    _mainWindowY = y;
    return;
}
