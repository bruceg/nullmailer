dnl TRY_CXX_FLAG(FLAG,[ACTION-IF-FOUND[,ACTION-IF-NOT-FOUND]])
AC_DEFUN([TRY_CXX_FLAG],
[echo >conftest.cc
if ${CXX-g++} ${CXXFLAGS} -c [$1] conftest.cc >/dev/null 2>&1; then
  ifelse([$2], , :, [rm -f conftest*
  $2])
else
  ifelse([$3], , :, [rm -f conftest*
  $3])
fi
rm -f conftest*])

AC_DEFUN([CXX_NO_RTTI],
[AC_CACHE_CHECK(whether ${CXX-g++} accepts -fno-rtti,
	local_cv_flag_NO_RTTI,
	TRY_CXX_FLAG(-fno-rtti,
		local_cv_flag_NO_RTTI=yes,
		local_cv_flag_NO_RTTI=no))
test "$local_cv_flag_NO_RTTI" = yes && CXXFLAGS="$CXXFLAGS -fno-rtti"
])

AC_DEFUN([CXX_NO_EXCEPTIONS],
[AC_CACHE_CHECK(whether ${CXX-g++} accepts -fno-exceptions,
	local_cv_flag_NO_EXCEPTIONS,
	TRY_CXX_FLAG(-fno-exceptions,
		local_cv_flag_NO_EXCEPTIONS=yes,
		local_cv_flag_NO_EXCEPTIONS=no))
test "$local_cv_flag_NO_EXCEPTIONS" = yes && CXXFLAGS="$CXXFLAGS -fno-exceptions"
])

dnl TRY_STRUCT_TM_MEMBER(MEMBER,FLAGNAME)
AC_DEFUN([TRY_STRUCT_TM_MEMBER],
[ AC_CACHE_CHECK(whether struct tm contains [$1],
	[$2],
	AC_COMPILE_IFELSE([
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
int main() { struct tm* foo; foo->[$1]; }
],
	[$2]=yes,
	[$2]=no))
])

AC_DEFUN([TEST_STRUCT_TM],[
	TRY_STRUCT_TM_MEMBER(tm_isdst, local_cv_flag_TM_HAS_ISDST)
	TRY_STRUCT_TM_MEMBER(__tm_isdst, local_cv_flag_TM_HAS___ISDST)
	if test "$local_cv_flag_TM_HAS_ISDST" = yes
	then AC_DEFINE(TM_HAS_ISDST,tm_isdst,[struct tm has tm_isdst member])
	elif test "$local_cv_flag_TM_HAS___ISDST" = yes
	then AC_DEFINE(TM_HAS_ISDST,__tm_isdst,[struct tm has tm_isdst member])
	fi
	TRY_STRUCT_TM_MEMBER(tm_gmtoff, local_cv_flag_TM_HAS_GMTOFF)
	TRY_STRUCT_TM_MEMBER(__tm_gmtoff, local_cv_flag_TM_HAS___GMTOFF)
	if test "$local_cv_flag_TM_HAS_GMTOFF" = yes
	then AC_DEFINE(TM_HAS_GMTOFF,tm_gmtoff,[struct tm has tm_gmtoff member])
	elif test "$local_cv_flag_TM_HAS___GMTOFF" = yes
	then AC_DEFINE(TM_HAS_GMTOFF,__tm_gmtoff,[struct tm has tm_gmtoff member])
	fi
])

dnl TRY_STRUCT_UTSNAME_MEMBER(MEMBER,FLAGNAME)
AC_DEFUN([TRY_STRUCT_UTSNAME_MEMBER],
[ AC_CACHE_CHECK(whether struct utsname contains [$1],
	[$2],
	AC_COMPILE_IFELSE([
#include <sys/utsname.h>
int main() { struct utsname* foo; foo->[$1]; }
],
	[$2]=yes,
	[$2]=no))
])

AC_DEFUN([TEST_STRUCT_UTSNAME],[
  TRY_STRUCT_UTSNAME_MEMBER(domainname, local_cv_flag_UTSNAME_HAS_DOMAINNAME)
  TRY_STRUCT_UTSNAME_MEMBER(__domainname,
                            local_cv_flag_UTSNAME_HAS___DOMAINNAME)
  if test "$local_cv_flag_UTSNAME_HAS_DOMAINNAME" = yes
  then AC_DEFINE(UTSNAME_HAS_DOMAINNAME,domainname,[struct utsname has domainname member])
  elif test "$local_cv_flag_UTSNAME_HAS___DOMAINNAME" = yes
  then AC_DEFINE(UTSNAME_HAS_DOMAINNAME,__domainname,[struct utsname has domainname member])
  fi
])

AC_DEFUN([CHECK_NAMED_PIPE_BUG],
[ AC_CACHE_CHECK(whether named pipes are buggy,
	  local_cv_flag_NAMEDPIPEBUG,
	  cat >conftest.c <<EOF
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char** argv)
{
  struct timeval tv;
  fd_set rfds;
  int fd = open(*(argv+1), O_RDONLY | O_NONBLOCK);
  FD_ZERO(&rfds);
  FD_SET(fd,&rfds);
  tv.tv_sec = tv.tv_usec = 0;
  return (select(fd+1, &rfds, 0, 0,&tv) > 0) ? 0 : 1;
}
EOF
	if ! ${CC} ${CFLAGS} conftest.c -o conftest 2>/dev/null
	then
		echo Compile failed
		exit 1
	fi
	mkfifo conftest.pipe
	if ./conftest conftest.pipe
	then
		AC_DEFINE(NAMEDPIPEBUG, 1, [Named pipes have read/write bug])
		local_cv_flag_NAMEDPIPEBUG=yes
	else
		local_cv_flag_NAMEDPIPEBUG=no
	fi
	rm -f conftest*)
])
