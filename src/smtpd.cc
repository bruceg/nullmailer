// nullmailer -- a simple relay-only MTA
// Copyright (C) 2012  Bruce Guenter <bruce@untroubled.org>
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

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/defines.h"
#include "fdbuf/fdbuf.h"
#include "mystring/mystring.h"

static const char resp_data_ok[] = "354 End your message with a period on a line by itself";
static const char resp_goodbye[] = "221 2.0.0 Good bye";
static const char resp_help[] = "214 2.0.0 Help not available";
static const char resp_mail_bad[] = "554 5.1.2 Sender invalid";
static const char resp_mail_ok[] = "250 2.1.0 Sender accepted";
static const char resp_need_param[] = "501 5.5.2 Syntax error, command requires a parameter";
static const char resp_no_mail[] = "503 5.5.1 You must send a valid sender first";
static const char resp_no_param[] = "501 5.5.2 Syntax error, no parameters allowed";
static const char resp_no_queue[] = "451 4.3.0 Starting nullmailer-queue failed";
static const char resp_no_rcpt[] = "503 5.5.1 You must send a valid recipient first";
static const char resp_ok[] = "250 2.3.0 OK";
static const char resp_queue_exiterr[] = "451 4.3.0 Error returned from nullmailer-queue";
static const char resp_queue_ok[] = "250 2.6.0 Accepted message";
static const char resp_queue_waiterr[] = "451 4.3.0 Error checking return status from nullmailer-queue";
static const char resp_qwrite_err[] = "451 4.3.0 Write to nullmailer-queue failed";
static const char resp_rcpt_bad[] = "554 5.1.2 Recipient invalid";
static const char resp_rcpt_ok[] = "250 2.1.5 Recipient accepted";
static const char resp_unimp[] = "500 5.5.1 Not implemented";

static mystring line;
static mystring sender;
static mystring recipients;

static int readline()
{
  if (!fin.getline(line))
    return 0;
  if (line.length() > 0
      && line[line.length()-1] == '\r')
    line = line.left(line.length()-1);
  return 1;
}

static mystring parse_addr_arg(mystring& param)
{
  mystring addr;
  unsigned i;
  char term;
  if ((i = param.find_first('<') + 1) > 0)
    term = '>';
  else {
    term = ' ';
    if ((i = param.find_first(':') + 1) <= 0)
      if ((i = param.find_first(' ') + 1) <= 0)
	return 0;
    while (i < param.length() && param[i] == ' ')
      ++i;
  }
  for (bool quoted = false; i < param.length() && (quoted || param[i] != term); ++i) {
    switch (param[i]) {
    case '"':
      quoted = !quoted;
      break;
    case '\\':
      ++i;
      // fall through
    default:
      addr += param[i];
    }
  }
  // strip source routing
  if (addr[0] == '@'
      && (i = addr.find_first(':')) > 0)
    addr = addr.right(i+1);
  
  return addr + "\n";
}

static void do_reset(void)
{
  sender = "";
  recipients = "";
}

static bool respond(const char* msg)
{
  fout << msg << '\r' << endl;
  return fout;
}

static pid_t qpid;

static int start_queue()
{
  int pfd[2];
  int fdnull;
  const char* args[2] = { 0 };
  mystring nqueue = getenv("NULLMAILER_QUEUE");
  if (!nqueue) {
    nqueue = SBIN_DIR;
    nqueue += "/nullmailer-queue";
  }
  args[0] = nqueue.c_str();

  if ((fdnull = open("/dev/null", O_WRONLY)) >= 0) {
    if (pipe(pfd) == 0) {
      if ((qpid = fork()) >= 0) {

	if (qpid == 0) {
	  dup2(pfd[0], 0);
	  dup2(fdnull, 1);
	  dup2(fdnull, 2);
	  close(pfd[0]);
	  close(pfd[1]);
	  close(fdnull);
	  execv(args[0], (char**)args);
	  _exit(111);
	}

	close(pfd[0]);
	return pfd[1];
      }
      close(pfd[0]);
      close(pfd[1]);
    }
    close(fdnull);
  }
  return -1;
}

static bool qwrite(int qfd, const char* data, size_t len)
{
  ssize_t wr;
  while (len > 0) {
    wr = write(qfd, data, len);
    if (wr <= 0)
      return false;
    len -= wr;
    data += wr;
  }
  return true;
}

static bool DATA(mystring& param)
{
  if (!!param)
    return respond(resp_no_param);
  if (!sender)
    return respond(resp_no_mail);
  if (!recipients)
    return respond(resp_no_rcpt);

  int qfd;
  if ((qfd = start_queue()) < 0)
    return respond(resp_no_queue);
  if (!qwrite(qfd, sender.c_str(), sender.length())
      || !qwrite(qfd, recipients.c_str(), recipients.length())
      || !qwrite(qfd, "\n", 1))
    return respond(resp_qwrite_err);

  if (!respond(resp_data_ok))
    return false;

  while (readline()) {
    if (line.length() == 1 && line[0] == '.')
      break;
    line += '\n';
    if (!qwrite(qfd, line.c_str(), line.length()))
      return respond(resp_qwrite_err);
  }
  close(qfd);

  int status;
  if (waitpid(qpid, &status, 0) != qpid)
    return respond(resp_queue_waiterr);

  return respond(status ? resp_queue_exiterr : resp_queue_ok);
}

static bool HELO(mystring& param)
{
  if (!param)
    return respond(resp_need_param);
  return respond(resp_ok);
}

static bool HELP(mystring&)
{
  return respond(resp_help);
}

static bool MAIL(mystring& param)
{
  if (!param)
    return respond(resp_need_param);
  sender = parse_addr_arg(param);
  return respond(!sender ? resp_mail_bad : resp_mail_ok);
}

static bool NOOP(mystring&)
{
  return respond(resp_ok);
}

static bool QUIT(mystring&)
{
  respond(resp_goodbye);
  return false;
}

static bool RCPT(mystring& param)
{
  if (!param)
    return respond(resp_need_param);
  if (!sender)
    return respond(resp_no_mail);
  mystring tmp = parse_addr_arg(param);
  recipients += tmp;
  return respond(!tmp ? resp_rcpt_bad : resp_rcpt_ok);
}

static bool RSET(mystring&)
{
  do_reset();
  return respond(resp_ok);
}

static bool VRFY(mystring&)
{
  return respond(resp_unimp);
}

typedef bool (*dispatch_fn)(mystring& param);
struct dispatch 
{
  const char* cmd;
  dispatch_fn fn;
};
static struct dispatch dispatch_table[] = {
  { "DATA", DATA },
  { "EHLO", HELO },
  { "HELO", HELO },
  { "HELP", HELP },
  { "MAIL", MAIL },
  { "NOOP", NOOP },
  { "QUIT", QUIT },
  { "RCPT", RCPT },
  { "RSET", RSET },
  { "VRFY", VRFY },
  { 0, 0 }
};

static bool dispatch()
{
  mystring cmd;
  mystring param;
  cmd = line;
  for (size_t i = 0; i < line.length(); i++) {
    if (line[i] == ' ') {
      cmd = line.left(i);
      param = line.right(i+1).lstrip();
      break;
    }
  }
  cmd = cmd.upper();
  struct dispatch* d;
  for (d = dispatch_table; d->cmd != 0; d++) {
    if (cmd == d->cmd)
      return d->fn(param);
  }
  return respond("500 5.5.1 Not implemented");
}

int main(void)
{
  mystring line;
  if (!respond("220 nullmailer-smtpd ready"))
    return 0;
  while (readline()) {
    if (!dispatch())
      return 0;
  }
  return 0;
}
