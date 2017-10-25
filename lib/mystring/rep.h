/* $Id: rep.h 616 2005-08-19 20:11:01Z bruce $ */
// Copyright (C) 2017 Bruce Guenter <bruce@untroubled.org>
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

#ifndef MYSTRING__REP__H__
#define MYSTRING__REP__H__

struct mystringrep
{
  unsigned length;
  unsigned references;
  unsigned size;
  char buf[1];

  void attach();
  void detach();
  mystringrep* append(const char*, unsigned);
  
  static mystringrep* alloc(unsigned);
  static mystringrep* dup(const char*, unsigned);
  static mystringrep* dup(const char*, unsigned,
			  const char*, unsigned);
};

#ifndef MYSTRING_TRACE
inline void mystringrep::attach()
{
  references++;
}
#endif

extern mystringrep nil;

#endif
