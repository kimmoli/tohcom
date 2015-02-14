#ifndef PICOCOM_H
#define PICOCOM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>

#include <QtCore/QCoreApplication>
#include <QString>
#include <QtDBus/QtDBus>

#include <getopt.h>

extern "C"
{
#include "term.h"
}


#define KEY_EXIT    '\x18' /* C-x: exit picocom */
#define KEY_QUIT    '\x11' /* C-q: exit picocom without reseting port */
#define KEY_PULSE   '\x10' /* C-p: pulse DTR */
#define KEY_TOGGLE  '\x14' /* C-t: toggle DTR */
#define KEY_BAUD_UP '\x15' /* C-u: increase baudrate (up) */
#define KEY_BAUD_DN '\x04' /* C-d: decrase baudrate (down) */
#define KEY_FLOW    '\x06' /* C-f: change flowcntrl mode */
#define KEY_PARITY  '\x19' /* C-y: change parity mode */
#define KEY_BITS    '\x02' /* C-b: change number of databits */
#define KEY_LECHO   '\x03' /* C-c: toggle local echo */
#define KEY_STATUS  '\x16' /* C-v: show program option */
#define KEY_SEND    '\x13' /* C-s: send file */
#define KEY_RECEIVE '\x12' /* C-r: receive file */
#define KEY_BREAK   '\x1c' /* C-\: break */

#define STO STDOUT_FILENO
#define STI STDIN_FILENO

/**********************************************************************/

/* implemented caracter mappings */
#define M_CRLF   (1 << 0) /* map CR  --> LF */
#define M_CRCRLF (1 << 1) /* map CR  --> CR + LF */
#define M_IGNCR  (1 << 2) /* map CR  --> <nothing> */
#define M_LFCR   (1 << 3) /* map LF  --> CR */
#define M_LFCRLF (1 << 4) /* map LF  --> CR + LF */
#define M_IGNLF  (1 << 5) /* map LF  --> <nothing> */
#define M_DELBS  (1 << 6) /* map DEL --> BS */
#define M_BSDEL  (1 << 7) /* map BS  --> DEL */
#define M_NFLAGS 8

/* default character mappings */
#define M_I_DFL 0
#define M_O_DFL 0
#define M_E_DFL (M_DELBS | M_CRCRLF)


struct Map_names_s
{
    const char *name;
    int flag;
};

struct Opts
{
    QString port;
    int baud;
    enum flowcntrl_e flow;
    QString flow_str;
    enum parity_e parity;
    QString parity_str;
    int databits;
    int lecho;
    int noinit;
    int noreset;
    unsigned char escape;
    QString send_cmd;
    QString receive_cmd;
    int imap;
    int omap;
    int emap;
};

class Picocom
{
public:
    Picocom(int argc, char *argv[]);
    ~Picocom();

    void loop(void);

private:
    Opts opts;

    int tty_fd;

    int parse_map (char *s);
    void print_map (int flags);
    void tohcom_send_dbus_command(QString line);
    ssize_t writen_ni(int fd, const void *buff, size_t n);
    int fd_printf (int fd, const char *format, ...);
    void fatal (const char *format, ...);
    int fd_readline (int fdi, int fdo, char *b, int bsz);
    int do_map (char *b, int map, char c);
    void map_and_write (int fd, int map, char c);
    int baud_up (int baud);
    int baud_down (int baud);
    flowcntrl_e flow_next(flowcntrl_e flow, char **flow_str);
    parity_e parity_next(parity_e parity, char **parity_str);
    int bits_next (int bits);
    //void child_empty_handler (int signum);
    void establish_child_signal_handlers (void);
    int run_cmd(int fd, ...);
    //void deadly_handler(int signum);
    void establish_signal_handlers (void);
    void show_usage(char *name);
    void parse_args(int argc, char *argv[]);


};


#endif // PICOCOM_H
