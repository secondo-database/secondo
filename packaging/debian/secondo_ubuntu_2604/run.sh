#!/bin/bash

docker run -t -i -v ${PWD}/volume:/volume ubuntu:26.04 /volume/build.sh
