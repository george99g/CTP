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
    _config->setPmSplitterSizes(ui->splitter->sizes());
    delete ui;
}

QStackedWidget* PrivateMessageWindow::stackedWidget()
{
    return ui->stackedWidget;
}

QListView* PrivateMessageWindow::listView()
{
    return ui->listView;
}

QList<int> PrivateMessageWindow::splitterSizes()
{
    return ui->splitter->sizes();
}

void PrivateMessageWindow::resizeEvent(QResizeEvent*)
{
    _config->setPmWindowParameters(width(), height());
    return;
}
