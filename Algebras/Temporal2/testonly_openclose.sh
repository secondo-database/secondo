#!/bin/bash

# run all tests and selftest

# for use with valgrind commandline parameter 1 is passed to TestRunner / Selftest
# provide either --valgrind or --valgrindlc (or nothing) depending on your needs

pushd . \

cd ~/secondo/bin/ && TestRunnerSj $1 -i ~/secondo/Algebras/Temporal2/Temporal2_open_close.test \

popd

