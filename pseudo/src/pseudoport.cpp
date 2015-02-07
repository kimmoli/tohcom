#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>

#include <QThread>

#include "pseudoport.h"

pseudoport::pseudoport(QObject *parent) :
    QObject(parent)
{
    printf("pseudoport started\n");
    doPoll = true;
}

void pseudoport::create()
{
    char master[1024];
    char slave[1024];

    int fd;
    fd_set rfds;
    int retval;

    fd = ptym_open(master, slave, 1024);

    printf("Created pseudo-terminal %s\n", slave);


    while(doPoll)
    {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        retval = select(fd + 1, &rfds, NULL, NULL, NULL);
        if (retval == -1)
        {
            perror("select");
            return;
        }


        if (FD_ISSET(fd, &rfds))
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
                printf("received %d\n", rc);
                emit received(buffer[0]);
            }
        }
    }

    close(fd);
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

