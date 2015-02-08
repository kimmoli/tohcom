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

pseudoport::pseudoport(QObject *parent) :
    QObject(parent)
{
}

pseudoport::~pseudoport()
{
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
        perror("chmod");

    tcflush(fd, TCIOFLUSH);

    sn = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    sn->setEnabled(true);
    sn->connect(sn, SIGNAL(activated(int)), this, SLOT(readyRead()));

    printf("Created pseudo-terminal %s (%s)\n", slave, master);
}

void pseudoport::readyRead()
{
    char buffer[1024];
    ssize_t rc = read(fd, buffer, 1024);

    if (rc < 0)
    {
        if (errno == EAGAIN || errno == EIO)
            rc = 0;
        else
        {
            perror("read");
            return;
        }
    }
    if (rc > 0)
    {
        /* we received something, emit it via signal */
        emit receive(QByteArray::fromRawData(buffer, rc));
    }
}

void pseudoport::transmit(QByteArray data)
{
    if (write(fd, data.data(), data.count()) < 0)
        perror("write");
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

