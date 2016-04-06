#ifndef USERMODEDIALOG_HPP
#define USERMODEDIALOG_HPP

#include <QDialog>
#include <QDebug>
#include <QListView>

namespace Ui
{
    class UserModeDialog;
}

class UserModeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserModeDialog(QWidget* parent = 0);
    ~UserModeDialog();
    QString username();
    void setUsername(QString username);
    bool student();
    void setStudent(bool student);
    bool teacher();
    void setTeacher(bool teacher);
    bool administrator();
    void setAdministrator(bool administrator);
    QListView* listView();
private:
    QString _username;
    bool _student;
    bool _teacher;
    bool _administrator;
    void handleSetRequest();
    Ui::UserModeDialog* ui;
};

#endif // USERMODEDIALOG_HPP
