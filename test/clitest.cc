#include "cli++/cli++.h"
#include "fdbuf/fdbuf.h"

const char* cli_program = "clitest";
const char* cli_help_prefix = "Nullmailer CLI test harness\n";
const char* cli_help_suffix = 0;
const char* cli_args_usage = "[args]";
const int cli_args_min = 0;
const int cli_args_max = -1;
const bool cli_only_long = CLI_ONLY_LONG;

static int a = 0;
static int b = 0;
static const char* c = 0;
static const char* d = 0;

cli_option cli_options[] = {
  { 'a', 0, cli_option::flag, 1, &a, "Test flag", 0 },
  { 0, "bb", cli_option::flag, 1, &b, "Test flag", 0 },
  { 'c', 0, cli_option::string, 1, &c, "Test string", 0 },
  { 0, "dd", cli_option::string, 1, &d, "Test string", 0 },
  CLI_OPTION_END
};

static void showcstr(const char* name, const char* value)
{
  fout << ' ' << name;
  if (value == NULL)
    fout << "=NULL";
  else
    fout << "=\"" << value << "\"";
}

int cli_main(int argc, char* argv[])
{
  fout << "argc=" << argc
       << " a=" << a
       << " b=" << b;
  showcstr("c", c);
  showcstr("d", d);
  fout << endl;
  for (int i = 0; i < argc; i++)
    fout << "argv[" << i << "]=\"" << argv[i] << "\"" << endl;
  return 0;
}
