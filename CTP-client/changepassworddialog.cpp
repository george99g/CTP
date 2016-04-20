#include "changepassworddialog.hpp"
#include "ui_changepassworddialog.h"

ChangePasswordDialog::ChangePasswordDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ChangePasswordDialog)
{
    ui->setupUi(this);
    retranslateUi();
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    delete ui;
}

QString ChangePasswordDialog::password()
{
    return ui->lineEditPassword->text();
}

QString ChangePasswordDialog::username()
{
    return ui->lineEditUsername->text();
}

void ChangePasswordDialog::retranslateUi()
{
    ui->retranslateUi(this);
    return;
}
