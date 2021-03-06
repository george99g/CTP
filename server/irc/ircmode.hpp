#ifndef IRCMODE_HPP
#define IRCMODE_HPP

#include <QString>

class IrcMode
{
public:
	IrcMode(QString mode = "");
    ~IrcMode();
    QString toString();
    void fromString(const QString &string);
    void setStudent(bool mode);
    bool student();
    void setTeacher(bool mode);
    bool teacher();
    void setAdministrator(bool mode);
    bool administrator();
    void addMode(QChar mode);
    void removeMode(QChar mode);
private:
    bool _student;
    bool _teacher;
    bool _administrator;
};

#endif // IRCMODE_HPP
