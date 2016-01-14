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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fdbuf/fdbuf.h"
#include "defines.h"
#include "setenv.h"
#include "cli++/cli++.h"

const char* cli_program = "sendmail";
const char* cli_help_prefix = "Nullmailer sendmail emulator\n";
const char* cli_help_suffix = 0;
const char* cli_args_usage = "[recipients] <message";
const int cli_args_min = 0;
const int cli_args_max = -1;
const bool cli_only_long = true;

enum mode { mode_normal, mode_mailq, mode_smtp };

static int o_dummyi;
static const char* o_dummys;
static const char* o_sender = 0;
static int o_mode = 0;
static char* o_from;
static int use_header = false;

cli_option cli_options[] = {
  { 'B', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  {  0, "bm",  cli_option::flag, mode_normal, &o_mode,
     "Read mail from standard input (default)", 0 },
  {  0, "bp",  cli_option::flag, mode_mailq, &o_mode,
     "List information about mail queue", 0 },
  {  0, "bs",  cli_option::flag, mode_smtp, &o_mode,
     "Handle SMTP commands on standard input", 0 },
  { 'C', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'd', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'F', 0,    cli_option::string, 0, &o_sender,
    "Set the full name of the sender", 0 },
  { 'f', 0,    cli_option::string, 0, &o_from,
    "Set the envelope sender address", 0 },
  { 'h', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'i', 0,    cli_option::flag, 0, &o_dummyi, "Ignored", 0 },
  { 'L', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'm', 0,    cli_option::flag,   0, &o_dummyi, "Ignored", 0 },
  { 'N', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'n', 0,    cli_option::flag, 0, &o_dummyi, "Ignored", 0 },
  { 'O', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'o', 0,    cli_option::string, 0, &o_dummys, "Set sendmail option, ignored", 0 },
  {  0, "em",  cli_option::flag, 0, &o_dummyi,
     "Ignored", 0 },
  {  0, "ep",  cli_option::flag, 0, &o_dummyi,
     "Ignored", 0 },
  {  0, "eq",  cli_option::flag, 0, &o_dummyi,
     "Ignored", 0 },
  { 'p', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'q', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'R', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'r', 0,    cli_option::string, 0, &o_from,
    "An alternate and obsolete form of the -f flag", 0 },
  { 't', 0,    cli_option::flag, 1, &use_header,
    "Read message for recipients", 0 },
  { 'U', 0,    cli_option::flag, 0, &o_dummyi, "Ignored", 0 },
  { 'V', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'v', 0,    cli_option::flag, 0, &o_dummyi, "Ignored", 0 },
  { 'X', 0,    cli_option::string, 0, &o_dummys, "Ignored", 0 },
  CLI_OPTION_END
};

bool setenvelope(char* str)
{
  char* at = strchr(str, '@');
  if(at) {
    *at = 0;
    setenv("NULLMAILER_HOST", at+1, 1);
  }
  setenv("NULLMAILER_USER", str, 1);
  return true;
}

int do_exec(const char* program, const char* xarg1, int argc, char* argv[])
{
  if(chdir(BIN_DIR) == -1) {
    ferr << "sendmail: Could not change directory to " << BIN_DIR << endl;
    return 1;
  }

  const char* newargv[argc+3];
  newargv[0] = program;
  int j = 1;
  if (xarg1)
    newargv[j++] = xarg1;
  for(int i = 0; i < argc; i++)
    newargv[j++] = argv[i];
  newargv[j] = 0;

  execv(newargv[0], (char**)newargv);
  ferr << "sendmail: Could not exec " << program << '.' << endl;
  return 1;
}

int cli_main(int argc, char* argv[])
{
  if(o_sender)
    setenv("NULLMAILER_NAME", o_sender, 1);
  if(o_from)
    if(!setenvelope(o_from))
      return -1;
  switch (o_mode) {
  case mode_smtp:
    return do_exec("nullmailer-smtpd", 0, 0, 0);
  case mode_mailq:
    return do_exec("mailq", 0, 0, 0);
  default:
    return do_exec("nullmailer-inject", use_header ? "-b" : "-e", argc, argv);
  }
}
