#include "channelmodedialog.hpp"
#include "ui_channelmodedialog.h"

ChannelModeDialog::ChannelModeDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ChannelModeDialog)
{
    ui->setupUi(this);
    connect(ui->pushButtonSet, &QPushButton::pressed, this, &ChannelModeDialog::handleSetRequest);
    connect(ui->listView, &QListView::doubleClicked, this, &ChannelModeDialog::handleSetRequest);
    connect(ui->pushButtonCancel, &QPushButton::pressed, this, &ChannelModeDialog::reject);
}

ChannelModeDialog::~ChannelModeDialog()
{
    delete ui;
}

QListView* ChannelModeDialog::listView()
{
    return ui->listView;
}

QString ChannelModeDialog::channelname()
{
    return _channelname;
}

void ChannelModeDialog::setChannelname(const QString &channelname)
{
    _channelname = channelname;
    return;
}

bool ChannelModeDialog::teacher()
{
    return _teacher;
}

void ChannelModeDialog::setTeacher(bool teacher)
{
    _teacher = teacher;
    return;
}

bool ChannelModeDialog::student()
{
    return _student;
}

void ChannelModeDialog::setStudent(bool student)
{
    _student = student;
    return;
}

bool ChannelModeDialog::administrator()
{
    return _administrator;
}

void ChannelModeDialog::setAdministrator(bool administrator)
{
    _administrator = administrator;
    return;
}

void ChannelModeDialog::handleSetRequest()
{
    QString channelname = ui->listView->currentIndex().data().toString();
    if(channelname == "")
        reject();
    _channelname = channelname;
    _administrator = ui->checkBoxAdministrator->isChecked();
    _teacher = ui->checkBoxTeacher->isChecked();
    _student = ui->checkBoxStudent->isChecked();
    accept();
    return;
}
