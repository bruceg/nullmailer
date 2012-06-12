tmp=$PWD/clitest.tmp
trap 'rm -f ${tmp}*' EXIT

testit() {
    test "x$1" = 'x!' && isfailed='-eq 0' || isfailed='-ne 0'
    "$@" >$tmp.out 2>/dev/null
    if [ $? $isfailed ]
    then
	echo "Test \"$*\" failed!"
	exit 1
    fi
    if [ "x$1" != 'x!' ]
    then
	cat >$tmp.exp
	if ! diff -u $tmp.exp $tmp.out
	then
	    echo "Test \"$*\" failed!"
	    exit 1
	fi
    fi
}

testit ./clitest0 <<EOF
argc=0 a=0 b=0 c=NULL d=NULL
EOF
testit ./clitest1 <<EOF
argc=0 a=0 b=0 c=NULL d=NULL
EOF

testit ! ./clitest0 -abb
testit ! ./clitest1 -abb

testit ! ./clitest0 -a -bb
testit ./clitest1 -a -bb <<EOF
argc=0 a=1 b=1 c=NULL d=NULL
EOF

testit ./clitest0 -a --bb <<EOF
argc=0 a=1 b=1 c=NULL d=NULL
EOF
testit ./clitest1 -a --bb <<EOF
argc=0 a=1 b=1 c=NULL d=NULL
EOF

testit ./clitest0 -ctest <<EOF
argc=0 a=0 b=0 c="test" d=NULL
EOF
testit ./clitest1 -ctest <<EOF
argc=0 a=0 b=0 c="test" d=NULL
EOF

testit ! ./clitest0 -ddtest
testit ! ./clitest1 -ddtest
testit ! ./clitest0 --ddtest
testit ! ./clitest1 --ddtest

testit ! ./clitest0 -dd test
testit ./clitest1 -dd test <<EOF
argc=0 a=0 b=0 c=NULL d="test"
EOF

testit ./clitest0 --dd test <<EOF
argc=0 a=0 b=0 c=NULL d="test"
EOF
testit ./clitest1 --dd test <<EOF
argc=0 a=0 b=0 c=NULL d="test"
EOF

testit ! ./clitest0 -dd=test
testit ./clitest1 -dd=test <<EOF
argc=0 a=0 b=0 c=NULL d="test"
EOF

testit ./clitest0 --dd=test <<EOF
argc=0 a=0 b=0 c=NULL d="test"
EOF
testit ./clitest1 --dd=test <<EOF
argc=0 a=0 b=0 c=NULL d="test"
EOF

echo "All tests passed."
