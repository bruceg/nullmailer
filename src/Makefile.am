bin_PROGRAMS = \
	mailq \
	nullmailer-dsn \
	nullmailer-inject \
	nullmailer-smtpd
sbin_PROGRAMS = \
	nullmailer-queue \
	nullmailer-send \
	sendmail

#noinst_PROGRAMS = address

AM_CPPFLAGS = -I$(top_srcdir)/lib

mailq_SOURCES = mailq.cc
mailq_LDADD = ../lib/libnullmailer.a

nullmailer_dsn_SOURCES = dsn.cc
nullmailer_dsn_LDADD = ../lib/libnullmailer.a ../lib/cli++/libcli++.a

nullmailer_inject_SOURCES = inject.cc
nullmailer_inject_LDADD = ../lib/libnullmailer.a ../lib/cli++/libcli++.a

nullmailer_queue_SOURCES = queue.cc
nullmailer_queue_LDADD = ../lib/libnullmailer.a ../lib/cli++/libcli++.a

nullmailer_send_SOURCES = send.cc
nullmailer_send_LDADD = ../lib/libnullmailer.a ../lib/cli++/libcli++.a

nullmailer_smtpd_SOURCES = smtpd.cc
nullmailer_smtpd_LDADD = ../lib/libnullmailer.a

sendmail_SOURCES = sendmail.cc
sendmail_LDADD = ../lib/cli++/libcli++.a ../lib/libnullmailer.a

