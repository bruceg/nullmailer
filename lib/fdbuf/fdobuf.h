// Copyright (C) 1999,2000 Bruce Guenter <bruce@untroubled.org>
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

#ifndef FDBUF__FDOBUF__H__
#define FDBUF__FDOBUF__H__

#include <unistd.h>

class fdobuf : protected fdbuf
{
public:
  enum openflags { create=O_CREAT, 
		   excl=O_EXCL,
		   trunc=O_TRUNC,
		   append=O_APPEND };

  fdobuf(const char* filename, int, int mode = 0666,
	 unsigned bufsz = FDBUF_SIZE);
  fdobuf(int fdesc, bool dc=false, unsigned bufsz = FDBUF_SIZE);
  virtual ~fdobuf();
  bool close();
  bool operator!() const;
  operator bool() const
    {
      return !operator!();
    }
  bool flush();
  bool sync();
  virtual bool write(char);
  bool write(unsigned char c) { return write((char)c); }
  bool write(signed char c) { return write((char)c); }
  virtual bool write(const char*, unsigned);
  bool write(const unsigned char* b, unsigned l) { return write((char*)b, l); }
  bool write(const signed char* b, unsigned l) { return write((char*)b, l); }
  virtual bool write_large(const char*, unsigned);
  unsigned last_count() { return count; }
  bool seek(unsigned o);
  bool rewind() { return seek(0); }
  unsigned tell() const { return offset + bufpos; }

  bool chown(uid_t, gid_t) const;
  bool chmod(mode_t) const;
  
  fdobuf& operator<<(const char* str)
    {
      write(str, strlen(str));
      return *this;
    }
  fdobuf& operator<<(char ch)
    {
      write(ch);
      return *this;
    }
  fdobuf& operator<<(fdobuf& (*manip)(fdobuf&))
    {
      return manip(*this);
    }
  fdobuf& operator<<(unsigned long);
  fdobuf& operator<<(signed long);
  fdobuf& operator<<(unsigned i) { return operator<<((unsigned long)i); }
  fdobuf& operator<<(signed i) { return operator<<((signed long)i); }
  fdobuf& operator<<(unsigned short i) { return operator<<((unsigned long)i); }
  fdobuf& operator<<(signed short i) { return operator<<((signed long)i); }

  int error_number() const { return errnum; }
protected:
  virtual bool nflush(bool withsync);

  unsigned bufpos;		// Current write position in the buffer
  unsigned count;		// Number of bytes written by last operation
};

fdobuf& endl(fdobuf& fd);

extern fdobuf fout;
extern fdobuf ferr;

#endif // FDBUF__FDOBUF__H__
