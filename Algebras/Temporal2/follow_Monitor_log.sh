#!/bin/bash

secondo_bin_dir=~/secondo/bin

monitor_log_name=${secondo_bin_dir}/nohup.out
touch $monitor_log_name
less +F $monitor_log_name


