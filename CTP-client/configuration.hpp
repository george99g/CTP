#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <QObject>
#include <QFile>
#include <QRect>
#include <QDebug>
#include <QDataStream>
#include <QList>

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
    int pmWindowX();
    int pmWindowY();
    void setPmWindowParameters(int x, int y);
    bool maximized();
    void setMaximized(bool maximized);
    bool pmMaximized();
    void setPmMaximized(bool maximized);
    void setMainWindowParameters(int x, int y);
    QList<int> pmSplitterSizes();
    void setPmSplitterSizes(const QList<int> &splitterSizes);
    QList<int> splitterSizes();
    void setSplitterSizes(const QList<int> &splitterSizes);
    QString language();
    void setLanguage(const QString &language);
private:
    bool _maximized;
    bool _pmMaximized;
    bool _autoLogin;
    QString _hostname;
    qint64 _port;
    QString _username;
    QString _password;
    int _mainWindowX;
    int _mainWindowY;
    int _pmWindowX;
    int _pmWindowY;
    QList<int> _splitterSizes;
    QList<int> _pmSplitterSizes;
    QString _language;
    QFile* _saveFile;
};

#endif // CONFIGURATION_HPP
