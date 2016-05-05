#!/bin/sh
aclocal -I .
autoheader
# automake needs a file named "ChangeLog"
if ! test -f "ChangeLog"; then
echo "INFO: creating phony ChangeLog for automake to succeed";
touch "ChangeLog";
fi
automake --add-missing
autoconf
