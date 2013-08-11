#!/bin/bash

# first step: add repositories

VERSION=$(cat /etc/issue | awk '{print $4}')

 add oss if not present
REP=$(zypper lr -d | grep "http://download.opensuse.org/distribution/$VERSION/repo/oss/" | wc -l)

if [ $REP -eq 0 ]; then
        echo "Add repository http://download.opensuse.org/distribution/$VERSION/repo/oss/"
        sudo zypper ar -f http://download.opensuse.org/distribution/$VERSION/repo/oss/ Secondo_OSS
else
        echo "repository http://download.opensuse.org/distribution/$VERSION/repo/oss/ is present"
fi


add non-oss if not present

REP=$(zypper lr -d | grep "http://download.opensuse.org/distribution/$VERSION/repo/non-oss/" | wc -l)

if [ $REP -eq 0 ]; then
        echo "Add repository http://download.opensuse.org/distribution/$VERSION/repo/non-oss/"
        sudo zypper ar -f http://download.opensuse.org/distribution/$VERSION/repo/non-oss/ Secondo_NON_OSS
else
        echo "Repository http://download.opensuse.org/distribution/$VERSION/repo/non-oss/ is present"
fi

 add update if not present


REP=$(zypper lr -d | grep "http://download.opensuse.org/update/$VERSION/" | wc -l)

if [ $REP -eq 0 ]; then
        echo "Add repository http://download.opensuse.org/update/$VERSION/ "
        sudo zypper ar -f http://download.opensuse.org/update/$VERSION/  Secondo_UPDATE
else
        echo "Repository http://download.opensuse.org/update/$VERSION/ is present"
fi



platform=$(uname -i)
# second step: install required packages

PACKAGES="swipl gmp-devel graphviz-devel recode-devel libxml2-devel"

if [ $platform == "x86_64" ]; then
   echo "64 bit platform detected"
   PACKAGES+=" java-1_7_0-openjdk-devel"
else
   PACKAGES+=" java-1_7_0-openjdk-devel"
fi



echo "Install the following packages:"
echo "$PACKAGES"

for p in $PACKAGES; do sudo zypper -q -n  in $p ; done


PLVER=$(swipl --version| awk {'print $3'})
echo "Create file $HOME/.secondorc"
if [ $platform == "x86_64" ]; then
  echo '   if [ -n "$1" ]; then' > $HOME/.secondorc
  echo '      export SECONDO_BUILD_DIR=$1' >> $HOME/.secondorc
  echo '   else' >> $HOME/.secondorc
  echo '     export SECONDO_BUILD_DIR=$PWD' >> $HOME/.secondorc
  echo '    fi' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    export SECONDO_PLATFORM=android' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    # berkeley db' >> $HOME/.secondorc
  echo '    export BDB_DIR=$HOME/BDB' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    export BERKELEY_DB_DIR=$BDB_DIR' >> $HOME/.secondorc
  echo '    export BERKELEY_DB_LIB=db_cxx' >> $HOME/.secondorc
  echo '    export BDB_INCLUDE_DIR=$BDB_DIR/include' >> $HOME/.secondorc
  echo '    export BDB_LIB_DIR=$BDB_DIR/lib' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    # java' >> $HOME/.secondorc
  echo '    export J2SDK_ROOT=/usr/lib64/jvm/java-1.7.0-openjdk-1.7.0/' >> $HOME/.secondorc
  echo '    export JAVAVER="1.6"' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    # prolog related' >> $HOME/.secondorc
  echo "    export SWI_HOME_DIR=/usr/lib64/swipl-$PLVER" >> $HOME/.secondorc
  echo '    export PL_LIB_DIR=$SWI_HOME_DIR/lib/x86_64-linux/' >> $HOME/.secondorc
  echo '    export PL_DLL_DIR=$PL_LIB_DIR' >> $HOME/.secondorc
  echo '    export PL_LIB=swipl' >> $HOME/.secondorc
  echo '    export PL_INCLUDE_DIR=$SWI_HOME_DIR/include' >> $HOME/.secondorc
  echo '    export PL_VERSION=51000' >> $HOME/.secondorc
  echo '    export JPL_DLL=$PL_LIB_DIR/libjpl.so' >> $HOME/.secondorc
  echo '    export JPL_JAR=$SWI_HOME_DIR/lib/jpl.jar' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    # other' >> $HOME/.secondorc
  echo '    export readline=true' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    export PATH=$PATH:.' >> $HOME/.secondorc
  echo '    export SECONDO_CONFIG=$SECONDO_BUILD_DIR/bin/SecondoConfig.ini' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    export PATH=.:$BDB_DIR/bin:$PATH' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    export LD_LIBRARY_PATH=$BDB_LIB_DIR:$PL_LIB_DIR' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '' >> $HOME/.secondorc
  echo '    # enable the pd system' >> $HOME/.secondorc
  echo '    export PATH=$PATH:$SECONDO_BUILD_DIR/Tools/pd' >> $HOME/.secondorc
  echo '    export PD_HEADER=$SECONDO_BUILD_DIR/Tools/pd/pd_header_listing' >> $HOME/.secondorc
