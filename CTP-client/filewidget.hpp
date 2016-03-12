#ifndef FILEWIDGET_HPP
#define FILEWIDGET_HPP

#include <QWidget>
#include <QTcpSocket>
#include <QListView>
#include <QStringList>
#include <QStringListModel>
#include <QModelIndex>

namespace Ui
{
    class FileWidget;
}

class FileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileWidget(QWidget* parent = 0);
    ~FileWidget();
    QListView* listView();
signals:
    void requestRefresh();
    void downloadFile(QString file);
private:
    void handleDownloadFileRequest();
    Ui::FileWidget *ui;
};

#endif // FILEWIDGET_HPP
