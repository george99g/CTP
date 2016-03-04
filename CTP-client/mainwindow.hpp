#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "logindialog.hpp"
#include "privatemessagewindow.hpp"
#include "chatboxwidget.hpp"
#include <QMainWindow>
#include <QInputDialog>
#include <QWindowStateChangeEvent>

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
private:
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
    void handleJoinChannelRequest();
    void handlePartChannelRequest();
    void handleChannelRefreshRequest();
    void handleChannelListChangeRequest();
    void handleLogoutRequest();
    void requestUsernamesForChannel(const QString& channel);
    void insertChatBoxWidget(const QString &target);
    void removeChatBoxWidget(const QString &target);
    void clearChatBoxWidgets();
    void insertPmChatBoxWidget(const QString &target);
    void removePmChatBoxWidget(const QString &target);
    void clearPmChatBoxWidgets();
    void handleShowPmRequest();
    Ui::MainWindow* ui;
    bool _isAdmin;
    bool _isStudent;
    bool _isTeacher;
    QTcpSocket* _socket;
    LoginDialog* _loginDialog;
    PrivateMessageWindow* _pmWindow;
    QString _username;
    QStringListModel _channelsModel;
    QPair<QString, QStringListModel> _channelUsersModel;
    QStringList _usernames;
    QMap<QString, ChatBoxWidget*> _pmTextBoxWidgets;
    QStringListModel _usernamesModel;
    QMap<QString, QStringList> _channelUsernames;
    QMap<QString, ChatBoxWidget*> _textBoxWidgets;
    Configuration _config;
};

#endif // MAINWINDOW_HPP
