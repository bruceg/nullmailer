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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fdbuf/fdbuf.h"
#include "defines.h"
#include "cli++/cli++.h"

const char* cli_program = "sendmail";
const char* cli_help_prefix = "Nullmailer sendmail emulator\n";
const char* cli_help_suffix = 0;
const char* cli_args_usage = "[recipients] <message";
const int cli_args_min = 0;
const int cli_args_max = -1;

static int o_dummyi;
static const char* o_dummys;
static const char* o_sender = 0;
static char* o_from;
static int use_header = false;

cli_option cli_options[] = {
  { 'B', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'b', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'C', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'd', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'F', 0, cli_option::string, 0, &o_sender,
    "Set the full name of the sender", 0 },
  { 'f', 0, cli_option::string, 0, &o_from,
    "Set the envelope sender address", 0 },
  { 'h', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'i', 0, cli_option::flag, 0, &o_dummyi, "Ignored", 0 },
  { 'L', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'm', 0, cli_option::flag,   0, &o_dummyi, "Ignored", 0 },
  { 'N', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'n', 0, cli_option::flag, 0, &o_dummyi, "Ignored", 0 },
  { 'O', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'o', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'p', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'q', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'R', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'r', 0, cli_option::string, 0, &o_from,
    "An alternate and obsolete form of the -f flag", 0 },
  { 't', 0, cli_option::flag, 1, &use_header,
    "Read message for recipients", 0 },
  { 'U', 0, cli_option::flag, 0, &o_dummyi, "Ignored", 0 },
  { 'V', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  { 'v', 0, cli_option::flag, 0, &o_dummyi, "Ignored", 0 },
  { 'X', 0, cli_option::string, 0, &o_dummys, "Ignored", 0 },
  {0}
};

#ifdef HAVE_SETENV
// Sometimes we need an explicit declaration.
extern "C" int setenv(const char*, const char*, int);
#else
// This is not really a full emulation of setenv, but close enough
int setenv(const char* var, const char* val, int overwrite)
{
  size_t varlen = strlen(var);
  size_t vallen = strlen(val);
  char* str = malloc(varlen+vallen+2);
  if (str == 0) return -1;
  memcpy(str, var, varlen);
  str[varlen] = '=';
  memcpy(str+varlen+1, val, vallen);
  str[varlen+vallen+1] = 0;
  return putenv(str);
}
#endif

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

int parseargs()
{
  if(o_sender)
    setenv("NULLMAILER_NAME", o_sender, 1);
  if(o_from)
    if(!setenvelope(o_from))
      return -1;
  return 0;
}

int cli_main(int argc, char* argv[])
{
  if(chdir(BIN_DIR) == -1) {
    ferr << "sendmail: Could not change directory to " << BIN_DIR << endl;
    return 1;
  }
  
  if(parseargs() < 0)
    return 1;

  char* newargv[argc+3];
  newargv[0] = "nullmailer-inject";
  int j = 1;
  if(use_header)
    newargv[j++] = "-b";
  else
    newargv[j++] = "-e";
  for(int i = 0; i < argc; i++)
    newargv[j++] = argv[i];
  newargv[j] = 0;

  execv(newargv[0], newargv);
  ferr << "sendmail: Could not exec nullmailer-inject." << endl;
  return 1;
}
