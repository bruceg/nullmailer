// nullmailer -- a simple relay-only MTA
// Copyright (C) 2018  Bruce Guenter <bruce@untroubled.org>
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

#include "config.h"
#include <ctype.h>
#include "canonicalize.h"
#include "mystring.h"

struct result
{
  const char* next;
  mystring str;
  mystring comment;
  mystring addr;

  result(const result&);
  result(const char* = 0);
  result(const char*, const mystring&, const mystring&, const mystring&);
  bool operator!() const
    {
      return !next;
    }
  operator bool() const
    {
      return next;
    }
};

result::result(const char* n)
  : next(n)
{
}

result::result(const char* n, const mystring& s,
	       const mystring& c, const mystring& l)
  : next(n), str(s), comment(c), addr(l)
{
}

result::result(const result& r)
  : next(r.next), str(r.str), comment(r.comment), addr(r.addr)
{
}

#ifndef TRACE
#define ENTER(R)
#define FAIL(MSG) return result()
#define RETURNR(R) return R
#define RETURN(N,S,C,L) return result(N,S,C,L);
#else
#include "fdbuf.h"
static const char indentstr[] = "                       ";
static const char* indent = indentstr + sizeof indentstr - 1;
#define ENTER(R) do{ fout.write(indent--); fout.write(__FUNCTION__); fout.write(": "); fout.write(ptr); fout.write(": "); fout.writeln(R); }while(0)
#define FAIL(MSG) do{ fout.write(++indent); fout.write(__FUNCTION__); fout.write(": failed: "); fout.writeln(MSG); return result(); }while(0)
#define RETURNR(R) do{ fout.write(++indent); fout.write(__FUNCTION__); fout.write(": succeeded str="); fout.write(R.str); fout.write(" comment="); fout.writeln(R.comment); return (R); }while(0)
#define RETURN(N,S,C,L) do{ result r(N,S,C,L); RETURNR(r); }while(0)
#endif

#define RULE(X) static result match_##X(const char* ptr)
#define SKIPSPACE do{ while(*ptr && isspace(*ptr)) ++ptr; }while(0)
#define SKIPDELIM(N) do{ ptr = skipdelim(ptr, N); }while(0)
#define MATCHCHAR(X) do{ if(*ptr != X) FAIL("*ptr is not " #X); else ++ptr; }while(0)
#define OR_RULE(ALT1,ALT2) { result r=match_##ALT1(ptr); if(r) RETURNR(r); }{ result r=match_##ALT2(ptr); if(r) RETURNR(r); } FAIL("did not match " #ALT1 " OR " #ALT2);

#define COLON ':'
#define SEMICOLON ';'
#define LABRACKET '<'
#define RABRACKET '>'
#define AT '@'
#define ESCAPE '\\'
#define PERIOD '.'
#define LSQBRACKET '['
#define RSQBRACKET ']'
#define QUOTE '"'
#define CR '\n'
#define LPAREN '('
#define RPAREN ')'
#define COMMA ','

static bool isspecial(char c)
{
  switch(c) {
  case LPAREN: case RPAREN:
  case LABRACKET: case RABRACKET:
  case LSQBRACKET: case RSQBRACKET:
  case AT: case COMMA:
  case SEMICOLON: case COLON:
  case ESCAPE: case QUOTE: case PERIOD:
    return true;
  default:
    return false;
  }
}

static bool isctl(char c)
{
  return (c >= 0 && c <= 31) || (c == 127);
}
  
static bool isqtext(char c)
{
  return c && c != QUOTE && c != ESCAPE && c != CR;
}

static bool isdtext(char c)
{
  return c && c != LSQBRACKET && c != RSQBRACKET &&
    c != ESCAPE && c != CR;
}

// quoted-pair = ESCAPE CHAR
static bool isqpair(const char* ptr)
{
  return *ptr && *ptr == ESCAPE &&
    *(ptr+1);
}

