#include "config.h"
#include <ctype.h>
#include "canonicalize.h"
#include "mystring/mystring.h"
#include "address.h"

#include "fdbuf/fdbuf.h"
#include "itoa.h"

static bool test(const mystring& in,
		 const mystring& out,
		 const mystring& list)
{
  mystring line = in;
  mystring tmplist;
  if(!parse_addresses(line, tmplist)) {
    fout << "Parsing of '" << in << "' failed." << endl;
    return false;
  }
  bool status = true;
  if(!!list && tmplist != list) {
    fout << "Parsing of '" << in << "' failed: bad result list, was:\n"
	 << tmplist
	 << "should be:\n"
	 << list;
    status = false;
  }
  if(!!out && line != out) {
    fout << "Parsing of '" << in << "' failed: bad result string, was:\n"
	 << line
	 << "should be:\n"
	 << out;
    status = false;
  }
  return status;
}

#define TEST(X,Y,Z) do{ ++count; if(!test(X,Y,Z)) ++failed; }while(0)

mystring defaulthost = "a";
mystring defaultdomain = "b.c";

int main()
{
  int count = 0;
  int failed = 0;
  // empty list
  TEST("",
       "",
       "");
  // empty list with comment
  TEST("(no addresses)",
       "(no addresses)",
       "");
  // periods in local
  TEST("a.b@c.d",
       "a.b@c.d",
       "a.b@c.d\n");
  // quoted local
  TEST("\"e\"@c.d",
       "e@c.d",
       "e@c.d\n");
  // missing host and domain
  TEST("e",
       "e@a.b.c",
       "e@a.b.c\n");
  // missing domain
  TEST("e@x",
       "e@x.b.c",
       "e@x.b.c\n");
  // trailing period
  TEST("e@c.d.",
       "e@c.d",
       "e@c.d\n");
  // comment <address> style
  TEST("x<y@a.b>",
       "x <y@a.b>",
       "y@a.b\n");
  TEST("<y@a.b>",
       "<y@a.b>",
       "y@a.b\n");
  // address (comment) style
  TEST("y@a.b(x)",
       "y@a.b (x)",
       "y@a.b\n");
  // internal comments before local
  TEST("(j)y@a.b",
       "y@a.b (j)",
       "y@a.b\n");
  // internal comments after local
  TEST("y(j)@a.b",
       "y@a.b (j)",
       "y@a.b\n");
  // internal comments before domain
  TEST("y@(j)a.b",
       "y@a.b (j)",
       "y@a.b\n");
  // internal comments before period
  TEST("y@a(j).b",
       "y@a.b (j)",
       "y@a.b\n");
  // internal comments after period
  TEST("y@a.(j)b",
       "y@a.b (j)",
       "y@a.b\n");
  // normal list
  TEST("a@b.c,d@e.f",
       "a@b.c, d@e.f",
       "a@b.c\nd@e.f\n");
  // list with comments
  TEST("a@b.c(j),d@e.f(k)",
       "a@b.c (j), d@e.f (k)",
       "a@b.c\nd@e.f\n");
  // list without commas
  TEST("a@b.c d@e.f",
       "a@b.c, d@e.f",
       "a@b.c\nd@e.f\n");
  // list without commas with comments
  TEST("a@b.c(j) d@e.f(k)",
       "a@b.c (j), d@e.f (k)",
       "a@b.c\nd@e.f\n");
  // simple group
  TEST("g: a@b.c, d@e.f;",
       "g: a@b.c, d@e.f;",
       "a@b.c\nd@e.f\n");
  // group with spaces in name
  TEST("g h: a@b.c, d@e.f;",
       "g h: a@b.c, d@e.f;",
       "a@b.c\nd@e.f\n");
  // empty group
  TEST("g: ;",
       "g: ;",
       "");
  // group with a comment
  TEST("g: a@b.c(j);",
       "g: a@b.c (j);",
       "a@b.c\n");
  // group with comments
  TEST("g:a@b.c(j),d@e.f(k);",
       "g: a@b.c (j), d@e.f (k);",
       "a@b.c\nd@e.f\n");
  // group with no commas
  TEST("g:a@b.c d@e.f;",
       "g: a@b.c, d@e.f;",
       "a@b.c\nd@e.f\n");
  // group with route addresses
  TEST("g:foo<a@b.c>;",
       "g: foo <a@b.c>;",
       "a@b.c\n");
  // route-path syntax (stripped)
  TEST("f<@g.h:a@b.c>",
       "f <a@b.c>",
       "a@b.c\n");
  // multiple route-path syntax
  TEST("f<@g.h@i.j:a@b.c>",
       "f <a@b.c>",
       "a@b.c\n");
  // comments with quoted brackets
  TEST("(f\\)\\()a@b.c",
       "a@b.c (f\\)\\()",
       "a@b.c\n");
  // nested comments
  TEST("(f(g)h)a@b.c",
       "a@b.c (f(g)h)",
       "a@b.c\n");
  // simple quoted addresses
  TEST("\"a\"@b.c",
       "a@b.c",
       "a@b.c\n");
  // quoted parts of address
  TEST("a.\"b\".c@d.e",
       "a.b.c@d.e",
       "a.b.c@d.e\n");
  // escaped characters within quotes
  TEST("\"s\\'b\"@d.e",
       "s'b@d.e",
       "s'b@d.e\n");
  // escaped specials
  TEST("\"s\\\"a\\\"b\"@d.e",
       "\"s\\\"a\\\"b\"@d.e",
       "s\"a\"b@d.e\n");
  // twisted syntax
  //TEST("\"\\\"d\\\" <\"<@_._:e@f.g>",
  //     "who knows",
  //     "e@f.g\n");
  
  fout << itoa(count) << " tests run, ";
  fout << itoa(failed) << " failed." << endl;
  return failed;
}
