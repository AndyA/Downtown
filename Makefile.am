DIST_SUBDIRS = m4 t

EXTRA_DIST = config misc

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libdowntown.la

bin_PROGRAMS = downtown downtown_filter testcard-yuv test-timebend test-filters get-stats

libdowntown_la_SOURCES = \
	average.h average.c \
	centre.h centre.c \
	colour.h colour.c \
	delay.h delay.c \
	delta.h delta.c \
	dumpframe.h dumpframe.c \
	frameinfo.h frameinfo.c \
	histogram.h histogram.c \
	injector.h injector.c \
	log.h log.c \
	merge.h merge.c \
	numlist.h numlist.c \
	numpipe.h numpipe.c \
	progress.h progress.c \
	quadtree.h quadtree.c \
	resample.h resample.c \
	sampler.h sampler.c \
	splitter.h splitter.c \
	tb_convolve.h tb_convolve.c \
	timebend.h timebend.c \
	util.h util.c \
	voronoi.h voronoi.c \
	y4m2png.h y4m2png.c \
	yuv4mpeg2.h yuv4mpeg2.c \
	zigzag.h zigzag.c

libsoundchip_la_CPPFLAGS = $(FFTW_CFLAGS) $(PNG_CFLAGS)
libdowntown_la_LDFLAGS = -avoid-version -static $(FFTW_LIBS) $(PNG_LIBS)

downtown_SOURCES = downtown.c
downtown_LDADD = libdowntown.la

downtown_filter_SOURCES = downtown_filter.c
downtown_filter_LDADD = libdowntown.la

testcard_yuv_SOURCES = testcard-yuv.c
testcard_yuv_LDADD = libdowntown.la

test_timebend_SOURCES = test-timebend.c
test_timebend_LDADD = libdowntown.la

test_filters_SOURCES = test-filters.c
test_filters_LDADD = libdowntown.la

get_stats_SOURCES = get-stats.c
get_stats_LDADD = libdowntown.la

test: all
	cd t && $(MAKE) test