static bool isctext(char c)
{
  return c && c != LPAREN && c != RPAREN &&
    c != ESCAPE && c != CR;
}

static bool isrtext(char c)
{
  return c && c != LABRACKET && c != RABRACKET &&
    c != ESCAPE && c != CR;
}

static mystring quote(const mystring& in)
{
  unsigned length = in.length();
  // The result will never be more than double the length of the input plus 2
  char out[length*2 + 2 + 1];
  char* ptrout = out;
  const char* ptrin = in.c_str();
  bool quoted = false;
  for(; length; ++ptrin, ++ptrout, --length) {
    if(isspecial(*ptrin)) {
      *ptrout++ = ESCAPE;
      quoted = true;
    }
    *ptrout = *ptrin;
  }
  *ptrout = 0;
  if(quoted)
    return mystringjoin("\"") + out + "\"";
  else
    return in;
}

static mystring unquote(const mystring& in)
{
  unsigned length = in.length();
  // The result will never be more than the length of the input
  char out[length+1];
  bool modified = false;
  const char* ptrin = in.c_str();
  char* ptrout = out;
  if(in[0] == QUOTE && in[length-1] == QUOTE) {
    length -= 2;
    ptrin++;
    modified = true;
  }
  for(; length; ++ptrin, ++ptrout, --length) {
    if(isqpair(ptrin)) {
      ++ptrin;
      --length;
      modified = true;
    }
    *ptrout = *ptrin;
  }
  *ptrout = 0;
  if(modified)
    return out;
  else
    return in;
}

RULE(comment)
{
  ENTER("LPAREN *(ctext / quoted-pair / comment) RPAREN");
  SKIPSPACE;
  const char* start = ptr;
  MATCHCHAR(LPAREN);
  for(;;) {
    if(isctext(*ptr))
      ++ptr;
    else if(isqpair(ptr))
      ptr += 2;
    else {
      result r = match_comment(ptr);
      if(r)
	ptr = r.next;
      else
	break;
    }
  }
  MATCHCHAR(RPAREN);
  char tmp[ptr-start+2];
  tmp[0] = ' ';
  memcpy(tmp+1, start, ptr-start);
  tmp[ptr-start+1] = 0;
  RETURN(ptr, tmp, "", "");
}

static const char* skipdelim(const char* ptr, mystring& comment)
{
  for(;;) {
    while(*ptr && isspace(*ptr))
      ++ptr;
    if(*ptr != LPAREN)
      break;
    result r = match_comment(ptr);
    if(r) {
      ptr = r.next;
      comment += r.str;
    }
    else
      break;
  }
  return ptr;
}

RULE(atom)
{
  ENTER("1*<any CHAR except specials, SPACE and CTLs>");
  mystring comment;
  SKIPDELIM(comment);
  const char* start = ptr;
  while(*ptr &&
	!isspace(*ptr) &&
	!isspecial(*ptr) &&
	!isctl(*ptr))
    ++ptr;
  if(start == ptr)
    FAIL("no CHARs matched");
  mystring atom(start, ptr-start);
  RETURN(ptr, atom, comment, atom);
}

RULE(quoted_string)
{
  ENTER("QUOTE *(qtext/quoted-pair) QUOTE");
  mystring comment;
  SKIPDELIM(comment);
  const char* start = ptr;
  MATCHCHAR(QUOTE);
  for(; *ptr; ++ptr) {
    if(isqtext(*ptr))
       continue;
    else if(isqpair(ptr))
      ++ptr;
    else
      break;
  }
  MATCHCHAR(QUOTE);
  mystring text(start, ptr-start);
  mystring addr(unquote(text));
  text = quote(addr);
  RETURN(ptr, text, comment, addr);
}
  
