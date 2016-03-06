#ifndef LOGINDIALOG_HPP
#define LOGINDIALOG_HPP

#include "configuration.hpp"
#include <QDialog>
#include <QTcpSocket>
#include <QMessageBox>

namespace Ui
{
    class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QTcpSocket* socket, Configuration* config, QWidget *parent = 0);
    ~LoginDialog();
    QString getUsername();
    static QString convertToNoSpace(QString string);
    static QString convertFromNoSpace(QString string);
    void retranslateUi();
signals:
    void loginCancelled();
    void loginAccepted();
private:
    Ui::LoginDialog *ui;
    QTcpSocket* _socket;
    void handleAutoLogin();
    void handleLoginCancel();
    void handleSocketReadyRead();
    void handleLoginRequest();
    void handleSocketConnection();
    void handleValidAuthentication();
    void handleInvalidAuthentication();
    void handleSocketError();
    void handleSocketStateChanged(QAbstractSocket::SocketState socketState);
    QString _username;
    Configuration* _config;
};

#endif // LOGINDIALOG_HPP
