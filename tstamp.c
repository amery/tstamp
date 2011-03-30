/* this file belongs to `tstamp` http://github.com/amery/tstamp
 *
 * Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>
 * All rights reserved. See COPYING for details.
 ***/

#define _POSIX_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include <time.h>
#include <sys/time.h>

#include <sys/uio.h>

#include "tstamp.h"

#define FMT_SHOW_TIME		(1 << 0)
#define FMT_SHOW_DATE		(1 << 1)

#define FMT_DATE	"%04u-%02u-%02u %02u:%02u:%02u.%06u"
#define FMT_TIME	(FMT_DATE + 15)

static struct tm *(*gen_tm)(const time_t *, struct tm *);
static size_t (*fmt_tstamp)(char *, size_t, time_t, suseconds_t);

static inline void write_all(struct iovec *iov, int iovcnt)
{
	while (iovcnt > 0) {
		ssize_t l = writev(STDOUT_FILENO, iov, iovcnt);
		if (l < 0 && errno != EAGAIN && errno != EINTR) {
			perror("writev");
			_exit(errno);
		}

		while (l > 0 && iovcnt > 0) {
			if ((unsigned)l < iov->iov_len) {
				iov->iov_len -= l;
				iov->iov_base += l;
			} else {
				l -= iov->iov_len;
				iovcnt--;
				iov++;
			}
		}

	}
}

static size_t fmt_tstamp_decimal(char *buf, size_t buf_len, time_t sec, suseconds_t usec)
{
	return snprintf(buf, buf_len, "%u.%06u",
			(unsigned)sec,
			(unsigned)usec);
}

static size_t fmt_tstamp_date(char *buf, size_t buf_len, time_t sec, suseconds_t usec)
{
	struct tm tm;
	gen_tm(&sec, &tm);

	return snprintf(buf, buf_len, FMT_DATE,
			1900+tm.tm_year, 1+tm.tm_mon, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			(unsigned)usec);
}

static size_t fmt_tstamp_time(char *buf, size_t buf_len, time_t sec, suseconds_t usec)
{
	struct tm tm;
	gen_tm(&sec, &tm);

	return snprintf(buf, buf_len, FMT_TIME,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			(unsigned)usec);
}

static inline void dump(FILE *f, /*const*/ char *del)
{
	char buf[MAXBUF];
	char tstamp_buf[32];

	struct iovec iov[] = {
		{tstamp_buf, 0},
		{del, strlen(del)},
		{buf, 0} };

	while (fgets(buf, sizeof(buf), f) != NULL) {
		struct timeval timeval;
		gettimeofday(&timeval, NULL);

		iov[0].iov_len = fmt_tstamp(tstamp_buf, sizeof(tstamp_buf),
					    timeval.tv_sec,
					    timeval.tv_usec);
		iov[2].iov_len = strlen(buf);

		write_all(iov, 3);
	}
}

int main(int argc, char **argv)
{
	int opt;
	char *del = ": ";
	unsigned flag = 0;

	gen_tm = gmtime_r;

	while ((opt = getopt(argc, argv, "?VltTd:")) != -1) {
		switch (opt) {
		case 'V':
			fputs("tstamp v" VERSION " <" HOME ">\n"
			      DESCRIPTION "\n"
			      "Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>\n",
			      stderr);
			return 0;
		case 'l':
			gen_tm = localtime_r;
		case 't':
			flag |= FMT_SHOW_TIME;
			break;
		case 'T':
			flag |= FMT_SHOW_DATE;
			break;
		case 'd':
			del = optarg;
			break;
		default:
			fprintf(stderr,
				"tstamp v" VERSION "\n" DESCRIPTION "\n\n"
				"Usage: %s [-V] [-ltT] [-d <del>]\n\n"
				"  -V   print version and exit\n"
				"  -d   delimiter (default: \": \")\n"
				"  -t   show time\n"
				"  -T   show date and time\n"
				"  -l   use localtime instead of GMT/UTC/Z\n",
				argv[0]);
			return 1;
		}
	}

	if (flag & FMT_SHOW_DATE)
		fmt_tstamp = fmt_tstamp_date;
	else if (flag & FMT_SHOW_TIME)
		fmt_tstamp = fmt_tstamp_time;
	else
		fmt_tstamp = fmt_tstamp_decimal;

	dump(stdin, del);
	return 0;
}
