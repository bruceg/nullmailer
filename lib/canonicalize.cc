#include "config.h"
#include "mystring.h"
#include "canonicalize.h"

extern mystring defaultdomain;
extern mystring defaulthost;

void canonicalize(mystring& domain)
{
  if(!domain)
    domain = defaulthost;
  if(domain.find('.') < 0) {
    if(!!defaultdomain) {
      domain += ".";
      domain += defaultdomain;
    }
  }
}

