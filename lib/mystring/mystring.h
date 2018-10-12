/* $Id: mystring.h 635 2005-11-02 17:37:50Z bruce $ */
// Copyright (C) 2018 Bruce Guenter <bruce@untroubled.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef MYSTRING__H__
#define MYSTRING__H__

#include <sys/types.h>
#include "mystring/rep.h"

class mystringjoin;
class mystring
{
  friend class mystringtmp;
  friend class mystringjoin;
private:
  mystringrep* rep;

protected:
  void dupnil();
  void dup(const char*, size_t);
  void dup(const char*);
  void assign(const char*);
  void assign(const char*, size_t);
public:
  static const mystring NUL;
  
  mystring() { dupnil(); }
  mystring(const char* s) { dup(s); }
  mystring(const mystring& s) { dup(s.rep->buf, s.rep->length); }
  mystring(const char* str, size_t len) { dup(str, len); }
  mystring(const mystringjoin&);
  ~mystring();

  const char* c_str() const { return rep->buf; }

  bool operator!() const { return empty(); }
  
  char operator[](size_t i) const { return rep->buf[i]; }
  
  size_t length() const { return rep->length; }

  bool empty() const { return rep->length == 0; }
  
  int operator!=(const char* in) const;
  int operator!=(const mystring& in) const;
  bool operator==(const char* in) const
    {
      return !operator!=(in);
    }
  bool operator==(const mystring& in) const
    {
      return !operator!=(in);
    }

  void operator=(const char* in) { assign(in); }
  void operator=(const mystring& in) { assign(in.rep->buf, in.rep->length); }
  void operator=(const mystringjoin& in);

  mystring subst(char from, char to) const;
  
  mystring lower() const;
  mystring upper() const;

  bool starts_with(const mystring&) const;
  bool starts_with(const char*) const;
  bool starts_with(const char*, size_t) const;

  int find_first(char, size_t = 0) const;
  int find_first_of(const mystring&, size_t = 0) const;
  int find_first_of(const char*, size_t = 0) const;
  int find_first_of(const char*, size_t, size_t) const;

  int find_last(char, size_t = (size_t)-1) const;
  int find_last_of(const mystring&, size_t = (size_t)-1) const;
  int find_last_of(const char*, size_t = 0) const;
  int find_last_of(const char*, size_t, size_t) const;

  mystring left(size_t) const;
  mystring right(size_t) const;
  mystring sub(size_t, size_t) const;

  mystring lstrip() const;
  mystring rstrip() const;
  mystring strip() const;

  unsigned count(char ch) const;
  
  void append(const char*);
  void append(const char*, size_t);

  void operator+=(const mystring& str) {append(str.rep->buf, str.rep->length);}
  void operator+=(const char* str) { append(str); }
  void operator+=(char ch)
    {
      char str[2] = { ch, 0 };
      append(str, 1);
    }
};

#ifndef MYSTRING_TRACE
inline mystring::~mystring()
{
  rep->detach();
}
#endif

#include "mystring/iter.h"
#include "mystring/join.h"

class fdobuf;
fdobuf& operator<<(fdobuf& out, const mystring& str);

//istream& operator>>(istream& in, mystring& str);

typedef mystring string;

#endif
