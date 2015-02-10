#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <termio.h>
#include <sys/stat.h>

#include <QThread>
#include <QFile>

#include "pseudoport.h"

const char * const ascii[] = { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS",
                               "TAB", "LF", "VT", "FF", "CR", "SO", "SI", "DLE",
                               "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB", "CAN",
                               "EM", "SUB", "ESC", "FS", "GS", "RS", "US" };

pseudoport::pseudoport(QObject *parent) :
    QObject(parent)
{
    debugPrints = false;
}

pseudoport::~pseudoport()
{
    printf("close pseudoport\n");
    snRead->disconnect(snRead, SIGNAL(activated(int)), this, SLOT(handleRead()));
    close(fd);
}

void pseudoport::create()
{
    char master[1024];
    char slave[1024];

    fd = ptym_open(master, slave, 1024);

    char mode[] = "0666";
    int i = strtol(mode, 0, 8);

    if (chmod (slave, i) < 0)
        perror("create()");

    /* Set to packet mode */
    int arg = 1;
    ioctl(fd, TIOCPKT, &arg);

    struct termios params;

    tcgetattr(fd, &params);
    cfmakeraw(&params);
    cfsetispeed(&params, B9600);
    cfsetospeed(&params, B9600);
    params.c_cflag |= (B9600 |CS8 | CLOCAL | CREAD);
    /* Set EXTPROC to get IOCTL */
    params.c_lflag |= EXTPROC;
    tcsetattr(fd, TCSANOW, &params);

    tcflush(fd, TCIOFLUSH);

    snRead = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    snRead->setEnabled(true);
    snRead->connect(snRead, SIGNAL(activated(int)), this, SLOT(handleRead()));

    printf("Created pseudo-terminal %s (%s)\n", slave, master);
}

void pseudoport::handleRead()
{
    char buffer[1024];
    ssize_t rc = read(fd, buffer, 1024);

    if (rc < 0)
    {
        if (errno == EAGAIN)
            rc = 0;
        else if (errno == EIO)
        {
            perror("handleRead()");
            respawn();
            return;
        }
        else
        {
            perror("handleRead()");
            return;
        }
    }
    if (rc > 0)
    {
        if (buffer[0] != TIOCPKT_DATA)
        {
            /* We got something else than data
             * discard possible data and process the control byte */
            processControlByte(buffer[0]);
        }
        else
        {
            /* we received data, remove control byte and emit rest via signal */
            emit receive(QByteArray::fromRawData(buffer, rc).remove(0, 1));
        }

        if (debugPrints)
        {
            printf("received %d:", rc);
            for (int x=1; x<rc ; x++)
            {
                if (buffer[x] < 32)
                    printf(" %c[%dm%s%c[%dm ", 27, 7, ascii[(int)buffer[x]], 27, 0);
                else
                    printf("%c", buffer[x]);
            }
            printf("\n");
        }
    }
}

void pseudoport::processControlByte(const char c)
{
    if (debugPrints)
    {
        printf("TIOCPKT");
        if (c & TIOCPKT_DOSTOP)     printf("_DOSTOP");
        if (c & TIOCPKT_FLUSHREAD)  printf("_FLUSHREAD");
        if (c & TIOCPKT_FLUSHWRITE) printf("_FLUSHWRITE");
        if (c & TIOCPKT_IOCTL)      printf("_IOCTL");
        if (c & TIOCPKT_NOSTOP)     printf("_NOSTOP");
        if (c & TIOCPKT_START)      printf("_START");
        if (c & TIOCPKT_STOP)       printf("_STOP");
        printf("\n");
    }

    /* At this age, we are interested in TIOCPKT_IOCTL only */
    if (c & TIOCPKT_IOCTL)
    {
        tcflush(fd, TCIOFLUSH);

        struct termios params;
        tcgetattr(fd, &params);

        if (debugPrints)
        {
            printf("c_iflag  = %06x\n", params.c_iflag);
            printf("c_oflag  = %06x\n", params.c_oflag);
            printf("c_cflag  = %06x\n", params.c_cflag);
            printf("c_lflag  = %06x\n", params.c_lflag);
            printf("c_ispeed = %06x\n", params.c_ispeed);
            printf("c_ospeed = %06x\n", params.c_ospeed);
        }
    }
}

void pseudoport::transmit(QByteArray data)
{
    if (write(fd, data.data(), data.count()) < 0)
        perror("transmit()");
}

int pseudoport::ptym_open(char *pts_name, char *pts_name_s, int pts_namesz)
{
    char *ptr;
    int fdm;

    strncpy(pts_name, "/dev/ptmx", pts_namesz);
    pts_name[pts_namesz - 1] = '\0';

    fdm = posix_openpt(O_RDWR | O_NONBLOCK);
    if (fdm < 0)
        return(-1);

    if (grantpt(fdm) < 0)
    {
        close(fdm);
        return(-2);
    }
    if (unlockpt(fdm) < 0)
    {
        close(fdm);
        return(-3);
    }
    if ((ptr = ptsname(fdm)) == NULL)
    {
        close(fdm);
        return(-4);
    }

    strncpy(pts_name_s, ptr, pts_namesz);
    pts_name[pts_namesz - 1] = '\0';

    return(fdm);
}

/* This is done in case of EIO, I/O Error */
void pseudoport::respawn()
{
    snRead->disconnect(snRead, SIGNAL(activated(int)), this, SLOT(handleRead()));
    tcflush(fd, TCIOFLUSH);

    close(fd);

    QThread::msleep(100);

    create();
}
