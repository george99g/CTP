#ifndef PRIVATEMESSAGEWINDOW_HPP
#define PRIVATEMESSAGEWINDOW_HPP

#include "configuration.hpp"
#include <QMainWindow>
#include <QStringListModel>
#include <QTcpSocket>
#include <QStackedWidget>
#include <QListView>
#include <QWindowStateChangeEvent>

namespace Ui
{
    class PrivateMessageWindow;
}

class PrivateMessageWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PrivateMessageWindow(Configuration* config, QWidget* parent = 0);
    ~PrivateMessageWindow();
    QStackedWidget* stackedWidget();
    QListView* listView();
    QList<int> splitterSizes();
    void changeEvent(QEvent* event);
signals:
    void switchUser(QString newUser);
private:
    void handleUserChangeRequest();
    void resizeEvent(QResizeEvent*);
    Ui::PrivateMessageWindow* ui;
    Configuration* _config;
};

#endif // PRIVATEMESSAGEWINDOW_HPP
