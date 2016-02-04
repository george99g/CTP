#include "ircmode.hpp"

IrcMode::IrcMode(QString mode, QObject* parent) : QObject(parent)
{
    _student = false;
    _teacher = false;
    _administrator = false;
    fromString(mode);
}

IrcMode::~IrcMode()
{

}

QString IrcMode::toString()
{
    QString string = "";
    if(_student)
        string += 'S';
    if(_teacher)
        string += 'T';
    if(_administrator)
        string += 'A';
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

void IrcMode::addMode(QChar mode)
{
    if(mode == 'S')
        _student = true;
    else if(mode == 'T')
        _teacher = true;
    else if(mode == 'A')
        _administrator = true;
    return;
}

void IrcMode::removeMode(QChar mode)
{
    if(mode == 'S')
        _student = false;
    else if(mode == 'T')
        _teacher = false;
    else if(mode == 'A')
        _administrator = false;
    return;
}
