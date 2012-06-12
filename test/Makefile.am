noinst_PROGRAMS = address-test clitest0 clitest1
EXTRA_DIST = address-trace.cc clitest.cc clitest.sh

INCLUDES = -I../lib

address_test_SOURCES = address-test.cc # address-trace.cc
address_test_LDADD = ../lib/libnullmailer.a

clitest0_CPPFLAGS = -DCLI_ONLY_LONG=false
clitest0_SOURCES = clitest.cc
clitest0_LDADD = ../lib/libnullmailer.a ../lib/cli++/libcli++.a

clitest1_CPPFLAGS = -DCLI_ONLY_LONG=true
clitest1_SOURCES = clitest.cc
clitest1_LDADD = ../lib/libnullmailer.a ../lib/cli++/libcli++.a

# The following makes sure that we can't produce a package without the
# tests executing properly
dist-hook: test
	cp -r tests $(distdir)

test: all
	./address-test
	sh clitest.sh
	@failed=0; \
	for test in `find tests -type f -not -name '.*.swp'`; do \
		echo Running test $$test...; \
		if env - SYSCONFDIR=$(sysconfdir)/nullmailer bash $$test; \
			then echo 'Done.'; \
			else echo '******************************Failed!******************************'; failed=1; \
		fi; \
	done 2>&1; exit $$failed
