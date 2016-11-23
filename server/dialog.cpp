#include "dialog.hpp"
#include "ui_dialog.h"

Dialog::Dialog(QWidget* parent) : QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->checkBoxRun, &QCheckBox::toggled, this, &Dialog::tickBoxToggled);
	connect(ui->pushButtonAddAdmin, &QPushButton::pressed, this, &Dialog::addAdmin);
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
    connect(_server.manager(), &IrcManager::ftpAddUsernameIdPair, _ftpServer.manager(), &FtpManager::addUsernameIdPair, Qt::BlockingQueuedConnection);
    connect(_server.manager(), &IrcManager::ftpRemoveRecord, _ftpServer.manager(), static_cast<void (FtpManager::*)(qint32)>(&FtpManager::removeRecord));
    connect(_server.manager(), &IrcManager::ftpGenerateHomeDirectoryForUser, _ftpServer.manager(), &FtpManager::generateHomeDirectoryForUser);
    connect(_server.manager(), &IrcManager::ftpRequestFileList, _ftpServer.manager(), &FtpManager::requestFileList);
    connect(_server.manager(), &IrcManager::sendFileToId, _ftpServer.manager(), &FtpManager::sendFileToId);
    connect(_server.manager(), &IrcManager::openFileForId, _ftpServer.manager(), &FtpManager::openFileForId, Qt::BlockingQueuedConnection);
    connect(_server.manager(), &IrcManager::closeFileForId, _ftpServer.manager(), &FtpManager::closeFileForId, Qt::QueuedConnection);
    connect(_server.manager(), &IrcManager::deleteFileForId, _ftpServer.manager(), &FtpManager::deleteFileForId);
    connect(_ftpServer.manager(), &FtpManager::sendFileList, _server.manager(), &IrcManager::ftpReceiveFileList);
    connect(_ftpServer.manager(), &FtpManager::sendMessageToId, _server.manager(), &IrcManager::ftpSendMessageToId);
    return;
}

void Dialog::disconnectServers()
{
    disconnect(_server.manager(), &IrcManager::ftpAddUsernameIdPair, _ftpServer.manager(), &FtpManager::addUsernameIdPair);
    disconnect(_server.manager(), &IrcManager::ftpRemoveRecord, _ftpServer.manager(), static_cast<void (FtpManager::*)(qint32)>(&FtpManager::removeRecord));
    disconnect(_server.manager(), &IrcManager::ftpGenerateHomeDirectoryForUser, _ftpServer.manager(), &FtpManager::generateHomeDirectoryForUser);
    disconnect(_server.manager(), &IrcManager::ftpRequestFileList, _ftpServer.manager(), &FtpManager::requestFileList);
    disconnect(_server.manager(), &IrcManager::sendFileToId, _ftpServer.manager(), &FtpManager::sendFileToId);
    disconnect(_server.manager(), &IrcManager::openFileForId, _ftpServer.manager(), &FtpManager::openFileForId);
    disconnect(_server.manager(), &IrcManager::closeFileForId, _ftpServer.manager(), &FtpManager::closeFileForId);
    disconnect(_server.manager(), &IrcManager::deleteFileForId, _ftpServer.manager(), &FtpManager::deleteFileForId);
    disconnect(_ftpServer.manager(), &FtpManager::sendFileList, _server.manager(), &IrcManager::ftpReceiveFileList);
    disconnect(_ftpServer.manager(), &FtpManager::sendMessageToId, _server.manager(), &IrcManager::ftpSendMessageToId);
	return;
}

void Dialog::addAdmin()
{
	if(!_server.isListening())
	{
		QMessageBox::warning(this, "Error", "Server not running. Could not add user.", QMessageBox::Ok);
		return;
	}
	IrcManager* manager = _server.manager();
	if(!manager)
	{
		QMessageBox::warning(this, "Error", "Could not get IRC manager pointer. User not added.", QMessageBox::Ok);
		return;
	}
	if(ui->lineEditUsername->text().isEmpty())
	{
		QMessageBox::warning(this, "Error", "You haven't specified a username.", QMessageBox::Ok);
		return;
	}
	if(ui->lineEditPassword->text().isEmpty())
	{
		QMessageBox::warning(this, "Error", "You haven't specified a password.", QMessageBox::Ok);
		return;
	}
	if(!manager->registerDatabaseLogin(ui->lineEditUsername->text(), ui->lineEditPassword->text(), IrcMode("A")))
	{
		QMessageBox::warning(this, "Error", "Registering the user failed.", QMessageBox::Ok);
		return;
	}
	ui->lineEditUsername->setText("");
	ui->lineEditPassword->setText("");
}
