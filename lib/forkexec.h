#ifndef NULLMAILER__FORK_EXEC__H
#define NULLMAILER__FORK_EXEC__H

#include <sys/types.h>
#include <sys/wait.h>
#include "mystring/mystring.h"
#include "autoclose.h"

mystring program_path(const char* basedir, const char* name, const char* envvar);

#define REDIRECT_NULL -2
#define REDIRECT_PIPE_FROM -3
#define REDIRECT_PIPE_TO -4

class fork_exec
{
 private:
  pid_t pid;
  const char* name;

 public:
  fork_exec(const char*);
  ~fork_exec();
  bool operator!() const;

  bool start(const char* args[], int redirn, int redirs[]);
  bool start(const char* program, int redirn, int redirs[]);
  bool wait();
  int wait_status();
  inline void kill(int sig) { ::kill(pid, sig); }
};

class queue_pipe : public fork_exec
{
  public:
  queue_pipe();
  int start();
};

#endif
