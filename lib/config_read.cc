#include "config.h"
#include "defines.h"
#include "configio.h"
#include "fdbuf.h"

bool config_read(const char* filename, mystring& result)
{
  mystring fullname = CONFIG_DIR;
  fullname += filename;
  fdibuf in(fullname.c_str());
  if(!in.getline(result))
    return false;
  result = result.strip();
  return result.length() > 0;
}
