#include "config.h"
#include "mystring.h"
#include "fdbuf.h"
#include "address.h"

int main(int argc, char* argv[])
{
  for(int i = 1; i < argc; i++) {
    mystring s(argv[i]);
    mystring l;
    if(!parse_addresses(s, l)) {
      fout.writeln("Parsing failed.");
    }
    else {
      fout.write(l);
      fout.write("To: ");
      fout.writeln(s);
    }
  }
  return 0;
}
