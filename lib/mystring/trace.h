/* $Id: trace.h 616 2005-08-19 20:11:01Z bruce $ */
#include "mystring.h"

#ifdef MYSTRING_TRACE
ostream& operator<<(ostream& out, const mystringtmp& s);
#define trace(X) cerr << (void*)this << "->" << __PRETTY_FUNCTION__ << X << endl
#define trace_static(X) cerr << __PRETTY_FUNCTION__ << X << endl
#else
#define trace(X) do { } while(0)
#define trace_static(X) do { } while(0)
#endif
