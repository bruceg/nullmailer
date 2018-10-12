// Copyright (C) 2018 Bruce Guenter <bruce@untroubled.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#include <errno.h>
#include <config.h>
#include "fdbuf/fdbuf.h"
#include <stdlib.h>
#include <string.h>
#include "cli++.h"

extern const char* argv0;

static void cli_msg(const char* prefix,
		    const char* a,
		    const char* b,
		    const char* c,
		    const char* d,
		    bool add_error = false)
{
  ferr << cli_program << ": " << prefix << a;
  if(b) ferr << b;
  if(c) ferr << c;
  if(d) ferr << d;
  if (add_error)
    ferr << ": " << strerror(errno);
  ferr << endl;
}

void cli_error(int exit_value,
	       const char* a,
	       const char* b,
	       const char* c,
	       const char* d)
{
  cli_msg("Error: ", a, b, c, d, false);
  exit(exit_value);
}

void cli_syserror(int exit_value,
		  const char* a,
		  const char* b,
		  const char* c,
		  const char* d)
{
  cli_msg("Error: ", a, b, c, d, true);
  exit(exit_value);
}

void cli_warning(const char* a,
	       const char* b,
	       const char* c,
	       const char* d)
{
  cli_msg("Warning: ", a, b, c, d, false);
}
