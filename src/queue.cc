// nullmailer -- a simple relay-only MTA
// Copyright (C) 1999-2003  Bruce Guenter <bruceg@em.ca>
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
// You can contact me at <bruceg@em.ca>.  There is also a mailing list
// available to discuss this package.  To subscribe, send an email to
// <nullmailer-subscribe@lists.em.ca>.

#include "config.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "itoa.h"
#include "defines.h"
#include "mystring/mystring.h"
#include "fdbuf/fdbuf.h"
#include "configio.h"
#include "canonicalize.h"
#include "hostname.h"

#define fail(MSG) do{ fout << "nullmailer-queue: " << MSG << endl; return false; }while(0)

pid_t pid = getpid();
uid_t uid = getuid();
time_t timesecs = time(0);
mystring adminaddr;
bool remapadmin = false;

bool is_dir(const char* path)
{
  struct stat buf;
  return !stat(path, &buf) && S_ISDIR(buf.st_mode);
}

bool is_exist(const char* path)
{
  struct stat buf;
  return !stat(path, &buf);
}

int fsyncdir(const char* path)
{
  int fd = open(path, O_RDONLY);
  if(fd == -1)
    return 0;
  int result = fsync(fd);
  if(result == -1 && errno != EIO)
    result = 0;
  close(fd);
  return result;
}

void trigger()
{
  int fd = open(QUEUE_TRIGGER, O_WRONLY|O_NONBLOCK, 0666);
  if(fd == -1)
    return;
  char x = 0;
  write(fd, &x, 1);
  close(fd);
}

bool validate_addr(mystring& addr, bool doremap)
{
  int i = addr.find_last('@');
  if(i < 0)
    return false;
  mystring hostname = addr.right(i+1);
  if(doremap && remapadmin) {
    if(hostname == me || hostname == "localhost")
      addr = adminaddr;
  }
  else if(hostname.find_first('.') < 0)
    return false;
  return true;
}

bool copyenv(fdobuf& out)
{
  mystring str;
  if(!fin.getline(str) || !str)
    fail("Could not read envelope sender.");
  if(!validate_addr(str, false))
    fail("Envelope sender address is invalid.");
  if(!(out << str << endl))
    fail("Could not write envelope sender.");
  unsigned count=0;
  while(fin.getline(str) && !!str) {
    if(!validate_addr(str, true))
      fail("Envelope recipient address is invalid.");
    if(!(out << str << endl))
      fail("Could not write envelope recipient.");
    ++count;
  }
  if(count == 0)
    fail("No envelope recipients read.");
  if(!(out << "\n"))
    fail("Could not write extra blank line to destination.");
  return true;
}

bool makereceived(fdobuf& out)
{
  mystring line("Received: (nullmailer pid ");
  line += itoa(pid);
  line += " invoked by uid ";
  line += itoa(uid);
  line += ");\n\t";
  char buf[100];
  if(!strftime(buf, 100, "%a, %d %b %Y %H:%M:%S -0000\n", gmtime(&timesecs)))
    fail("Error generating a date string.");
  line += buf;
  if(!(out << line))
    fail("Could not write received line to message.");
  return true;
}

bool dump(int fd)
{
  fdobuf out(fd);
  if(!copyenv(out))
    return false;
  if(!makereceived(out))
    return false;
  if(!fdbuf_copy(fin, out))
    fail("Error copying the message to the queue file.");
  if(!out.flush())
    fail("Error flushing the output file.");
  if(!out.close())
    fail("Error closing the output file.");
  return true;
}

bool deliver()
{
  if(!is_dir(QUEUE_MSG_DIR) || !is_dir(QUEUE_TMP_DIR))
    fail("Installation error: queue directory is invalid.");

  // Notes:
  // - temporary file name is unique to the currently running
  //   nullmailer-queue program
  // - destination file name is unique to the system
  // - if the temporary file previously existed, it did so because
  //   the previous nullmailer-queue process crashed, and it can be
  //   safely overwritten
  const mystring pidstr = itoa(pid);
  const mystring timestr = itoa(timesecs);
  const mystring tmpfile = QUEUE_TMP_DIR + pidstr;
  const mystring newfile = QUEUE_MSG_DIR + timestr + "." + pidstr;

  int out = open(tmpfile.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0666);
  if(out < 0)
    fail("Could not open temporary file for writing");
  if(!dump(out)) {
    unlink(tmpfile.c_str());
    return false;
  }
  if(link(tmpfile.c_str(), newfile.c_str()))
    fail("Error linking the temp file to the new file.");
  if(fsyncdir(QUEUE_MSG_DIR))
    fail("Error syncing the new directory.");
  if(unlink(tmpfile.c_str()))
    fail("Error unlinking the temp file.");
  return true;
}

int main(int, char*[])
{
  if(config_read("adminaddr", adminaddr) && !!adminaddr) {
    remapadmin = true;
    read_hostnames();
  }
  
  if(!deliver())
    return 1;
  trigger();
  return 0;
}
