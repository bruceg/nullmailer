#ifndef NULLMAILER__CONFIGIO__H__
#define NULLMAILER__CONFIGIO__H__

#include "mystring/mystring.h"
#include "list.h"

bool config_read(const char* filename, mystring& result);
bool config_readlist(const char* filename, list<mystring>& result);
bool config_readint(const char* filename, int& result);

#endif // NULLMAILER__CONFIGIO__H__
