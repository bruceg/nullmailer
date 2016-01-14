#ifndef NULLMAILER_AUTOCLOSE__H__
#define NULLMAILER_AUTOCLOSE__H__

#include <unistd.h>

// Simple inline wrapper to automatically close an open file descriptor
class autoclose
{
  private:
  int fd;

  public:
  inline autoclose(int f) : fd(f) { }
  inline ~autoclose() { close(); }
  inline operator int() const { return fd; }
  inline void close()
  {
    if (fd >= 0) {
      ::close(fd);
      fd = -1;
    }
  }
};

#endif // NULLMAILER_AUTOCLOSE__H__
