libexecdir = @libexecdir@/nullmailer

libexec_PROGRAMS = smtp qmqp
CXXLINK = $(CC) $(CXXFLAGS) $(LDFLAGS) -o $@
INCLUDES = -I../lib -I../lib/cli

smtp_SOURCES = smtp.cc protocol.cc protocol.h
smtp_LDADD = ../lib/libnullmailer.a ../lib/cli/libcli.a

qmqp_SOURCES = qmqp.cc protocol.cc protocol.h
qmqp_LDADD = ../lib/libnullmailer.a ../lib/cli/libcli.a
