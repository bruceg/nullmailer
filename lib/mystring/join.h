/* $Id: join.h 616 2005-08-19 20:11:01Z bruce $ */
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

#ifndef MYSTRING__JOIN__H__
#define MYSTRING__JOIN__H__

class mystringjoin
{
private:
  const mystringjoin* prev;
  mystringrep* rep;
  const char* str;

  mystringjoin();
public:
  mystringjoin(const mystringjoin& j)
    : prev(j.prev), rep(j.rep), str(j.str)
    {
      rep->attach();
    }
  mystringjoin(const mystring& s)
    : prev(0), rep(s.rep), str(s.rep->buf)
    {
      rep->attach();
    }
  mystringjoin(const char* s)
    : prev(0), rep(0), str(s)
    {
    }
  mystringjoin(const mystringjoin& p, const mystring& s)
    : prev(&p), rep(s.rep), str(s.rep->buf)
    {
      rep->attach();
    }
  mystringjoin(const mystringjoin& p, const char* s)
    : prev(&p), rep(0), str(s)
    {
    }
  ~mystringjoin()
    {
      if(rep) rep->detach();
    }
  mystringrep* traverse() const;
};

inline mystring::mystring(const mystringjoin& j)
  : rep(j.traverse())
{
  rep->attach();
}

inline mystringjoin operator+(const mystringjoin& a, const mystring& b)
{
  return mystringjoin(a, b);
}

inline mystringjoin operator+(const mystringjoin& a, const char* b)
{
  return mystringjoin(a, b);
}

#endif