RULE(domain_literal)
{
  ENTER("LSQBRACKET *(dtext/quoted-pair) RSQBRACKET");
  SKIPSPACE;
  MATCHCHAR(LSQBRACKET);
  SKIPSPACE;
  const char* start = ptr;
  mystring str;
  for(; *ptr; ++ptr) {
    if(isdtext(*ptr))
      continue;
    else if(isqpair(ptr))
      ++ptr;
    else
      break;
  }
  SKIPSPACE;
  MATCHCHAR(RSQBRACKET);
  mystring text(start, ptr-start);
  RETURN(ptr, text, "", text);
}

RULE(sub_domain)
{
  // NOTE: domain-ref  =  atom
  ENTER("domain-ref / domain-literal");
  OR_RULE(atom, domain_literal);
}

RULE(domain)
{
  ENTER("sub-domain *(PERIOD sub-domain)");
  result r = match_sub_domain(ptr);
  if(!r) FAIL("did not match sub-domain");
  mystring comment;
  for(;;) {
    ptr = r.next;
    SKIPDELIM(comment);
    r.next = ptr;
    if(*ptr++ != PERIOD)
      break;
    result r1 = match_sub_domain(ptr);
    if(!r1) break;
    r.next = r1.next;
    r.str += ".";
    r.str += r1.str;
    comment += r1.comment;
    r.addr += ".";
    r.addr += r1.addr;
  }
  r.comment += comment;
  RETURNR(r);
}

RULE(route)
{
  ENTER("1#(AT domain) COLON");
  unsigned count=0;
  mystring str;
  mystring comment;
  for(;;) {
    if(*ptr != AT) break;
    ++ptr;
    result r = match_domain(ptr);
    if(!r) FAIL("did not match domain");
    str += "@";
    str += r.str;
    comment += r.comment;
    ++count;
    ptr = r.next;
  }
  if(count == 0)
    FAIL("matched no domains");
  SKIPDELIM(comment);
  MATCHCHAR(COLON);
  RETURN(ptr, str, comment, "");
}

RULE(word)
{
  ENTER("atom / quoted-string");
  OR_RULE(atom, quoted_string);
}

RULE(local_part)
{
  ENTER("word *(PERIOD word)");
  result r = match_word(ptr);
  if(!r) FAIL("did not match word");
  for(;;) {
    ptr = r.next;
    SKIPDELIM(r.comment);
    r.next = ptr;
    if(*ptr++ != PERIOD)
      break;
    result r1 = match_word(ptr);
    if(!r1)
      break;
    r.next = r1.next;
    r.str += ".";
    r.str += r1.str;
    r.comment += r1.comment;
    r.addr += ".";
    r.addr += r1.addr;
  }
  RETURNR(r);
}
  
RULE(addr_spec)
{
  ENTER("local-part *( AT domain )");
  result r = match_local_part(ptr);
  if(!r) FAIL("did not match local-part");
  mystring domain;
  for(;;) {
    ptr = r.next;
    SKIPDELIM(r.comment);
    r.next = ptr;
    if(*ptr++ != AT)
      break;
    result r2 = match_domain(ptr);
    if(!r2) break;
    if(!!domain) {
      r.str += "@";
      r.str += domain;
      r.addr += "@";
      r.addr += domain;
    }
    domain = r2.addr;
    r.comment += r2.comment;
    r.next = r2.next;
  }
  canonicalize(domain);
  RETURN(r.next, r.str + "@" + domain, r.comment,
	 r.addr + "@" + domain + "\n");
}

RULE(route_addr) 
{
  ENTER("LABRACKET [route] addr-spec RABRACKET");
  mystring comment;
  SKIPDELIM(comment);
  MATCHCHAR(LABRACKET);
  result r1 = match_route(ptr);
  if(r1) ptr = r1.next;
  comment += r1.comment;
  result r2 = match_addr_spec(ptr);
  if(!r2) FAIL("did not match addr-spec");
  ptr = r2.next;
  comment += r2.comment;
  SKIPDELIM(comment);
  MATCHCHAR(RABRACKET);
  RETURN(ptr, "<" + r2.str + ">" + comment, "", r2.addr);
}

