#include "config.h"
#include "defines.h"
#include "fdbuf/fdbuf.h"
#include "itoa.h"
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define fail(X) do{ fout << X << endl; return 1; }while(0)

int main(int, char*[])
{
  if(chdir(QUEUE_MSG_DIR))
    fail("Cannot change directory to queue.");
  DIR* dir = opendir(QUEUE_MSG_DIR);
  if(!dir)
    fail("Cannot open queue directory.");
  struct dirent* entry;
  while((entry = readdir(dir)) != 0) {
    const char* name = entry->d_name;
    if(name[0] == '.')
      continue;
    time_t time = atoi(name);
    char timebuf[100];
    strftime(timebuf, 100, "%Y-%m-%d %H:%M:%S  ", localtime(&time));
    fout << timebuf;
    struct stat statbuf;
    if(stat(name, &statbuf) == -1) 
      fout << "?????";
    else
      fout << itoa(statbuf.st_size);
    fout << " bytes" << endl;
  }
  closedir(dir);
  return 0;
}
