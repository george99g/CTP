#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <QDialog>
#include <QMessageBox>
#include "config.hpp"
#include "irc/ircserver.hpp"
#include "ftp/ftpserver.hpp"

namespace Ui
{
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget* parent = 0);
    ~Dialog();
private slots:
    void tickBoxToggled(bool state);
private:
    void connectServers();
    void disconnectServers();
	void addAdmin();
    Ui::Dialog* ui;
    IrcServer _server;
    FtpServer _ftpServer;
};

#endif // DIALOG_HPP
