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

const char* errorstr(int code)
{
  switch (code) {
  case 0: return "No error";
  case ERR_HOST_NOT_FOUND: return "Host not found";
  case ERR_NO_ADDRESS: return "Host has no address";
  case ERR_GHBN_FATAL: return "Fatal error in gethostbyname";
  case ERR_GHBN_TEMP: return "Temporary error in gethostbyname";
  case ERR_SOCKET: return "Socket failed";
  case ERR_CONN_REFUSED: return "Connection refused";
  case ERR_CONN_TIMEDOUT: return "Connection timed out";
  case ERR_CONN_UNREACHABLE: return "Host or network unreachable";
  case ERR_CONN_FAILED: return "Connection failed";
  case ERR_PROTO: return "Protocol error";
  case ERR_MSG_OPEN: return "Could not open message";
  case ERR_MSG_READ: return "Could not read message";
  case ERR_MSG_WRITE: return "Could not write message";
  case ERR_EXEC_FAILED: return "Could not exec program";
  case ERR_MSG_TEMPFAIL: return "Temporary error in sending the message";
  case ERR_CONFIG: return "Could not read config files";
  case ERR_MSG_REFUSED: return "Server refused the message";
  case ERR_MSG_PERMFAIL: return "Permanent error in sending the message";
  case ERR_BIND_FAILED: return "Failed to bind source address";
  }
  return (code & ERR_PERMANENT_FLAG)
    ? "Unspecified permanent error"
    : "Unspecified temporary error";
}
