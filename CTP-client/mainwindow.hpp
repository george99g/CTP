#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "logindialog.hpp"
#include <QStringListModel>
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
    void changeEvent(QEvent* event);
    void resizeEvent(QResizeEvent*);
    void handleSocketReadyRead();
    void connectSocketSignals();
    void disconnectSocketSignals();
    void requestChannelListPopulation();
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
    Ui::MainWindow* ui;
    bool _isAdmin;
    bool _isStudent;
    bool _isTeacher;
    QTcpSocket* _socket;
    LoginDialog* _loginDialog;
    QString _username;
    QStringListModel _channelsModel;
    QPair<QString, QStringListModel> _channelUsersModel;
    QMap<QString, QStringList> _channelUsernames;
    Configuration _config;
};

#endif // MAINWINDOW_HPP
