// nullmailer -- a simple relay-only MTA
// Copyright (C) 2018  Bruce Guenter <bruce@untroubled.org>
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

static int fdnull = -1;

fork_exec::fork_exec(const char* p)
  : pid(-1), name(p)
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

bool fork_exec::start(const char* args[], int redirn, int redirs[])
{
  autoclose_pipe pipes[redirn];
  for (int i = 0; i < redirn; i++) {
    if (redirs[i] == REDIRECT_PIPE_TO || redirs[i] == REDIRECT_PIPE_FROM)
      if (!pipes[i].open())
        FAIL("Could not create pipe");
    if (redirs[i] == REDIRECT_NULL)
      if (fdnull < 0)
        if ((fdnull = open("/dev/null", O_RDWR)) < 0)
          FAIL("Could not open \"/dev/null\"");
  }
  if ((pid = fork()) < 0)
    FAIL("Could not fork");
  if (pid == 0) {
    // Child process, exec program
    for (int i = 0; i < redirn; i++) {
      int r = redirs[i];
      if (r == REDIRECT_NULL)
        dup2(fdnull, i);
      else if (r == REDIRECT_PIPE_FROM) {
        dup2(pipes[i][1], i);
        pipes[i].close();
      }
      else if (r == REDIRECT_PIPE_TO) {
        dup2(pipes[i][0], i);
        pipes[i].close();
      }
      else if (r > 0) {
        dup2(r, i);
        if (r >= redirn)
          close(r);
      }
    }
    execv(args[0], (char**)args);
    ERR("Could not exec " << name);
    _exit(ERR_EXEC_FAILED);
  }
  for (int i = 0; i < redirn; i++) {
    if (redirs[i] == REDIRECT_PIPE_TO)
      redirs[i] = pipes[i].extract(1);
    else if (redirs[i] == REDIRECT_PIPE_FROM)
      redirs[i] = pipes[i].extract(0);
  }
  return true;
}

bool fork_exec::start(const char* program, int redirn, int redirs[])
{
  const char* args[2] = { program, NULL };
  return start(args, redirn, redirs);
}

int fork_exec::wait_status()
{
  if (pid > 0) {
    int status;
    if (waitpid(pid, &status, 0) == pid) {
      pid = -1;
      return status;
    }
  }
  return -1;
}

bool fork_exec::wait()
{
  if (pid > 0) {
    int status = wait_status();
    if (status < 0)
      FAIL("Error catching the return value from " << name);
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

mystring program_path(const char* program)
{
  return CONFIG_PATH(BIN, NULL, program);
}
    
static const char* nqpath()
{
  static mystring cache;
  if (!cache) {
    const char* env;
    if ((env = getenv("NULLMAILER_QUEUE")) != NULL)
      cache = env;
    else
      cache = CONFIG_PATH(SBIN, NULL, "nullmailer-queue");
  }
  return cache.c_str();
}

queue_pipe::queue_pipe()
  : fork_exec("nullmailer-queue")
{
}

int queue_pipe::start()
{
  int redirs[] = { REDIRECT_PIPE_TO, REDIRECT_NULL, REDIRECT_NULL };
  if (!fork_exec::start(nqpath(), 3, redirs))
    return -1;
  return redirs[0];
}
