// nullmailer -- a simple relay-only MTA
// Copyright (C) 2007  Bruce Guenter <bruce@untroubled.org>
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
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "itoa.h"
#include "defines.h"
#include "mystring/mystring.h"
#include "fdbuf/fdbuf.h"
#include "canonicalize.h"
#include "configio.h"
#include "hostname.h"
#include "makefield.h"

#define die1sys(MSG) do{ fout << "nullmailer-bounce: " << MSG << strerror(errno) << endl; exit(111); }while(0)
#define die1(MSG) do{ fout << "nullmailer-bounce: " << MSG << endl; exit(111); }while(0)

static time_t timesecs = time(0);
static mystring sender;
static mystring doublebounceto;
static mystring line;

static mystring idhost;
static const mystring boundary = make_boundary();

int main(int, char*[])
{
  if (!config_read("doublebounceto", doublebounceto)
      || !doublebounceto)
    config_read("adminaddr", doublebounceto);
  read_hostnames();
  if (!config_read("idhost", idhost))
    idhost = me;
  else
    canonicalize(idhost);

  if (!fin.getline(sender))
    die1sys("Could not read sender address from message: ");
  if (!sender && !doublebounceto)
    die1("Nowhere to send double bounce");

  fout << "Date: " << make_date() << '\n';
  fout << "From: Message Delivery Subsystem <MAILER-DAEMON@" << me << ">\n";
  fout << "Message-Id: " << make_messageid(idhost) << '\n';
  fout << "To: <" << sender << ">\n";
  fout << "Subject: Returned mail: Could not send message\n";
  fout << "MIME-Version: 1.0\n";
  fout << "Content-Type: multipart/report; report-type=delivery-status\n"
    "\tboundary=\"" << boundary << "\"\n";
  fout << "\n--" << boundary << "\n"
    "\n"
    "The original message was received at ???\n"
    "from " << sender;

  while (fin.getline(line)) {
    if (!line)
      break;
    fout << '<' << line << ">\n";
  }

  fout << "\n--- Below this line is a copy of the message.\n\n";
  while (fin.getline(line))
    fout << line << '\n';

  return 0;
}
