#include "usermodedialog.hpp"
#include "ui_usermodedialog.h"

UserModeDialog::UserModeDialog(QWidget* parent) : QDialog(parent), ui(new Ui::UserModeDialog)
{
    ui->setupUi(this);
    connect(ui->pushButtonSet, &QPushButton::pressed, this, &UserModeDialog::handleSetRequest);
    connect(ui->listView, &QListView::doubleClicked, this, &UserModeDialog::handleSetRequest);
    connect(ui->pushButtonCancel, &QPushButton::pressed, this, &UserModeDialog::reject);
}

UserModeDialog::~UserModeDialog()
{
    delete ui;
}

QString UserModeDialog::username()
{
    return _username;
}

void UserModeDialog::setUsername(QString username)
{
    _username = username;
    return;
}

bool UserModeDialog::student()
{
    return _student;
}

void UserModeDialog::setStudent(bool student)
{
    _student = student;
    return;
}

bool UserModeDialog::teacher()
{
    return _teacher;
}

void UserModeDialog::setTeacher(bool teacher)
{
    _teacher = teacher;
    return;
}

bool UserModeDialog::administrator()
{
    return _administrator;
}

void UserModeDialog::setAdministrator(bool administrator)
{
    _administrator = administrator;
    return;
}

QListView* UserModeDialog::listView()
{
    return ui->listView;
}

void UserModeDialog::handleSetRequest()
{
    QString username = ui->listView->currentIndex().data().toString();
    if(username == "")
        reject();
    _username = username;
    _administrator = ui->checkBoxAdministrator->isChecked();
    _teacher = ui->checkBoxTeacher->isChecked();
    _student = ui->checkBoxStudent->isChecked();
    accept();
    return;
}
