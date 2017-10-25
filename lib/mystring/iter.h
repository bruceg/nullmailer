/* $Id: iter.h 616 2005-08-19 20:11:01Z bruce $ */
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

#ifndef MYSTRING__ITER__H__
#define MYSTRING__ITER__H__

class mystring_iter
{
  const mystring str;
  const char sep;
  int pos;
  mystring part;

  void advance();
public:
  mystring_iter(const mystring&, char = '\0');
  ~mystring_iter();
  
  operator bool() const { return pos >= 0; }
  bool operator!() const { return pos < 0; }
  mystring operator*() const { return part; }
  void operator++() { advance(); }
};

#endif
