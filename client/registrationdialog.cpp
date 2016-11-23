#include "registrationdialog.hpp"
#include "ui_registrationdialog.h"

RegistrationDialog::RegistrationDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RegistrationDialog)
{
    ui->setupUi(this);
    retranslateUi();
}

QString RegistrationDialog::username()
{
    return ui->lineEditUsername->text();
}

QString RegistrationDialog::password()
{
    return ui->lineEditPassword->text();
}

RegistrationDialog::~RegistrationDialog()
{
    delete ui;
}

void RegistrationDialog::retranslateUi()
{
    ui->retranslateUi(this);
    return;
}
