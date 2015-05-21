#ifndef NULLMAILER__ARGPARSE__H
#define NULLMAILER__ARGPARSE__H

#include "mystring/mystring.h"
#include "list.h"

typedef list<mystring> arglist;
unsigned parse_args(arglist&, const mystring& str);

#endif
