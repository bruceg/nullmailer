set -e
archive="$1"
shift
tmpdir=".libmerge.$archive.$$.$RANDOM.$USER"
mkdir "$tmpdir"
cd "$tmpdir"
trap 'cd ..; rm -rf "$tmpdir"' EXIT
for input in "$@"; do
	dir="`basename "$input"`"
	mkdir "$dir"
	cd "$dir"
	ar x ../../"$input"
	cd ..
done
ar rc ../"$archive" */*
ranlib ../"$archive"