else
  echo 'if [ -n "$1" ]; then' >$HOME/.secondorc
  echo '  export SECONDO_BUILD_DIR=$1' >>$HOME/.secondorc
  echo 'else' >>$HOME/.secondorc
  echo '    export SECONDO_BUILD_DIR=$PWD' >>$HOME/.secondorc
  echo 'fi ' >>$HOME/.secondorc
  echo '' >>$HOME/.secondorc
  echo 'export SECONDO_PLATFORM=linux' >>$HOME/.secondorc
  echo 'export BDB_DIR=$HOME/BDB' >>$HOME/.secondorc
  echo 'export BDB_LIB_DIR=$BDB_DIR/lib' >>$HOME/.secondorc
  echo '' >>$HOME/.secondorc
  echo 'export BERKELEY_DB_DIR=$BDB_DIR' >>$HOME/.secondorc
  echo 'export BERKELEY_DB_LIB=db_cxx' >>$HOME/.secondorc
  echo '' >>$HOME/.secondorc
  echo 'export J2SDK_ROOT=/usr/lib/jvm/java-1.7.0-openjdk/' >>$HOME/.secondorc
  echo 'export JAVA_HOME=$J2SDK_ROOT' >>$HOME/.secondorc
  echo "export SWI_HOME_DIR=/usr/lib/swipl-$PLVER/" >>$HOME/.secondorc
  echo 'export PL_LIB_DIR=$SWI_HOME_DIR/lib/i686-linux' >>$HOME/.secondorc
  echo 'export PL_LIB=swipl' >>$HOME/.secondorc
  echo 'export PL_INCLUDE_DIR=$SWI_HOME_DIR/include' >>$HOME/.secondorc
  echo 'export PL_VERSION=50647' >>$HOME/.secondorc
  echo 'export readline=true' >>$HOME/.secondorc
  echo 'export PATH=$PATH:.' >>$HOME/.secondorc
  echo 'export SECONDO_CONFIG=$SECONDO_BUILD_DIR/bin/SecondoConfig.ini' >>$HOME/.secondorc
  echo 'export JAVAVER="1.6"' >>$HOME/.secondorc
  echo '' >>$HOME/.secondorc
echo 'export NDK_MODULE_PATH=$SECONDO_BUILD_DIR/api_cpp/android/lib:' >>$HOME/.secondorc
  echo 'export ANDROID_NDK=$HOME/android-ndk' >>$HOME/.secondorc
fi

echo "Secondo SDK almost installed." 

echo "Please insert the following line into $HOME/.bashrc"
echo "source $HOME/.secondorc $HOME/secondo"
echo 
#echo "Please download the latest berkeley db version without encryption (e.g. db-5.3.21.NC.tar.gz) "
#echo "e.g. from http://www.oracle.com/technetwork/database/berkeleydb/downloads/index.html"
#echo 
#echo "After that, install the berkeley db into $HOME/BDB by"
#echo "Open a terminal and switch to the directory containing the file db-5.3.21.NC.tar.gz"
#echo "Unpack the file by saying: tar -xzf db-5.3.21.NC.tar.gz"
#echo "Go into the build directory: cd db-5.3.21.NC/build_unix"
#echo 'Configure the berkeley db by typing:  ../dist/configure --prefix=$HOME/BDB --enable-cxx'
#echo "Compile the berkeley db: make "
#echo 'Create the target directory:  mkdir $HOME/BDB'
#echo "Install berkeley db: make install"
echo
echo "Now, you can download the latest secondo version from"
echo "http://dna.fernuni-hagen.de/secondo"
echo "Unpack and compile secondo accoring to its user manual"
echo "Have fun"

