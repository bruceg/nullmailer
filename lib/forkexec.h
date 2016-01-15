#ifndef NULLMAILER__FORK_EXEC__H
#define NULLMAILER__FORK_EXEC__H

#include <sys/types.h>
#include <sys/wait.h>
#include "autoclose.h"

class fork_exec
{
 private:
  autoclose wfd;
  pid_t pid;
  const char* name;

 public:
  fork_exec(const char*);
  ~fork_exec();
  bool operator!() const;
  inline int fd_to() const { return wfd; }

  bool start(const char* program, int redir_from = -1, int redir_to = -1);
  void close();
  bool wait();
  int wait_status();
  inline void kill(int sig) { ::kill(pid, sig); }
};

class queue_pipe : public fork_exec
{
  public:
  queue_pipe();
  bool start();
};

#endif
