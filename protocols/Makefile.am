libexecdir = @libexecdir@/nullmailer

libexec_PROGRAMS = smtp qmqp
INCLUDES = -I../lib -I../lib/cli++

smtp_SOURCES = smtp.cc protocol.cc protocol.h
smtp_LDADD = ../lib/cli++/libcli++.a ../lib/libnullmailer.a

qmqp_SOURCES = qmqp.cc protocol.cc protocol.h
qmqp_LDADD = ../lib/cli++/libcli++.a ../lib/libnullmailer.a
