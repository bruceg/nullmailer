SUBDIRS = cli++ fdbuf mystring
noinst_LIBRARIES = libmisc.a libnullmailer.a
noinst_HEADERS = list.h
EXTRA_DIST = make_defines.sh listtest.cc mergelib.sh
CLEANFILES = defines.cc

libmisc_a_SOURCES = \
	ac/dirent.h ac/time.h ac/wait.h \
	address.h address.cc \
	argparse.h argparse.cc \
	autoclose.h \
	base64.h base64.cc \
	canonicalize.h canonicalize.cc \
	configio.h config_path.cc \
	config_read.cc config_readlist.cc config_readint.cc config_syserr.cc \
	connect.h tcpconnect.cc \
	defines.h \
	errcodes.h errcodes.cc \
	hostname.h hostname.cc \
	itoa.h itoa.cc \
	makefield.cc makefield.h \
	netstring.h netstring.cc \
	forkexec.cc forkexec.h \
	selfpipe.cc selfpipe.h \
	setenv.cc setenv.h
nodist_libmisc_a_SOURCES = defines.cc

libnullmailer_a_SOURCES =
libnullmailer.a: mergelib.sh libmisc.a fdbuf/libfdbuf.a \
	mystring/libmystring.a Makefile
	$(RM) -f libnullmailer.a
	sh $(srcdir)/mergelib.sh libnullmailer.a \
		libmisc.a \
		fdbuf/libfdbuf.a \
		mystring/libmystring.a

defines.cc: Makefile make_defines.sh
	@echo Creating defines.cc
	@sh $(srcdir)/make_defines.sh \
		@localstatedir@/spool/nullmailer \
		@sysconfdir@/nullmailer \
		@libexecdir@/nullmailer \
		@bindir@ \
		@sbindir@
