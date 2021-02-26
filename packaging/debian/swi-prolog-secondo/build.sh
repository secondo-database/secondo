#/bin/bash
#
# This script creates a debian package of the swi-prolog package
# that can be used with secondo
#####################################################################

# Install dependencies
apt-get install fakeroot debhelper libgmp-dev libjpeg8-dev libreadline-dev autotools-dev default-jdk junit 

PLVER=6.6.5

if [ ! -f pl-$PLVER.tar.gz ]; then
    wget http://www.swi-prolog.org/download/stable/src/pl-$PLVER.tar.gz
    tar -xzf pl-$PLVER.tar.gz
fi

cd pl-$PLVER

# Link our debian files to the package
if [ ! -h debian ]; then
   rmdir debian
   ln -s ../debian debian
fi 

dpkg-buildpackage -rfakeroot
