#ifndef NULLMAILER__CONFIGIO__H__
#define NULLMAILER__CONFIGIO__H__

#include "mystring/mystring.h"
#include "list.h"

mystring config_path(const char* dflt, const char* testdir, const char* subdir, const char* filename);
#define CONFIG_PATH(NAME, SUBDIR, FILENAME) config_path(NAME##_DIR, NAME##_TEST_DIR, SUBDIR, FILENAME)

bool config_read(const char* filename, mystring& result);
bool config_readlist(const char* filename, list<mystring>& result);
bool config_readint(const char* filename, int& result);
bool config_syserr(const char* filename);

#endif // NULLMAILER__CONFIGIO__H__
