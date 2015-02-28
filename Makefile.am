DIST_SUBDIRS = m4 t

EXTRA_DIST = config misc

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libdowntown.la

bin_PROGRAMS = downtown

libdowntown_la_SOURCES = \
	colour.h colour.c \
	yuv4mpeg2.h yuv4mpeg2.c \
	zigzag.h zigzag.c

libdowntown_la_LDFLAGS = -avoid-version -static

downtown_SOURCES = downtown.c

downtown_LDADD = libdowntown.la

test: all
	cd t && $(MAKE) test

