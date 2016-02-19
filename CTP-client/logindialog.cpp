#include "logindialog.hpp"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QTcpSocket *socket, QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    _socket = socket;
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &LoginDialog::handleLoginCancel);
    connect(this, &LoginDialog::rejected, this, &LoginDialog::handleLoginCancel);
    connect(ui->pushButtonLogin, &QPushButton::clicked, this, &LoginDialog::handleLoginRequest);
    connect(_socket, &QTcpSocket::connected, this, &LoginDialog::handleSocketConnection);
    connect(_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &LoginDialog::handleSocketError);
    connect(_socket, &QTcpSocket::readyRead, this, &LoginDialog::handleSocketReadyRead);
    ui->progressBarConnection->setMinimum(0);
    ui->progressBarConnection->setValue(0);
    _username = "";
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

QString LoginDialog::getUsername()
{
    return _username;
}

void LoginDialog::handleLoginCancel()
{
    if(_socket->isOpen())
        _socket->close();
    emit loginCancelled();
    return;
}

void LoginDialog::handleSocketReadyRead()
{
    while(_socket->canReadLine())
    {
        QString currentLine = _socket->readLine().trimmed();
        if(currentLine.startsWith("LOGIN_FAIL"))
            handleInvalidAuthentication();
        else if(currentLine.startsWith("WRONG_ARGUMENTS"))
            handleInvalidAuthentication();
        else if(currentLine.startsWith("AUTH"))
        {
            QStringList splitLine = currentLine.split(' ', QString::SkipEmptyParts);
            if(splitLine.count() >= 2)
                _username = splitLine.at(1);
            handleValidAuthentication();
        }
    }
    return;
}

void LoginDialog::handleLoginRequest()
{
    if(!_socket->isOpen())
    {
        _socket->connectToHost(ui->lineEditIP->text(), ui->spinBoxPort->value());
        ui->progressBarConnection->setMaximum(0);
    }
    return;
}

void LoginDialog::handleSocketConnection()
{
    if(ui->lineEditUsername->text().length() <= 0 || ui->lineEditPassword->text().length() <= 0)
    {
        handleInvalidAuthentication();
        return;
    }
    QString loginMessage = "LOGIN ";
    loginMessage += ui->lineEditUsername->text().replace(' ', "\\s");
    loginMessage += ' ';
    loginMessage += ui->lineEditPassword->text().replace(' ', "\\s");
    loginMessage += "\r\n";
    _socket->write(loginMessage.toUtf8());
    return;
}

void LoginDialog::handleValidAuthentication()
{
    emit loginAccepted();
    return;
}

void LoginDialog::handleInvalidAuthentication()
{
    QMessageBox::warning(this, tr("warning.invalidAuthentication"), tr("warning.invalidAuthenticationText"));
    ui->progressBarConnection->setMaximum(100);
    if(_socket->isOpen())
        _socket->close();
    return;
}

void LoginDialog::handleSocketError()
{
    QMessageBox::warning(this, tr("warning.connectionFailed"), tr("warning.connectionFailedText:\n%1").arg(_socket->errorString()));
    ui->progressBarConnection->setMaximum(100);
    if(_socket->isOpen())
        _socket->close();
    return;
}
