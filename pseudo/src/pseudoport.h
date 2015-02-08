#ifndef PSEUDOPORT_H
#define PSEUDOPORT_H

#include <QObject>
#include <QByteArray>
#include <QSocketNotifier>

class pseudoport : public QObject
{
    Q_OBJECT
public:
    explicit pseudoport(QObject *parent = 0);
    ~pseudoport();

    int ptym_open(char *pts_name, char *pts_name_s , int pts_namesz);

signals:
    void receive(QByteArray data);

public slots:
    void create();
    void transmit(QByteArray data);
    void handleRead();

private:
    int fd;
    QSocketNotifier* snRead;
    void respawn();
};

#endif // PSEUDOPORT_H
