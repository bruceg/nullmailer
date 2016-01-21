libexecdir = @libexecdir@/nullmailer

libexec_PROGRAMS = smtp qmqp
AM_CPPFLAGS = -I$(top_srcdir)/lib

if TLS
TLS_SOURCES = tls_gnutls.cc
TLS_LDADD = -lgnutls
else
TLS_SOURCES = tls_none.cc
TLS_LDADD =
endif

smtp_SOURCES = smtp.cc protocol.cc $(TLS_SOURCES) protocol.h
smtp_LDADD = ../lib/cli++/libcli++.a ../lib/libnullmailer.a $(TLS_LDADD)

qmqp_SOURCES = qmqp.cc protocol.cc $(TLS_SOURCES) protocol.h
qmqp_LDADD = ../lib/cli++/libcli++.a ../lib/libnullmailer.a $(TLS_LDADD)
