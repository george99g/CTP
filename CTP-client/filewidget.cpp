#include "filewidget.hpp"
#include "ui_filewidget.h"

FileWidget::FileWidget(QWidget* parent) : QWidget(parent), ui(new Ui::FileWidget)
{
    ui->setupUi(this);
    connect(ui->pushButtonRefresh, &QPushButton::pressed, this, &FileWidget::requestRefresh);
    connect(ui->pushButtonDownload, &QPushButton::pressed, this, &FileWidget::handleDownloadFileRequest);
}

FileWidget::~FileWidget()
{
    delete ui;
}

QListView *FileWidget::listView()
{
    return ui->listView;
}

void FileWidget::handleDownloadFileRequest()
{
    QString file = "";
    file = ui->listView->currentIndex().data().toString();
    emit downloadFile(file);
    return;
}
