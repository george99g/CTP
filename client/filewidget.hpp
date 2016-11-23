#ifndef FILEWIDGET_HPP
#define FILEWIDGET_HPP

#include <QWidget>
#include <QTcpSocket>
#include <QListView>
#include <QStringList>
#include <QStringListModel>
#include <QModelIndex>
#include <QFileDialog>

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
    void retranslateUi();
signals:
    void requestRefresh();
    void downloadFile(QString file);
    void sendFile(QString file);
    void deleteFile(QString file);
private:
    void handleDownloadFileRequest();
    void handleSendFileRequest();
    void handleDeleteFileRequest();
    Ui::FileWidget *ui;
};

#endif // FILEWIDGET_HPP
