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

#include "errcodes.h"
#include "protocol.h"

void tls_init(const char* remote)
{
  protocol_fail(ERR_USAGE, "SSL/TLS not supported in this build");
  (void)remote;
}

void tls_send(fdibuf& in, int fd)
{
  protocol_fail(ERR_USAGE, "SSL/TLS not supported in this build");
  (void)in;
  (void)fd;
}
