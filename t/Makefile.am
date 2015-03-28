TESTBIN =                     \
	average                      \
	basic                        \
	bytelist                     \
	bytelist-one                 \
	charlist                     \
	colour                       \
	csv                          \
	numlist                      \
	numpipe                      \
	quadtree                     \
	resample                     \
	sampler                      \
	tb_convolve                  \
	util                         \
	yuv4mpeg2                    \
	zigzag

TESTPERL =

noinst_PROGRAMS = wrap $(TESTBIN)

noinst_LTLIBRARIES = libtest-support.la

libtest_support_la_SOURCES =  \
	tap.c tap.h framework.c framework.h

libtest_support_la_LDFLAGS = -static

LDADD = libtest-support.la ../libdowntown.la

test: $(TESTBIN)
	prove -e ./wrap $(TESTBIN) $(TESTPERL)

valgrind: $(TESTBIN)
	prove -e 'valgrind -q' $(addprefix ./,$(TESTBIN))
