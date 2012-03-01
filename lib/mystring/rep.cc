#include "mystring.h"
#include "trace.h"
#include <ctype.h>
#include <string.h>

mystringrep nil = { 0, 1, 1, "" };

static const unsigned replength = sizeof(unsigned)*3;

static const unsigned sizestep = sizeof(unsigned);
static const unsigned slackdiv = 4;
static const unsigned slackmax = 16;

#ifdef MYSTRINGREP_STATS

#include "fdbuf.h"

struct _rep_stats
{
  unsigned allocs;
  unsigned alloc_size;
  unsigned alloc_len;
  
  unsigned appends;
  unsigned appends_dup;

  _rep_stats()
    : allocs(0)
    {
    }
  
  void stat(const char* name, unsigned value)
    {
      ferr << "mystringrep: " << name << ": " << value << '\n';
    }
  void pcnt(const char* name, unsigned denom, unsigned divis)
    {
      ferr << "mystringrep: " << name << ": "
	   << denom << '/' << divis << '=';
      if(divis) ferr << denom * 100 / divis << '%';
      else ferr << "N/A";
      ferr << '\n';
    }
  
  ~_rep_stats()
    {
      stat("     size step", sizestep);
      stat(" slack divisor", slackdiv);
      stat(" slack maximum", slackmax);
      stat("        allocs", allocs);
      stat("  alloc length", alloc_len);
      stat("    alloc size", alloc_size);
      pcnt("   alloc slack", alloc_size-alloc_len, alloc_len);
      stat("alloc overhead", allocs*replength);
      pcnt("  appends->dup", appends_dup, appends);
    }
};

static _rep_stats stats;

#define ACCOUNT(NAME,VALUE) stats. NAME += VALUE

#else // MYSTRINGREP_STATS

#define ACCOUNT(NAME,VALUE)

#endif // MYSTRINGREP_STATS

///////////////////////////////////////////////////////////////////////////////
// class mystringrep
///////////////////////////////////////////////////////////////////////////////
mystringrep* mystringrep::alloc(unsigned length)
{
  ACCOUNT(allocs, 1);
  trace_static("length=" << length);
  if(length == 0)
    return &nil;

  ACCOUNT(alloc_len, length);
  unsigned slack = length / slackdiv;
  if(slack > slackmax)
    slack = slackmax;
  unsigned size = length+1 + sizestep-1 + slack;
  size = size - size % sizestep;
  ACCOUNT(alloc_size, size);

  mystringrep* ptr = (mystringrep*)new char[size+replength];
  ptr->length = length;
  ptr->references = 0;
  ptr->size = size;
  return ptr;
}

mystringrep* mystringrep::dup(const char* str, unsigned length)
{
  trace_static("str=" << (void*)str << " length=" << length);
  if(length == 0)
    return &nil;
  mystringrep* ptr = alloc(length);
  memcpy(ptr->buf, str, length);
  ptr->buf[length] = 0;
  return ptr;
}

mystringrep* mystringrep::dup(const char* str1, unsigned length1,
			      const char* str2, unsigned length2)
{
  trace_static("");
  if(length1+length2 == 0)
    return &nil;
  mystringrep* ptr = alloc(length1+length2);
  memcpy(ptr->buf, str1, length1);
  memcpy(ptr->buf+length1, str2, length2);
  ptr->buf[length1+length2] = 0;
  return ptr;
}

mystringrep* mystringrep::append(const char* str, unsigned len)
{
  ACCOUNT(appends, 1);
  unsigned newlen = length + len;
  // If there are more than one references, always make a duplicate
  // Also, if this does not have enough space to add the new string, dup it
  if(references > 1 || newlen >= size) {
    ACCOUNT(appends_dup, 1);
    mystringrep* tmp = dup(buf, length, str, len);
    tmp->attach();
    detach();
    return tmp;
  }
  // Otherwise, just add the new string to the end of this
  else {
    memcpy(buf+length, str, len);
    buf[newlen] = 0;
    length = newlen;
    return this;
  }
}

#ifdef MYSTRING_TRACE    
void mystringrep::attach()
{
  trace("references=" << references);
  ++references;
}
#endif

void mystringrep::detach()
{
  trace("references=" << references);
  
  --references;
  if(!references) {
    trace("deleting this");
    delete this;
  }
}
