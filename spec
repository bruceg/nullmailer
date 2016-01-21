Name: nullmailer
Summary: Simple relay-only mail transport agent
Version: @VERSION@
Release: 1
License: GPL
Group: Networking/Daemons
Source: http://untroubled.org/nullmailer/archive/%{version}/nullmailer-%{version}.tar.gz
BuildRoot: /tmp/nullmailer-root
URL: http://untroubled.org/nullmailer/
Packager: Bruce Guenter <bruce@untroubled.org>
Provides: smtpdaemon
Conflicts: sendmail
Conflicts: qmail
Requires: supervise-scripts >= 3.2
Requires: gnutls
BuildRequires: gnutls-devel
Requires(pre,preun): shadow-utils

%description
Nullmailer is a mail transport agent designed to only relay all its
messages through a fixed set of "upstream" hosts.  It is also designed
to be secure.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" \
./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var --enable-tls

make

%install
rm -fr $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/var/service/nullmailer/log
mkdir -p $RPM_BUILD_ROOT/var/log/nullmailer

make DESTDIR=$RPM_BUILD_ROOT install-strip
ln -s ../sbin/sendmail $RPM_BUILD_ROOT/usr/lib/sendmail
install scripts/nullmailer.run $RPM_BUILD_ROOT/var/service/nullmailer/run
install scripts/nullmailer-log.run $RPM_BUILD_ROOT/var/service/nullmailer/log/run

%clean
rm -rf $RPM_BUILD_ROOT

%pre
PATH="/sbin:/usr/sbin:$PATH" export PATH
if [ "$1" = 1 ]; then
	# pre-install instructions
	grep ^nullmail: /etc/group >/dev/null || groupadd -r nullmail
	grep ^nullmail: /etc/passwd >/dev/null || useradd -d /var/lock/svc/nullmailer -g nullmail -M -r -s /bin/true nullmail
fi

%post
if ! [ -L /service/nullmailer ]; then
	svc-add /var/service/nullmailer
fi
if ! [ -s /etc/nullmailer/me ]; then
	/bin/hostname --fqdn >/etc/nullmailer/me
fi
if ! [ -s /etc/nullmailer/defaultdomain ]; then
	/bin/hostname --domain >/etc/nullmailer/defaultdomain
fi

%preun
if [ "$1" = 0 ]; then
	svc-remove nullmailer
fi

%postun
if [ "$1" = 0 ]; then
	# post-erase instructions
	/usr/sbin/userdel nullmail
	/usr/sbin/groupdel nullmail
fi

%files
%defattr(-,nullmail,nullmail)
%doc AUTHORS BUGS ChangeLog COPYING INSTALL NEWS README TODO doc/DIAGRAM
%dir /etc/nullmailer
%attr(04711,nullmail,nullmail) /usr/bin/mailq
/usr/bin/nullmailer-inject
/usr/bin/nullmailer-smtpd
/usr/lib/sendmail
%dir /usr/libexec/nullmailer
/usr/libexec/nullmailer/*
%{_mandir}/*/*
%attr(04711,nullmail,nullmail) /usr/sbin/nullmailer-queue
/usr/sbin/nullmailer-send
/usr/sbin/sendmail
%dir /var/log/nullmailer
/var/service/nullmailer
/var/spool/nullmailer
