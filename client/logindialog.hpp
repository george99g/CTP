#ifndef LOGINDIALOG_HPP
#define LOGINDIALOG_HPP

#include "configuration.hpp"
#include <QDialog>
#include <QTcpSocket>
#include <QMessageBox>
#include <QHostAddress>
#include <QInputDialog>

namespace Ui
{
    class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
	explicit LoginDialog(QTcpSocket* socket, Configuration* config, const QStringList& supportedLanguages, QWidget *parent = 0);
    ~LoginDialog();
    QString getUsername();
    QString getHostname();
    static QString convertToNoSpace(QString string);
    static QString convertFromNoSpace(QString string);
    void retranslateUi();
signals:
    void loginCancelled();
    void loginAccepted();
	void languageChanged(QString language);
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
	void handleLanguageChangeRequest();
    QString _username;
    QString _hostname;
    Configuration* _config;
	QStringList _supportedLanguages;
};

#endif // LOGINDIALOG_HPP
