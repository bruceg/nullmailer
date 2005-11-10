// nullmailer -- a simple relay-only MTA
// Copyright (C) 1999-2003  Bruce Guenter <bruce@untroubled.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact me at <bruce@untroubled.org>.  There is also a mailing list
// available to discuss this package.  To subscribe, send an email to
// <nullmailer-subscribe@lists.untroubled.org>.

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
