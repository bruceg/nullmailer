// Copyright (C) 1999,2000,2005 Bruce Guenter <bruce@untroubled.org>
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

#include <config.h>
#include "cli++.h"
#include "fdbuf/fdbuf.h"

const char* cli_program = "clitest";
const char* cli_help_prefix = "Does nothing but set flags\n";
const char* cli_help_suffix = "";
const char* cli_args_usage = "";
const int cli_args_min = 0;
const int cli_args_max = -1;
int o_flag = 0;
int o_int = 0;
char* o_string = "nostring";
cli_option cli_options[] = { 
  { 'f', "flag", cli_option::flag, 1, &o_flag, "Sets a flag", 0 },
  { 'i', "int", cli_option::integer, 0, &o_int, "Sets an integer", 0 },
  { 's', "str", cli_option::string, 0, &o_string, "Sets a string", 0},
  {0} };

int cli_main(int argc, char* argv[])
{
  fout << "argv0=" << argv0 << endl
       << " argv0dir=" << argv0dir << endl
       << " argv0base=" << argv0base << endl;
  fout << "The flag is set to " << o_flag << endl;
  fout << "The integer is set to " << o_int << endl;
  fout << "The string is set to " << o_string << endl;
  for(int i = 0; i < argc; i++)
    fout << "argv[" << i << "] = '" << argv[i] << "'\n";
  return 0;
}
