// nullmailer -- a simple relay-only MTA
// Copyright (C) 2007  Bruce Guenter <bruce@untroubled.org>
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
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ac/systime.h"
#include "configio.h"
#include "defines.h"
#include "errcodes.h"
#include "fdbuf/fdbuf.h"
#include "hostname.h"
#include "itoa.h"
#include "list.h"
#include "selfpipe.h"
#include "setenv.h"

selfpipe selfpipe;

typedef enum { tempfail=-1, permfail=0, success=1 } tristate;

struct message
{
  time_t timestamp;
  mystring filename;
};

typedef list<mystring> slist;
typedef list<struct message> msglist;

#define msg1(MSG) do{ fout << MSG << endl; }while(0)
#define msg2(MSG1,MSG2) do{ fout << MSG1 << MSG2 << endl; }while(0)
#define msg1sys(MSG) do{ fout << MSG << strerror(errno) << endl; }while(0)
#define fail(MSG) do { msg1(MSG); return false; } while(0)
#define fail2(MSG1,MSG2) do{ msg2(MSG1,MSG2); return false; }while(0)
#define fail1sys(MSG) do{ msg1sys(MSG); return false; }while(0)
#define tempfail1sys(MSG) do{ msg1sys(MSG); return tempfail; }while(0)

struct remote
{
  static const mystring default_proto;
  
  mystring host;
  mystring proto;
  slist options;
  remote(const slist& list);
  ~remote();
};

const mystring remote::default_proto = "smtp";

remote::remote(const slist& lst)
{
  slist::const_iter iter = lst;
  host = *iter;
  ++iter;
  if(!iter)
    proto = default_proto;
  else {
    proto = *iter;
    for(++iter; iter; ++iter)
      options.append(*iter);
  }
}

remote::~remote() { }

typedef list<remote> rlist;

unsigned ws_split(const mystring& str, slist& lst)
{
  lst.empty();
  const char* ptr = str.c_str();
  const char* end = ptr + str.length();
  unsigned count = 0;
  for(;;) {
    while(ptr < end && isspace(*ptr))
      ++ptr;
    const char* start = ptr;
    while(ptr < end && !isspace(*ptr))
      ++ptr;
    if(ptr == start)
      break;
    lst.append(mystring(start, ptr-start));
    ++count;
  }
  return count;
}

static rlist remotes;
static int pausetime = 60;
static int sendtimeout = 60*60;
static int queuelifetime = 7*24*60*60;

bool load_remotes()
{
  slist rtmp;
  if(!config_readlist("remotes", rtmp) ||
     rtmp.count() == 0)
    return false;
  remotes.empty();
  for(slist::const_iter r(rtmp); r; r++) {
    if((*r)[0] == '#')
      continue;
    slist parts;
    if(!ws_split(*r, parts))
      continue;
    remotes.append(remote(parts));
  }
  return remotes.count() > 0;
}

bool load_config()
{
  mystring hh;
  bool result = true;

  if (!config_read("helohost", hh))
    hh = me;
  setenv("HELOHOST", hh.c_str(), 1);

  if(!load_remotes())
    result = false;
  
  if(!config_readint("pausetime", pausetime))
    pausetime = 60;
  if(!config_readint("sendtimeout", sendtimeout))
    sendtimeout = 60*60;
  if(!config_readint("queuelifetime", queuelifetime))
    queuelifetime = 7*24*60*60;

  return result;
}

static msglist messages;
static bool reload_messages = false;

void catch_alrm(int)
{
  signal(SIGALRM, catch_alrm);
  reload_messages = true;
}

bool load_messages()
{
  reload_messages = false;
  fout << "Rescanning queue." << endl;
  DIR* dir = opendir(".");
  if(!dir)
    fail1sys("Cannot open queue directory: ");
  messages.empty();
  struct dirent* entry;
  while((entry = readdir(dir)) != 0) {
    const char* name = entry->d_name;
    struct stat st;
    if (stat(name, &st) < 0) {
      fout << "Could not stat " << name << ", skipping." << endl;
      continue;
    }
    struct message m = { st.st_mtime, name };
    messages.append(m);
  }
  closedir(dir);
  return true;
}

void exec_protocol(int fd, remote& remote)
{
  if(close(0) == -1 || dup2(fd, 0) == -1 || close(fd) == -1)
    return;
  mystring program = PROTOCOL_DIR + remote.proto;
  const char* args[3+remote.options.count()];
  unsigned i = 0;
  args[i++] = program.c_str();
  for(slist::const_iter opt(remote.options); opt; opt++)
    args[i++] = strdup((*opt).c_str());
  args[i++] = remote.host.c_str();
  args[i++] = 0;
  execv(args[0], (char**)args);
}

