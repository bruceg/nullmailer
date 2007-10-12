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
#include "cli++/cli++.h"
#include "itoa.h"
#include "defines.h"
#include "mystring/mystring.h"
#include "fdbuf/fdbuf.h"
#include "canonicalize.h"
#include "configio.h"
#include "hostname.h"
#include "makefield.h"

static unsigned long opt_timestamp = 0;
static unsigned long opt_last_attempt = 0;

const char* cli_program = "nullmailer-dsn";
const char* cli_help_prefix =
"Reformat a queued message into a delivery status notification (DSN)\n";
const char* cli_help_suffix = "";
const char* cli_args_usage = "< message";
const int cli_args_min = 0;
const int cli_args_max = 0;
cli_option cli_options[] = {
  { 0, "orig-timestamp", cli_option::uinteger, 0, &opt_timestamp,
    "UNIX timestamp on the original message",
    "ctime on the input message" },
  { 0, "last-attempt", cli_option::uinteger, 0, &opt_last_attempt,
    "UNIX timestamp of the last attempt",
    "access time on the input message" },
  {0, 0, cli_option::flag, 0, 0, 0, 0}
};

#define die1sys(MSG) do{ fout << "nullmailer-dsn: " << MSG << strerror(errno) << endl; exit(111); }while(0)
#define die1(MSG) do{ fout << "nullmailer-dsn: " << MSG << endl; exit(111); }while(0)

static mystring sender;
static mystring doublebounceto;
static mystring line;

static mystring idhost;
static const mystring boundary = make_boundary();

int cli_main(int, char*[])
{
  struct stat msgstat;
  if (fstat(0, &msgstat) < 0)
    die1sys("Could not stat the source message");
  if (opt_timestamp == 0)
    opt_timestamp = msgstat.st_ctime;
  if (opt_last_attempt == 0)
    opt_last_attempt = msgstat.st_atime;

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

  if (!!sender)
    fout << '\n' << sender;
  else
    fout << "#@[]\n" << doublebounceto;

  fout << "\n"
    "\n"
    "From: Message Delivery Subsystem <MAILER-DAEMON@" << me << ">\n"
    "To: <" << sender << ">\n"
    "Subject: Returned mail: Could not send message\n"
    "Date: " << make_date() << "\n"
    "Message-Id: " << make_messageid(idhost) << "\n"
    "MIME-Version: 1.0\n"
    "Content-Type: multipart/report; report-type=delivery-status\n"
    "\tboundary=\"" << boundary << "\"\n";

  /* Human readable text portion */
  fout << "\n"
    "--" << boundary << "\n"
    "Content-Type: text/plain; charset=us-ascii\n"
    "\n"
    "The original message was received at " << make_date(opt_timestamp) << "\n"
    "from " << sender << "\n"
    "\n"
    "   ----- The following addresses had permanent fatal errors -----\n";

  while (fin.getline(line)) {
    if (!line)
      break;
    fout << '<' << line << ">\n";
  }

  /* delivery-status portion */
  fout << "\n"
    "--" << boundary << "\n"
    "Content-Type: message/delivery-status\n"
    "\n"
    "Reporting-MTA: dns; HOSTNAME\n"
    "Received-From-MTA: DNS; [IP]\n"
    "Arrival-Date: " << make_date(opt_timestamp) << "\n"
    "\n"
    "Final-Recipient: RFC822; ADDR\n"
    "X-Actual-Recipient: RFC822; ADDR\n"
    "Action: failed\n"
    "Status: #.#.#\n"
    "Diagnostic-Code: SMTP; 552 #.#.# MESSAGE\n"
    "Last-Attempt-Date: " << make_date(opt_last_attempt) << '\n';

  /* Copy the message */
  fout << "\n"
    "--" << boundary << "\n"
    "Content-Type: message/rfc822\n"
    "\n";
  while (fin.getline(line))
    fout << line << '\n';

  fout << "\n"
    "--" << boundary << "--\n";

  return 0;
}
