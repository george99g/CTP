#ifndef REGISTRATIONDIALOG_HPP
#define REGISTRATIONDIALOG_HPP

#include <QDialog>

namespace Ui
{
    class RegistrationDialog;
}

class RegistrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegistrationDialog(QWidget* parent = 0);
    QString username();
    QString password();
    ~RegistrationDialog();
    void retranslateUi();
private:
    Ui::RegistrationDialog* ui;
};

#endif // REGISTRATIONDIALOG_HPP
