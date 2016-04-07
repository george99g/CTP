#ifndef CHANNELMODEDIALOG_HPP
#define CHANNELMODEDIALOG_HPP

#include <QListView>
#include <QDialog>

namespace Ui
{
    class ChannelModeDialog;
}

class ChannelModeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChannelModeDialog(QWidget* parent = 0);
    ~ChannelModeDialog();
    QListView* listView();
    QString channelname();
    void setChannelname(const QString& channelname);
    bool teacher();
    void setTeacher(bool teacher);
    bool student();
    void setStudent(bool student);
    bool administrator();
    void setAdministrator(bool administrator);
private:
    void handleSetRequest();
    QString _channelname;
    bool _teacher;
    bool _student;
    bool _administrator;
    Ui::ChannelModeDialog* ui;
};

#endif // CHANNELMODEDIALOG_HPP
