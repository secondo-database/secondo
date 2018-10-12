#!/bin/bash

# call make in current directory to get quick response on problems in last changes
# if successful, build TTY and run all tests

# for use with valgrind commandline parameter 1 is passed to TestRunner / Selftest
# provide either --valgrind or --valgrindlc (or nothing) depending on your needs

pushd . \
&& checkpd --verystrong *.{cpp,h} \
&& make \
&& cd ~/secondo/ && make TTY \
&& cd ~/secondo/bin/ \
&& SecondoBDB -test -i ~/secondo/Algebras/Temporal2/Temporal2.test \
&& SecondoBDB -test -i ~/secondo/Algebras/Temporal2/Temporal2_open_close.test \
&& Selftest $1 tmp/Temporal2.examples \
&& SecondoTTYBDB
popd

# skip these for now - functionality not yet there
#&& TestRunnerSj $1 --no-tmp -i ~/secondo/Algebras/Temporal2/Temporal2_start_stop_secondo_1.test \
#&& TestRunnerSj $1 --no-tmp -i ~/secondo/Algebras/Temporal2/Temporal2_start_stop_secondo_2.test
