#ifndef PARTCHANNELDIALOG_HPP
#define PARTCHANNELDIALOG_HPP

#include <QDialog>
#include <QListView>

namespace Ui
{
    class PartChannelDialog;
}

class PartChannelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PartChannelDialog(QWidget* parent = 0);
    ~PartChannelDialog();
    void retranslateUi();
    QListView* listView();
private:
    Ui::PartChannelDialog* ui;
};

#endif // PARTCHANNELDIALOG_HPP
