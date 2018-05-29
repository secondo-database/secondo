#!/bin/bash
cwd=$(pwd)
make && \
cd ../.. && \
make TTY && \
cd bin && \
SecondoTTYBDB
cd $cwd

