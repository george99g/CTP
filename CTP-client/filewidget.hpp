#ifndef FILEWIDGET_HPP
#define FILEWIDGET_HPP

#include <QWidget>
#include <QTcpSocket>

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
    void setSockets(QTcpSocket* ircSocket, QTcpSocket* ftpSocket);
private:
    QTcpSocket* _ircSocket;
    QTcpSocket* _ftpSocket;
    Ui::FileWidget *ui;
};

#endif // FILEWIDGET_HPP
