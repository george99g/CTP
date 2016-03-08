#include "partchanneldialog.hpp"
#include "ui_partchanneldialog.h"

PartChannelDialog::PartChannelDialog(QWidget* parent) : QDialog(parent), ui(new Ui::PartChannelDialog)
{
    ui->setupUi(this);
    connect(ui->listView, &QListView::doubleClicked, this, &PartChannelDialog::accept);
}

PartChannelDialog::~PartChannelDialog()
{
    delete ui;
}

void PartChannelDialog::retranslateUi()
{
    ui->retranslateUi(this);
    return;
}

QListView *PartChannelDialog::listView()
{
    return ui->listView;
}
