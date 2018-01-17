echo 250 ME
echo 250-ME
echo 250-AUTH PLAIN
echo 250 OK
cat $1
rm -f $1
echo 250 OK
echo 250 OK
echo 351 OK
echo 220 OK
sleep 1
