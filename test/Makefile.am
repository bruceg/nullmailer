noinst_PROGRAMS = address-test
EXTRA_DIST = address-trace.cc

INCLUDES = -I../lib

address_test_SOURCES = address-test.cc # address-trace.cc
address_test_LDADD = ../lib/libnullmailer.a

# The following makes sure that we can't produce a package without the
# tests executing properly
dist-hook: test
	cp -r tests $(distdir)

test: all
	./address-test
	@failed=0; \
	for test in `find tests -type f -not -name '.*.swp'`; do \
		echo Running test $$test...; \
		if env - SYSCONFDIR=$(sysconfdir)/nullmailer bash $$test; \
			then echo 'Done.'; \
			else echo '******************************Failed!******************************'; failed=1; \
		fi; \
	done 2>&1; exit $$failed
