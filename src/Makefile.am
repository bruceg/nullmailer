bin_PROGRAMS = \
	mailq \
	nullmailer-inject
sbin_PROGRAMS = \
	nullmailer-queue \
	nullmailer-send \
	sendmail

#noinst_PROGRAMS = address

INCLUDES = -I../lib -I../lib/cli++

mailq_SOURCES = mailq.cc
mailq_LDADD = ../lib/libnullmailer.a

nullmailer_inject_SOURCES = inject.cc
nullmailer_inject_LDADD = ../lib/cli++/libcli++.a ../lib/libnullmailer.a

nullmailer_queue_SOURCES = queue.cc
nullmailer_queue_LDADD = ../lib/libnullmailer.a

nullmailer_send_SOURCES = send.cc
nullmailer_send_LDADD = ../lib/libnullmailer.a

sendmail_SOURCES = sendmail.cc
sendmail_LDADD = ../lib/cli++/libcli++.a ../lib/libnullmailer.a

