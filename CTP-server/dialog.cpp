#include "dialog.hpp"
#include "ui_dialog.h"

Dialog::Dialog(QWidget* parent) : QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->checkBoxRun, &QCheckBox::toggled, this, &Dialog::tickBoxToggled);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::tickBoxToggled(bool state)
{
    if(state)
    {
        _server.setFtpPort(ui->spinBoxFtpPort->value());
        if(_server.listen(QHostAddress::Any, ui->spinBox->value()) && _ftpServer.listen(QHostAddress::Any, ui->spinBoxFtpPort->value()))
        {
            ui->checkBoxRun->setChecked(true);
            ui->spinBox->setEnabled(false);
            ui->spinBoxFtpPort->setEnabled(false);
            connectServers();
        }
    }
    else
    {
        disconnectServers();
        _server.close();
        _ftpServer.close();
        ui->checkBoxRun->setChecked(false);
        ui->spinBox->setEnabled(true);
        ui->spinBoxFtpPort->setEnabled(true);
    }
    return;
}

//Don't forget to connect the IrcManager signals to the FtpManager slots!
void Dialog::connectServers()
{
    connect(_server.manager(), &IrcManager::ftpAddUsernameIdPair, _ftpServer.manager(), &FtpManager::addUsernameIdPair);
    connect(_server.manager(), &IrcManager::ftpRemoveRecord, _ftpServer.manager(), static_cast<void (FtpManager::*)(qint32)>(&FtpManager::removeRecord));
    connect(_server.manager(), &IrcManager::ftpGenerateHomeDirectoryForUser, _ftpServer.manager(), &FtpManager::generateHomeDirectoryForUser);
    return;
}

void Dialog::disconnectServers()
{
    disconnect(_server.manager(), &IrcManager::ftpAddUsernameIdPair, _ftpServer.manager(), &FtpManager::addUsernameIdPair);
    disconnect(_server.manager(), &IrcManager::ftpRemoveRecord, _ftpServer.manager(), static_cast<void (FtpManager::*)(qint32)>(&FtpManager::removeRecord));
    disconnect(_server.manager(), &IrcManager::ftpGenerateHomeDirectoryForUser, _ftpServer.manager(), &FtpManager::generateHomeDirectoryForUser);
    return;
}
