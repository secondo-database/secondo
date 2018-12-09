#!/bin/bash

# call make in current directory to get quick response on problems in last changes
# if successful, build TTY and run all tests

# for use with valgrind commandline parameter 1 is passed to TestRunner / Selftest
# provide either --valgrind or --valgrindlc (or nothing) depending on your needs

pushd . \
&& cd ~/secondo/bin/ \
&& SecondoBDB -test  -i ~/secondo/Algebras/Temporal2/Temporal2_start_stop_secondo_1.test \
&& SecondoBDB -test  -i ~/secondo/Algebras/Temporal2/Temporal2_start_stop_secondo_2.test
popd

