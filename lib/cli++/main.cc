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
#include "ac/time.h"
#include "fdbuf/fdbuf.h"
#include <stdlib.h>
#include <string.h>
#include "cli++.h"

#ifndef HAVE_SRANDOM
void srandom(unsigned int seed);
#endif

static bool do_show_usage = false;
const char* argv0;
const char* argv0base;
const char* argv0dir;

static cli_option help_option = { 'h', "help", cli_option::flag,
				  true, &do_show_usage,
				  "Display this help and exit", 0 };

static cli_option** options;
static unsigned optionc;

static void build_options()
{
  for(optionc = 0;
      cli_options[optionc].ch || cli_options[optionc].name;
      optionc++) ;
  optionc++;
  options = new cli_option*[optionc];
  for(unsigned i = 0; i < optionc-1; i++)
    options[i] = &cli_options[i];
  options[optionc-1] = &help_option;
}

static inline unsigned max(unsigned a, unsigned b)
{
  return (a>b) ? a : b;
}

static const char* fill(unsigned i)
{
  static unsigned lastlen = 0;
  static char* buf = 0;
  if(i > lastlen) {
    delete[] buf;
    buf = new char[i+1];
    lastlen = i;
  }
  memset(buf, ' ', i);
  buf[i] = 0;
  return buf;
}
  
static void show_usage()
{
  fout << "usage: " << cli_program << " [flags] " << cli_args_usage << endl;
}

static unsigned calc_max_width()
{
  // maxwidth is the maximum width of the long argument
  unsigned maxwidth = 0;
  for(unsigned i = 0; i < optionc; i++) {
    unsigned width = 0;
    cli_option* o = options[i];
    if(o->name) {
      width += strlen(o->name);
      switch(o->type) {
      case cli_option::string:     width += 6; break;
      case cli_option::integer:    width += 4; break;
      case cli_option::uinteger:   width += 4; break;
      case cli_option::stringlist: width += 5; break;
      case cli_option::flag:       break;
      case cli_option::counter:    break;
      }
    }
    if(width > maxwidth)
      maxwidth = width;
  }
  return maxwidth;
}

static void show_option(cli_option* o, unsigned maxwidth)
{
  if(o == &help_option)
    fout << '\n';
  if(o->ch)
    fout << "  -" << o->ch;
  else
    fout << "    ";
  fout << (o->ch && o->name ? ", " : "  ");
  if(o->name) {
    const char* extra = "";
    switch(o->type) {
    case cli_option::string:     extra = "=VALUE"; break;
    case cli_option::integer:    extra = "=INT"; break;
    case cli_option::uinteger:   extra = "=UNS"; break;
    case cli_option::stringlist: extra = "=ITEM"; break;
    case cli_option::flag:       break;
    case cli_option::counter:    break;
    }
    fout << "--" << o->name << extra
	 << fill(maxwidth - strlen(o->name) - strlen(extra) + 2);
  }
  else
    fout << fill(maxwidth+4);
  fout << o->helpstr << '\n';
  if(o->defaultstr)
    fout << fill(maxwidth+10) << "(Defaults to " << o->defaultstr << ")\n";
}

static void show_help()
{
  if(cli_help_prefix)
    fout << cli_help_prefix;
  unsigned maxwidth = calc_max_width();
  for(unsigned i = 0; i < optionc; i++)
    show_option(options[i], maxwidth);
  if(cli_help_suffix)
    fout << cli_help_suffix;
}

void usage(int exit_value, const char* errorstr)
{
  if(errorstr)
    ferr << cli_program << ": " << errorstr << endl;
  show_usage();
  show_help();
  exit(exit_value);
}

cli_stringlist* stringlist_append(cli_stringlist* node, const char* newstr)
{
  cli_stringlist* newnode = new cli_stringlist(newstr);
  if(node) {
    cli_stringlist* head = node;
    while(node->next)
      node = node->next;
    node->next = newnode;
    return head;
  }
  else
    return newnode;
}

