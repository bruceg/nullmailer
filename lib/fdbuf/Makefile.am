noinst_LIBRARIES = libfdbuf.a
INCLUDES = -I..

if FDBUF_NO_MYSTRING
mystring_sources =
else
mystring_sources = fdibuf_mystring.cc fdibuf_netstring.cc
endif

libfdbuf_a_SOURCES = \
	fdbuf.h \
	fdbuf.cc \
	fdbuf_copy.cc \
	fdibuf.h \
	fdibuf.cc \
	fdobuf.h \
	fdobuf.cc \
	fdobuf_chownmod.cc \
	fdobuf_seek.cc \
	fdobuf_signed.cc \
	fdobuf_unsigned.cc \
	$(mystring_sources)
