#ifndef PSEUDOPORT_H
#define PSEUDOPORT_H

#include <QObject>

class pseudoport : public QObject
{
    Q_OBJECT
public:
    explicit pseudoport(QObject *parent = 0);

    int ptym_open(char *pts_name, char *pts_name_s , int pts_namesz);
    bool doPoll;

signals:
    void received(char c);

public slots:
    void create();


};

#endif // PSEUDOPORT_H
