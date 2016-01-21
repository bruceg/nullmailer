noinst_PROGRAMS = address-test argparse-test clitest0 clitest1
EXTRA_DIST = address-trace.cc clitest.cc clitest.sh functions.in runtests \
	accept-qmqp.sh accept-smtp.sh
noinst_SCRIPTS = functions
CLEANFILES = functions

AM_CPPFLAGS = -I$(top_srcdir)/lib

address_test_SOURCES = address-test.cc # address-trace.cc
address_test_LDADD = ../lib/libnullmailer.a

argparse_test_SOURCES = argparse-test.cc
argparse_test_LDADD = ../lib/libnullmailer.a

clitest0_CPPFLAGS = $(AM_CPPFLAGS) -DCLI_ONLY_LONG=false
clitest0_SOURCES = clitest.cc
clitest0_LDADD = ../lib/libnullmailer.a ../lib/cli++/libcli++.a

clitest1_CPPFLAGS = $(AM_CPPFLAGS) -DCLI_ONLY_LONG=true
clitest1_SOURCES = clitest.cc
clitest1_LDADD = ../lib/libnullmailer.a ../lib/cli++/libcli++.a

functions: functions.in Makefile
	sed -e 's,[@]SRCDIR[@],$(abs_top_srcdir),g; s,[@]BUILDDIR[@],$(abs_top_builddir),g;' < $< > $@

# The following makes sure that we can't produce a package without the
# tests executing properly
dist-hook:
	cp -r $(srcdir)/tests $(distdir)

check: all
	./address-test
	./argparse-test
	sh $(srcdir)/clitest.sh
	$(srcdir)/runtests `find $(abs_srcdir)/tests -type f -not -name '.*'`
