#ifndef CHANGEPASSWORDDIALOG_HPP
#define CHANGEPASSWORDDIALOG_HPP

#include <QDialog>

namespace Ui
{
    class ChangePasswordDialog;
}

class ChangePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePasswordDialog(QWidget *parent = 0);
    ~ChangePasswordDialog();
    QString password();
    QString username();
    void retranslateUi();
private:
    Ui::ChangePasswordDialog *ui;
};

#endif // CHANGEPASSWORDDIALOG_HPP
