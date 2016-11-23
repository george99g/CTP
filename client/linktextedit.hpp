#ifndef LINKTEXTEDIT_HPP
#define LINKTEXTEDIT_HPP

#include <QObject>
#include <QTextEdit>
#include <QMouseEvent>
#include <QDesktopServices>

class LinkTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit LinkTextEdit(QWidget* parent = 0);
    void mouseReleaseEvent(QMouseEvent* e);
};

#endif // LINKTEXTEDIT_HPP
