#include "privatemessagewindow.hpp"
#include "ui_privatemessagewindow.h"

PrivateMessageWindow::PrivateMessageWindow(Configuration* config, QWidget* parent) : QMainWindow(parent), ui(new Ui::PrivateMessageWindow)
{
    ui->setupUi(this);
    _config = config;
    _config->loadFromFile();
    ui->splitter->setSizes(_config->pmSplitterSizes());
}

PrivateMessageWindow::~PrivateMessageWindow()
{
    delete ui;
    _config->setPmSplitterSizes(ui->splitter->sizes());
    _config->saveToFile();
}
