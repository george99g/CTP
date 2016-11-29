#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _socket = new QTcpSocket(this);
    _ftpSocket = new QTcpSocket(this);
    _config.loadFromFile();
	_pmWindow = nullptr;
	_fileWidget = nullptr;
    _ftpUid = 0;
    _hostname = "localhost";
    _ftpPort = 0;
    connect(ui->actionPartChannel, &QAction::triggered, this, &MainWindow::handlePartChannelRequest);
    connect(ui->actionJoinChannel, &QAction::triggered, this, &MainWindow::handleJoinChannelRequest);
    connect(ui->actionRefreshChannels, &QAction::triggered, this, &MainWindow::handleChannelRefreshRequest);
    connect(ui->actionLogOut, &QAction::triggered, this, &MainWindow::handleLogoutRequest);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionSendQuery, &QAction::triggered, this, &MainWindow::handleSendQueryRequest);
    connect(ui->actionShowPrivateMessages, &QAction::triggered, this, &MainWindow::handleShowPmRequest);
    connect(ui->actionRegisterUser, &QAction::triggered, this, &MainWindow::handleRegisterUserRequest);
    connect(ui->listViewChannels, &QListView::clicked, this, &MainWindow::handleChannelListChangeRequest);
    connect(ui->listViewUsers, &QListView::doubleClicked, this, &MainWindow::handlePmUserOpenRequest);
    connect(ui->actionChangeUserMode, &QAction::triggered, this, &MainWindow::handleChangeUserModeRequest);
    connect(ui->actionChangeChannelMode, &QAction::triggered, this, &MainWindow::handleChangeChannelModeRequest);
    connect(ui->actionChangePassword, &QAction::triggered, this, &MainWindow::handleChangeUserPasswordRequest);
    _channelsModel.setStringList(_channelUsernames.keys());
    ui->listViewChannels->setModel(&_channelsModel);
    ui->listViewUsers->setModel(&_channelUsersModel.second);
    _isAdmin = false;
    _isStudent = false;
    _isTeacher = false;
    _populatedUserlist = false;
    _ftpConnected = false;
    _readQueue = QStringList();
    _currentLanguage = "";
	_uploadingFile = "";
	configureLanguages();
	_loginDialog = new LoginDialog(_socket, &_config, _installedLanguages);
	connect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
	connect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
	connect(_loginDialog, &LoginDialog::languageChanged, this, &MainWindow::handleLoginDialogLanguageChanged);
	_loginDialog->show();
}

MainWindow::~MainWindow()
{
    clearEverything();
	if(_loginDialog)
    {
        _loginDialog->deleteLater();
		_loginDialog = nullptr;
    }
	if(_pmWindow)
    {
        _config.setPmSplitterSizes(_pmWindow->splitterSizes());
        _config.saveToFile();
        _pmWindow->deleteLater();
		_pmWindow = nullptr;
    }
	if(_loginDialog)
    {
        _loginDialog->deleteLater();
		_loginDialog = nullptr;
    }
	if(_socket)
    {
        if(_socket->isOpen())
        {
            disconnectSocketSignals();
            _socket->close();
        }
        _socket->deleteLater();
		_socket = nullptr;
    }
	if(_ftpSocket)
    {
        if(_ftpSocket->isOpen())
        {
            disconnectSocketSignals();
            _ftpSocket->close();
        }
        _ftpSocket->deleteLater();
		_socket = nullptr;
    }
    _config.saveToFile();
    delete ui;
}

void MainWindow::loginCancelled()
{
    close();
    exit(0);
    return;
}

