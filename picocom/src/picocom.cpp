#include "picocom.h"

#include <QThread>
#include <QtCore/QCoreApplication>
#include <QString>
#include <QtDBus/QtDBus>

/* Constructor */
Picocom::Picocom()
{
    /* Some defaults*/

    opts.port = "";
    opts.baud = 9600;
    opts.flow = FC_NONE;
    opts.flow_str = "none";
    opts.parity = P_NONE;
    opts.parity_str = "none";
    opts.databits = 8;
    opts.lecho = 0;
    opts.noinit = 0;
    opts.noreset = 0;
    opts.escape = '\x01';
    opts.send_cmd = "sz -vv";
    opts.receive_cmd = "rz -vv";
    opts.imap = M_I_DFL;
    opts.omap = M_O_DFL;
    opts.emap = M_E_DFL;
    opts.dbus = 0;
}

bool Picocom::init()
{
    printf("picocom v%s\n", VERSION_STR);
    printf("\n");
    printf("port is        : %s\n", opts.port.toLocal8Bit().data());
    printf("flowcontrol    : %s\n", opts.flow_str.toLocal8Bit().data());
    printf("baudrate is    : %d\n", opts.baud);
    printf("parity is      : %s\n", opts.parity_str.toLocal8Bit().data());
    printf("databits are   : %d\n", opts.databits);
    printf("escape is      : C-%c\n", 'a' + opts.escape - 1);
    printf("local echo is  : %s\n", opts.lecho ? "yes" : "no");
    printf("noinit is      : %s\n", opts.noinit ? "yes" : "no");
    printf("noreset is     : %s\n", opts.noreset ? "yes" : "no");
    printf("send_cmd is    : %s\n", opts.send_cmd.toLocal8Bit().data());
    printf("receive_cmd is : %s\n", opts.receive_cmd.toLocal8Bit().data());
    printf("imap is        : "); print_map(opts.imap);
    printf("omap is        : "); print_map(opts.omap);
    printf("emap is        : "); print_map(opts.emap);
    printf("\n");

    int r;

    establish_signal_handlers();

    r = term_lib_init();
    if ( r < 0 )
    {
        fatal("term_init failed: %s", term_strerror(term_errno, errno));
        return false;
    }

    tty_fd = open(opts.port.toLocal8Bit().data(), O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (tty_fd < 0)
    {
        fatal("cannot open %s: %s", opts.port.toLocal8Bit().data(), strerror(errno));
        return false;
    }

    if ( opts.noinit ) {
        r = term_add(tty_fd);
    } else {
//        r = term_set(tty_fd,
//                     1,              /* raw mode. */
//                     opts.baud,      /* baud rate. */
//                     opts.parity,    /* parity. */
//                     opts.databits,  /* data bits. */
//                     opts.flow,      /* flow control. */
//                     1,              /* local or modem */
//                     !opts.noreset); /* hup-on-close. */
        r = term_add(tty_fd);
        tohcom_send_dbus_command(QString("baud %1").arg(opts.baud));
        tohcom_send_dbus_command(QString("parity %1").arg(opts.parity_str));
        tohcom_send_dbus_command(QString("bits %1").arg(opts.databits));
        tohcom_send_dbus_command(QString("stop %1").arg(1)); /* stop bits */
        tohcom_send_dbus_command(QString("flow %1").arg(opts.flow_str));
    }
    if ( r < 0 )
    {
        fatal("failed to add device %s: %s",
              opts.port.toLocal8Bit().data(), term_strerror(term_errno, errno));
        return false;
    }
    r = term_apply(tty_fd);
    if ( r < 0 )
    {
        fatal("failed to config device %s: %s",
              opts.port.toLocal8Bit().data(), term_strerror(term_errno, errno));
        return false;
    }

    r = term_add(STI);
    if ( r < 0 )
    {
        fatal("failed to add I/O device: %s",
              term_strerror(term_errno, errno));
        return false;
    }
    term_set_raw(STI);
    r = term_apply(STI);
    if ( r < 0 )
    {
        fatal("failed to set I/O device to raw mode: %s",
              term_strerror(term_errno, errno));
        return false;
    }

    fd_printf(STO, "Terminal ready\r\n");

    return true;
}

/* Desctructor */
Picocom::~Picocom()
{
    fd_printf(STO, "\r\n");
    if ( opts.noreset ) {
        fd_printf(STO, "Skipping tty reset...\r\n");
        term_erase(tty_fd);
    }

    fd_printf(STO, "Closing picocom...\r\n");

    QThread::msleep(500);

    if (opts.dbus)
        tohcom_send_dbus_command("quit");
}


/**********************************************************************/

/* character mapping names */
Map_names_s map_names[] =
{
	{ "crlf", M_CRLF },
	{ "crcrlf", M_CRCRLF },
	{ "igncr", M_IGNCR },
    { "lfcr", M_LFCR },
	{ "lfcrlf", M_LFCRLF },
	{ "ignlf", M_IGNLF },
	{ "delbs", M_DELBS },
	{ "bsdel", M_BSDEL },
	/* Sentinel */
	{ NULL, 0 } 
};

int Picocom::parse_map (char *s)
{
    const char *m, *t;
	int f, flags, i;

	flags = 0;
	while ( (t = strtok(s, ", \t")) ) {
		for (i=0; (m = map_names[i].name); i++) {
			if ( ! strcmp(t, m) ) {
				f = map_names[i].flag;
				break;
			}
		}
		if ( m ) flags |= f;
		else { flags = -1; break; }
		s = NULL;
	}

	return flags;
}

void Picocom::print_map (int flags)
{
	int i;

	for (i = 0; i < M_NFLAGS; i++)
		if ( flags & (1 << i) )
			printf("%s,", map_names[i].name);
	printf("\n");
}

/**********************************************************************/



void Picocom::tohcom_send_dbus_command(QString line)
{
    QDBusInterface tohcom("com.kimmoli.tohcom", "/", "com.kimmoli.tohcom", QDBusConnection::systemBus());
    tohcom.setTimeout(2000);
    QList<QVariant> args;
    args.append(QString(line));
    tohcom.callWithArgumentList(QDBus::AutoDetect, "command", args);
}

QString Picocom::readFromDaemon(QString method)
{
    QDBusInterface getCall("com.kimmoli.tohcom", "/", "com.kimmoli.tohcom", QDBusConnection::systemBus());
    getCall.setTimeout(2000);

    QDBusMessage getCallReply = getCall.call(QDBus::AutoDetect, method);

    if (getCallReply.type() == QDBusMessage::ErrorMessage)
    {
        fatal("Error in %s: %s\n", qPrintable(method), qPrintable(getCallReply.errorMessage()));
        return QString();
    }

    return getCallReply.arguments().at(0).toString();
}


/**********************************************************************/

ssize_t Picocom::writen_ni(int fd, const void *buff, size_t n)
{
	size_t nl; 
	ssize_t nw;
	const char *p;

    p = (const char *)buff;
	nl = n;
	while (nl > 0) {
		do {
			nw = write(fd, p, nl);
		} while ( nw < 0 && errno == EINTR );
		if ( nw <= 0 ) break;
		nl -= nw;
		p += nw;
	}
	
	return n - nl;
}

int Picocom::fd_printf (int fd, const char *format, ...)
{
	char buf[256];
	va_list args;
	int len;
	
	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	buf[sizeof(buf) - 1] = '\0';
	va_end(args);
	
	return writen_ni(fd, buf, len);
}

void Picocom::fatal (const char *format, ...)
{
    const char *s;
    char buf[256];
	va_list args;
	int len;

	term_reset(STO);
	term_reset(STI);
	
	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	buf[sizeof(buf) - 1] = '\0';
	va_end(args);
	
	s = "\r\nFATAL: ";
	writen_ni(STO, s, strlen(s));
	writen_ni(STO, buf, len);
	s = "\r\n";
	writen_ni(STO, s, strlen(s));
}

/**/
#define cput(fd, c) do { int cl = c; int wrc = write((fd), &(cl), 1); (void) wrc; } while(0)

int Picocom::fd_readline (int fdi, int fdo, char *b, int bsz)
{
	int r;
	unsigned char c;
	unsigned char *bp, *bpe;
	
	bp = (unsigned char *)b;
	bpe = (unsigned char *)b + bsz - 1;

	while (1) {
		r = read(fdi, &c, 1);
		if ( r <= 0 ) { r--; goto out; }

		switch (c) {
		case '\b':
			if ( bp > (unsigned char *)b ) { 
				bp--;
				cput(fdo, c); cput(fdo, ' '); cput(fdo, c);
			} else {
				cput(fdo, '\x07');
			}
			break;
		case '\r':
			*bp = '\0';
			r = bp - (unsigned char *)b; 
			goto out;
		default:
			if ( bp < bpe ) { *bp++ = c; cput(fdo, c); }
			else { cput(fdo, '\x07'); }
			break;
		}
	}

out: /* TODO: get rid of goto */
	return r;
}

#undef cput

/* maximum number of chars that can replace a single characted
   due to mapping */
#define M_MAXMAP 4

int Picocom::do_map (char *b, int map, char c)
{
	int n;

	switch (c) {
	case '\x7f':
		/* DEL mapings */
		if ( map & M_DELBS ) {
			b[0] = '\x08'; n = 1;
		} else {
			b[0] = c; n = 1;
		}
		break;
	case '\x08':
		/* BS mapings */
		if ( map & M_BSDEL ) {
			b[0] = '\x7f'; n = 1;
		} else {
			b[0] = c; n = 1;
		}
		break;
	case '\x0d':
		/* CR mappings */
		if ( map & M_CRLF ) {
			b[0] = '\x0a'; n = 1;
		} else if ( map & M_CRCRLF ) {
			b[0] = '\x0d'; b[1] = '\x0a'; n = 2;
		} else if ( map & M_IGNCR ) {
			n = 0;
		} else {
			b[0] = c; n = 1;
		}
		break;
	case '\x0a':
		/* LF mappings */
		if ( map & M_LFCR ) {
			b[0] = '\x0d'; n = 1;
		} else if ( map & M_LFCRLF ) {
			b[0] = '\x0d'; b[1] = '\x0a'; n = 2;
		} else if ( map & M_IGNLF ) {
			n = 0;
		} else {
			b[0] = c; n = 1;
		}
		break;
	default:
		b[0] = c; n = 1;
		break;
	}

	return n;
}

void Picocom::map_and_write (int fd, int map, char c)
{
	char b[M_MAXMAP];
	int n;
		
	n = do_map(b, map, c);
	if ( n )
		if ( writen_ni(fd, b, n) < n )
        {
			fatal("write to stdout failed: %s", strerror(errno));		
            emit wantsToQuit();
        }
}

/**********************************************************************/

void Picocom::baud_up ()
{
    int baud = opts.baud;

	if ( baud < 300 )
		baud = 300;
	else if ( baud == 38400 )
		baud = 57600;
	else	
		baud = baud * 2;
#ifndef HIGH_BAUD
	if ( baud > 115200 )
		baud = 115200;
#else
	if ( baud > 921600 )
		baud = 921600;
#endif

    if (baud != opts.baud)
    {
        opts.baud = baud;
        tohcom_send_dbus_command(QString("baud %1").arg(baud));
    }
}

void Picocom::baud_down ()
{
    int baud = opts.baud;

#ifndef HIGH_BAUD
	if ( baud > 115200 )
		baud = 115200;
#else
	if ( baud > 921600 )
		baud = 921600;
#endif
	else if ( baud == 57600 )
		baud = 38400;
	else
		baud = baud / 2;

	if ( baud < 300)
		baud = 300;

    if (baud != opts.baud)
    {
        opts.baud = baud;
        tohcom_send_dbus_command(QString("baud %1").arg(baud));
    }
}

void Picocom::flow_next()
{
    flowcntrl_e flow = opts.flow;
    QString flow_str;

    switch(opts.flow) {
	case FC_NONE:
        flow = FC_RTSCTS;
        flow_str = "RTS/CTS";
		break;
	case FC_RTSCTS:
        flow = FC_XONXOFF;
        flow_str = "xon/xoff";
		break;
	case FC_XONXOFF:
        flow = FC_NONE;
        flow_str = "none";
		break;
	default:
        flow = FC_NONE;
        flow_str = "none";
		break;
	}

    if (flow != opts.flow)
    {
        opts.flow = flow;
        opts.flow_str = flow_str;
        tohcom_send_dbus_command(QString("flow %1").arg(flow_str));
    }
}

void Picocom::parity_next ()
{
    parity_e parity = opts.parity;
    QString parity_str;

    switch(parity) {
	case P_NONE:
        parity = P_EVEN;
        parity_str = "even";
		break;
	case P_EVEN:
        parity = P_ODD;
        parity_str = "odd";
		break;
	case P_ODD:
        parity = P_NONE;
        parity_str = "none";
		break;
	default:
        parity = P_NONE;
        parity_str = "none";
		break;
	}

    if (parity != opts.parity)
    {
        opts.parity = parity;
        opts.parity_str = parity_str;
        tohcom_send_dbus_command(QString("parity %1").arg(parity_str));
    }
}

void Picocom::bits_next ()
{
    int bits = opts.databits;

    bits++;
	if (bits > 8) bits = 5;

    if (bits != opts.databits)
    {
        opts.databits = bits;
        tohcom_send_dbus_command(QString("bits %1").arg(bits));
    }
}

/**********************************************************************/

void child_empty_handler (int signum)
{
    (void) signum;
}

void Picocom::establish_child_signal_handlers (void)
{
	struct sigaction empty_action;
	
	/* Set up the structure to specify the "empty" action. */
    empty_action.sa_handler = child_empty_handler;
	sigemptyset (&empty_action.sa_mask);
	empty_action.sa_flags = 0;
	
	sigaction (SIGINT, &empty_action, NULL);
	sigaction (SIGTERM, &empty_action, NULL);
}

int Picocom::run_cmd(int fd, ...)
{
	pid_t pid;
	sigset_t sigm, sigm_old;

	/* block signals, let child establish its own handlers */
	sigemptyset(&sigm);
	sigaddset(&sigm, SIGTERM);
	sigprocmask(SIG_BLOCK, &sigm, &sigm_old);

	pid = fork();
	if ( pid < 0 ) {
		sigprocmask(SIG_SETMASK, &sigm_old, NULL);
		fd_printf(STO, "*** cannot fork: %s\n", strerror(errno));
		return -1;
	} else if ( pid ) {
		/* father: picocom */
		int r;

		/* reset the mask */
		sigprocmask(SIG_SETMASK, &sigm_old, NULL);
		/* wait for child to finish */
		waitpid(pid, &r, 0);
		/* reset terminal (back to raw mode) */
		term_apply(STI);
		/* check and report child return status */
		if ( WIFEXITED(r) ) { 
			fd_printf(STO, "\r\n*** exit status: %d\r\n", 
					  WEXITSTATUS(r));
			return WEXITSTATUS(r);
		} else {
			fd_printf(STO, "\r\n*** abnormal termination: 0x%x\r\n", r);
			return -1;
		}
	} else {
		/* child: external program */
		int r;
		long fl;
		char cmd[512];

		establish_child_signal_handlers();
		sigprocmask(SIG_SETMASK, &sigm_old, NULL);
		/* unmanage terminal, and reset it to canonical mode */
		term_remove(STI);
		/* unmanage serial port fd, without reset */
		term_erase(fd);
		/* set serial port fd to blocking mode */
		fl = fcntl(fd, F_GETFL); 
		fl &= ~O_NONBLOCK;
		fcntl(fd, F_SETFL, fl);
		/* connect stdin and stdout to serial port */
		close(STI);
		close(STO);
		dup2(fd, STI);
		dup2(fd, STO);
		{
			/* build command-line */
			char *c, *ce;
			const char *s;
			int n;
			va_list vls;
			
			c = cmd;
			ce = cmd + sizeof(cmd) - 1;
			va_start(vls, fd);
			while ( (s = va_arg(vls, const char *)) ) {
				n = strlen(s);
				if ( c + n + 1 >= ce ) break;
				memcpy(c, s, n); c += n;
				*c++ = ' ';
			}
			va_end(vls);
			*c = '\0';
		}
		/* run extenral command */
		fd_printf(STDERR_FILENO, "%s\n", cmd);
		r = system(cmd);
		if ( WIFEXITED(r) ) exit(WEXITSTATUS(r));
		else exit(128);
	}
}


void Picocom::show_help()
{
    fd_printf(STO, "C-%c C-%c   exit picocom\r\n", 'a' + opts.escape -1, 'a' + KEY_EXIT-1);
    fd_printf(STO, "C-%c C-%c   exit picocom without reseting port\r\n", 'a' + opts.escape -1, 'a' + KEY_QUIT-1);
    fd_printf(STO, "C-%c C-%c   pulse DTR\r\n", 'a' + opts.escape -1, 'a' + KEY_PULSE  -1);
    fd_printf(STO, "C-%c C-%c   toggle DTR\r\n", 'a' + opts.escape -1, 'a' + KEY_TOGGLE -1);
    fd_printf(STO, "C-%c C-%c   increase baudrate (up)\r\n", 'a' + opts.escape -1, 'a' + KEY_BAUD_UP-1);
    fd_printf(STO, "C-%c C-%c   decrase baudrate (down)\r\n", 'a' + opts.escape -1, 'a' + KEY_BAUD_DN-1);
    fd_printf(STO, "C-%c C-%c   change flowcntrl mode\r\n", 'a' + opts.escape -1, 'a' + KEY_FLOW   -1);
    fd_printf(STO, "C-%c C-%c   change parity mode\r\n", 'a' + opts.escape -1, 'a' + KEY_PARITY -1);
    fd_printf(STO, "C-%c C-%c   change number of databits\r\n", 'a' + opts.escape -1, 'a' + KEY_BITS   -1);
    fd_printf(STO, "C-%c C-%c   toggle local echo\r\n", 'a' + opts.escape -1, 'a' + KEY_LECHO  -1);
    fd_printf(STO, "C-%c C-%c   show program option\r\n", 'a' + opts.escape -1, 'a' + KEY_STATUS -1);
    fd_printf(STO, "C-%c C-%c   send file\r\n", 'a' + opts.escape -1, 'a' + KEY_SEND   -1);
    fd_printf(STO, "C-%c C-%c   receive file\r\n", 'a' + opts.escape -1, 'a' + KEY_RECEIVE-1);
    fd_printf(STO, "C-%c C-%c   break\r\n", 'a' + opts.escape -1, 'a' + KEY_BREAK  -1);
    fd_printf(STO, "C-%c C-%c   help\r\n", 'a' + opts.escape -1, 'a' + KEY_HELP   -1);
}

/**********************************************************************/

#define TTY_Q_SZ 256

struct tty_q {
	int len;
	unsigned char buff[TTY_Q_SZ];
} tty_q;

/**********************************************************************/

void Picocom::loop()
{
	enum {
		ST_COMMAND,
		ST_TRANSPARENT
	} state;
	int dtr_up;
	fd_set rdset, wrset;
	char fname[128];
	int r, n;
	unsigned char c;


	tty_q.len = 0;
	state = ST_TRANSPARENT;
	dtr_up = 0;

	for (;;) {
		FD_ZERO(&rdset);
		FD_ZERO(&wrset);
		FD_SET(STI, &rdset);
		FD_SET(tty_fd, &rdset);
		if ( tty_q.len ) FD_SET(tty_fd, &wrset);

		if (select(tty_fd + 1, &rdset, &wrset, NULL, NULL) < 0)
        {
			fatal("select failed: %d : %s", errno, strerror(errno));
            emit wantsToQuit();
        }

		if ( FD_ISSET(STI, &rdset) ) {

			/* read from terminal */

			do {
				n = read(STI, &c, 1);
			} while (n < 0 && errno == EINTR);
			if (n == 0) {
				fatal("stdin closed");
                emit wantsToQuit();
			} else if (n < 0) {
				/* is this really necessary? better safe than sory! */
				if ( errno != EAGAIN && errno != EWOULDBLOCK ) 
                {
					fatal("read from stdin failed: %s", strerror(errno));
                    emit wantsToQuit();
                }
				else
					goto skip_proc_STI;
			}

			switch (state) {

			case ST_COMMAND:
				if ( c == opts.escape ) {
					state = ST_TRANSPARENT;
					/* pass the escape character down */
					if (tty_q.len + M_MAXMAP <= TTY_Q_SZ) {
						n = do_map((char *)tty_q.buff + tty_q.len, 
								   opts.omap, c);
						tty_q.len += n;
						if ( opts.lecho ) 
							map_and_write(STO, opts.emap, c);
					} else 
						fd_printf(STO, "\x07");
					break;
				}
				state = ST_TRANSPARENT;
				switch (c) {
				case KEY_EXIT:
                    emit wantsToQuit();
					return;
				case KEY_QUIT:
					term_set_hupcl(tty_fd, 0);
					term_flush(tty_fd);
					term_apply(tty_fd);
					term_erase(tty_fd);
                    emit wantsToQuit();
					return;
				case KEY_STATUS:
					fd_printf(STO, "\r\n");
					fd_printf(STO, "*** baud: %d\r\n", opts.baud);
                    fd_printf(STO, "*** flow: %s\r\n", opts.flow_str.toLocal8Bit().data());
                    fd_printf(STO, "*** parity: %s\r\n", opts.parity_str.toLocal8Bit().data());
					fd_printf(STO, "*** databits: %d\r\n", opts.databits);
					fd_printf(STO, "*** dtr: %s\r\n", dtr_up ? "up" : "down");
					break;
				case KEY_PULSE:
					fd_printf(STO, "\r\n*** pulse DTR ***\r\n");
					if ( term_pulse_dtr(tty_fd) < 0 )
						fd_printf(STO, "*** FAILED\r\n");
					break;
				case KEY_TOGGLE:
					if ( dtr_up )
						r = term_lower_dtr(tty_fd);
					else
						r = term_raise_dtr(tty_fd);
					if ( r >= 0 ) dtr_up = ! dtr_up;
					fd_printf(STO, "\r\n*** DTR: %s ***\r\n", 
							  dtr_up ? "up" : "down");
					break;
				case KEY_BAUD_UP:
                    baud_up();
					fd_printf(STO, "\r\n*** baud: %d ***\r\n", opts.baud);
					break;
				case KEY_BAUD_DN:
                    baud_down();
                    fd_printf(STO, "\r\n*** baud: %d ***\r\n", opts.baud);
					break;
				case KEY_FLOW:
                    flow_next();
                    fd_printf(STO, "\r\n*** flow: %s ***\r\n", opts.flow_str.toLocal8Bit().data());
					break;
				case KEY_PARITY:
                    parity_next();
                    fd_printf(STO, "\r\n*** parity: %s ***\r\n", opts.parity_str.toLocal8Bit().data());
					break;
				case KEY_BITS:
                    bits_next();
                    fd_printf(STO, "\r\n*** databits: %d ***\r\n", opts.databits);
					break;
				case KEY_LECHO:
					opts.lecho = ! opts.lecho;
					fd_printf(STO, "\r\n*** local echo: %s ***\r\n", 
							  opts.lecho ? "yes" : "no");
					break;
				case KEY_SEND:
					fd_printf(STO, "\r\n*** file: ");
					r = fd_readline(STI, STO, fname, sizeof(fname));
					fd_printf(STO, "\r\n");
					if ( r < -1 && errno == EINTR ) break;
					if ( r <= -1 )
                    {
						fatal("cannot read filename: %s", strerror(errno));
                        emit wantsToQuit();
                    }
                    run_cmd(tty_fd, opts.send_cmd.toLocal8Bit().data(), fname, NULL);
					break;
				case KEY_RECEIVE:
					fd_printf(STO, "*** file: ");
					r = fd_readline(STI, STO, fname, sizeof(fname));
					fd_printf(STO, "\r\n");
					if ( r < -1 && errno == EINTR ) break;
					if ( r <= -1 )
                    {
						fatal("cannot read filename: %s", strerror(errno));
                        emit wantsToQuit();
                    }
					if ( fname[0] )
                        run_cmd(tty_fd, opts.receive_cmd.toLocal8Bit().data(), fname, NULL);
					else
                        run_cmd(tty_fd, opts.receive_cmd.toLocal8Bit().data(), NULL);
					break;
				case KEY_BREAK:
					term_break(tty_fd);
					fd_printf(STO, "\r\n*** break sent ***\r\n");
					break;
                case KEY_HELP:
                    show_help();
                    break;
				default:
					break;
				}
				break;

			case ST_TRANSPARENT:
				if ( c == opts.escape ) {
					state = ST_COMMAND;
				} else {
					if (tty_q.len + M_MAXMAP <= TTY_Q_SZ) {
						n = do_map((char *)tty_q.buff + tty_q.len, 
								   opts.omap, c);
						tty_q.len += n;
						if ( opts.lecho ) 
							map_and_write(STO, opts.emap, c);
					} else 
						fd_printf(STO, "\x07");
				}
				break;

			default:
				assert(0);
				break;
			}
		}
	skip_proc_STI:

		if ( FD_ISSET(tty_fd, &rdset) ) {

			/* read from port */

            do
            {
				n = read(tty_fd, &c, 1);
			} while (n < 0 && errno == EINTR);

            if (n == 0)
            {
				fatal("term closed");
                emit killMeNow();
            }
            else if ( n < 0 )
            {
				if ( errno != EAGAIN && errno != EWOULDBLOCK )
                {
					fatal("read from term failed: %s", strerror(errno));
                    emit wantsToQuit();
                }
            }
            else
            {
				map_and_write(STO, opts.imap, c);
			}
		}

		if ( FD_ISSET(tty_fd, &wrset) ) {

			/* write to port */

			do {
				n = write(tty_fd, tty_q.buff, tty_q.len);
			} while ( n < 0 && errno == EINTR );
			if ( n <= 0 )
            {
				fatal("write to term failed: %s", strerror(errno));
                emit wantsToQuit();
            }
			memcpy(tty_q.buff, tty_q.buff + n, tty_q.len - n);
			tty_q.len -= n;
		}
	}
}

/**********************************************************************/

void deadly_handler(int signum)
{
    (void) signum;

	kill(0, SIGTERM);
	sleep(1);
	exit(EXIT_FAILURE);
}

void Picocom::establish_signal_handlers (void)
{
    struct sigaction exit_action, ign_action;

    /* Set up the structure to specify the exit action. */
    exit_action.sa_handler = deadly_handler;
    sigemptyset (&exit_action.sa_mask);
    exit_action.sa_flags = 0;

    /* Set up the structure to specify the ignore action. */
    ign_action.sa_handler = SIG_IGN;
    sigemptyset (&ign_action.sa_mask);
    ign_action.sa_flags = 0;

    sigaction (SIGTERM, &exit_action, NULL);

    sigaction (SIGINT, &ign_action, NULL);
    sigaction (SIGHUP, &ign_action, NULL);
    sigaction (SIGALRM, &ign_action, NULL);
    sigaction (SIGUSR1, &ign_action, NULL);
    sigaction (SIGUSR2, &ign_action, NULL);
    sigaction (SIGPIPE, &ign_action, NULL);
}

/**********************************************************************/

void Picocom::show_usage(char *name)
{
	char *s;

	s = strrchr(name, '/');
	s = s ? s+1 : name;

	printf("picocom v%s\n", VERSION_STR);
	printf("Usage is: %s [options] <tty device>\n", s);
	printf("Options are:\n");
	printf("  --<b>aud <baudrate>\n");
	printf("  --<f>low s (=soft) | h (=hard) | n (=none)\n");
	printf("  --<p>arity o (=odd) | e (=even) | n (=none)\n");
	printf("  --<d>atabits 5 | 6 | 7 | 8\n");
	printf("  --<e>scape <char>\n");
	printf("  --e<c>ho\n");
	printf("  --no<i>nit\n");
	printf("  --no<r>eset\n");
	printf("  --<s>end-cmd <command>\n");
	printf("  --recei<v>e-cmd <command>\n");
	printf("  --imap <map> (input mappings)\n");
	printf("  --omap <map> (output mappings)\n");
	printf("  --emap <map> (local-echo mappings)\n");
    printf("  --dbus (start tohcom over dbus)\n");
	printf("  --<h>elp\n");
	printf("<map> is a comma-separated list of one or more of:\n");
	printf("  crlf : map CR --> LF\n");
	printf("  crcrlf : map CR --> CR + LF\n");
	printf("  igncr : ignore CR\n");
	printf("  lfcr : map LF --> CR\n");
	printf("  lfcrlf : map LF --> CR + LF\n");
	printf("  ignlf : ignore LF\n");
	printf("  bsdel : map BS --> DEL\n");
	printf("  delbs : map DEL --> BS\n");
	printf("<?> indicates the equivalent short option.\n");
	printf("Short options are prefixed by \"-\" instead of by \"--\".\n");
}

/**********************************************************************/

void Picocom::parse_args(int argc, char *argv[])
{
	static struct option longOptions[] =
	{
		{"receive-cmd", required_argument, 0, 'v'},
		{"send-cmd", required_argument, 0, 's'},
        {"imap", required_argument, 0, 'I' },
        {"omap", required_argument, 0, 'O' },
        {"emap", required_argument, 0, 'E' },
		{"escape", required_argument, 0, 'e'},
		{"echo", no_argument, 0, 'c'},
		{"noinit", no_argument, 0, 'i'},
		{"noreset", no_argument, 0, 'r'},
		{"flow", required_argument, 0, 'f'},
		{"baud", required_argument, 0, 'b'},
		{"parity", required_argument, 0, 'p'},
		{"databits", required_argument, 0, 'd'},
		{"help", no_argument, 0, 'h'},
        {"dbus", no_argument, 0, 'D'},
		{0, 0, 0, 0}
	};

	while (1) {
		int optionIndex = 0;
		int c;
		int map;

		/* no default error messages printed. */
		opterr = 0;

		c = getopt_long(argc, argv, "hirlcv:s:r:e:f:b:p:d:",
						longOptions, &optionIndex);

		if (c < 0)
			break;

		switch (c) {
		case 's':
            opts.send_cmd = QString(optarg);
            //strncpy(opts.send_cmd, optarg, sizeof(opts.send_cmd));
			opts.send_cmd[sizeof(opts.send_cmd) - 1] = '\0';
			break;
		case 'v':
            opts.receive_cmd = QString(optarg);
            //strncpy(opts.receive_cmd, optarg, sizeof(opts.receive_cmd));
			opts.receive_cmd[sizeof(opts.receive_cmd) - 1] = '\0';
			break;
		case 'I':
			map = parse_map(optarg);
			if (map >= 0) opts.imap = map;
			else fprintf(stderr, "Invalid imap, ignored\n");
			break;
		case 'O':
			map = parse_map(optarg);
			if (map >= 0) opts.omap = map;
			else fprintf(stderr, "Invalid omap, ignored\n");
			break;
		case 'E':
			map = parse_map(optarg);
			if (map >= 0) opts.emap = map;
			else fprintf(stderr, "Invalid emap, ignored\n");
			break;
		case 'c':
			opts.lecho = 1;
			break;
		case 'i':
			opts.noinit = 1;
			break;
		case 'r':
			opts.noreset = 1;
			break;
		case 'e':
			opts.escape = optarg[0] & 0x1f;
			break;
		case 'f':
			switch (optarg[0]) {
			case 'X':
			case 'x':
				opts.flow_str = "xon/xoff";
				opts.flow = FC_XONXOFF;
				break;
			case 'H':
			case 'h':
				opts.flow_str = "RTS/CTS";
				opts.flow = FC_RTSCTS;
				break;
			case 'N':
			case 'n':
				opts.flow_str = "none";
				opts.flow = FC_NONE;
				break;
			default:
				fprintf(stderr, "--flow '%c' ignored.\n", optarg[0]);
				fprintf(stderr, "--flow can be one off: 'x', 'h', or 'n'\n");
				break;
			}
			break;
		case 'b':
			opts.baud = atoi(optarg);
			break;
		case 'p':
			switch (optarg[0]) {
			case 'e':
				opts.parity_str = "even";
				opts.parity = P_EVEN;
				break;
			case 'o':
				opts.parity_str = "odd";
				opts.parity = P_ODD;
				break;
			case 'n':
				opts.parity_str = "none";
				opts.parity = P_NONE;
				break;
			default:
				fprintf(stderr, "--parity '%c' ignored.\n", optarg[0]);
				fprintf(stderr, "--parity can be one off: 'o', 'e', or 'n'\n");
				break;
			}
			break;
		case 'd':
			switch (optarg[0]) {
			case '5':
				opts.databits = 5;
				break;
			case '6':
				opts.databits = 6;
				break;
			case '7':
				opts.databits = 7;
				break;
			case '8':
				opts.databits = 8;
				break;
			default:
				fprintf(stderr, "--databits '%c' ignored.\n", optarg[0]);
				fprintf(stderr, "--databits can be one off: 5, 6, 7 or 8\n");
				break;
			}
			break;
        case 'D':
            opts.dbus = 1;
            break;
		case 'h':
			show_usage(argv[0]);
			exit(EXIT_SUCCESS);
		case '?':
		default:
			fprintf(stderr, "Unrecognized option.\n");
			fprintf(stderr, "Run with '--help'.\n");
			exit(EXIT_FAILURE);
		}
	} /* while */

    /* we get port from dbus if so said */
    if (!opts.dbus)
    {
        if ( (argc - optind) < 1)
        {
            fprintf(stderr, "No port given\n");
            exit(EXIT_FAILURE);
        }
        opts.port = QString(argv[optind]);
    }

}

/**********************************************************************/


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Picocom *picocom = new Picocom();
    QThread *thread = new QThread();

    int ret = EXIT_SUCCESS;

    picocom->parse_args(argc, argv);

    if (picocom->opts.dbus)
    {
        QString daemonVersion = picocom->readFromDaemon("getVersion");

        if (daemonVersion != QString())
        {
            printf("Tohcom Daemon version: %s\n", qPrintable(daemonVersion));
            QThread::msleep(300);
            picocom->opts.port = picocom->readFromDaemon("getPseudoDevice");
        }
        else
        {
            printf("Daemon not running.\n");
            ret = EXIT_FAILURE;
        }
    }

    if (ret == EXIT_SUCCESS)
        if (picocom->init())
        {
            picocom->moveToThread(thread);

            QObject::connect(thread, SIGNAL(started()), picocom, SLOT(loop()));
            QObject::connect(picocom, SIGNAL(wantsToQuit()), picocom, SLOT(deleteLater()), Qt::DirectConnection);
            QObject::connect(picocom, SIGNAL(killMeNow()), &app, SLOT(quit()), Qt::DirectConnection);
            QObject::connect(picocom, SIGNAL(destroyed()), &app, SLOT(quit()), Qt::DirectConnection);

            thread->start();

            ret = app.exec();
        }

    return ret;
}

/**********************************************************************/
