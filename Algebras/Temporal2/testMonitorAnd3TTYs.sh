#!/bin/bash

pushd . 
cd ~/secondo/bin
rm server.msg/*
rm nohup.out

nohup SecondoMonitor -s $* </dev/null &
pid=$!
echo $pid > startmoni.pid
echo "SecondoMonitor started in background with process id = $pid."

gnome-terminal --window-with-profile=bash -e 'SecondoTTYCS -test  -i /home/simon/secondo/Algebras/Temporal2/Temporal2.test'

gnome-terminal --window-with-profile=bash -e 'SecondoTTYCS -test  -i /home/simon/secondo/Algebras/Temporal2/Temporal2.test '
 
gnome-terminal --window-with-profile=bash -e 'SecondoTTYCS -test  -i /home/simon/secondo/Algebras/Temporal2/Temporal2.test'

read -rsp $'Press escape to continue...\n' -d $'\e'

killall -SIGTERM SecondoCS

while read p; do
	echo $p
	kill -SIGTERM $p
done < startmoni.pid

rm startmoni.pid
popd
exit $?