void MainWindow::loginAccepted()
{
    _loginDialog->hide();
    _hostname = _loginDialog->getHostname();
    _username = _loginDialog->getUsername();
	disconnect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
	disconnect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
	disconnect(_loginDialog, &LoginDialog::languageChanged, this, &MainWindow::handleLoginDialogLanguageChanged);
    ui->splitter->setSizes(_config.splitterSizes());
    if(!_config.maximized())
    {
        resize(_config.mainWindowX(), _config.mainWindowY());
        show();
    }
    else
        showMaximized();
    _loginDialog->deleteLater();
	_loginDialog = nullptr;
    connectSocketSignals();
    _pmWindow = new PrivateMessageWindow(&_config);
    connect(_pmWindow, &PrivateMessageWindow::switchUser, this, &MainWindow::handleUserChangeRequest);
    _pmWindow->listView()->setModel(&_usernamesModel);
    _fileWidget = new FileWidget(this);
    connect(_fileWidget, &FileWidget::requestRefresh, this, &MainWindow::requestFileList);
    connect(_fileWidget, &FileWidget::downloadFile, this, &MainWindow::handleFtpDownloadFileRequest);
    connect(_fileWidget, &FileWidget::sendFile, this, &MainWindow::handleFtpUploadRequest);
    connect(_fileWidget, &FileWidget::deleteFile, this, &MainWindow::handleFtpDeleteRequest);
    ui->filesTab->layout()->addWidget(_fileWidget);
    ui->tabWidget->setCurrentIndex(0);
    requestChannelListPopulation();
    requestUserlistPopulation();
    requestMode(_username);
    requestFileList();
    requestAllChannels();
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

Configuration* MainWindow::config()
{
    return &_config;
}

void MainWindow::languageChanged(QAction* action)
{
	if(action != nullptr)
        loadLanguage(action->data().toString());
	return;
}

void MainWindow::handleLoginDialogLanguageChanged(QString language)
{
	loadLanguage(language);
}

void MainWindow::configureLanguages()
{
    QActionGroup* languageGroup = new QActionGroup(ui->languageMenu);
    languageGroup->setExclusive(true);
    connect(languageGroup, &QActionGroup::triggered, this, &MainWindow::languageChanged);
    QString langPath = QApplication::applicationDirPath();
    langPath.append("/languages");
    QDir dir(langPath);
    QStringList filenames = dir.entryList(QStringList("CTP_*.qm"));
    if(filenames.count() <= 0)
        QMessageBox::critical(this, "Critical error", "No translation files found.\nPlease add translation files to /languages in order to properly use this application.");
    qDebug()<<_config.language();
	for(int i = 0; i < filenames.count(); i++)
    {
		QString locale = filenames.at(i);
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf('_') + 1);
        _installedLanguages.push_back(locale);
        QString lang = QLocale::languageToString(QLocale(locale).language());
        QIcon icon(QString("%1/%2.png").arg(langPath).arg(locale));
        QAction* action = new QAction(icon, lang, this);
        action->setCheckable(true);
        action->setData(locale);
        ui->languageMenu->addAction(action);
        languageGroup->addAction(action);
        if(_config.language() == locale)
            action->setChecked(true);

    }
    if(!loadLanguage(_config.language()))
        if(!loadLanguage("en"))
            if(_installedLanguages.count() > 0)
                loadLanguage(_installedLanguages.at(0));
    return;
}

bool MainWindow::loadLanguage(const QString &language)
{
    if(_currentLanguage == language)
        return true;
    _currentLanguage = language;
    _config.setLanguage(language);
    _config.saveToFile();
    QLocale locale = QLocale(_currentLanguage);
    QLocale::setDefault(locale);
    if(!switchTranslator(_translator, QString("CTP_%1.qm").arg(_currentLanguage)))
        return false;
    switchTranslator(_translatorQt, QString("qt_%1.qm").arg(_currentLanguage));
    switchTranslator(_translatorBaseQt, QString("qtbase_%1.qm").arg(_currentLanguage));
    return true;
}

bool MainWindow::switchTranslator(QTranslator &translator, const QString &filename)
{
    QString langPath = QApplication::applicationDirPath();
    langPath.append("/languages/");
    langPath.append(filename);
    qApp->removeTranslator(&translator);
    if(translator.load(langPath))
        qApp->installTranslator(&translator);
    else
    {
        qDebug()<<"Failed to load translator for "<<langPath;
        return false;
    }
    return true;
}

