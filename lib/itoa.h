#ifndef ITOA__H__
#define ITOA__H__

#ifndef INTLENGTH
#define INTLENGTH 64
/* 40 digits is long enough to handle unsigned 128-bit numbers */
#endif

const char *itoa(long, int = 0);

#endif
