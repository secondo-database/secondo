#!/bin/bash

pushd . 
cd ~/secondo/bin
rm server.msg/*
rm nohup.out

nohup SecondoMonitor -s $* </dev/null &
pid=$!
echo $pid > startmoni.pid
echo "SecondoMonitor started in background with process id = $pid."

gnome-terminal --window-with-profile=bash_close -e SecondoTTYCS

gnome-terminal --window-with-profile=bash_close -e SecondoTTYCS

gnome-terminal --window-with-profile=bash_close -e SecondoTTYCS

read -rsp $'Press escape to continue...\n' -d $'\e'

killall -SIGTERM SecondoCS

while read p; do
	echo $p
	kill -SIGTERM $p
done < startmoni.pid

rm startmoni.pid
popd
exit $?

