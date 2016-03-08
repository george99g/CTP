#ifndef CHATBOXWIDGET_HPP
#define CHATBOXWIDGET_HPP

#include "linktextedit.hpp"
#include <QWidget>
#include <QDateTime>
#include <QScrollBar>
#include <QTcpSocket>

namespace Ui
{
    class ChatBoxWidget;
}

class ChatBoxWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatBoxWidget(QTcpSocket* socket, const QString &target, const QString &self, QWidget* parent = 0);
    ~ChatBoxWidget();
    void insertMessage(const QString &sender, const QString &message, const QDateTime &time = QDateTime::currentDateTime());
    void insertSystemMessage(const QString &type, const QString &message, const QDateTime &time = QDateTime::currentDateTime());
    void handleSendMessage();
    QString insertHtmlLinks(QString string);
    QString getTarget() const;
    void clear();
private:
    QString convertToNoSpace(QString string);
    Ui::ChatBoxWidget *ui;
    QString _target;
    QString _self;
    QTcpSocket* _socket;
    LinkTextEdit* _textEdit;
};

#endif // CHATBOXWIDGET_HPP
