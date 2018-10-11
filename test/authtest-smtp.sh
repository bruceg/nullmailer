echo 250 ME
read line
echo 250-ME
echo 250-AUTH PLAIN
echo 250 OK
read line
cat $1
rm -f $1
read line
echo 250 OK
read line
echo 250 OK
read line
echo 351 OK
read line
echo 220 OK