tristate catchsender(pid_t pid)
{
  int status;

  for (;;) {
    switch (selfpipe.waitsig(sendtimeout)) {
    case 0:			// timeout
      kill(pid, SIGTERM);
      fout << "Sending timed out, killing protocol" << endl;
      waitpid(pid, &status, 0);
      return tempfail;
    case -1:
      msg1sys("Error waiting for the child signal: ");
      return tempfail;
    case SIGCHLD:
      break;
    default:
      continue;
    }
    break;
  }

  if(waitpid(pid, &status, 0) == -1) {
    fout << "Error catching the child process return value: "
	 << strerror(errno) << endl;
    return tempfail;
  }
  else {
    if(WIFEXITED(status)) {
      status = WEXITSTATUS(status);
      if(status) {
	fout << "Sending failed: " << errorstr(status) << endl;
	return (status & ERR_PERMANENT_FLAG) ? permfail : tempfail;
      }
      else {
	fout << "Sent file." << endl;
	return success;
      }
    }
    else {
      fout << "Sending process crashed or was killed." << endl;
      return tempfail;
    }
  }
}

tristate send_one(mystring filename, remote& remote)
{
  int fd = open(filename.c_str(), O_RDONLY);
  if(fd == -1) {
    fout << "Can't open file '" << filename << "'" << endl;
    return tempfail;
  }
  fout << "Starting delivery: protocol: " << remote.proto
       << " host: " << remote.host
       << " file: " << filename << endl;
  pid_t pid = fork();
  switch(pid) {
  case -1:
    msg1sys("Fork failed: ");
    return tempfail;
  case 0:
    exec_protocol(fd, remote);
    exit(ERR_EXEC_FAILED);
  default:
    close(fd);
    return catchsender(pid);
  }
}

bool bounce_msg(struct message& msg)
{
  mystring failed = "../failed/";
  failed += msg.filename;
  fout << "Moving message " << msg.filename << " into failed" << endl;
  if (rename(msg.filename.c_str(), failed.c_str()) == -1) {
    fout << "Can't rename file: " << strerror(errno) << endl;
    return false;
  }
  return true;
}

void send_all()
{
  if(!load_config()) {
    fout << "Could not load the config" << endl;
    return;
  }
  if(remotes.count() <= 0) {
    fout << "No remote hosts listed for delivery";
    return;
  }
  if(messages.count() == 0)
    return;
  fout << "Starting delivery, "
       << itoa(messages.count()) << " message(s) in queue." << endl;
  for(rlist::iter remote(remotes); remote; remote++) {
    msglist::iter msg(messages);
    while(msg) {
      switch (send_one((*msg).filename, *remote)) {
      case tempfail:
	if (time(0) - (*msg).timestamp > queuelifetime) {
	  if (bounce_msg(*msg)) {
	    messages.remove(msg);
	    continue;
	  }
	}
	msg++;
	break;
      case permfail:
	if (bounce_msg(*msg))
	  messages.remove(msg);
	else
	  msg++;
	break;
      default:
	if(unlink((*msg).filename.c_str()) == -1) {
	  fout << "Can't unlink file: " << strerror(errno) << endl;
	  msg++;
	}
	else
	  messages.remove(msg);
      }
    }
  }
  fout << "Delivery complete, "
       << itoa(messages.count()) << " message(s) remain." << endl;
}

static int trigger;
#ifdef NAMEDPIPEBUG
static int trigger2;
#endif

bool open_trigger()
{
  trigger = open(QUEUE_TRIGGER, O_RDONLY|O_NONBLOCK);
#ifdef NAMEDPIPEBUG
  trigger2 = open(QUEUE_TRIGGER, O_WRONLY|O_NONBLOCK);
#endif
  if(trigger == -1)
    fail1sys("Could not open trigger file: ");
  return true;
}

bool read_trigger()
{
  if(trigger != -1) {
    char buf[1024];
    read(trigger, buf, sizeof buf);
#ifdef NAMEDPIPEBUG
    close(trigger2);
#endif
    close(trigger);
  }
  return open_trigger();
}

bool do_select()
{
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(trigger, &readfds);
  struct timeval timeout;
  timeout.tv_sec = pausetime;
  timeout.tv_usec = 0;
  int s = select(trigger+1, &readfds, 0, 0,
		 (messages.count() == 0) ? 0 : &timeout);
  if(s == 1) {
    fout << "Trigger pulled." << endl;
    read_trigger();
    reload_messages = true;
  }
  else if(s == -1 && errno != EINTR)
    fail1sys("Internal error in select: ");
  else if(s == 0)
    reload_messages = true;
  if(reload_messages)
    load_messages();
  return true;
}

int main(int, char*[])
{
  read_hostnames();

  if(!selfpipe) {
    fout << "Could not set up self-pipe." << endl;
    return 1;
  }
  selfpipe.catchsig(SIGCHLD);
  
  if(!open_trigger())
    return 1;
  if(chdir(QUEUE_MSG_DIR) == -1) {
    fout << "Could not chdir to queue message directory." << endl;
    return 1;
  }
  
  signal(SIGALRM, catch_alrm);
  signal(SIGHUP, SIG_IGN);
  load_config();
  load_messages();
  for(;;) {
    send_all();
    if (pausetime == 0) break;
    do_select();
  }
  return 0;
}
