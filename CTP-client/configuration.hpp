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
    int getMainWindowX();
    int getMainWindowY();
    void setMainWindowParameters(int x, int y);
private:
    int _mainWindowX;
    int _mainWindowY;
    QFile* _saveFile;
};

#endif // CONFIGURATION_HPP
