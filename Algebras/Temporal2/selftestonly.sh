#!/bin/bash

# run all tests and selftest

# for use with valgrind commandline parameter 1 is passed to TestRunner / Selftest
# provide either --valgrind or --valgrindlc (or nothing) depending on your needs

pushd . \

cd ~/secondo/bin/ &&  Selftest $1 tmp/Temporal2.examples 

popd

