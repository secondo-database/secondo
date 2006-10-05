
This directory contains two versions of JPL an interface
between Java and SWI prolog. The first version located in 
the subdirectory 10 runs only with prolog versions less than
5.2.x. The other version runs with prolog versions greater or
equal to this version. 

The default setting is to use the 1.0.x version. 
If a prolog version >= 5.2.x is installed, the environment
variable PL_VERSION is to set, e.g. to 50407 if prolog 5.4.7
is the current prolog version. 

Some prolog installations come with precompiled versions of JPL.
In this case, jpl must not be build in the Secondo system. Instead,
the existing objects can be used. To do this, export the following
environments variables:

JPL_JAR    : pointing to the jar file containing jpl class files
JPL_DLL    : the location of the jpl shared object
PL_DLL     : the shared objects containing the prolog engine
PL_DLL_DIR : directory containing the prolog shared objects

Note that on windows platforms with a prolog version >=5.2.x it is
necessary to set these variables because the jpl code cannot be
compiled on windows platforms using the gcc. 

It's recommended to set these variables in the ~/.secondo.<platform>rc
file.

An example for a linux platform is:

...
#SWI Prolog
export SWI_HOME_DIR="/usr/lib/pl-5.4.7"
export PL_INCLUDE_DIR=$SWI_HOME_DIR/include
export PL_LIB_DIR=$SWI_HOME_DIR/lib/i686-linux

export PL_VERSION=50407
export PL_DLL_DIR=PL_LIB_DIR
export JPL_JAR=$SWI_HOME_DIR/lib/jpl.jar
export JPL_DLL=$PL_DLL_DIR/libjpl.so 

# PL_DLL is not used on linux platforms 

......

An example for a windows platform is:

#SWI Prolog libraries
export PL_INCLUDE_DIR="$SECONDO_SDK/pl/include"
export PL_LIB_DIR="$SECONDO_SDK/pl/lib"

export PL_VERSION=50620
export PL_DLL_DIR="$SECONDO_SDK/pl/bin"
export JPL_DLL=$(PL_DLL_DIR)/jpl.dll
export PL_DLL=$(PL_DLL_DIR)/libpl.dll
export JPL_JAR=$PL_LIB_DIR/jpl.jar

......


On windows platforms the variable PL_DLL has to  be set 
even if the jpl dll is build within the secondo system. 






