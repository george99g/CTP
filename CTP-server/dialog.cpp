#include "dialog.hpp"
#include "ui_dialog.h"

Dialog::Dialog(QWidget* parent) : QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->checkBoxRun, &QCheckBox::toggled, this, &Dialog::tickBoxToggled);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::tickBoxToggled(bool state)
{
    if(state)
    {
        if(_server.listen(QHostAddress::Any, ui->spinBox->value()))
        {
            ui->checkBoxRun->setChecked(true);
        }
    }
    else
    {
        _server.close();
        ui->checkBoxRun->setChecked(false);
    }
    return;
}
