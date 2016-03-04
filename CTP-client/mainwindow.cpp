#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _socket = new QTcpSocket(this);
    _config.loadFromFile();
    _loginDialog = new LoginDialog(_socket, &_config, this);
    _pmWindow = (PrivateMessageWindow*)0;
    connect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
    connect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
    connect(ui->actionPartChannel, &QAction::triggered, this, &MainWindow::handlePartChannelRequest);
    connect(ui->actionJoinChannel, &QAction::triggered, this, &MainWindow::handleJoinChannelRequest);
    connect(ui->actionRefreshChannels, &QAction::triggered, this, &MainWindow::handleChannelRefreshRequest);
    connect(ui->actionLogOut, &QAction::triggered, this, &MainWindow::handleLogoutRequest);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionSendQuery, &QAction::triggered, this, &MainWindow::handleSendQueryRequest);
    connect(ui->actionShowPrivateMessages, &QAction::triggered, this, &MainWindow::handleShowPmRequest);
    connect(ui->listViewChannels, &QListView::clicked, this, &MainWindow::handleChannelListChangeRequest);
    _channelsModel.setStringList(_channelUsernames.keys());
    ui->listViewChannels->setModel(&_channelsModel);
    ui->listViewUsers->setModel(&_channelUsersModel.second);
    _loginDialog->show();
    _isAdmin = false;
    _isStudent = false;
    _isTeacher = false;
}

MainWindow::~MainWindow()
{
    if(_loginDialog != (LoginDialog*)0)
        _loginDialog->deleteLater();
    if(_pmWindow != (PrivateMessageWindow*)0)
        _pmWindow->deleteLater();
    if(_socket != (QTcpSocket*)0)
    {
        if(_socket->isOpen())
            _socket->close();
        _socket->deleteLater();
    }
    clearChatBoxWidgets();
    _config.saveToFile();
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
    ui->splitter->setSizes(_config.splitterSizes());
    if(!_config.maximized())
    {
        resize(_config.mainWindowX(), _config.mainWindowY());
        show();
    }
    else
        showMaximized();
    _loginDialog->deleteLater();
    _loginDialog = (LoginDialog*)0;
    connectSocketSignals();
    _pmWindow = new PrivateMessageWindow(&_config, this);
    requestChannelListPopulation();
    requestMode(_username);
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
    string.replace("\\\\", "\\");
    string.replace("\\s", " ");
    return string;
}

void MainWindow::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent* stateChangeEvent = static_cast<QWindowStateChangeEvent*>(event);
        if(stateChangeEvent->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized )
            _config.setMaximized(true);
        else if(stateChangeEvent->oldState() == Qt::WindowMaximized && this->windowState() == Qt::WindowNoState)
            _config.setMaximized(false);
    }
    return;
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    _config.setMainWindowParameters(size().width(), size().height());
    return;
}

