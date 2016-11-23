#include "filewidget.hpp"
#include "ui_filewidget.h"

FileWidget::FileWidget(QWidget* parent) : QWidget(parent), ui(new Ui::FileWidget)
{
    ui->setupUi(this);
    connect(ui->pushButtonRefresh, &QPushButton::pressed, this, &FileWidget::requestRefresh);
    connect(ui->pushButtonDownload, &QPushButton::pressed, this, &FileWidget::handleDownloadFileRequest);
    connect(ui->pushButtonUpload, &QPushButton::pressed, this, &FileWidget::handleSendFileRequest);
    connect(ui->pushButtonDelete, &QPushButton::pressed, this, &FileWidget::handleDeleteFileRequest);
    retranslateUi();
}

FileWidget::~FileWidget()
{
    delete ui;
}

QListView *FileWidget::listView()
{
    return ui->listView;
}

void FileWidget::retranslateUi()
{
    ui->retranslateUi(this);
    return;
}

void FileWidget::handleDownloadFileRequest()
{
    QString file = "";
    file = ui->listView->currentIndex().data().toString();
    emit downloadFile(file);
    return;
}

void FileWidget::handleSendFileRequest()
{
    QString file = "";
    file = QFileDialog::getOpenFileName(this, QString(), QString(), QString("All files (*)"));
    if(file == "")
        return;
    emit sendFile(file);
    return;
}

void FileWidget::handleDeleteFileRequest()
{
    QString file = "";
    file = ui->listView->currentIndex().data().toString();
    emit deleteFile(file);
    return;
}
