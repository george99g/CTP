#include "privatemessagewindow.hpp"
#include "ui_privatemessagewindow.h"

PrivateMessageWindow::PrivateMessageWindow(Configuration* config, QWidget* parent) : QMainWindow(parent), ui(new Ui::PrivateMessageWindow)
{
    ui->setupUi(this);
    _config = config;
    _config->loadFromFile();
    ui->splitter->setSizes(_config->pmSplitterSizes());
    connect(ui->listView, &QListView::clicked, this, &PrivateMessageWindow::handleUserChangeRequest);
    retranslateUi();
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

void PrivateMessageWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent* stateChangeEvent = static_cast<QWindowStateChangeEvent*>(event);
        if(stateChangeEvent->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized )
            _config->setPmMaximized(true);
        else if(stateChangeEvent->oldState() == Qt::WindowMaximized && this->windowState() == Qt::WindowNoState)
            _config->setPmMaximized(false);
    }
    return;
}

void PrivateMessageWindow::setUser(const QString &username)
{
    emit switchUser(username);
    return;
}

void PrivateMessageWindow::retranslateUi()
{
    ui->retranslateUi(this);
    return;
}

void PrivateMessageWindow::handleUserChangeRequest()
{
    QString selectedUser = ui->listView->currentIndex().data().toString();
    emit switchUser(selectedUser);
    return;
}

void PrivateMessageWindow::resizeEvent(QResizeEvent*)
{
    _config->setPmWindowParameters(width(), height());
    return;
}
