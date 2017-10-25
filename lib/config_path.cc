// nullmailer -- a simple relay-only MTA
// Copyright (C) 2017  Bruce Guenter <bruce@untroubled.org>
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
// You can contact me at <bruce@untroubled.org>.  There is also a mailing list
// available to discuss this package.  To subscribe, send an email to
// <nullmailer-subscribe@lists.untroubled.org>.

#include "config.h"
#include "defines.h"
#include <stdlib.h>
#include "configio.h"
#include "fdbuf/fdbuf.h"

mystring config_BIN_DIR;
mystring config_CONFIG_DIR;
mystring config_HOME_DIR;
mystring config_PROTOCOLS_DIR;
mystring config_SBIN_DIR;

static mystring test_prefix;
static bool initialized = false;

mystring config_path(const char* dflt, const char* testdir, const char* subdir, const char* filename)
{
  if (!initialized) {
    // Check if the program is running setuid, to avoid privilege escalation.
    if (getuid() == geteuid())
      test_prefix = getenv("NULLMAILER_TEST_PREFIX");
  }
  mystring result = test_prefix;
  if (!result)
    result = dflt;
  else {
    result += '/';
    result += testdir;
  }
  result += '/';
  if (subdir) {
    result += subdir;
    result += '/';
  }
  result += filename;
  return result;
}