void MainWindow::clearEverything()
{
    clearChatBoxWidgets();
    clearPmChatBoxWidgets();
    _username.clear();
    _channelUsernames.clear();
    _channelsModel.setStringList(QStringList());
    _channelUsersModel.first.clear();
    _channelUsersModel.second.setStringList(QStringList());
    _usernames.clear();
    _usernamesModel.setStringList(QStringList());
    if(_pmWindow != (PrivateMessageWindow*)0)
    {
        _config.setPmSplitterSizes(_pmWindow->splitterSizes());
        disconnect(_pmWindow, &PrivateMessageWindow::switchUser, this, &MainWindow::handleUserChangeRequest);
        _pmWindow->deleteLater();
        _pmWindow = (PrivateMessageWindow*)0;
    }
    if(_fileWidget != (FileWidget*)0)
    {
        disconnect(_fileWidget, &FileWidget::requestRefresh, this, &MainWindow::requestFileList);
        disconnect(_fileWidget, &FileWidget::downloadFile, this, &MainWindow::handleFtpDownloadFileRequest);
        disconnect(_fileWidget, &FileWidget::sendFile, this, &MainWindow::handleFtpUploadRequest);
        disconnect(_fileWidget, &FileWidget::deleteFile, this, &MainWindow::handleFtpDeleteRequest);
        _fileWidget->deleteLater();
        _fileWidget = (FileWidget*)0;
    }
    _config.setSplitterSizes(ui->splitter->sizes());
    _config.saveToFile();
    return;
}

