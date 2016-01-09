#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <QDialog>
#include "tcp/tcpserver.hpp"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
private slots:
    void tickBoxToggled(bool state);
private:
    Ui::Dialog *ui;
    TcpServer _server;
};

#endif // DIALOG_HPP
