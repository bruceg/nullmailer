#include "errcodes.h"

const char* const errorstr[] = {
  "No error",
  "Unspecified error",
  "Host not found",
  "Host has no address",
  "Fatal error in gethostbyname",
  "Temporary error in gethostbyname",
  "Socket failed",
  "Connection refused",
  "Connection timed out",
  "Host or network unreachable",
  "Connection failed",
  "Protocol error",
  "Could not open message",
  "Could not read message",
  "Could not write message",
  "Could not exec program",
  "Server refused the message",
  "Temporary error in sending the message",
  "Permanent error in sending the message",
  "Command-line usage error",
};
