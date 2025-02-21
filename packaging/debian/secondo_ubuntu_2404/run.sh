#!/bin/bash

docker run -t -i -v ${PWD}/volume:/volume ubuntu:24.04 /volume/build.sh
