#ifndef NULLMAILER_AUTOCLOSE__H__
#define NULLMAILER_AUTOCLOSE__H__

#include <unistd.h>

// Simple inline wrapper to automatically close an open file descriptor
class autoclose
{
  private:
  int fd;

  public:
  inline autoclose(int f = -1) : fd(f) { }
  inline ~autoclose() { close(); }
  inline operator int() const { return fd; }
  inline int operator =(int f)
  {
    close();
    return fd = f;
  }
  inline void close()
  {
    if (fd >= 0) {
      ::close(fd);
      fd = -1;
    }
  }
};

// Simple inline wrapper to handle opening and closing a pipe pair
class autoclose_pipe
{
  private:
  int fds[2];

  public:
  inline autoclose_pipe()
  {
    fds[0] = fds[1] = -1;
  }
  inline ~autoclose_pipe()
  {
    close();
  }
  inline int operator[](int i) const { return fds[i]; }
  inline bool open()
  {
    return pipe(fds) == 0;
  }
  inline void close()
  {
    if (fds[0] >= 0) {
      ::close(fds[0]);
      ::close(fds[1]);
      fds[0] = fds[1] = -1;
    }
  }
  // Close one half of the pair, return the other, and mark both as if they were closed.
  inline int extract(int which)
  {
    int result = fds[which];
    ::close(fds[1-which]);
    fds[0] = fds[1] = -1;
    return result;
  }
};

#endif // NULLMAILER_AUTOCLOSE__H__
