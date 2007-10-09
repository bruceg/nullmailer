SUBDIRS = cli++ fdbuf mystring
noinst_LIBRARIES = libmisc.a libnullmailer.a
noinst_HEADERS = list.h
EXTRA_DIST = make_defines.sh listtest.cc mergelib.sh
CLEANFILES = defines.cc

libmisc_a_SOURCES = \
	address.h address.cc \
	base64.h base64.cc \
	canonicalize.h canonicalize.cc \
	configio.h config_read.cc config_readlist.cc config_readint.cc \
	connect.h tcpconnect.cc \
	defines.h defines.cc \
	errcodes.h errcodes.cc \
	hostname.h hostname.cc \
	itoa.h itoa.cc \
	makefield.cc makefield.h \
	netstring.h netstring.cc \
	selfpipe.cc selfpipe.h \
	setenv.cc setenv.h

libnullmailer_a_SOURCES =
libnullmailer.a: mergelib.sh libmisc.a fdbuf/libfdbuf.a \
	mystring/libmystring.a Makefile
	$(RM) -f libnullmailer.a
	sh mergelib.sh libnullmailer.a \
		libmisc.a \
		fdbuf/libfdbuf.a \
		mystring/libmystring.a

defines.cc: Makefile.am make_defines.sh
	@echo Creating defines.cc
	@sh make_defines.sh \
		@localstatedir@/nullmailer \
		@sysconfdir@/nullmailer \
		@libexecdir@/nullmailer \
		@bindir@ \
		@sbindir@

dist-hook:
	$(RM) -f $(distdir)/defines.cc
	mkdir $(distdir)/ac
	cp -dP `find ac -name CVS -prune -o -type f -print` $(distdir)/ac/
