#ifndef MAINDATABASE_HPP
#define MAINDATABASE_HPP

#include <QObject>
#include <QSqlDatabase>

class MainDatabase : public QObject
{
    Q_OBJECT
public:
    explicit MainDatabase(QObject *parent = 0);
    ~MainDatabase();
signals:

public slots:

};

#endif // MAINDATABASE_HPP
