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
	"${AR:-ar}" x ../../"$input"
	cd ..
done
"${AR:-ar}" rc ../"$archive" */*
"${RANLIB:-ranlib}" ../"$archive"
