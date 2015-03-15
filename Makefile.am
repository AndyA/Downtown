DIST_SUBDIRS = m4 t

EXTRA_DIST = config misc

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libdowntown.la

bin_PROGRAMS = downtown downtown_filter

libdowntown_la_SOURCES = \
	colour.h colour.c \
	delta.h delta.c \
	log.h log.c \
	merge.h merge.c \
	quadtree.h quadtree.c \
	sampler.h sampler.c \
	util.h util.c \
	voronoi.h voronoi.c \
	yuv4mpeg2.h yuv4mpeg2.c \
	zigzag.h zigzag.c

libsoundchip_la_CPPFLAGS = $(FFTW_CFLAGS)
libdowntown_la_LDFLAGS = -avoid-version -static $(FFTW_LIBS)

downtown_SOURCES = downtown.c
downtown_LDADD = libdowntown.la

downtown_filter_SOURCES = downtown_filter.c
downtown_filter_LDADD = libdowntown.la

test: all
	cd t && $(MAKE) test

