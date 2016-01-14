noinst_PROGRAMS = address-test argparse-test clitest0 clitest1
EXTRA_DIST = address-trace.cc argparse.cc clitest.cc clitest.sh runtests.in
noinst_SCRIPTS = runtests
CLEANFILES = runtests

AM_CPPFLAGS = -I../lib

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

runtests: runtests.in Makefile
	sed -e 's,[@]PREFIX[@],$(prefix),g; s,[@]SRCDIR[@],$(abs_top_srcdir),g;' < $< > $@
	chmod +x $@

# The following makes sure that we can't produce a package without the
# tests executing properly
dist-hook: test
	cp -r tests $(distdir)

test: all
	./address-test
	./argparse-test
	sh clitest.sh
	./runtests `find tests -type f -not -name '.*.swp'`
