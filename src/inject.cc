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
#include "defines.h"
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "mystring/mystring.h"
#include "list.h"
#include "hostname.h"
#include "fdbuf/fdbuf.h"
#include "itoa.h"
#include "address.h"
#include "canonicalize.h"
#include "configio.h"
#include "cli++/cli++.h"
#include "makefield.h"
#include "forkexec.h"

enum {
  use_args, use_both, use_either, use_header
};
static int use_recips = use_either;
static int show_message = false;
static int show_envelope = false;
static const char* o_from = 0;

const char* cli_program = "nullmailer-inject";
const char* cli_help_prefix = "Reformat and inject a message into the nullmailer queue\n";
const char* cli_help_suffix = "";
const char* cli_args_usage = "[recipients] <message";
const int cli_args_min = 0;
const int cli_args_max = -1;
cli_option cli_options[] = {
  { 'a', "use-args", cli_option::flag, use_args, &use_recips,
    "Use only command-line arguments for recipients", 0 },
  { 'b', "use-both", cli_option::flag, use_both, &use_recips,
    "Use both command-line and message header for recipients", 0 },
  { 'e', "use-either", cli_option::flag, use_either, &use_recips,
    "Use either command-line and message header for recipients", 0 },
  { 'h', "use-header", cli_option::flag, use_header, &use_recips,
    "Use only message header for recipients", 0 },
  { 'f', "from", cli_option::string, 0, &o_from,
    "Set the sender address", 0 },
  { 'n', "no-queue", cli_option::flag, 1, &show_message,
    "Send the formatted message to standard output", 0 },
  { 'v', "show-envelope", cli_option::flag, 1, &show_envelope,
    "Show the envelope with the message", 0 },
  {0, 0, cli_option::flag, 0, 0, 0, 0}
};

#define fail(MSG) do{ ferr << "nullmailer-inject: " << MSG << endl; return false; }while(0)
#define fail_sys(MSG) do{ ferr << "nullmailer-inject: " << MSG << ": " << strerror(errno) << endl; return false; }while(0)
#define bad_hdr(LINE,MSG) do{ header_has_errors = true; ferr << "nullmailer-inject: Invalid header line:\n  " << LINE << "\n  " MSG << endl; }while(0)

typedef list<mystring> slist;
// static bool do_debug = false;

static mystring cur_line;

///////////////////////////////////////////////////////////////////////////////
// Configuration
///////////////////////////////////////////////////////////////////////////////
static mystring idhost;

extern void canonicalize(mystring& domain);

void read_config()
{
  mystring tmp;
  read_hostnames();
  if(!config_read("idhost", idhost))
    idhost = me;
  else
    canonicalize(idhost);
}

///////////////////////////////////////////////////////////////////////////////
// Envelope processing
///////////////////////////////////////////////////////////////////////////////
static slist recipients;
static mystring sender;
static bool use_header_recips = true;
static bool use_header_sender = true;

void parse_recips(const mystring& list)
{
  if(!!list) {
    int start = 0;
    int end;
    while((end = list.find_first('\n', start)) >= 0) {
      recipients.append(list.sub(start, end-start));
      start = end+1;
    }
  }
}

bool parse_recip_arg(mystring str)
{
  mystring list;
  if(!parse_addresses(str, list))
    return false;
  parse_recips(list);
  return true;
}

