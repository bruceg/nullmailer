#include "config.h"
#include "defines.h"
#include <sys/time.h>
#include <unistd.h>
#include "itoa.h"
#include "mystring/mystring.h"

mystring make_date()
{
  char buf[256];
  time_t t = time(0);
  struct tm* l = localtime(&t);
  strftime(buf, 256, "%a, %d %b %Y %H:%M:%S ", l);
#ifdef TM_HAS_GMTOFF
  long tznum = l->TM_HAS_GMTOFF/60;
#else
  long tznum = -timezone/60;
#if TM_HAS_ISDST
  int daylight = l->TM_HAS_ISDST;
#endif // TM_HAS_ISDST
  if(daylight)
    tznum += 60;
#endif // TM_HAS_GMTOFF
  char tz[6];
  tz[0] = '+';
  if(tznum < 0) {
    tznum = -tznum;
    tz[0] = '-';
  }
  long tzhours = tznum / 60;
  tz[1] = (tzhours/10)%10 + '0';
  tz[2] = tzhours%10 + '0';
  long tzmins = tznum % 60;
  tz[3] = (tzmins/10)%10 + '0';
  tz[4] = tzmins%10 + '0';
  tz[5] = 0;
  return mystringjoin(buf) + tz;
}

extern mystring idhost;

// Message ID strings have the form SECONDS.USEC.PID.nullmailer@HOST
mystring make_messageid()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  mystring tmp = mystringjoin("<") + itoa(tv.tv_sec) + ".";
  tmp = tmp + itoa(tv.tv_usec, 6) + ".";
  return tmp + itoa(getpid()) + ".nullmailer@" + idhost + ">";
}
