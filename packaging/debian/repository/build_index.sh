#!/bin/bash

apt-ftparchive packages . > Packages
gzip -c Packages > Packages.gz

apt-ftparchive release . > Release
