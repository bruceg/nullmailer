// nullmailer -- a simple relay-only MTA
// Copyright (C) 1999,2000  Bruce Guenter <bruceg@em.ca>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact me at <bruceg@em.ca>.  There is also a mailing list
// available to discuss this package.  To subscribe, send an email to
// <nullmailer-subscribe@lists.em.ca>.

#include "config.h"
#include "mystring/mystring.h"
#include <unistd.h>
#include <sys/utsname.h>

static mystring* hostname_cache = 0;
static mystring* domainname_cache = 0;

static void getnames()
{
  if(hostname_cache)
    return;
  struct utsname buf;
  uname(&buf);
  hostname_cache = new mystring(buf.nodename);

#ifdef UTSNAME_HAS_DOMAINNAME
  domainname_cache = new mystring(buf.UTSNAME_HAS_DOMAINNAME);
#elif HAVE_GETDOMAINNAME
  char buf[256];
  getdomainname(buf, 255);
  domainname_cache = new mystring(buf);
#else
  domainname_cache = new mystring;
#endif

  // Tricky logic: if the node name does not contains the domain name
  // as a proper suffix, paste them together.
  char* nodename_end = buf.nodename + hostname_cache->length() -
    domainname_cache->length() - 1;
  if(domainname_cache->length() > 0) {
    if(nodename_end <= buf.nodename ||
       strcmp(nodename_end + 1, domainname_cache->c_str()) ||
       *nodename_end != '.')
      *hostname_cache = *hostname_cache + "." + *domainname_cache;
  }
  // Otherwise, the domain name is empty, try to determine it from
  // the host name.
  else {
    int i = hostname_cache->find_first('.');
    if(i != -1)
      *domainname_cache = hostname_cache->right(i+1);
  }
}

mystring hostname()
{
  getnames();
  return *hostname_cache;
}

mystring domainname()
{
  getnames();
  return *domainname_cache;
}
