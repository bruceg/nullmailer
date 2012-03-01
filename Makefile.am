localstatedir = @localstatedir@/nullmailer
sysconfdir = @sysconfdir@/nullmailer

EXTRA_DIST = BUGS HOWTO INSTALL TODO scripts/*
SUBDIRS = doc lib protocols src test

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/queue
	chmod 700 $(DESTDIR)$(localstatedir)/queue
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/tmp
	chmod 700 $(DESTDIR)$(localstatedir)/tmp
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)
	$(RM) -f $(DESTDIR)$(localstatedir)/trigger
	mkfifo $(DESTDIR)$(localstatedir)/trigger
	chmod 600 $(DESTDIR)$(localstatedir)/trigger

install-root:
	chown nullmail $(DESTDIR)$(localstatedir)/*
	chown nullmail $(DESTDIR)$(sbindir)/nullmailer-queue
	chmod u+s $(DESTDIR)$(sbindir)/nullmailer-queue
	chown nullmail $(DESTDIR)$(bindir)/mailq
	chmod u+s $(DESTDIR)$(bindir)/mailq

dist-hook:
	sed -e s/@VERSION\@/@VERSION@/ \
		<spec >$(distdir)/nullmailer-@VERSION@.spec
	find $(distdir)/scripts -name '*~' | xargs -r rm
