#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <QObject>
#include <QMutex>

class Interrupt : public QObject
{
    Q_OBJECT
public:
    explicit Interrupt(QObject *parent = 0);
    ~Interrupt();
    void abort();

signals:
    void pollingRequested();
    void interruptCaptured();
    void finished();

public slots:
    void doPoll();
    void requestPolling();

private:
    QMutex mutex;
    bool m_abort;
    bool m_polling;
    int m_gpio_fd;
    int requestGpioInterrupt(const char *gpio, const char *edge);
    void releaseGpioInterrupt(int fd, const char* gpio);
};

#endif // INTERRUPT_H
