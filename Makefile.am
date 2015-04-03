DIST_SUBDIRS = m4 t

EXTRA_DIST = config misc

SUBDIRS = . t

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libdowntown.la

bin_PROGRAMS =                \
	confound                     \
	downtown                     \
	downtown-sig                 \
	downtown-filter              \
	get-stats                    \
	test-convolve                \
	test-filters                 \
	test-timebend                \
	testcard-yuv

libdowntown_la_SOURCES =      \
	average.h average.c          \
	bytelist.h bytelist.c        \
	centre.h centre.c            \
	charlist.h charlist.c        \
	colour.h colour.c            \
	csv.h csv.c                  \
	delay.h delay.c              \
	delta.h delta.c              \
	downtown.h downtown-core.c   \
	dumpframe.h dumpframe.c      \
	frameinfo.h frameinfo.c      \
	histogram.h histogram.c      \
	injector.h injector.c        \
	json.h json.c                \
	log.h log.c                  \
	merge.h merge.c              \
	numlist.h numlist.c          \
	numpipe.h numpipe.c          \
	profile.h profile.c          \
	progress.h progress.c        \
	ptrlist.h ptrlist.c          \
	quadtree.h quadtree.c        \
	resample.h resample.c        \
	sampler.h sampler.c          \
	signature.h signature.c      \
	splitter.h splitter.c        \
	tb_convolve.h tb_convolve.c  \
	timebend.h timebend.c        \
	util.h util.c                \
	voronoi.h voronoi.c          \
	y4m2png.h y4m2png.c          \
	yuv4mpeg2.h yuv4mpeg2.c      \
	zigzag.h zigzag.c

libsoundchip_la_CPPFLAGS = $(FFTW_CFLAGS) $(PNG_CFLAGS)
libdowntown_la_LDFLAGS = -avoid-version -static $(FFTW_LIBS) $(PNG_LIBS)

confound_LDADD = libdowntown.la
confound_SOURCES = confound.c

downtown_LDADD = libdowntown.la
downtown_SOURCES = downtown.c

downtown_filter_LDADD = libdowntown.la
downtown_filter_SOURCES = downtown-filter.c

downtown_sig_LDADD = libdowntown.la
downtown_sig_SOURCES = downtown-sig.c

get_stats_LDADD = libdowntown.la
get_stats_SOURCES = get-stats.c

test_convolve_LDADD = libdowntown.la
test_convolve_SOURCES = test-convolve.c

test_filters_LDADD = libdowntown.la
test_filters_SOURCES = test-filters.c

test_timebend_LDADD = libdowntown.la
test_timebend_SOURCES = test-timebend.c

testcard_yuv_LDADD = libdowntown.la
testcard_yuv_SOURCES = testcard-yuv.c

test: all
	cd t && $(MAKE) test

