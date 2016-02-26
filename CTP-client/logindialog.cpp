#include "logindialog.hpp"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QTcpSocket *socket, Configuration* config, QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    _socket = socket;
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &LoginDialog::handleLoginCancel);
    connect(this, &LoginDialog::rejected, this, &LoginDialog::handleLoginCancel);
    connect(ui->pushButtonLogin, &QPushButton::clicked, this, &LoginDialog::handleLoginRequest);
    connect(_socket, &QTcpSocket::connected, this, &LoginDialog::handleSocketConnection);
    connect(_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &LoginDialog::handleSocketError);
    connect(_socket, &QTcpSocket::readyRead, this, &LoginDialog::handleSocketReadyRead);
    connect(_socket, &QTcpSocket::stateChanged, this, &LoginDialog::handleSocketStateChanged);
    ui->progressBarConnection->setMinimum(0);
    ui->progressBarConnection->setValue(0);
    ui->progressBarConnection->setMaximum(5);
    ui->progressBarConnection->setFormat("");
    _username = "";
    _config = config;
    if(_config->autoLogin())
        handleAutoLogin();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

QString LoginDialog::getUsername()
{
    return _username;
}

QString LoginDialog::convertToNoSpace(QString string)
{
    string.replace("\\", "\\\\");
    string.replace(" ", "\\s");
    return string;
}

QString LoginDialog::convertFromNoSpace(QString string)
{
    string.replace("\\\\", "\\");
    string.replace("\\s", " ");
    return string;
}

void LoginDialog::handleAutoLogin()
{
    ui->checkBoxAutoLogin->setChecked(_config->autoLogin());
    ui->lineEditIP->setText(_config->hostname());
    ui->spinBoxPort->setValue(_config->port());
    ui->lineEditUsername->setText(_config->username());
    ui->lineEditPassword->setText(_config->password());
    if(!_socket->isOpen())
        _socket->connectToHost(_config->hostname(), _config->port());
    return;
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
        _socket->connectToHost(ui->lineEditIP->text(), ui->spinBoxPort->value());
    return;
}

void LoginDialog::handleSocketConnection()
{
    ui->progressBarConnection->setValue(4);
    ui->progressBarConnection->setFormat(tr("login.loggingIn"));
    if(ui->lineEditUsername->text().length() <= 0 || ui->lineEditPassword->text().length() <= 0)
    {
        handleInvalidAuthentication();
        return;
    }
    QString loginMessage = "LOGIN ";
    loginMessage += ui->lineEditUsername->text();
    loginMessage += ' ';
    loginMessage += ui->lineEditPassword->text().replace(' ', "\\s");
    loginMessage += "\r\n";
    _socket->write(loginMessage.toUtf8());
    return;
}

void LoginDialog::handleValidAuthentication()
{
    if(ui->checkBoxAutoLogin->isChecked())
    {
        _config->setAutoLogin(true);
        _config->setHostParameters(ui->lineEditIP->text(), ui->spinBoxPort->value());
        _config->setLogin(convertToNoSpace(ui->lineEditUsername->text()), convertToNoSpace(ui->lineEditPassword->text()));
        _config->saveToFile();
    }
    else
    {
        _config->setAutoLogin(false);
        _config->setHostParameters(ui->lineEditIP->text(), ui->spinBoxPort->value());
        _config->setLogin("", "");
        _config->saveToFile();
    }
    emit loginAccepted();
    return;
}

void LoginDialog::handleInvalidAuthentication()
{
    QMessageBox::warning(this, tr("warning.invalidAuthentication"), tr("warning.invalidAuthenticationText"));
    ui->progressBarConnection->setValue(0);
    ui->progressBarConnection->setFormat("");
    if(_socket->isOpen())
        _socket->close();
    return;
}

void LoginDialog::handleSocketError()
{
    QMessageBox::warning(this, tr("warning.connectionFailed"), tr("warning.connectionFailedText:\n%1").arg(_socket->errorString()));
    ui->progressBarConnection->setValue(0);
    ui->progressBarConnection->setFormat("");
    if(_socket->isOpen())
        _socket->close();
    return;
}

void LoginDialog::handleSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    switch(socketState)
    {
    case QAbstractSocket::UnconnectedState:
        ui->progressBarConnection->setValue(0);
        ui->progressBarConnection->setFormat("");
        break;
    case QAbstractSocket::HostLookupState:
        ui->progressBarConnection->setValue(1);
        ui->progressBarConnection->setFormat(tr("login.hostLookup"));
        break;
    case QAbstractSocket::ConnectingState:
        ui->progressBarConnection->setValue(2);
        ui->progressBarConnection->setFormat(tr("login.connecting"));
        break;
    case QAbstractSocket::ConnectedState:
        ui->progressBarConnection->setValue(3);
        ui->progressBarConnection->setFormat(tr("login.connected"));
    default:
        break;
    }
    return;
}
