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

static size_t fmt_tstamp(char *buf, size_t buf_len)
{
	struct timeval timeval;
	struct tm tm;

	gettimeofday(&timeval, NULL);
	localtime_r(&timeval.tv_sec, &tm);

	return snprintf(buf, buf_len, "%02u:%02u:%02u.%06lu",
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec,
			(unsigned long)timeval.tv_usec);
}

static inline void dump(FILE *f, /*const*/ char *del)
{
	char buf[MAXBUF];
	char tstamp_buf[20];

	struct iovec iov[] = {
		{tstamp_buf, 0},
		{del, strlen(del)},
		{buf, 0} };

	while (fgets(buf, sizeof(buf), f) != NULL) {
		iov[0].iov_len = fmt_tstamp(tstamp_buf, sizeof(tstamp_buf));
		iov[2].iov_len = strlen(buf);

		write_all(iov, 3);
	}
}

int main(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "?V")) != -1) {
		switch (opt) {
		case 'V':
			fputs("tstamp v" VERSION " <" HOME ">\n"
			      DESCRIPTION "\n"
			      "Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>\n",
			      stderr);
			return 0;
		default:
			fprintf(stderr,
				"tstamp v" VERSION "\n" DESCRIPTION "\n\n"
				"Usage: %s [-V]\n\n"
				"  -V   print version and exit\n",
				argv[0]);
			return 1;
		}
	}

	dump(stdin, ": ");
	return 0;
}
