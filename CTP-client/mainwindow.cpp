#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _socket = new QTcpSocket(this);
    _loginDialog = new LoginDialog(_socket, this);
    connect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
    connect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
    connect(ui->actionPartChannel, &QAction::triggered, this, &MainWindow::handlePartChannelRequest);
    connect(ui->actionJoinChannel, &QAction::triggered, this, &MainWindow::handleJoinChannelRequest);
    connect(ui->actionLogOut, &QAction::triggered, this, &MainWindow::handleLogoutRequest);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    _loginDialog->show();
}

MainWindow::~MainWindow()
{
    if(_loginDialog != (LoginDialog*)0)
        _loginDialog->deleteLater();
    if(_socket != (QTcpSocket*)0)
    {
        if(_socket->isOpen())
            _socket->close();
        _socket->deleteLater();
    }
    delete ui;
}

void MainWindow::loginCancelled()
{
    close();
    return;
}

void MainWindow::loginAccepted()
{
    _loginDialog->hide();
    _username = _loginDialog->getUsername();
    disconnect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
    disconnect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
    show();
    _loginDialog->deleteLater();
    _loginDialog = (LoginDialog*)0;
    connectSocketSignals();
    requestChannelListPopulation();
    return;
}

QString MainWindow::convertToNoSpace(QString string)
{
    string.replace("\\", "\\\\");
    string.replace(" ", "\\s");
    return string;
}

QString MainWindow::convertFromNoSpace(QString string)
{
    string.replace("\\", "\\\\");
    string.replace(" ", "\\s");
    return string;
}

void MainWindow::handleSocketReadyRead()
{
    if(_socket->canReadLine())
    {
        QString line = _socket->readLine().trimmed();
        qDebug()<<line;
        QStringList messageParameters = line.split(' ', QString::SkipEmptyParts);
        if(messageParameters.count() <= 0)
            return;
        if(messageParameters.at(0) == "MY_CHANNELLIST" && messageParameters.count() > 1)
            for(unsigned i = 1; i < (unsigned)messageParameters.count(); i++)
                requestUsernamesForChannel(messageParameters.at(i));
    }
    return;
}

void MainWindow::connectSocketSignals()
{
    connect(_socket, &QTcpSocket::readyRead, this, &MainWindow::handleSocketReadyRead);
    connect(_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &MainWindow::handleSocketError);
    connect(_socket, &QTcpSocket::disconnected, this, &MainWindow::handleSocketDisconnected);
    return;
}

void MainWindow::disconnectSocketSignals()
{
    disconnect(_socket, &QTcpSocket::readyRead, this, &MainWindow::handleSocketReadyRead);
    disconnect(_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &MainWindow::handleSocketError);
    disconnect(_socket, &QTcpSocket::disconnected, this, &MainWindow::handleSocketDisconnected);
    return;
}

void MainWindow::requestChannelListPopulation()
{
    if(!_socket->isOpen())
        return;
    _socket->write("GET_MY_CHANNELLIST\r\n");
    _socket->flush();
    return;
}

void MainWindow::handleSocketError()
{
    if(_loginDialog == (LoginDialog*)0)
    {
        this->hide();
        disconnectSocketSignals();
        if(_socket->isOpen())
            _socket->close();
        _loginDialog = new LoginDialog(_socket, this);
        connect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
        connect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
        _loginDialog->show();
    }
    return;
}

void MainWindow::handleSocketDisconnected()
{
    if(_loginDialog == (LoginDialog*)0)
    {
        this->hide();
        disconnectSocketSignals();
        if(_socket->isOpen())
            _socket->close();
        _loginDialog = new LoginDialog(_socket, this);
        connect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
        connect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
        _loginDialog->show();
    }
    return;
}

void MainWindow::handleJoinChannelRequest()
{
    bool accepted;
    QString text = QInputDialog::getText(this, tr("dialog.getJoinChannelName.title"), tr("dialog.getJoinChannelName.content"), QLineEdit::Normal, "", &accepted);
    if(!accepted)
        return;
    if(text.isEmpty())
        return;
    text = convertToNoSpace(text);
    _socket->write(QString("JOIN "+text+"\r\n").toUtf8());
    _socket->flush();
    return;
}

void MainWindow::handlePartChannelRequest()
{
    bool accepted;
    QString text = QInputDialog::getText(this, tr("dialog.getPartChannelName.title"), tr("dialog.getPartChannelName.content"), QLineEdit::Normal, "", &accepted);
    if(!accepted)
        return;
    if(text.isEmpty())
        return;
    text = convertToNoSpace(text);
    _socket->write(QString("PART "+text+"\r\n").toUtf8());
    _socket->flush();
    return;
}

void MainWindow::handleLogoutRequest()
{
    _socket->write("LOGOUT\r\n");
    _socket->flush();
    _socket->close();
    return;
}

void MainWindow::requestUsernamesForChannel(const QString &channel)
{
    _socket->write(QString("GET_USERLIST " + channel + "\r\n").toUtf8());
    _socket->flush();
    return;
}
