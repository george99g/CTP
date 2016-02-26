#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "logindialog.hpp"
#include "configuration.hpp"
#include <QMainWindow>
#include <QInputDialog>

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
    void resizeEvent(QResizeEvent*);
    void handleSocketReadyRead();
    void connectSocketSignals();
    void disconnectSocketSignals();
    void requestChannelListPopulation();
    void handleSocketError();
    void handleSocketDisconnected();
    void handleJoinChannelRequest();
    void handlePartChannelRequest();
    void handleLogoutRequest();
    void requestUsernamesForChannel(const QString& channel);
    Ui::MainWindow* ui;
    QTcpSocket* _socket;
    LoginDialog* _loginDialog;
    QString _username;
    QMap<QString, QStringList> _channelUsernames;
    Configuration _config;
};

#endif // MAINWINDOW_HPP
