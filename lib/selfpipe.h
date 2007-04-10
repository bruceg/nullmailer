#ifndef NULLMAILER_SELFPIPE__H__
#define NULLMAILER_SELFPIPE__H__

class selfpipe
{
 public:
  selfpipe();

  operator bool() const;

  void catchsig(int sig);
  int caught();
  int waitsig(int timeout = 0);
};

#endif // NULLMAILER_SELFPIPE__H__
