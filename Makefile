# this file belongs to `tstamp` http://github.com/amery/tstamp
#
# Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>
# All rights reserved. See COPYING for details.

CC = gcc

CFLAGS = -W -Wall -std=c99
LDFLAGS =

ifeq ($(PROFILE),yes)
CFLAGS += -O2 -g -pg
LDFLAGS += -pg
else
ifeq ($(DEBUG),yes)
CFLAGS += -O2 -g
else
CFLAGS += -O3
endif
endif

APP = tstamp
SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)
OBJS = $(patsubs %.c,%.o,$(SRCS))

DESTDIR =
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

.PHONY: all gen clean install

all: $(APP)

gen: .gitignore

$(APP): $(OBJS)

clean:
	rm -vf $(APP) *.o *~

install:
	chmod 0755 $(APP)
	cp $(APP) $(DESTDIR)$(BINDIR)/

.gitignore: Makefile
	for x in '*.o' $(APP) Makefile.deps; do \
		echo "$$x"; \
	done > $@

Makefile.deps: Makefile $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -MM $(SRCS) > $@