RULE(phrase)
{
  ENTER("word *word");
  result r1 = match_word(ptr);
  if(!r1) FAIL("did not match word");
  for(;;) {
    result r2 = match_word(r1.next);
    if(!r2)
      break;
    r1.str += " ";
    r1.str += r2.str;
    r1.comment += r2.comment;
    r1.next = r2.next;
  }
  RETURNR(r1);
}

RULE(route_phrase)
{
  ENTER("*(rtext/quoted-pair)");
  SKIPSPACE;
  const char* start = ptr;
  for(;; ++ptr) {
    if(isrtext(*ptr))
      continue;
    else if(isqpair(ptr))
      ++ptr;
    else
      break;
  }
  RETURN(ptr, mystring(start, ptr-start), "", "");
}

RULE(route_spec)
{
  ENTER("route-phrase route-addr");
  result r1 = match_route_phrase(ptr);
  if(!r1) FAIL("did not match route-phrase");
  result r2 = match_route_addr(r1.next);
  if(!r2) FAIL("did not match route-addr");
  r2.str = r1.str + r1.comment + " " + r2.str + r2.comment;
  RETURNR(r2);
}

RULE(mailbox)
{
  ENTER("route-spec / addr-spec");
  OR_RULE(route_spec, addr_spec);
}

RULE(mailboxes)
{
  ENTER("mailbox *(*(COMMA) mailbox)");
  result r1 = match_mailbox(ptr);
  if(!r1) FAIL("did not match mailbox");
  r1.str += r1.comment;
  r1.comment = "";
  for(;;) {
    ptr = r1.next;
    for(;;) {
      SKIPDELIM(r1.str);
      if(*ptr == COMMA) ++ptr;
      else break;
    }
    if(!*ptr)
      break;
    result r2 = match_mailbox(ptr);
    if(!r2) break;
    r1.next = r2.next;
    r1.str = r1.str + ", " + r2.str + r2.comment;
    r1.addr += r2.addr;
  }
  SKIPDELIM(r1.str);
  r1.next = ptr;
  RETURNR(r1);
}

RULE(group)
{
  ENTER("phrase COLON [#mailboxes] SEMICOLON");
  result r1 = match_phrase(ptr);
  if(!r1) FAIL("did not match phrase");
  ptr = r1.next;
  SKIPSPACE;
  MATCHCHAR(COLON);
  result r2 = match_mailboxes(ptr);
  if(r2) ptr = r2.next;
  mystring comment;
  SKIPDELIM(comment);
  MATCHCHAR(SEMICOLON);
  RETURN(ptr, r1.str + ": " + r2.str + r2.comment + comment + ";",
	 "", r2.addr);
}

RULE(address)
{
  ENTER("group / mailbox");
  OR_RULE(group, mailbox);
}

RULE(part)
{
  ENTER("address / comment");
  OR_RULE(address, comment);
}

RULE(addresses)
{
  ENTER("part *(*(COMMA) part) EOF");
  result r1 = match_part(ptr);
  if(!r1) FAIL("did not match part");
  r1.str += r1.comment;
  r1.comment = "";
  for(;;) {
    ptr = r1.next;
    for(;;) {
      SKIPDELIM(r1.str);
      if(*ptr == COMMA) ++ptr;
      else break;
    }
    if(!*ptr)
      break;
    result r2 = match_part(ptr);
    if(!r2) break;
    r1.next = r2.next;
    r1.str = r1.str + ", " + r2.str + r2.comment;
    r1.addr += r2.addr;
  }
  SKIPDELIM(r1.str);
  if(*ptr) FAIL("Rule ended before EOF");
  RETURNR(r1);
}
    
bool parse_addresses(mystring& line, mystring& list)
{
  result r = match_addresses(line.c_str());
  if(r) {
    line = r.str;
    list = r.addr;
    return true;
  }
  else
    return false;
}
