#include "ircmode.hpp"

IrcMode::IrcMode(QString mode, QObject* parent) : QObject(parent)
{
    fromString(mode);
}

IrcMode::~IrcMode()
{

}

QString IrcMode::toString()
{
    QString string = "";
    if(_student) string += 'S';
    if(_teacher) string += 'T';
    if(_administrator) string += 'A';
    return string;
}

void IrcMode::fromString(const QString &string)
{
    if(string.contains('S'))
        _student = true;
    if(string.contains('T'))
        _teacher = true;
    if(string.contains('A'))
        _administrator = true;
    return;
}

void IrcMode::setStudent(bool mode)
{
    _student = mode;
    return;
}

bool IrcMode::student()
{
    return _student;
}

void IrcMode::setTeacher(bool mode)
{
    _teacher = mode;
    return;
}

bool IrcMode::teacher()
{
    return _teacher;
}

void IrcMode::setAdministrator(bool mode)
{
    _administrator = mode;
    return;
}

bool IrcMode::administrator()
{
    return _administrator;
}
