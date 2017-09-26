#include "argparse.h"

#include "fdbuf/fdbuf.h"
#include "itoa.h"

static bool doit(const char* teststr, unsigned count, const char** result)
{
  arglist args;
  unsigned c = parse_args(args, teststr);
  if (c != count) {
    fout << "Parsing of \"" << teststr << "\" failed, wrong count, was: " << c << " should be: " << count << endl;
    return false;
  }
  arglist::const_iter iter(args);
  for (unsigned i = 0; i < c; i++, iter++) {
    if (*iter != result[i]) {
      fout << "Parsing of \"" << teststr << "\" failed, wrong string, was:\n"
	   << *iter << " should be:\n" << result[i] << endl;
      return false;
    }
  }
  return true;
}

#define TEST(X,Y,...) do{ const char* result[] = {__VA_ARGS__}; ++count; if (!doit(X, Y, result)) ++failed; } while(0)

int main()
{
  int count = 0;
  int failed = 0;

  TEST("", 0);

  TEST("one", 1, "one");	// Simple case
  TEST(" two", 1, "two");	// Leading space
  TEST("three ", 1, "three");	// Trailing space
  TEST("one\\ two", 1, "one two"); // Escaped internal space
  TEST(" one two  three  ", 3, "one","two","three"); // Spaces between args
  TEST("'one'", 1, "one");			     // Simple single quoted
  TEST("'one two'", 1, "one two");		     // Single quoted with space
  TEST("'one\\'two", 1, "one\\two");		     // Single quoted with backslash
  TEST("\"one two\"", 1, "one two");		     // Double quoted
  TEST(" one \"two\"three four", 3, "one", "twothree", "four"); // Mixed quoted and unquoted
  TEST("\"one\\\" two\"", 1, "one\" two");			// Double quoted with escaped quote
  TEST("one='two three' four", 2, "one=two three", "four");	// Single quotes within an arg with multiple args
  TEST("one=\"two three\" four", 2, "one=two three", "four");	// Double quotes within an arg with multiple args

  fout << itoa(count) << " tests run, ";
  fout << itoa(failed) << " failed." << endl;
  return failed;
}
