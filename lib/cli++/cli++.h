#ifndef VMAILMGR__CLIPP__CLIPP__H__
#define VMAILMGR__CLIPP__CLIPP__H__

typedef bool (*cli_funcptr)(void*);

struct cli_stringlist
{
  const char* string;
  cli_stringlist* next;

  cli_stringlist(const char* s)
    : string(s), next(0)
    {
    }
};

struct cli_option
{
  char ch;
  const char* name;
  enum { flag, counter, integer, string, stringlist, uinteger } type;
  int flag_value;
  void* dataptr;
  const char* helpstr;
  const char* defaultstr;

  int set(const char* arg);
  int parse_long_eq(const char* arg);
  int parse_long_noeq(const char* arg);
};

/* The following are required from the CLI program */
extern const char* cli_program;
extern const char* cli_help_prefix;
extern const char* cli_help_suffix;
extern const char* cli_args_usage;
extern const int cli_args_min;
extern const int cli_args_max;
extern cli_option cli_options[];
extern int cli_main(int argc, char* argv[]);

/* The following are provided to the CLI program */
extern const char* argv0;
extern const char* argv0base;
extern const char* argv0dir;
extern void usage(int exit_value, const char* errorstr = 0);

extern void cli_error(int exit_value,
		      const char*,
		      const char* = 0,
		      const char* = 0,
		      const char* = 0);

extern void cli_warning(const char*,
		      const char* = 0,
		      const char* = 0,
		      const char* = 0);

#endif
