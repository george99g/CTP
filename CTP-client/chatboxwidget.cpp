#include "chatboxwidget.hpp"
#include "ui_chatboxwidget.h"

ChatBoxWidget::ChatBoxWidget(QTcpSocket* socket, const QString &target, const QString &self, QWidget* parent) : QWidget(parent), ui(new Ui::ChatBoxWidget)
{
    ui->setupUi(this);
    _textEdit = new LinkTextEdit(ui->lineEdit);
    ui->verticalLayout->insertWidget(0, _textEdit);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &ChatBoxWidget::handleSendMessage);
    connect(ui->toolButton, &QToolButton::pressed, this, &ChatBoxWidget::handleSendMessage);
    _socket = socket;
    _target = target;
    _self = self;
}

ChatBoxWidget::~ChatBoxWidget()
{
    delete ui;
    _textEdit->deleteLater();
}

void ChatBoxWidget::insertMessage(const QString &sender, const QString &message, const QDateTime &time)
{
    bool atEnd = false;
    if(_textEdit->verticalScrollBar()->value() == _textEdit->verticalScrollBar()->maximum())
        atEnd = true;
    QString formattedMessage = "<font color=\"#0534CE\">[</font><font color=\"#01840A\">";
    formattedMessage += time.toString(Qt::TextDate);
    formattedMessage += "</font><font color=\"#0534CE\">]</font><font color=\"#02A292\">";
    formattedMessage += sender.toHtmlEscaped();
    formattedMessage += ":</font> ";
    formattedMessage += insertHtmlLinks(message.toHtmlEscaped());
    formattedMessage += "<br>";
    _textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    _textEdit->insertHtml(formattedMessage);
    if(atEnd)
        _textEdit->verticalScrollBar()->setValue(_textEdit->verticalScrollBar()->maximum());
    return;
}

void ChatBoxWidget::insertSystemMessage(const QString &type, const QString &message, const QDateTime &time)
{
    bool atEnd = false;
    if(_textEdit->verticalScrollBar()->value() == _textEdit->verticalScrollBar()->maximum())
        atEnd = true;
    QString formattedMessage = "<font color=\"#0534CE\">[</font><font color=\"#01840A\">";
    formattedMessage += time.toString(Qt::TextDate);
    formattedMessage += "</font><font color=\"#0534CE\">]</font><font color=\"#970005\">{";
    formattedMessage += type.toHtmlEscaped();
    formattedMessage += "}:</font> ";
    formattedMessage += insertHtmlLinks(message.toHtmlEscaped());
    formattedMessage += "<br>";
    _textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    _textEdit->insertHtml(formattedMessage);
    if(atEnd)
        _textEdit->verticalScrollBar()->setValue(_textEdit->verticalScrollBar()->maximum());
    return;
}

void ChatBoxWidget::handleSendMessage()
{
    if(!_socket->isOpen())
        return;
    if(ui->lineEdit->text().isEmpty())
        return;
    QString sendMessage = "PRIVMSG ";
    sendMessage += convertToNoSpace(_target);
    sendMessage += ' ';
    sendMessage += ui->lineEdit->text();
    sendMessage += "\r\n";
    _socket->write(sendMessage.toUtf8());
    _socket->flush();
    insertMessage(_self, ui->lineEdit->text());
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
    return;
}

QString ChatBoxWidget::insertHtmlLinks(QString string)
{
    QStringList explodedList = string.split(' ', QString::SkipEmptyParts);
    for(unsigned i = 0; i < (unsigned)explodedList.count(); i++)
    {
        if((explodedList.at(i).startsWith("https://") || explodedList.at(i).startsWith("http://"))&&explodedList.at(i).contains('.'))
            string = string.replace(explodedList.at(i), QString("<a href='"+explodedList.at(i)+"'>"+explodedList.at(i)+"</a>"));
        else if(explodedList.at(i).contains('@') && explodedList.at(i).contains('.') &&
                explodedList.at(i).lastIndexOf('.') != explodedList.at(i).count() - 1)
            string = string.replace(explodedList.at(i), QString("<a href='mailto:"+explodedList.at(i)+"'>"+explodedList.at(i)+"</a>"));
    }
    return string;
}

QString ChatBoxWidget::getTarget() const
{
    return _target;
}

void ChatBoxWidget::clear()
{
    _textEdit->clear();
    return;
}

QString ChatBoxWidget::convertToNoSpace(QString string)
{
    string.replace("\\", "\\\\");
    string.replace(" ", "\\s");
    return string;
}
