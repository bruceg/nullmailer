#include "config.h"
#include <stdlib.h>
#include "defines.h"
#include "configio.h"
#include "fdbuf.h"

bool config_readint(const char* filename, int& result)
{
  mystring tmp;
  if(!config_read(filename, tmp))
    return false;
  char* endptr;
  result = strtol(tmp.c_str(), &endptr, 10);
  return endptr > tmp.c_str();
}