bool parse_sender(const mystring& list)
{
  int end = list.find_first('\n');
  if(end > 0 && list.find_first('\n', end+1) < 0) {
    sender = list.sub(0, end);
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Header processing
///////////////////////////////////////////////////////////////////////////////
static slist headers;

static bool header_is_resent = false;
static bool header_has_errors = false;
static bool header_add_to = false;

struct header_field
{
  // member information
  const char* name;
  unsigned length;
  bool is_address;
  bool is_recipient;
  bool is_sender;
  bool is_resent;
  bool remove;			// remove means strip after parsing
  bool ignore;			// ignore means do not parse

  bool present;

  bool parse(mystring& line, bool& rm) 
    {
      if(strncasecmp(line.c_str(), name, length))
	return false;
      rm = remove;
      if(ignore)
	return true;
      if(is_resent) {
	if(!header_is_resent) {
	  if(use_header_sender)
            sender = "";
	  if(use_header_recips)
	    recipients.empty();
	}
	header_is_resent = true;
      }
      if(is_address) {
	mystring tmp = line.right(length);
	mystring list;
	if(!parse_addresses(tmp, list))
	  bad_hdr(line, "Unable to parse the addresses.");
	else if(!tmp) {
	  rm = true;
	  return true;
	}
	else {
	  line = mystringjoin(name) + " " + tmp;
	  if(is_recipient) {
	    if(is_resent == header_is_resent && use_header_recips)
	      parse_recips(list);
	  }
	  else if(is_sender) {
	    if(is_resent == header_is_resent && use_header_sender)
	      parse_sender(list);
	  }
	}
      }
      present = true;
      return true;
    }
};

#define F false
#define T true
#define X(N,IA,IR,IS,IRS,R) { #N ":",strlen(#N ":"),\
  IA,IR,IS,IRS,R,false, false }
static header_field header_fields[] = {
  // Sender address fields, in order of priority
  X(Sender,            T,F,F,F,F), // 0
  X(From,              T,F,F,F,F), // 1
  X(Reply-To,          T,F,F,F,F), // 2
  X(Return-Path,       T,F,T,F,T), // 3
  X(Return-Receipt-To, T,F,F,F,F), // 4
  X(Errors-To,         T,F,F,F,F), // 5
  X(Resent-Sender,     T,F,F,T,F), // 6
  X(Resent-From,       T,F,F,T,F), // 7
  X(Resent-Reply-To,   T,F,F,T,F), // 8
  // Destination address fields
  X(To,                T,T,F,F,F), // 9
  X(Cc,                T,T,F,F,F), // 10
  X(Bcc,               T,T,F,F,T), // 11
  X(Apparently-To,     T,T,F,F,F), // 12
  X(Resent-To,         T,T,F,T,F), // 13
  X(Resent-Cc,         T,T,F,T,F), // 14
  X(Resent-Bcc,        T,T,F,T,T), // 15
  // Other fields of interest
  X(Date,              F,F,F,F,F), // 16
  X(Message-Id,        F,F,F,F,F), // 17
  X(Resent-Date,       F,F,F,T,F), // 18
  X(Resent-Message-Id, F,F,F,T,F), // 19
  X(Content-Length,    F,F,F,F,T), // 20
};
#undef X
#undef F
#undef T
#define header_field_count (sizeof header_fields/sizeof(header_field))
static bool& header_has_from = header_fields[1].present;
static bool& header_has_rfrom = header_fields[7].present;
static bool& header_has_to = header_fields[9].present;
static bool& header_has_cc = header_fields[10].present;
static bool& header_has_rto = header_fields[13].present;
static bool& header_has_rcc = header_fields[14].present;
static bool& header_has_date = header_fields[16].present;
static bool& header_has_mid = header_fields[17].present;
static bool& header_has_rdate = header_fields[18].present;
static bool& header_has_rmid = header_fields[19].present;
static header_field& header_field_from = header_fields[1];
static header_field& header_field_mid = header_fields[17];
static header_field& header_field_rpath = header_fields[3];

static bool use_name_address_style = true;
static mystring from;

bool allmailfrom=false;
mystring allmailfrom_user;
mystring allmailfrom_hostname;

bool
config_allmailfrom(){
	char* str = NULL;
	char* char_pos = NULL;
	mystring amf_config;
	if(config_read("allmailfrom", amf_config)){
		str=(char*)amf_config.c_str();
		//remove newline character
		char_pos=strchr(str, '\n');
		if(char_pos)
			*char_pos = '\0';
		char_pos=strchr(str, '@');
		if(char_pos){
			*char_pos='\0';
			allmailfrom_user = mystring( str );
			allmailfrom_hostname = mystring( char_pos+1 );
		}else{
			// address doesn't contain @, probablty malformed
			return false;
		}
		// success, this flag overwrites 'From: ' field
		// which may already be in the mail header.
		allmailfrom=true;
		return true;
	}
	return false;
}


void setup_from()
{
  mystring user = getenv("NULLMAILER_USER");
  if(!user) user = getenv("MAILUSER");
  if(!user) user = getenv("USER");
  if(!user) user = getenv("LOGNAME");
  if(!user) {
      struct passwd *pw = getpwuid(getuid());
      if (pw) user = pw->pw_name;
  }
  if(!user) user = "unknown";

  mystring host = getenv("NULLMAILER_HOST");
  if(!host) host = getenv("MAILHOST");
  if(!host) host = getenv("HOSTNAME");
  if(!host) host = defaulthost;
  canonicalize(host);

  mystring name = getenv("NULLMAILER_NAME");
  if(!name) name = getenv("MAILNAME");
  if(!name) name = getenv("NAME");
  if(!name) name = user;

  if(allmailfrom){
    user = allmailfrom_user;
    host = allmailfrom_hostname;}

  if(use_name_address_style) {
    if(!name) from = "<" + user + "@" + host + ">";
    else      from = name + " <" + user + "@" + host + ">";
  }
  else {
    if(!name) from = user + "@" + host;
    else      from = user + "@" + host + " (" + name + ")";
  }
  
  mystring suser = getenv("NULLMAILER_SUSER");
  if(!suser) suser = user;

  mystring shost = getenv("NULLMAILER_SHOST");
  if(!shost) shost = host;
  canonicalize(shost);
  
  if(use_header_sender && !sender)
    sender = suser + "@" + shost;
}

bool parse_line(mystring& line)
{
  bool valid = false;
  for(unsigned i = 0; i < line.length(); i++) {
    char ch = line[i];
    if(isspace(ch))
      break;
    if(ch == ':') {
      valid = (i > 0);
      break;
    }
  }
  if(!valid)
    //bad_hdr(line, "Missing field name.");
    return false;
  bool remove = false;
  for(unsigned i = 0; i < header_field_count; i++) {
    header_field& h = header_fields[i];
    if(h.parse(line, remove))
      break;
  }
  if(!remove)
    headers.append(line);
  return true;
}

bool is_header(const mystring& line)
{
  for(unsigned i = 0; i < line.length(); i++) {
    char ch = line[i];
    if(isspace(ch))
      return false;
    if(ch == ':')
      return true;
  }
  return false;
}

bool is_continuation(const mystring& line)
{
  return isspace(line[0]);
}

bool read_header()
{
  mystring cur_line;
  mystring whole;
  bool first = true;
  for (;;) {
    if (!fin.getline(cur_line))
      cur_line = "";
    if(!cur_line || cur_line == "\r")
      break;
    // Ignore a leading RFC 976 "From " line mistakenly inserted by some programs.
    if(first && (cur_line.starts_with("From ") || cur_line.starts_with(">From ")))
      continue;
    first = false;
    if(!!whole && is_continuation(cur_line)) {
      //if(!whole)
      //bad_hdr(cur_line, "First line cannot be a continuation line.");
      //else
      whole += "\n" + cur_line;
    }
    else if(!is_header(cur_line)) {
      cur_line += '\n';
      break;
    }
    else {
      if(!!whole)
	parse_line(whole);
      whole = cur_line;
      cur_line = "";
    }
  }
  if(!!whole)
    parse_line(whole);
  return !header_has_errors;
}

mystring make_recipient_list()
{
  mystring result;
  bool first = true;
  for(slist::iter iter(recipients); iter; iter++) {
    if(!first)
      result = result + ", " + *iter;
    else
      result = *iter;
    first = false;
  }
  return result;
}

bool fix_header()
{
  setup_from();
  if(!header_is_resent) {
    if(!header_has_date)
      headers.append("Date: " + make_date());
    if(!header_has_mid)
      headers.append("Message-Id: " + make_messageid(idhost));
    if(!header_has_from)
      headers.append("From: " + from);
    if(header_has_from && allmailfrom)
      headers.append("From: " + from);
    if(!header_has_to && !header_has_cc && header_add_to &&
       recipients.count() > 0) {
      header_has_to = true;
      headers.append("To: " + make_recipient_list());
    }
  }
  else {
    if(!header_has_rdate)
      headers.append("Resent-Date: " + make_date());
    if(!header_has_rmid)
      headers.append("Resent-Message-Id: " + make_messageid(idhost));
    if(!header_has_rfrom)
      headers.append("Resent-From: " + from);
    if(!header_has_rto && !header_has_rcc && header_add_to &&
       recipients.count() > 0) {
      header_has_rto = true;
      headers.append("Resent-To: " + make_recipient_list());
    }
  }
  if(!header_has_to && !header_has_cc)
    headers.append("Cc: recipient list not shown: ;");
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Message sending
///////////////////////////////////////////////////////////////////////////////
bool send_env(fdobuf& out)
{
  if(!(out << sender << "\n"))
    fail("Error sending sender to nullmailer-queue.");
  for(slist::iter iter(recipients); iter; iter++)
    if(!(out << *iter << "\n"))
      fail("Error sending recipients to nullmailer-queue.");
  if(!(out << endl))
    fail("Error sending recipients to nullmailer-queue.");
  return true;
}

bool send_header(fdobuf& out)
{
  for(slist::iter iter(headers); iter; iter++)
    if(!(out << *iter << "\n"))
      fail("Error sending header to nullmailer-queue.");
  if(!(out << endl))
    fail("Error sending header to nullmailer-queue.");
  return true;
}

bool send_body(fdobuf& out)
{
  if(!(out << cur_line) ||
     !fdbuf_copy(fin, out))
    fail("Error sending message body to nullmailer-queue.");
  return true;
}

bool send_message_stdout()
{
  if(show_envelope)
    send_env(fout);
  send_header(fout);
  send_body(fout);
  return true;
}

bool send_message_nqueue()
{
  queue_pipe nq;
  autoclose wfd = nq.start();
  if (wfd < 0)
    return false;
  fdobuf nqout(wfd);
  if (!send_env(nqout) || !send_header(nqout) || !send_body(nqout))
    return false;
  nqout.flush();
  wfd.close();
  return nq.wait();
}

bool send_message()
{
  return show_message ? send_message_stdout() : send_message_nqueue();
}

///////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////
bool parse_flags()
{
  for(const char* flagstr = getenv("NULLMAILER_FLAGS");
      flagstr && *flagstr; flagstr++) {
    switch(*flagstr) {
    case 'c': use_name_address_style=false; break;
    case 'f': header_field_from.ignore=header_field_from.remove=true; break;
    case 'i': header_field_mid.ignore=header_field_mid.remove=true; break;
    case 's': header_field_rpath.ignore=header_field_rpath.remove=true; break;
    case 't': header_add_to = true; break;
    default:
      // Just ignore any flags we can't handle
      break;
    }
  }
  return true;
}

bool parse_args(int argc, char* argv[])
{
  if(!parse_flags())
    return false;
  config_allmailfrom();
  if(allmailfrom){
    header_field_from.ignore=header_field_from.remove=true;}
  if(o_from) {
    mystring list;
    mystring tmp(o_from);
    if(tmp == "" || tmp == "<>")
      sender = "";
    else if(!parse_addresses(tmp, list) ||
            !parse_sender(list)) {
      ferr << "nullmailer-inject: Invalid sender address: " << o_from << endl;
      return false;
    }
    use_header_sender = false;
  }
  use_header_recips = (use_recips != use_args);
  if(use_recips == use_header)
    return true;
  if(use_recips == use_either && argc > 0)
    use_header_recips = false;
  bool result = true;
  for(int i = 0; i < argc; i++) {
    if(!parse_recip_arg(argv[i])) {
      ferr << "Invalid recipient: " << argv[i] << endl;
      result = false;
    }
  }
  return result;
}

int cli_main(int argc, char* argv[])
{
  read_config();
  if(!parse_args(argc, argv) ||
     !read_header() ||
     !fix_header())
    return 1;
  if(recipients.count() == 0) {
    ferr << "No recipients were listed." << endl;
    return 1;
  }
  if(!send_message())
    return 1;
  return 0;
}
