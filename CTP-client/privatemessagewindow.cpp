#include "privatemessagewindow.hpp"
#include "ui_privatemessagewindow.h"

PrivateMessageWindow::PrivateMessageWindow(Configuration* config, QTcpSocket* socket, QWidget* parent) : QMainWindow(parent), ui(new Ui::PrivateMessageWindow)
{
    ui->setupUi(this);
    _config = config;
    _socket = socket;
}

PrivateMessageWindow::~PrivateMessageWindow()
{
    delete ui;
}