int cli_option::set(const char* arg)
{
  char* endptr;
  switch(type) {
  case flag:
    *(int*)dataptr = flag_value;
    return 0;
  case counter:
    *(int*)dataptr += flag_value;
    return 0;
  case integer:
    *(int*)dataptr = strtol(arg, &endptr, 10);
    if(*endptr) {
      ferr << argv0 << ": invalid integer: " << arg << endl;
      return -1;
    }
    return 1;
  case uinteger:
    *(unsigned*)dataptr = strtoul(arg, &endptr, 10);
    if(*endptr) {
      ferr << argv0 << ": invalid unsigned integer: " << arg << endl;
      return -1;
    }
    return 1;
  case stringlist:
    *(cli_stringlist**)dataptr =
      stringlist_append(*(cli_stringlist**)dataptr, arg);
    return 1;
  default: // string
    *(const char**)dataptr = arg;
    return 1;
  }
}

static int parse_short(int argc, char* argv[])
{
  int end = strlen(argv[0]) - 1;
  for(int i = 1; i <= end; i++) {
    int ch = argv[0][i];
    unsigned j;
    for(j = 0; j < optionc; j++) {
      cli_option* o = options[j];
      if(o->ch == ch) {
	if(o->type != cli_option::flag &&
	   o->type != cli_option::counter) {
	  if(i < end) {
	    if(o->set(argv[0]+i+1) != -1)
	      return 0;
	  }
	  else if(argc <= 1) {
	    ferr << argv0 << ": option -" << o->ch
		 << " requires a value." << endl;
	  }
	  else
	    if(o->set(argv[1]) != -1)
	      return 1;
	}
	else if(o->set(0) != -1)
	  break;
	return -1;
      }
    }
    if(j >= optionc) {
      ferr << argv0 << ": unknown option letter -" << argv[0][i] << endl;
      return -1;
    }
  }
  return 0;
}

int cli_option::parse_long_eq(const char* arg)
{
  if(type == flag || type == counter) {
    ferr << argv0 << ": option --" << name
	 << " does not take a value." << endl;
    return -1;
  }
  else
    return set(arg)-1;
}

int cli_option::parse_long_noeq(const char* arg)
{
  if(type == flag || type == counter)
    return set(0);
  else if(arg)
    return set(arg);
  else {
    ferr << argv0 << ": option --" << name
	 << " requires a value." << endl;
    return -1;
  }
}

static int parse_long(int, char* argv[])
{
  const char* arg = argv[0]+2;
  for(unsigned j = 0; j < optionc; j++) {
    cli_option* o = options[j];
    if(o->name) {
      size_t len = strlen(o->name);
      if(!memcmp(arg, o->name, len)) {
	if(arg[len] == '\0')
	  return o->parse_long_noeq(argv[1]);
	else if(arg[len] == '=')
	  return o->parse_long_eq(arg+len+1);
      }
    }
  }
  ferr << argv0 << ": unknown option string: '--" << arg << "'" << endl;
  return -1;
}

static int parse_args(int argc, char* argv[])
{
  build_options();
  int i;
  for(i = 1; i < argc; i++) {
    const char* arg = argv[i];
    // Stop at the first non-option argument
    if(arg[0] != '-')
      break;
    // Stop after the first "-" or "--"
    if(arg[1] == '\0' ||
       (arg[1] == '-' && arg[2] == '\0')) {
      i++;
      break;
    }
    int j = (arg[1] != '-') ?
      parse_short(argc-i, argv+i) :
      parse_long(argc-i, argv+i);
    if(j < 0)
      usage(1);
    else
      i += j;
  }
  return i;
}

static void set_argv0(const char* p)
{
  argv0 = p;
  static const char* empty = "";
  const char* s = strrchr(p, '/');
  if(s) {
    ++s;
    argv0base = s;
    size_t length = s-p;
    char* tmp = new char[length+1];
    memcpy(tmp, p, length);
    tmp[length] = 0;
    argv0dir = tmp;
  }
  else {
    argv0base = p;
    argv0dir = empty;
  }
}

int main(int argc, char* argv[])
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  srandom(tv.tv_usec ^ tv.tv_sec);
  
  set_argv0(argv[0]);
  int lastarg = parse_args(argc, argv);

  if(do_show_usage)
    usage(0);

  argc -= lastarg;
  argv += lastarg;
  if(argc < cli_args_min)
    usage(1, "Too few command-line arguments");
  if(cli_args_max >= cli_args_min && argc > cli_args_max)
    usage(1, "Too many command-line arguments");
  
  return cli_main(argc, argv);
}
