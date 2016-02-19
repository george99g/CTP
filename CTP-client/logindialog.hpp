#ifndef LOGINDIALOG_HPP
#define LOGINDIALOG_HPP

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
    explicit LoginDialog(QTcpSocket* socket, QWidget *parent = 0);
    ~LoginDialog();
    QString getUsername();
signals:
    void loginCancelled();
    void loginAccepted();
private:
    Ui::LoginDialog *ui;
    QTcpSocket* _socket;
    void handleLoginCancel();
    void handleSocketReadyRead();
    void handleLoginRequest();
    void handleSocketConnection();
    void handleValidAuthentication();
    void handleInvalidAuthentication();
    void handleSocketError();
    QString _username;
};

#endif // LOGINDIALOG_HPP
