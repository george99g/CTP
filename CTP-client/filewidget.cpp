#include "filewidget.hpp"
#include "ui_filewidget.h"

FileWidget::FileWidget(QWidget* parent) : QWidget(parent), ui(new Ui::FileWidget)
{
    ui->setupUi(this);
    _ircSocket = (QTcpSocket*)0;
    _ftpSocket = (QTcpSocket*)0;
}

FileWidget::~FileWidget()
{
    delete ui;
}

void FileWidget::setSockets(QTcpSocket *ircSocket, QTcpSocket *ftpSocket)
{
    _ircSocket = ircSocket;
    _ftpSocket = ftpSocket;
    return;
}
