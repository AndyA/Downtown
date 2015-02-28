DIST_SUBDIRS = m4 t

EXTRA_DIST = config misc

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libdowntown.la

bin_PROGRAMS = downtown

libdowntown_la_SOURCES = \
	average.h average.c \
	colour.h colour.c \
	filter.h filter.c \
	hist.h hist.c \
	histogram.h histogram.c \
	massive.h massive.c \
	merge.h merge.c \
	minmax.h minmax.c \
	model.h model.c \
	pass.h pass.c \
	peak.h peak.c \
	streak.h streak.c \
	stretch.h stretch.c \
	yuv4mpeg2.h yuv4mpeg2.c

libdowntown_la_LDFLAGS = -avoid-version -static

downtown_SOURCES = downtown.c

downtown_LDADD = libdowntown.la

test: all
	cd t && $(MAKE) test

