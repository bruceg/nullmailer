// nullmailer -- a simple relay-only MTA
// Copyright (C) 1999-2003  Bruce Guenter <bruceg@em.ca>
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

#include "errcodes.h"

const char* const errorstr[] = {
  "No error",
  "Unspecified error",
  "Host not found",
  "Host has no address",
  "Fatal error in gethostbyname",
  "Temporary error in gethostbyname",
  "Socket failed",
  "Connection refused",
  "Connection timed out",
  "Host or network unreachable",
  "Connection failed",
  "Protocol error",
  "Could not open message",
  "Could not read message",
  "Could not write message",
  "Could not exec program",
  "Server refused the message",
  "Temporary error in sending the message",
  "Permanent error in sending the message",
  "Command-line usage error",
};
