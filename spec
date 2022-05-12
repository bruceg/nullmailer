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
Requires: gnutls
BuildRequires: gnutls-devel
BuildRequires: systemd-rpm-macros
Requires(pre,preun): shadow-utils
Requires(postun,preun): systemd

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
mkdir -p $RPM_BUILD_ROOT/var/log/nullmailer

make DESTDIR=$RPM_BUILD_ROOT install
ln -s ../sbin/sendmail $RPM_BUILD_ROOT/usr/lib/sendmail
install -D -t $RPM_BUILD_ROOT/usr/lib/systemd/system scripts/nullmailer.service

%clean
rm -rf $RPM_BUILD_ROOT

%pre
PATH="/sbin:/usr/sbin:$PATH" export PATH
if [ "$1" = 1 ]; then
	# pre-install instructions
	grep ^nullmail: /etc/group >/dev/null || groupadd -r nullmail
	grep ^nullmail: /etc/passwd >/dev/null || useradd -d /var/spool/nullmailer -g nullmail -M -r -s /bin/false nullmail
fi

%post
if ! [ -s /etc/nullmailer/me ]; then
	/bin/hostname --fqdn >/etc/nullmailer/me
fi
if ! [ -s /etc/nullmailer/defaultdomain ]; then
	/bin/hostname --domain >/etc/nullmailer/defaultdomain
fi

%preun
%systemd_preun nullmailer.service

%postun
%systemd_postun_with_restart nullmailer.service

%files
%defattr(-,nullmail,nullmail)
%doc AUTHORS BUGS ChangeLog COPYING INSTALL NEWS README TODO doc/DIAGRAM
%dir /etc/nullmailer
%attr(04711,nullmail,nullmail) /usr/bin/mailq
/usr/bin/nullmailer-dsn
/usr/bin/nullmailer-inject
/usr/bin/nullmailer-smtpd
/usr/lib/sendmail
/usr/lib/systemd/system/nullmailer.service
%dir /usr/libexec/nullmailer
/usr/libexec/nullmailer/*
%{_mandir}/*/*
%attr(04711,nullmail,nullmail) /usr/sbin/nullmailer-queue
/usr/sbin/nullmailer-send
/usr/sbin/sendmail
%dir /var/log/nullmailer
/var/spool/nullmailer
