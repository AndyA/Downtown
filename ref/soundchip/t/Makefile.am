TESTBIN = basic

noinst_PROGRAMS = wrap $(TESTBIN)

noinst_LTLIBRARIES = libtest-support.la

libtest_support_la_SOURCES = tap.c tap.h
libtest_support_la_LDFLAGS = -static

LDADD = libtest-support.la ../libsoundchip.la
AM_CPPFLAGS = $(FFTW_CFLAGS) $(SNDFILE_CFLAGS) $(LIBPNG_CFLAGS)
AM_LDFLAGS = $(FFTW_LIBS) $(SNDFILE_LIBS) $(LIBPNG_LIBS)

wrap_LDADD =
wrap_LDFLAGS =
wrap_CFLAGS =

test: $(TESTBIN)
	prove -e ./wrap $(TESTBIN) $(TESTPERL)

valgrind: $(TESTBIN)
	prove -e 'valgrind -q' $(addprefix ./,$(TESTBIN))
