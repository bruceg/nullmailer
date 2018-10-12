// nullmailer -- a simple relay-only MTA
// Copyright (C) 2018  Bruce Guenter <bruce@untroubled.org>
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
#include <sys/types.h>
#include <ctype.h>
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
#include "list.h"
#include "mystring/mystring.h"
#include "fdbuf/fdbuf.h"
#include "canonicalize.h"
#include "configio.h"
#include "hostname.h"
#include "makefield.h"

typedef list<mystring> slist;

static time_t opt_timestamp = 0;
static time_t opt_last_attempt = 0;
static time_t opt_retry_until = 0;
static const char* opt_envelope_id = 0;
static const char* opt_status = 0;
static const char* opt_remote = 0;
static const char* opt_diagnostic_code = 0;
static int opt_lines = -1;
static bool opt_ddn = false;

const char* cli_program = "nullmailer-dsn";
const char* cli_help_prefix =
"Reformat a queued message into a delivery status notification (DSN)\n";
const char* cli_help_suffix =
"\n"
"The status code must be in the form 4.#.# or 5.#.#. If the status\n"
"code starts with 4, a delivery delay notification is generated.\n";
const char* cli_args_usage = "status-code < message";
const int cli_args_min = 1;
const int cli_args_max = 1;
cli_option cli_options[] = {
  { 0, "diagnostic-code", cli_option::string, 0, &opt_diagnostic_code,
    "Diagnostic code message", 0 },
  { 0, "envelope-id", cli_option::string, 0, &opt_envelope_id,
    "Original envelope ID", 0 },
  { 0, "last-attempt", cli_option::ulong, 0, &opt_last_attempt,
    "UNIX timestamp of the last attempt",
    "access time on the input message" },
  { 0, "orig-timestamp", cli_option::ulong, 0, &opt_timestamp,
    "UNIX timestamp on the original message",
    "ctime on the input message" },
  { 0, "remote", cli_option::string, 0, &opt_remote,
    "Name of remote server", 0 },
  { 0, "retry-until", cli_option::ulong, 0, &opt_retry_until,
    "UNIX timestamp of the (future) final attempt", 0 },
  { 0, "max-lines", cli_option::integer, 0, &opt_lines,
    "Maximum number of lines of the original message to copy", "none" },
  {0, 0, cli_option::flag, 0, 0, 0, 0}
};

#define die1sys(MSG) do{ fout << "nullmailer-dsn: " << MSG << strerror(errno) << endl; exit(111); }while(0)
#define die1(MSG) do{ fout << "nullmailer-dsn: " << MSG << endl; exit(111); }while(0)

static mystring sender;
static mystring bounceto;
static mystring doublebounceto;
static mystring line;
static slist recipients;

static mystring idhost;
static const mystring boundary = make_boundary();

int cli_main(int, char* argv[])
{
  struct stat msgstat;
  if (fstat(0, &msgstat) < 0)
    die1sys("Could not stat the source message");
  if (opt_timestamp == 0)
    opt_timestamp = msgstat.st_ctime;
  if (opt_last_attempt == 0)
    opt_last_attempt = msgstat.st_atime;
  opt_status = argv[0];
  if ((opt_status[0] != '4' && opt_status[0] != '5')
      || opt_status[1] != '.'
      || !isdigit(opt_status[2])
      || opt_status[3] != '.'
      || !isdigit(opt_status[4])
      || opt_status[5] != '\0')
    die1("Status must be in the format 4.#.# or 5.#.#");
  opt_ddn = opt_status[0] == '4';
  if (opt_lines < 0) {
    config_readint("bouncelines", opt_lines);
    if (opt_lines < 0)
      opt_lines = 0;
  }

  if (!config_read("doublebounceto", doublebounceto)
      || !doublebounceto)
    config_read("adminaddr", doublebounceto);
  read_hostnames();
  if (!config_read("idhost", idhost))
    idhost = me;
  else
    canonicalize(idhost);
  config_read("bounceto", bounceto);

  if (!fin.getline(sender))
    die1sys("Could not read sender address from message: ");
  if (!sender && !doublebounceto)
    die1("Nowhere to send double bounce");
  while (fin.getline(line)) {
    if (!line)
      break;
    recipients.append(line);
  }
  if (recipients.count() == 0)
    die1("No recipients were read from message");

  if (!!sender)
    // Bounces either go to the sender or bounceto, if configured
    fout << '\n' << (!!bounceto ? bounceto : sender);
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
    "Content-Type: multipart/report; report-type=delivery-status;\n"
    "\tboundary=\"" << boundary << "\"\n";

  /* Human readable text portion */
  fout << "\n"
    "--" << boundary << "\n"
    "Content-Type: text/plain; charset=us-ascii\n"
    "\n"
    "This is the nullmailer delivery system.  The message attached below\n"
       << (opt_ddn
	   ? "has not been"
	   : "could not be")
       << " delivered to one or more of the intended recipients:\n"
    "\n";
  for (slist::const_iter recipient(recipients); recipient; recipient++)
    fout << "\t<" << (*recipient) << ">\n";
  if (opt_ddn) {
    if (opt_retry_until > 0)
      fout << "\nDelivery will continue to be attempted until "
	   << make_date(opt_retry_until) << '\n';
    fout << "\n"
      "A final delivery status notification will be generated if delivery\n"
      "proves to be impossible within the configured time limit.\n";
  }

  /* delivery-status portion */
  fout << "\n"
    "--" << boundary << "\n"
    "Content-Type: message/delivery-status\n"
    "\n"
    "Reporting-MTA: x-local-hostname; " << me << "\n"
    "Arrival-Date: " << make_date(opt_timestamp) << "\n";
  if (opt_envelope_id != 0)
    fout << "Original-Envelope-Id: " << opt_envelope_id << '\n';

  for (slist::const_iter recipient(recipients); recipient; recipient++) {
    fout << "\n"
      "Final-Recipient: rfc822; " << (*recipient) << "\n"
      "Action: " << (opt_ddn ? "delayed": "failed") << "\n"
      "Status: " << opt_status << "\n"
      "Last-Attempt-Date: " << make_date(opt_last_attempt) << '\n';
    if (opt_remote != 0)
      fout << "Remote-MTA: dns; " << opt_remote << '\n';
    if (opt_diagnostic_code != 0)
      fout << "Diagnostic-Code: " << opt_diagnostic_code << '\n';
    if (opt_ddn and opt_retry_until > 0)
      fout << "Will-Retry-Until: " << make_date(opt_retry_until) << '\n';
  }

  // Copy the message
  fout << "\n"
    "--" << boundary << "\n"
    "Content-Type: message/rfc822\n"
    "\n";
  // Copy the header
  while (fin.getline(line) && !!line)
    fout << line << '\n';
  // Optionally copy the body
  if (opt_lines) {
    fout << '\n';
    for (int i = 0; (opt_lines < 0 || i < opt_lines) && fin.getline(line); i++)
      fout << line << '\n';
  }

  fout << "\n"
    "--" << boundary << "--\n";

  return 0;
}
