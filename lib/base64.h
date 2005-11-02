#ifndef NULLMAILER_BASE64__H__
#define NULLMAILER_BASE64__H__

#include "mystring/mystring.h"

extern void base64_encode(const mystring& in, mystring& out);
extern void base64_encode_chunk(const unsigned char bin[3], unsigned len,
				char encoded[4]);

#endif // NULLMAILER_BASE64__H__
