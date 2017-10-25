// nullmailer -- a simple relay-only MTA
// Copyright (C) 2017  Bruce Guenter <bruce@untroubled.org>
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
//
// You can contact me at <bruce@untroubled.org>.  There is also a mailing list
// available to discuss this package.  To subscribe, send an email to
// <nullmailer-subscribe@lists.untroubled.org>.

#include "base64.h"

static char basis[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const mystring& in, mystring& out)
{
  size_t length;
  const unsigned char* ptr;
  char buf[4];
  for (length = in.length(), ptr = (const unsigned char*)in.c_str();
       length >= 3;
       length -= 3, ptr += 3) {
    base64_encode_chunk(ptr, 3, buf);
    out.append(buf, 4);
  }
  if (length > 0) {
    base64_encode_chunk(ptr, length, buf);
    out.append(buf, 4);
  }
}

void base64_encode_chunk(const unsigned char bin[3], unsigned len,
			 char encoded[4])
{
  encoded[0] = basis[bin[0] >> 2];
  switch(len) {
  case 1:
    encoded[1] = basis[(bin[0] << 4) & 0x3f];
    encoded[2] = encoded[3] = '=';
    break;
  case 2:
    encoded[1] = basis[(bin[0] << 4 | bin[1] >> 4) & 0x3f];
    encoded[2] = basis[(bin[1] << 2) & 0x3f];
    encoded[3] = '=';
    break;
  case 3:
    encoded[1] = basis[(bin[0] << 4 | bin[1] >> 4) & 0x3f];
    encoded[2] = basis[(bin[1] << 2 | bin[2] >> 6) & 0x3f];
    encoded[3] = basis[bin[2] & 0x3f];
  }
}
