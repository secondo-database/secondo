#/bin/bash
#
# This script creates a debian package of the swi-prolog package
# that can be used with secondo
#####################################################################

# Install dependencies
apt-get install flex bison gcc g++ libdb5.1 libdb5.1-dev libdb5.1++ libdb5.1++-dev db5.1-util libjpeg62 libjpeg62-dev libgsl0-dev libreadline-dev librecode-dev libgmp-dev libncurses-dev libxml2-dev libboost-all-dev build-essential debhelper fakeroot openjdk-7-jdk libxml2 libxml2-dev wget swi-prolog-nox swi-prolog-java swi-prolog libquadmath0 libgmp-dev libgmp10 libboost-thread-dev libboost-thread1.54.0 libboost-system-dev libboost-system1.54.0

VER=v410

if [ ! -f secondo-$VER-LAT1.tgz ]; then
    wget http://dna.fernuni-hagen.de/Secondo.html/files/Sources/secondo-$VER-LAT1.tgz 
    tar -xzf secondo-$VER-LAT1.tgz 
fi

# Activate additional algebras
sed -i 's|#ALGEBRA_DIRS += MapMatching|ALGEBRA_DIRS += MapMatching|g' secondo/makefile.algebras
sed -i 's|#ALGEBRAS     += MapMatchingAlgebra|ALGEBRAS     += MapMatchingAlgebra|g' secondo/makefile.algebras
sed -i 's|#ALGEBRA_DEPS += xml2|ALGEBRA_DEPS += xml2|g' secondo/makefile.algebras

sed -i 's|#ALGEBRA_DIRS += OSM|ALGEBRA_DIRS += OSM|g' secondo/makefile.algebras
sed -i 's|#ALGEBRAS     += OsmAlgebra|ALGEBRAS     += OsmAlgebra|g' secondo/makefile.algebras
sed -i 's|#ALGEBRA_DEPS += xml2|ALGEBRA_DEPS += xml2|g' secondo/makefile.algebras

sed -i 's|#ALGEBRA_DEP_DIRS += /usr/lib/libxml2|ALGEBRA_DEP_DIRS += /usr/lib/libxml2|g' secondo/makefile.algebras
sed -i 's|#ALGEBRA_INCLUDE_DIRS += /usr/include/libxml2|ALGEBRA_INCLUDE_DIRS += /usr/include/libxml2|g' secondo/makefile.algebras

#sed -i 's|#ALGEBRA_DIRS += MP3|ALGEBRA_DIRS += MP3|g' secondo/makefile.algebras
#sed -i 's|#ALGEBRAS     += MP3Algebra|ALGEBRAS     += MP3Algebra|g' secondo/makefile.algebras

sed -i 's|#ALGEBRA_DIRS += OptAux|ALGEBRA_DIRS += OptAux|g' secondo/makefile.algebras
sed -i 's|#ALGEBRAS     += OptAuxAlgebra|ALGEBRAS     += OptAuxAlgebra|g' secondo/makefile.algebras

#sed -i 's|#ALGEBRAS     += TinAlgebra|ALGEBRAS     += TinAlgebra|g' secondo/makefile.algebras
#sed -i 's|#ALGEBRA_DEPS += quadmath gmp|ALGEBRA_DEPS += quadmath gmp|g' secondo/makefile.algebras

sed -i '278s|#ALGEBRA_DIRS += Distributed2|ALGEBRA_DIRS += Distributed2|g' secondo/makefile.algebras
sed -i '279s|#ALGEBRAS += Distributed2Algebra|ALGEBRAS += Distributed2Algebra|g' secondo/makefile.algebras
sed -i '280s|#DEFAULTCCFLAGS += -pthread -DTHREAD_SAFE|DEFAULTCCFLAGS += -pthread -DTHREAD_SAFE|g' secondo/makefile.algebras
sed -i '281s|#CCFLAGS += -pthread -DTHREAD_SAFE|CCFLAGS += -pthread -DTHREAD_SAFE|g' secondo/makefile.algebras
sed -i '282s|#COMMON_LD_FLAGS += -lboost_thread -lboost_system|COMMON_LD_FLAGS += -lboost_thread -lboost_system|g' secondo/makefile.algebras

sed -i 's|#ALGEBRA_DIRS += RobustGeometry|ALGEBRA_DIRS += RobustGeometry|g' secondo/makefile.algebras
sed -i 's|#ALGEBRAS   += RobustGeometryAlgebra|ALGEBRAS   += RobustGeometryAlgebra|g' secondo/makefile.algebras

sed -i 's|#ALGEBRA_DIRS += MONTree|ALGEBRA_DIRS += MONTree|g' secondo/makefile.algebras
sed -i 's|#ALGEBRAS     += MONTreeAlgebra|ALGEBRAS     += MONTreeAlgebra|g' secondo/makefile.algebras

sed -i 's|# DEFAULTCCFLAGS += -DSM_FILE_ID|DEFAULTCCFLAGS += -DSM_FILE_ID|g' secondo/makefile.env 

cd secondo

# Link our debian files to the package
if [ ! -h debian ]; then
   rmdir debian
   ln -s ../debian debian
fi 

dpkg-buildpackage -rfakeroot -nc -b