void MainWindow::handleSocketReadyRead()
{
    while(_socket->canReadLine())
    {
        QString line = _socket->readLine().trimmed();
        qDebug()<<line;
        QStringList messageParameters = line.split(' ', QString::SkipEmptyParts);
        if(messageParameters.count() <= 0)
            return;
        if(messageParameters.at(0) == "MY_CHANNELLIST" && messageParameters.count() > 1)
            for(unsigned i = 1; i < (unsigned)messageParameters.count(); i++)
                requestUsernamesForChannel(messageParameters.at(i));
        else if(messageParameters.at(0) == "CHANNEL_USERLIST" && messageParameters.count() > 2)
        {
            QString channelname = convertFromNoSpace(messageParameters.at(1));
            QStringList userlist;
            for(unsigned i = 2; i < (unsigned)messageParameters.count(); i++)
                userlist.push_back(convertFromNoSpace(messageParameters.at(i)));
            if(!_channelUsernames.contains(convertFromNoSpace(channelname)))
            {
                _channelUsernames.insert(convertFromNoSpace(channelname), userlist);
                _channelsModel.setStringList(_channelUsernames.keys());
                insertChatBoxWidget(convertFromNoSpace(channelname));
            }
            else
            {
                _channelUsernames.remove(channelname);
                _channelUsernames.insert(channelname, userlist);
            }
        }
        else if(messageParameters.at(0) == "CHANNEL_JOINED" && messageParameters.count() > 1)
        {
            requestUsernamesForChannel(messageParameters.at(1));
            insertChatBoxWidget(convertFromNoSpace(messageParameters.at(1)));
        }
        else if(messageParameters.at(0) == "CHANNEL_PARTED" && messageParameters.count() > 1)
        {
            _channelUsernames.remove(convertFromNoSpace(messageParameters.at(1)));
            removeChatBoxWidget(convertFromNoSpace(messageParameters.at(1)));
            _channelsModel.setStringList(_channelUsernames.keys());
        }
        else if(messageParameters.count() > 2 && messageParameters.at(1) == "JOIN")
        {
            QString user = messageParameters.at(0);
            QString channel = messageParameters.at(2);
            QStringList userlist = _channelUsernames.value(convertFromNoSpace(channel));
            if(!userlist.contains(convertFromNoSpace(user)))
                userlist.push_back(convertFromNoSpace(user));
            _channelUsernames.insert(convertFromNoSpace(channel), userlist);
            if(_channelUsersModel.first == convertFromNoSpace(channel))
                _channelUsersModel.second.setStringList(_channelUsernames.value(convertFromNoSpace(channel)));
        }
        else if(messageParameters.count() > 2 && messageParameters.at(1) == "PART")
        {
            QString user = messageParameters.at(0);
            QString channel = messageParameters.at(2);
            QStringList userlist = _channelUsernames.value(convertFromNoSpace(channel));
            userlist.removeAll(convertFromNoSpace(user));
            _channelUsernames.insert(convertFromNoSpace(channel), userlist);
            if(_channelUsersModel.first == convertFromNoSpace(channel))
                _channelUsersModel.second.setStringList(_channelUsernames.value(convertFromNoSpace(channel)));
        }
        else if(messageParameters.at(0) == "MODE" && messageParameters.count() > 2)
        {
            if(convertFromNoSpace(messageParameters.at(1)) == _username)
            {
                QString mode = messageParameters.at(2);
                if(mode.contains('A'))
                    _isAdmin = true;
                else _isAdmin = false;
                if(mode.contains('S'))
                    _isStudent = true;
                else _isStudent = false;
                if(mode.contains('T'))
                    _isTeacher = true;
                else _isTeacher = false;
                ui->administrationMenu->setEnabled(_isAdmin);
            }
        }
        else if(messageParameters.count() > 2 && messageParameters.at(1) == "PRIVMSG")
        {
            QString sender = messageParameters.at(0);
            QString target = messageParameters.at(2);
            int pos = line.indexOf(' ', 0) + 1;
            pos = line.indexOf(' ', pos) + 1;
            pos = line.indexOf(' ', pos) + 1;
            QString message = line.mid(pos, -1);
            if(_textBoxWidgets.keys().contains(convertFromNoSpace(target)))
                _textBoxWidgets.value(target)->insertMessage(convertFromNoSpace(sender), message);
        }
        else if(messageParameters.at(0) == "PING")
        {
            _socket->write("PONG\r\n");
            _socket->flush();
        }
        else if(messageParameters.at(0) == "CHANNEL_DOES_NOT_EXIST")
            QMessageBox::warning(this, tr("warning.channelDoesNotExist"), tr("warning.channelDoesNotExist.text"));
        else if(messageParameters.at(0) == "USER_DOES_NOT_EXIST")
            QMessageBox::warning(this, tr("warning.userDoesNotExist"), tr("warning.userDoesNotExist.text"));
        else if(messageParameters.at(0) == "WRONG_ARGUMENTS")
            QMessageBox::warning(this, tr("warning.wrongParameters"), tr("warning.wrongParameters.text"));
        else if(messageParameters.at(0) == "USER_IS_NOT_LOGGED_IN" && messageParameters.count() > 1)
            QMessageBox::warning(this, tr("warning.userIsNotLoggedIn"), tr("warning.userIsNotLoggedIn.text:%1").arg(messageParameters.at(1)));
        else if(messageParameters.at(0) == "NOT_IN_CHANNEL" && messageParameters.count() > 1)
            QMessageBox::warning(this, tr("warning.notInChannel"), tr("warning.notInChannel.text:%1").arg(messageParameters.at(1)));
        else if(messageParameters.at(0) == "NOT_ADMINISTRATOR")
            QMessageBox::warning(this, tr("warning.notAdministrator"), tr("warning.notAdministrator.text"));
        else if(messageParameters.at(0) == "ALREADY_JOINED" && messageParameters.count() > 1)
            QMessageBox::warning(this, tr("warning.alreadyInChannel"), tr("warning.alreadyInChannel.text:%1").arg(messageParameters.at(1)));
        else if(messageParameters.at(0) == "CHANNEL_MODE_DOES_NOT_ALLOW_ACCESS")
            QMessageBox::warning(this, tr("warning.notAllowedInChannel"), tr("warning.notAllowedInChannel.text"));
        else if(messageParameters.at(0) == "QUERY_ERROR")
            QMessageBox::warning(this, tr("warning.queryError"), tr("warning.queryError.text"));
        else if(messageParameters.at(0) == "NOT_LOGGED_IN")
            handleSocketDisconnected();
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

void MainWindow::requestMode(const QString &target)
{
    QString sendMessage = "MODE ";
    sendMessage += target;
    sendMessage += "\r\n";
    _socket->write(sendMessage.toUtf8());
    _socket->flush();
}

void MainWindow::handleSocketError()
{
    if(_loginDialog == (LoginDialog*)0)
    {
        this->hide();
        if(_pmWindow != (PrivateMessageWindow*)0)
        {
            _pmWindow->deleteLater();
            _pmWindow = (PrivateMessageWindow*)0;
        }
        _config.setSplitterSizes(ui->splitter->sizes());
        clearChatBoxWidgets();
        _config.saveToFile();
        disconnectSocketSignals();
        if(_socket->isOpen())
            _socket->close();
        _loginDialog = new LoginDialog(_socket, &_config, this);
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
        if(_pmWindow != (PrivateMessageWindow*)0)
        {
            _pmWindow->deleteLater();
            _pmWindow = (PrivateMessageWindow*)0;
        }
        _config.setSplitterSizes(ui->splitter->sizes());
        clearChatBoxWidgets();
        _config.saveToFile();
        _loginDialog = new LoginDialog(_socket, &_config, this);
        connect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
        connect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
        _loginDialog->show();
    }
    return;
}

void MainWindow::handleSendQueryRequest()
{
    bool accepted;
    QString text = QInputDialog::getText(this, tr("dialog.getQuery.title"), tr("dialog.getQuery.content"), QLineEdit::Normal, "", &accepted);
    if(!accepted)
        return;
    if(text.isEmpty())
        return;
    _socket->write(QString("QUERY "+text+"\r\n").toUtf8());
    _socket->flush();
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

void MainWindow::handleChannelRefreshRequest()
{
    requestChannelListPopulation();
    return;
}

void MainWindow::handleChannelListChangeRequest()
{
    QString selectedChannel = ui->listViewChannels->currentIndex().data().toString();
    ui->stackedWidgetChat->setCurrentWidget(_textBoxWidgets.value(selectedChannel));
    _channelUsersModel.first = selectedChannel;
    _channelUsersModel.second.setStringList(_channelUsernames.value(selectedChannel));
    return;
}

void MainWindow::handleLogoutRequest()
{
    _config.setAutoLogin(false);
    _config.setLogin("","");
    _config.setHostParameters("localhost", 2000);
    _config.saveToFile();
    _socket->write("LOGOUT\r\n");
    _socket->flush();
    _socket->close();
    _username.clear();
    _channelUsernames.clear();
    _channelsModel.setStringList(QStringList());
    _channelUsersModel.first.clear();
    _channelUsersModel.second.setStringList(QStringList());
    if(_pmWindow != (PrivateMessageWindow*)0)
    {
        _pmWindow->deleteLater();
        _pmWindow = (PrivateMessageWindow*)0;
    }
    _config.setSplitterSizes(ui->splitter->sizes());
    clearChatBoxWidgets();
    _config.saveToFile();
    return;
}

void MainWindow::requestUsernamesForChannel(const QString &channel)
{
    _socket->write(QString("GET_USERLIST " + channel + "\r\n").toUtf8());
    _socket->flush();
    return;
}

void MainWindow::insertChatBoxWidget(const QString &target)
{
    ChatBoxWidget* widget = new ChatBoxWidget(_socket, target, _username, this);
    ui->stackedWidgetChat->addWidget(widget);
    _textBoxWidgets.insert(target, widget);
    return;
}

void MainWindow::removeChatBoxWidget(const QString &target)
{
    ChatBoxWidget* widget = _textBoxWidgets.value(target);
    ui->stackedWidgetChat->removeWidget(widget);
    widget->deleteLater();
    _textBoxWidgets.remove(target);
    return;
}

void MainWindow::clearChatBoxWidgets()
{
    for(unsigned i = 0; i < (unsigned)_textBoxWidgets.values().count(); i++)
        removeChatBoxWidget(_textBoxWidgets.values().at(i)->getTarget());
    return;
}

void MainWindow::handleShowPmRequest()
{
    if(_pmWindow == (PrivateMessageWindow*)0)
        return;
    _pmWindow->show();
}
