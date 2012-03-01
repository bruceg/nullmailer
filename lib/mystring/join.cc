#include "mystring.h"
#include <string.h>

// This "join" class relies on one fairly obscure detail in the C++
// standard: temporaries are destructed only after the entire
// "full-expression" has completed.  That is, if the sequence:
// f(f(f(x))) creates three temporary objects, the inner objects are
// destroyed only after the execution has completed.  This allows us
// to build a complete linked-list on the stack.  Tricky, but efficient!

struct tmpitem
{
  const char* str;
  unsigned len;
};

mystringrep* mystringjoin::traverse() const
{
  // At first glance, a recursive implementation would be the most logical
  // way of doing this, but it turned out to be a significant loss.  This
  // method traverses the pointer chain to first determine the length, and
  // then to do the actual copying.

  // Note the use of do/while loops to avoid a test at the head of the loop
  // which will always succeed (there is always at least one node or item).
  unsigned count = 0;
  const mystringjoin* node = this;
  do {
    ++count;
  } while((node = node->prev) != 0);

  // The use of a temporary array avoids re-traversing the pointer
  // chain, which is a slight performance win.
  tmpitem items[count];
  
  unsigned length = 0;
  node = this;
  tmpitem* item = items;
  do {
    unsigned l = node->rep ? node->rep->length : strlen(node->str);
    length += l;
    item->str = node->str;
    item->len = l;
    ++item;
  } while((node = node->prev) != 0);

  // Since the chain is constructed such that the last item is the
  // first node, the string gets constructed in reverse order.
  mystringrep* rep = mystringrep::alloc(length);
  char* buf = rep->buf + length;
  item = items;
  do {
    unsigned l = item->len;
    buf -= l;
    memcpy(buf, item->str, l);
    ++item;
  } while(--count != 0);

  rep->buf[length] = 0;
  return rep;
}