void MainWindow::changeEvent(QEvent* event)
{
	if(!event)
        return;
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        if(_pmWindow != (PrivateMessageWindow*)0)
            _pmWindow->retranslateUi();
        if(_loginDialog != (LoginDialog*)0)
            _loginDialog->retranslateUi();
        if(_fileWidget != (FileWidget*)0)
            _fileWidget->retranslateUi();
    }
    else if(event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent* stateChangeEvent = static_cast<QWindowStateChangeEvent*>(event);
		if(stateChangeEvent->oldState() == Qt::WindowNoState && windowState() == Qt::WindowMaximized )
            _config.setMaximized(true);
		else if(stateChangeEvent->oldState() == Qt::WindowMaximized && windowState() == Qt::WindowNoState)
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
    while(_socket->canReadLine() || (_readQueue.count() > 0 && _populatedUserlist))
    {
        QString line = "";
        if(_socket->canReadLine())
            line = QString(_socket->readLine()).trimmed();
        else if(_readQueue.count() > 0 && _populatedUserlist)
        {
            line = _readQueue.at(0);
            _readQueue.removeAt(0);
        }
        if(line == "")
            return;
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
        else if(messageParameters.at(0) == "CHANNELLIST" && messageParameters.count() > 1)
        {
            QStringList channellist;
            for(unsigned i = 1; i < (unsigned)messageParameters.count(); i++)
                channellist.push_back(convertFromNoSpace(messageParameters.at(i)));
            _allChannels = channellist;
            _allChannelsModel.setStringList(_allChannels);
        }
        else if(messageParameters.at(0) == "OFFLINE_USERLIST" && messageParameters.count() > 1)
        {
            QStringList userlist;
            for(unsigned i = 1; i < (unsigned)messageParameters.count(); i++)
            {
                userlist.push_back(convertFromNoSpace(messageParameters.at(i)));
                if(!_usernames.contains(convertFromNoSpace(messageParameters.at(i))))
                    insertPmChatBoxWidget(convertFromNoSpace(messageParameters.at(i)));
            }
			if(!(userlist.count() > _username.count()))
                for(unsigned i = 0; i < (unsigned)_usernames.count(); i++)
                    if(!userlist.contains(_usernames.at(i)))
                        removePmChatBoxWidget(_usernames.at(i));
            _usernames = userlist;
            _usernamesModel.setStringList(_usernames);
            _populatedUserlist = true;
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
            ChatBoxWidget* chatBox = _textBoxWidgets.value(channel);
            if(chatBox != (ChatBoxWidget*)0)
                chatBox->insertSystemMessage(tr("message.join"), convertFromNoSpace(user));
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
            ChatBoxWidget* chatBox = _textBoxWidgets.value(channel);
            if(chatBox != (ChatBoxWidget*)0)
                chatBox->insertSystemMessage(tr("message.part"), convertFromNoSpace(user));
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
                _textBoxWidgets.value(convertFromNoSpace(target))->insertMessage(convertFromNoSpace(sender), message);
            else if(_pmTextBoxWidgets.keys().contains(convertFromNoSpace(sender)) && convertFromNoSpace(target) == _username)
            {
                _pmTextBoxWidgets.value(convertFromNoSpace(sender))->insertMessage(convertFromNoSpace(sender), message);
                if(_pmWindow->isHidden())
                {
                    if(_config.pmMaximized())
                        _pmWindow->showMaximized();
                    else
                    {
                        _pmWindow->resize(_config.pmWindowX(), _config.pmWindowY());
                        _pmWindow->show();
                    }
                }
            }
        }
        else if(messageParameters.count() > 3 && messageParameters.at(0) == "OFFLINE_MESSAGE")
        {
            QString sender = messageParameters.at(1);
            QDateTime time = QDateTime::fromTime_t(QString(messageParameters.at(2)).toUInt());
            if(!_pmTextBoxWidgets.keys().contains(convertFromNoSpace(sender)))
            {
                _readQueue.push_back(line);
                handleSocketReadyRead();
                continue;
            }
            int pos = line.indexOf(' ', 0) + 1;
            pos = line.indexOf(' ', pos) + 1;
            pos = line.indexOf(' ', pos) + 1;
            QString message = line.mid(pos, -1);
            _pmTextBoxWidgets.value(convertFromNoSpace(sender))->insertMessage(convertFromNoSpace(sender), message, time);
            if(_pmWindow->isHidden())
            {
                if(_config.pmMaximized())
                    _pmWindow->showMaximized();
                else
                {
                    _pmWindow->resize(_config.pmWindowX(), _config.pmWindowY());
                    _pmWindow->show();
                }
                _pmWindow->setUser(convertFromNoSpace(sender));
            }
        }
        else if(messageParameters.count() > 2 && messageParameters.at(0) == "FTP_PORT")
        {
            qint64 port = messageParameters.at(1).toLongLong();
            qint32 id = messageParameters.at(2).toInt();
            _ftpPort = port;
            _ftpUid = id;
            if(_ftpSocket == (QTcpSocket*)0)
            {
                _ftpSocket = new QTcpSocket(this);
                connect(_ftpSocket, &QTcpSocket::readyRead, this, &MainWindow::handleFtpSocketReadyRead);
            }
            if(!_ftpSocket->isOpen())
            {
                _ftpSocket->connectToHost(_hostname, port);
                _ftpSocket->waitForConnected(2000);
                QByteArray data;
                QDataStream dataStream(&data, QIODevice::ReadWrite);
                dataStream << (qint64)0;
                dataStream << id;
                dataStream << QString("\r\n");
                dataStream.device()->close();
                _ftpSocket->write(data);
                _ftpSocket->flush();
            }
        }
        else if(messageParameters.at(0) == "FILE_LIST")
        {
            QStringList files;
            for(unsigned i = 1; i < (unsigned)messageParameters.count(); i++)
                files << convertFromNoSpace(messageParameters.at(i));
            _fileList.setStringList(files);
            _fileWidget->listView()->setModel(&_fileList);
        }
        else if(messageParameters.at(0) == "FTP_DELETED_FILE" && messageParameters.count() == 2)
        {
            QString file = messageParameters.at(1);
            file = convertFromNoSpace(file);
            QStringList files = _fileList.stringList();
            files.removeAll(file);
            _fileList.setStringList(files);
            _fileWidget->listView()->setModel(&_fileList);
        }
        else if(messageParameters.at(0) == "UPLOAD_FILE_ACK")
        {
            QFile fileObj(_uploadingFile);
            if(!fileObj.open(QFile::ReadOnly))
            {
                _socket->write("STOP_UPLOAD_FILE\r\n");
                _socket->flush();
                _socket->waitForBytesWritten();
                fileObj.close();
                requestFileList();
                return;
            }
            while(fileObj.bytesAvailable() > 0)
            {
                QByteArray data;
                QDataStream ds(&data, QIODevice::ReadWrite);
                ds << (qint64)1;
                if(fileObj.bytesAvailable() > 1024)
                    ds << fileObj.read(1024).toBase64().append("\r\n");
                else
                    ds << fileObj.readAll().toBase64().append("\r\n");
                _ftpSocket->write(data);
                _ftpSocket->flush();
                _ftpSocket->waitForBytesWritten();
            }
            QByteArray data;
            QDataStream dataStream(&data, QIODevice::ReadWrite);
            dataStream << (qint64)2;
            dataStream << QString("\r\n");
            dataStream.device()->close();
            _ftpSocket->write(data);
            _ftpSocket->flush();
            _ftpSocket->waitForBytesWritten();
            fileObj.close();
        }
        else if(messageParameters.at(0) == "UPLOAD_FILE_RECV")
        {
            _socket->write("STOP_UPLOAD_FILE\r\n");
            _socket->flush();
            _socket->waitForBytesWritten();
            requestFileList();
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
        else if(messageParameters.at(0) == "USERS_CANNOT_CHANGE_CHANNELS")
            QMessageBox::warning(this, tr("warning.usersCannotChangeChannels"), tr("warning.usersCannotChangeChannels.text"));
        else if(messageParameters.at(0) == "FTP_OPEN_FILE_ERROR")
            QMessageBox::warning(this, tr("warning.ftp.fileError"), tr("warning.ftp.fileError.text"));
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
    if(_ftpSocket != (QTcpSocket*)0)
    {
        connect(_ftpSocket, &QTcpSocket::readyRead, this, &MainWindow::handleFtpSocketReadyRead);
        connect(_ftpSocket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &MainWindow::handleFtpSocketError);
        connect(_ftpSocket, &QTcpSocket::disconnected, this, &MainWindow::handleFtpSocketDisconnected);
    }
    return;
}

void MainWindow::disconnectSocketSignals()
{
    disconnect(_socket, &QTcpSocket::readyRead, this, &MainWindow::handleSocketReadyRead);
    disconnect(_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &MainWindow::handleSocketError);
    disconnect(_socket, &QTcpSocket::disconnected, this, &MainWindow::handleSocketDisconnected);
    if(_ftpSocket != (QTcpSocket*)0)
    {
        disconnect(_ftpSocket, &QTcpSocket::readyRead, this, &MainWindow::handleFtpSocketReadyRead);
        disconnect(_ftpSocket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &MainWindow::handleFtpSocketError);
        disconnect(_ftpSocket, &QTcpSocket::disconnected, this, &MainWindow::handleFtpSocketDisconnected);
    }
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

void MainWindow::requestUserlistPopulation()
{
    if(!_socket->isOpen())
        return;
    _socket->write("GET_OFFLINE_USERLIST\r\n");
    _socket->flush();
}

void MainWindow::requestMode(const QString &target)
{
    QString sendMessage = "MODE ";
    sendMessage += target;
    sendMessage += "\r\n";
    _socket->write(sendMessage.toUtf8());
    _socket->flush();
}

void MainWindow::requestAllChannels()
{
    _socket->write("GET_CHANNELLIST\r\n");
    _socket->flush();
    return;
}

void MainWindow::handleSocketError()
{
	if(!_loginDialog)
    {
        this->hide();
        disconnectSocketSignals();
        clearEverything();
        if(_socket->isOpen())
            _socket->close();
        _config.setAutoLogin(false);
        _config.saveToFile();
		_loginDialog = new LoginDialog(_socket, &_config, _installedLanguages, this);
        connect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
        connect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
		connect(_loginDialog, &LoginDialog::languageChanged, this, &MainWindow::handleLoginDialogLanguageChanged);
        _loginDialog->show();
    }
    return;
}

void MainWindow::handleSocketDisconnected()
{
	if(!_loginDialog)
    {
        this->hide();
        disconnectSocketSignals();
        clearEverything();
        if(_socket->isOpen())
            _socket->close();
        _config.setAutoLogin(false);
        _config.saveToFile();
		_loginDialog = new LoginDialog(_socket, &_config, _installedLanguages, this);
        connect(_loginDialog, &LoginDialog::loginCancelled, this, &MainWindow::loginCancelled);
        connect(_loginDialog, &LoginDialog::loginAccepted, this, &MainWindow::loginAccepted);
		connect(_loginDialog, &LoginDialog::languageChanged, this, &MainWindow::handleLoginDialogLanguageChanged);
        _loginDialog->show();
    }
    return;
}

void MainWindow::handleFtpSocketDisconnected()
{
    connect(_ftpSocket, &QTcpSocket::connected, this, &MainWindow::handleFtpConnected);
    disconnect(_ftpSocket, &QTcpSocket::disconnected, this, &MainWindow::handleFtpSocketDisconnected);
    _ftpSocket->connectToHost(_hostname, _ftpPort);
    return;
}

void MainWindow::handleFtpSocketError()
{
    handleFtpSocketDisconnected();
    return;
}

void MainWindow::handleFtpSocketReadyRead()
{
    QFile file(_ftpSavingFile);
    if(!file.open(QFile::WriteOnly|QFile::Append))
        return;
    while(!_ftpSocket->atEnd())
        file.write(_ftpSocket->read(2048*8));
    file.close();
    return;
}

void MainWindow::handleFtpDownloadFileRequest(QString file)
{
    QString saveFile = QFileDialog::getSaveFileName(this, tr("saveFileDialog.saveFile"),  QDir::homePath()+'/'+file);
    if(saveFile == "")
        return;
    QFile saveFileObj(saveFile);
    if(!saveFileObj.open(QFile::WriteOnly|QFile::Truncate))
        return;
    _ftpSavingFile = saveFile;
    saveFileObj.close();
    if(_ftpSocket == (QTcpSocket*)0)
    {
        _ftpSocket = new QTcpSocket(this);
        connect(_ftpSocket, &QTcpSocket::readyRead, this, &MainWindow::handleFtpSocketReadyRead);
    }
    if(!_ftpSocket->isOpen())
    {
        _socket->write("GET_FTP_PORT\r\n");
        _socket->flush();
        return;
    }
    _socket->write(QString("DOWNLOAD_FILE "+convertToNoSpace(file)+"\r\n").toUtf8());
    _socket->flush();
    return;
}

void MainWindow::handleFtpDeleteRequest(QString file)
{
    QString message = "DELETE_FILE ";
    message += convertToNoSpace(file);
    message += "\r\n";
    _socket->write(message.toUtf8());
    _socket->flush();
    return;
}

void MainWindow::requestFileList()
{
    _socket->write("GET_FILE_LIST\r\n");
    _socket->flush();
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

void MainWindow::handleRegisterUserRequest()
{
    QString username = "";
    QString password = "";
    RegistrationDialog regDialog(this);
    regDialog.retranslateUi();
    regDialog.setModal(true);
    regDialog.exec();
    if(regDialog.result() != QDialog::Accepted)
        return;
    username = regDialog.username();
    if(username == "")
        return;
    password = regDialog.password();
    if(password == "")
        return;
    QString sendMessage = "ADD_USER ";
    sendMessage += convertToNoSpace(username);
    sendMessage += ' ';
    sendMessage += convertToNoSpace(password);
    sendMessage += "\r\n";
    _socket->write(sendMessage.toUtf8());
    _socket->flush();
    return;
}

void MainWindow::handleChangeUserPasswordRequest()
{
    QString username = "";
    QString password = "";
    ChangePasswordDialog chpassDialog(this);
    chpassDialog.setModal(true);
    chpassDialog.exec();
    if(chpassDialog.result() != QDialog::Accepted)
        return;
    username = chpassDialog.username();
    if(username == "")
        return;
    password = chpassDialog.password();
    if(password == "")
        return;
    QString sendMessage = "CHPASS ";
    sendMessage += convertToNoSpace(username);
    sendMessage += ' ';
    sendMessage += convertToNoSpace(password);
    sendMessage += "\r\n";
    _socket->write(sendMessage.toUtf8());
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
    QString channel = "";
    PartChannelDialog dialog(this);
    dialog.retranslateUi();
    _channelnamesModel.setStringList(_channelUsernames.keys());
    dialog.listView()->setModel(&_channelnamesModel);
    dialog.setModal(true);
    dialog.exec();
    if(dialog.result() != QDialog::Accepted)
        return;
    channel = dialog.listView()->currentIndex().data().toString();
    if(channel == "")
        return;
    channel = convertToNoSpace(channel);
    _socket->write(QString("PART "+channel+"\r\n").toUtf8());
    _socket->flush();
    return;
}

void MainWindow::handleChannelRefreshRequest()
{
    requestChannelListPopulation();
    requestUserlistPopulation();
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

void MainWindow::handlePmUserOpenRequest()
{
    QString selectedUser = ui->listViewUsers->currentIndex().data().toString();
    if(_pmWindow->isHidden())
    {
        if(_config.pmMaximized())
            _pmWindow->showMaximized();
        else
        {
            _pmWindow->resize(_config.pmWindowX(), _config.pmWindowY());
            _pmWindow->show();
        }
    }
    _pmWindow->setUser(selectedUser);
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
    handleSocketDisconnected();
    return;
}

void MainWindow::handleChangeUserModeRequest()
{
    UserModeDialog dialog;
    dialog.listView()->setModel(&_usernamesModel);
    dialog.setModal(true);
    dialog.exec();
    if(dialog.result() != QDialog::Accepted)
        return;
    bool teacher = dialog.teacher(), student = dialog.student(), administrator = dialog.administrator();
    QString username = dialog.username();
    if(username == "")
        return;
    username = convertFromNoSpace(username);
    if(teacher)
    {
        _socket->write(QString("MODE "+username+" +T\r\n").toUtf8());
        _socket->flush();
    }
    else
    {
        _socket->write(QString("MODE "+username+" -T\r\n").toUtf8());
        _socket->flush();
    }
    if(student)
    {
        _socket->write(QString("MODE "+username+" +S\r\n").toUtf8());
        _socket->flush();
    }
    else
    {
        _socket->write(QString("MODE "+username+" -S\r\n").toUtf8());
        _socket->flush();
    }
    if(administrator)
    {
        _socket->write(QString("MODE "+username+" +A\r\n").toUtf8());
        _socket->flush();
    }
    else
    {
        _socket->write(QString("MODE "+username+" -A\r\n").toUtf8());
        _socket->flush();
    }
    return;
}

void MainWindow::handleChangeChannelModeRequest()
{
    ChannelModeDialog dialog;
    dialog.listView()->setModel(&_allChannelsModel);
    dialog.setModal(true);
    dialog.exec();
    if(dialog.result() != QDialog::Accepted)
        return;
    bool teacher = dialog.teacher(), student = dialog.student(), administrator = dialog.administrator();
    QString channelname = dialog.channelname();
    if(channelname == "")
        return;
    channelname = convertFromNoSpace(channelname);
    if(teacher)
    {
        _socket->write(QString("MODE "+channelname+" +T\r\n").toUtf8());
        _socket->flush();
    }
    else
    {
        _socket->write(QString("MODE "+channelname+" -T\r\n").toUtf8());
        _socket->flush();
    }
    if(student)
    {
        _socket->write(QString("MODE "+channelname+" +S\r\n").toUtf8());
        _socket->flush();
    }
    else
    {
        _socket->write(QString("MODE "+channelname+" -S\r\n").toUtf8());
        _socket->flush();
    }
    if(administrator)
    {
        _socket->write(QString("MODE "+channelname+" +A\r\n").toUtf8());
        _socket->flush();
    }
    else
    {
        _socket->write(QString("MODE "+channelname+" -A\r\n").toUtf8());
        _socket->flush();
    }
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

void MainWindow::insertPmChatBoxWidget(const QString &target)
{
    ChatBoxWidget* widget = new ChatBoxWidget(_socket, target, _username, this);
    _pmWindow->stackedWidget()->addWidget(widget);
    _pmTextBoxWidgets.insert(target, widget);
    return;
}

void MainWindow::removePmChatBoxWidget(const QString &target)
{
    ChatBoxWidget* widget = _pmTextBoxWidgets.value(target);
    if(_pmWindow != (PrivateMessageWindow*)0)
        _pmWindow->stackedWidget()->removeWidget(widget);
    widget->deleteLater();
    _pmTextBoxWidgets.remove(target);
    return;
}

void MainWindow::clearPmChatBoxWidgets()
{
    for(unsigned i = 0; i < (unsigned)_pmTextBoxWidgets.values().count(); i++)
        removePmChatBoxWidget(_pmTextBoxWidgets.values().at(i)->getTarget());
    return;
}

void MainWindow::handleShowPmRequest()
{
    if(_pmWindow == (PrivateMessageWindow*)0)
        return;
    if(_config.pmMaximized())
        _pmWindow->showMaximized();
    else
    {
        _pmWindow->resize(_config.pmWindowX(), _config.pmWindowY());
        _pmWindow->show();
    }
}

void MainWindow::handleUserChangeRequest(QString newUser)
{
    _pmWindow->stackedWidget()->setCurrentWidget(_pmTextBoxWidgets.value(newUser));
    _pmWindow->listView()->setCurrentIndex(_pmWindow->listView()->model()->index(_usernames.indexOf(newUser), 0));
    return;
}

void MainWindow::handleFtpConnected()
{
    disconnect(_ftpSocket, &QTcpSocket::connected, this, &MainWindow::handleFtpConnected);
    connect(_ftpSocket, &QTcpSocket::disconnected, this, &MainWindow::handleFtpSocketDisconnected);
    _ftpConnected = true;
    return;
}

void MainWindow::handleFtpUploadRequest(QString file)
{
    QFile fileObj(file);
    if(!fileObj.open(QFile::ReadOnly))
        return;
    QString savingFile = convertToNoSpace(file.mid(file.lastIndexOf('/') + 1, -1));
    if(_ftpSocket == (QTcpSocket*)0)
    {
        _ftpSocket = new QTcpSocket(this);
        connect(_ftpSocket, &QTcpSocket::readyRead, this, &MainWindow::handleFtpSocketReadyRead);
    }
    if(!_ftpSocket->isOpen())
    {
        _socket->write("GET_FTP_PORT\r\n");
        _socket->flush();
        return;
    }
    _socket->write(QString("UPLOAD_FILE "+savingFile+"\r\n").toUtf8());
    _socket->flush();
    _socket->waitForBytesWritten();
    _uploadingFile = file;
    fileObj.close();
    return;
}
