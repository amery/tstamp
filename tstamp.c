/* this file belongs to `tstamp` http://github.com/amery/tstamp
 *
 * Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>
 * All rights reserved. See COPYING for details.
 ***/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

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

static inline void dump(FILE *f)
{
	char buf[MAXBUF];
	struct iovec iov[] = { {": ",2}, {buf,0} };

	while (fgets(buf, sizeof(buf), f) != NULL) {
		size_t l = strlen(buf);

		iov[1].iov_len = l;

		write_all(iov, 2);
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

	dump(stdin);
	return 0;
}
