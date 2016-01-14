#include "fdbuf.h"

int main()
{
  fdibuf in("testfile");
  char buf[17]; buf[16] = 0;
  
  in.read(buf, 16); fout.write(buf); fout.flush();
  
  in.seek(1024);
  in.read(buf, 16); fout.write(buf); fout.flush();
  
  in.seek(8192+16);
  in.read(buf, 16); fout.write(buf); fout.flush();
  return 0;
}
