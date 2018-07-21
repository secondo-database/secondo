#!/bin/bash

secondo_bin_dir=~/secondo/bin
script_dir=$(pwd)

pushd $secondo_bin_dir

byobu new-session -d -s $USER

echo monitor $(date)
# monitor window
byobu rename-window -t $USER:0 'monitor'
byobu send-keys "StartMonitor" C-m
byobu split-window -v
byobu send-keys "${script_dir}/follow_Monitor_log.sh" C-m
byobu select-pane -U


echo first $(date)
# first console
byobu new-window -t $USER:1 -n 'CS1'
byobu send-keys "SecondoCS -test < ~/secondo/Algebras/Temporal2/Temporal2_multiproccess_main.test" C-m
byobu split-window -v
echo first.1 $(date)
byobu send-keys "${script_dir}/follow_last_CS_log.sh" C-m
byobu select-pane -U

echo second $(date)
# second console
byobu new-window -t $USER:2 -n 'CS2'
byobu send-keys "SecondoCS -test < ~/secondo/Algebras/Temporal2/Temporal2_multiproccess_dep_1.test" C-m
byobu split-window -v
echo second.1 $(date)
byobu send-keys "${script_dir}/follow_last_CS_log.sh" C-m
byobu select-pane -U

echo third $(date)
# third console
byobu new-window -t $USER:3 -n 'CS3'
byobu send-keys "SecondoCS -test < ~/secondo/Algebras/Temporal2/Temporal2_multiproccess_dep_2.test" C-m
byobu split-window -v
echo third.1 $(date)
byobu send-keys "${script_dir}/follow_last_CS_log.sh" C-m
byobu select-pane -U

echo select $(date)
# Set default window as the first console
byobu select-window -t $USER:1

echo attach $(date)
# Attach to the session you just created
# (flip between windows with alt -left and right)
byobu attach-session -t $USER


# now we exited - kill everything:
echo '**** cleaning up: ****'
ps aux | grep Secondo

echo '**** killing SecondoCS: ****'
killall SecondoCS
echo '**** done: killing SecondoCS ****'
ps aux | grep Secondo

echo '**** killing SecondoMonitor: ****'
killall SecondoMonitor
echo '**** done: killing SecondoMonitor ****'
ps aux | grep Secondo

# hopefully everything has been killed... now remove session
byobu kill-session -t $USER

# killall SecondoMonitor
# killall SecondoBDB

rm /dev/shm/*_*

popd
