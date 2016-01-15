#ifndef NULLMAILER__FORK_EXEC__H
#define NULLMAILER__FORK_EXEC__H

#include <sys/types.h>
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
};

class queue_pipe : public fork_exec
{
  public:
  queue_pipe();
  bool start();
};

#endif
