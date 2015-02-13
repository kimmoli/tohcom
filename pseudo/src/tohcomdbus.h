#ifndef TOHCOMDBUS_H
#define TOHCOMDBUS_H

#include <QObject>
#include <QtDBus/QtDBus>

#define SERVICE_NAME "com.kimmoli.tohcom"

class QDBusInterface;
class TohcomDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    explicit TohcomDBus(QObject *parent = 0);
    virtual ~TohcomDBus();
    void registerDBus();

signals:
    void commandFromDbus(QString line);

public slots:
    void command(const QString &line);
    QString getVersion();

private:
    bool m_dbusRegistered;

};

#endif // TOHCOMDBUS_H
