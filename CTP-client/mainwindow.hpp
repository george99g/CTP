#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "logindialog.hpp"
#include <QMainWindow>

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
private:
    Ui::MainWindow* ui;
    QTcpSocket* _socket;
    LoginDialog* loginDialog;
    QString _username;
};

#endif // MAINWINDOW_HPP
