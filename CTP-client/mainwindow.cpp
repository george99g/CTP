#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _socket = new QTcpSocket(this);
    loginDialog = new LoginDialog(_socket, this);
    connect(loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
    connect(loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    loginDialog->show();
}

MainWindow::~MainWindow()
{
    if(loginDialog != (LoginDialog*)0)
        loginDialog->deleteLater();
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
    loginDialog->hide();
    show();
    loginDialog->deleteLater();
    loginDialog = (LoginDialog*)0;
    return;
}
