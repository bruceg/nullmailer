Name: nullmailer
Summary: Simple relay-only mail transport agent
Version: @VERSION@
Release: 1
Copyright: GPL
Group: Networking/Daemons
Source: http://em.ca/~bruceg/nullmailer/archive/%{version}/nullmailer-%{version}.tar.gz
BuildRoot: /tmp/nullmailer-root
URL: http://em.ca/~bruceg/nullmailer/
Packager: Bruce Guenter <bruceg@em.ca>
Provides: smtpdaemon
Conflicts: sendmail
Requires: supervise-scripts >= 2.2
PreReq: shadow-utils

%description
Nullmailer is a mail transport agent designed to only relay all its
messages through a fixed set of "upstream" hosts.  It is also designed
to be secure.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" \
./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var

make

%install
rm -fr $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/{usr/lib,etc/rc.d/init.d}
mkdir -p $RPM_BUILD_ROOT/var/lock/svc/nullmailer/log
mkdir -p $RPM_BUILD_ROOT/var/log/nullmailer

make DESTDIR=$RPM_BUILD_ROOT install-strip
ln -s ../sbin/sendmail $RPM_BUILD_ROOT/usr/lib/sendmail
install scripts/init $RPM_BUILD_ROOT/etc/rc.d/init.d/nullmailer
install scripts/run-svc $RPM_BUILD_ROOT/var/lock/svc/nullmailer/run
install scripts/run-log $RPM_BUILD_ROOT/var/lock/svc/nullmailer/log/run

%clean
rm -rf $RPM_BUILD_ROOT

%pre
PATH="/sbin:/usr/sbin:$PATH" export PATH
case "$1" in
1)
	# pre-install instructions
	grep ^nullmail: /etc/group >/dev/null || groupadd -r nullmail
	grep ^nullmail: /etc/passwd >/dev/null || useradd -d /var/lock/svc/nullmailer -g nullmail -M -r -s /bin/true nullmail
	;;
esac

%post
/sbin/chkconfig --add nullmailer

%preun
/sbin/chkconfig --del nullmailer

%postun
case "$1" in
0)
	# post-erase instructions
	/usr/sbin/userdel nullmail
	/usr/sbin/groupdel nullmail
	;;
esac

%files
%defattr(-,nullmail,nullmail)
%doc AUTHORS BUGS ChangeLog COPYING INSTALL NEWS README TODO YEAR2000
/etc/rc.d/init.d/nullmailer
%dir /etc/nullmailer
%attr(04711,nullmail,nullmail) /usr/bin/mailq
/usr/bin/nullmailer-inject
/usr/lib/sendmail
%dir /usr/libexec/nullmailer
/usr/libexec/nullmailer/*
/usr/man/man1/*
/usr/man/man8/*
%attr(04711,nullmail,nullmail) /usr/sbin/nullmailer-queue
/usr/sbin/nullmailer-send
/usr/sbin/sendmail
%dir /var/lock/svc/nullmailer
/var/lock/svc/nullmailer/run
%dir /var/lock/svc/nullmailer/log
/var/lock/svc/nullmailer/log/run
%dir /var/log/nullmailer
%dir /var/nullmailer
%dir /var/nullmailer/queue
%dir /var/nullmailer/tmp
/var/nullmailer/trigger
