#include "linktextedit.hpp"

LinkTextEdit::LinkTextEdit(QWidget* parent) : QTextEdit(parent)
{
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    setReadOnly(true);
}

void LinkTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    QString url = anchorAt(e->pos());
    if(url == "")
        return;
    QDesktopServices::openUrl(QUrl(url));
    return;
}
