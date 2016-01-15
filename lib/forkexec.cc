// nullmailer -- a simple relay-only MTA
// Copyright (C) 2016  Bruce Guenter <bruce@untroubled.org>
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
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "cli++/cli++.h"
#include "fdbuf/fdbuf.h"
#include "mystring/mystring.h"
#include "defines.h"
#include "errcodes.h"
#include "forkexec.h"

#define ERR(MSG) do{ ferr << cli_program << ": " << MSG << ": " << strerror(errno) << endl; } while(0)
#define FAIL(MSG) do{ ERR(MSG); return false; }while(0)

fork_exec::fork_exec(const char* p)
  : wfd(-1), pid(-1), name(p)
{
}

fork_exec::~fork_exec()
{
  wait();
}

bool fork_exec::operator!() const
{
  return pid < 0;
}

bool fork_exec::start(const char* program)
{
  int pipe1[2];
  autoclose fdnull;
  if (pipe(pipe1) < 0)
    FAIL("Could not create pipe");
  if ((fdnull = open("/dev/null", O_RDWR)) < 0) {
    ::close(pipe1[0]);
    ::close(pipe1[1]);
    FAIL("Could not open \"/dev/null\"");
  }
  if ((pid = fork()) < 0) {
    ::close(pipe1[0]);
    ::close(pipe1[1]);
    FAIL("Could not fork");
  }
  if (pid == 0) {
    // Child process, exec program
    dup2(pipe1[0], 0);
    ::close(pipe1[0]);
    ::close(pipe1[1]);
    dup2(fdnull, 1);
    dup2(fdnull, 2);
    fdnull.close();
    const char* argv[] = { program, NULL };
    execv(argv[0], (char**)argv);
    ERR("Could not exec " << name);
    _exit(ERR_EXEC_FAILED);
  }
  wfd = pipe1[1];
  ::close(pipe1[0]);
  return true;
}

void fork_exec::close()
{
  wfd.close();
}

bool fork_exec::wait()
{
  if (pid > 0) {
    close();
    int status;
    if (waitpid(pid, &status, 0) < 0)
      FAIL("Error catching the return value from " << name);
    pid = -1;
    if (WIFEXITED(status)) {
      status = WEXITSTATUS(status);
      if (status) {
        ferr << cli_program << ": " << name << " failed: " << status << endl;
        return false;
      }
    }
    else
      FAIL(name << " crashed or was killed");
  }
  return true;
}

static const char* nqpath()
{
  static mystring cache;
  if (!cache) {
    const char* env;
    if ((env = getenv("NULLMAILER_QUEUE")) != 0)
      cache = env;
    else {
      cache = SBIN_DIR;
      cache += "/nullmailer-queue";
    }
  }
  return cache.c_str();
}

queue_pipe::queue_pipe()
  : fork_exec("nullmailer-queue")
{
}

bool queue_pipe::start()
{
  return fork_exec::start(nqpath());
}
