#include "config.h"
#include "defines.h"
#include "configio.h"
#include "fdbuf.h"

bool config_readlist(const char* filename, list<mystring>& result)
{
  mystring fullname = CONFIG_DIR;
  fullname += filename;
  fdibuf in(fullname.c_str());
  if(!in)
    return false;
  mystring tmp;
  bool nonempty = false;
  while(in.getline(tmp)) {
    tmp = tmp.strip();
    if(tmp[0] != '#' && tmp.length() > 0) {
      result.append(tmp);
      nonempty = true;
    }
  }
  return nonempty;
}
