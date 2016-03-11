#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "logindialog.hpp"
#include "privatemessagewindow.hpp"
#include "chatboxwidget.hpp"
#include "registrationdialog.hpp"
#include "partchanneldialog.hpp"
#include "filewidget.hpp"
#include <QMainWindow>
#include <QInputDialog>
#include <QDir>
#include <QActionGroup>
#include <QTranslator>
#include <QString>
#include <QStringList>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
public slots:
    void loginCancelled();
    void loginAccepted();
    static QString convertToNoSpace(QString string);
    static QString convertFromNoSpace(QString string);
    Configuration* config();
private:
    void languageChanged(QAction* action);
    void configureLanguages();
    bool loadLanguage(const QString &language);
    bool switchTranslator(QTranslator &translator, const QString &filename);
    void clearEverything();
    void changeEvent(QEvent* event);
    void resizeEvent(QResizeEvent*);
    void handleSocketReadyRead();
    void connectSocketSignals();
    void disconnectSocketSignals();
    void requestChannelListPopulation();
    void requestUserlistPopulation();
    void requestMode(const QString &target);
    void handleSocketError();
    void handleSocketDisconnected();
    void handleSendQueryRequest();
    void handleRegisterUserRequest();
    void handleJoinChannelRequest();
    void handlePartChannelRequest();
    void handleChannelRefreshRequest();
    void handleChannelListChangeRequest();
    void handlePmUserOpenRequest();
    void handleLogoutRequest();
    void requestUsernamesForChannel(const QString& channel);
    void insertChatBoxWidget(const QString &target);
    void removeChatBoxWidget(const QString &target);
    void clearChatBoxWidgets();
    void insertPmChatBoxWidget(const QString &target);
    void removePmChatBoxWidget(const QString &target);
    void clearPmChatBoxWidgets();
    void handleShowPmRequest();
    void handleUserChangeRequest(QString newUser);
    void handleFtpConnected();
    Ui::MainWindow* ui;
    bool _isAdmin;
    bool _isStudent;
    bool _isTeacher;
    bool _populatedUserlist;
    bool _ftpConnected;
    QTcpSocket* _socket;
    QTcpSocket* _ftpSocket;
    qint32 _ftpUid;
    QStringList _readQueue;
    LoginDialog* _loginDialog;
    FileWidget* _fileWidget;
    PrivateMessageWindow* _pmWindow;
    QString _username;
    QStringListModel _channelsModel;
    QPair<QString, QStringListModel> _channelUsersModel;
    QStringList _usernames;
    QMap<QString, ChatBoxWidget*> _pmTextBoxWidgets;
    QStringListModel _usernamesModel;
    QMap<QString, QStringList> _channelUsernames;
    QMap<QString, ChatBoxWidget*> _textBoxWidgets;
    QStringListModel _channelnamesModel;
    QString _hostname;
    QString _currentLanguage;
    QStringList _installedLanguages;
    Configuration _config;
    QTranslator _translator;
    QTranslator _translatorQt;
    QTranslator _translatorBaseQt;
};

#endif // MAINWINDOW_HPP
