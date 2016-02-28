#include "chatboxwidget.hpp"
#include "ui_chatboxwidget.h"

ChatBoxWidget::ChatBoxWidget(QTcpSocket* socket, const QString &target, const QString &self, QWidget* parent) : QWidget(parent), ui(new Ui::ChatBoxWidget)
{
    ui->setupUi(this);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &ChatBoxWidget::handleSendMessage);
    connect(ui->toolButton, &QToolButton::pressed, this, &ChatBoxWidget::handleSendMessage);
    _socket = socket;
    _target = target;
    _self = self;
}

ChatBoxWidget::~ChatBoxWidget()
{
    delete ui;
}

void ChatBoxWidget::insertMessage(const QString &sender, const QString &message, const QDateTime &time)
{
    bool atEnd = false;
    if(ui->textEdit->verticalScrollBar()->value() == ui->textEdit->verticalScrollBar()->maximum())
        atEnd = true;
    QString formattedMessage = "<font color=\"#0534CE\">[</font><font color=\"#01840A\">";
    formattedMessage += time.toString(Qt::TextDate);
    formattedMessage += "</font><font color=\"#0534CE\">]</font><font color=\"#02A292\">(";
    formattedMessage += sender.toHtmlEscaped();
    formattedMessage += "):</font> ";
    formattedMessage += message.toHtmlEscaped();
    formattedMessage += "<br>";
    ui->textEdit->insertHtml(formattedMessage);
    if(atEnd)
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
    return;
}

void ChatBoxWidget::insertSystemMessage(const QString &type, const QString &message, const QDateTime &time)
{
    bool atEnd = false;
    if(ui->textEdit->verticalScrollBar()->value() == ui->textEdit->verticalScrollBar()->maximum())
        atEnd = true;
    QString formattedMessage = "<font color=\"#0534CE\">[</font><font color=\"#01840A\">";
    formattedMessage += time.toString(Qt::TextDate);
    formattedMessage += "</font><font color=\"#0534CE\">]</font><font color=\"#970005\">{";
    formattedMessage += type.toHtmlEscaped();
    formattedMessage += "}:</font> ";
    formattedMessage += message.toHtmlEscaped();
    formattedMessage += "<br>";
    ui->textEdit->insertHtml(formattedMessage);
    if(atEnd)
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
    return;
}

void ChatBoxWidget::handleSendMessage()
{
    if(!_socket->isOpen())
        return;
    if(ui->lineEdit->text().isEmpty())
        return;
    QString sendMessage = "PRIVMSG ";
    sendMessage += _target;
    sendMessage += ' ';
    sendMessage += ui->lineEdit->text();
    sendMessage += "\r\n";
    _socket->write(sendMessage.toUtf8());
    _socket->flush();
    insertMessage(_self, ui->lineEdit->text());
    return;
}

void ChatBoxWidget::clear()
{
    ui->textEdit->clear();
    return;
}
