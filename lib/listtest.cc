#include <iostream.h>
#include "list.h"

typedef list<int> ilist;
typedef ilist::iter iiter;

void test_remove_first()
{
  ilist l;
  l.append(1);
  l.append(2);
  iiter i(l);
  l.remove(i);
  if(!i) cout << "After removing first, iter no longer valid\n";
  else if(*i != 2) cout << "After removing first, iter is wrong\n";
  if(l.count() != 1) cout << "After removing first, count is wrong\n";
}

void test_remove_mid()
{
  ilist l;
  l.append(1);
  l.append(2);
  l.append(3);
  iiter i(l);
  i++;
  l.remove(i);
  if(!i) cout << "After removing middle, iter no longer valid\n";
  else if(*i != 3) cout << "After removing middle, iter is wrong\n";
  if(l.count() != 2) cout << "After removing middle, count is wrong\n";
}

void test_remove_last()
{
  ilist l;
  l.append(1);
  l.append(2);
  iiter i(l);
  i++;
  l.remove(i);
  if(i) cout << "After removing last, iter is still valid\n";
  if(l.count() != 1) cout << "After removing last, count is wrong\n";
}

int main() {
  test_remove_first();
  test_remove_mid();
  test_remove_last();
  return 0;
}
