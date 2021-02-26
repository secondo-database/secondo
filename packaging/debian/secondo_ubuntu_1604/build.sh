#/bin/bash
#
# This script creates a debian package of the swi-prolog package
# that can be used with secondo
#####################################################################

# Install dependencies
apt-get install flex bison gcc g++ libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev db5.3-util libjpeg62 libjpeg62-dev libgsl0-dev libreadline-dev librecode-dev libgmp-dev libncurses-dev libxml2-dev libboost-all-dev build-essential debhelper fakeroot openjdk-8-jdk libxml2 libxml2-dev wget swi-prolog-nox swi-prolog-java swi-prolog libquadmath0 libgmp-dev libgmp10 libboost-thread-dev libboost-thread1.58.0 libboost-system-dev libboost-system1.58.0

VER=v420

if [ ! -f secondo-$VER-LAT1.tgz ]; then
    wget http://dna.fernuni-hagen.de/Secondo.html/files/Sources/secondo-$VER-LAT1.tgz 
    tar -xzf secondo-$VER-LAT1.tgz 
fi

cd secondo
cp makefile.algebras.sample makefile.algebras

# Activate additional algebras
sed -i 's|#ALGEBRA_DIRS += MapMatching|ALGEBRA_DIRS += MapMatching|g' makefile.algebras
sed -i 's|#ALGEBRAS     += MapMatchingAlgebra|ALGEBRAS     += MapMatchingAlgebra|g' makefile.algebras
sed -i 's|#ALGEBRA_DEPS += xml2|ALGEBRA_DEPS += xml2|g' makefile.algebras

sed -i 's|#ALGEBRA_DIRS += OSM|ALGEBRA_DIRS += OSM|g' makefile.algebras
sed -i 's|#ALGEBRAS     += OsmAlgebra|ALGEBRAS     += OsmAlgebra|g' makefile.algebras
sed -i 's|#ALGEBRA_DEPS += xml2|ALGEBRA_DEPS += xml2|g' makefile.algebras

sed -i 's|#ALGEBRA_DEP_DIRS += /usr/lib/libxml2|ALGEBRA_DEP_DIRS += /usr/lib/libxml2|g' makefile.algebras
sed -i 's|#ALGEBRA_INCLUDE_DIRS += /usr/include/libxml2|ALGEBRA_INCLUDE_DIRS += /usr/include/libxml2|g' makefile.algebras

#sed -i 's|#ALGEBRA_DIRS += MP3|ALGEBRA_DIRS += MP3|g' makefile.algebras
#sed -i 's|#ALGEBRAS     += MP3Algebra|ALGEBRAS     += MP3Algebra|g' makefile.algebras

sed -i 's|#ALGEBRA_DIRS += OptAux|ALGEBRA_DIRS += OptAux|g' makefile.algebras
sed -i 's|#ALGEBRAS     += OptAuxAlgebra|ALGEBRAS     += OptAuxAlgebra|g' makefile.algebras

#sed -i 's|#ALGEBRAS     += TinAlgebra|ALGEBRAS     += TinAlgebra|g' makefile.algebras
#sed -i 's|#ALGEBRA_DEPS += quadmath gmp|ALGEBRA_DEPS += quadmath gmp|g' makefile.algebras

sed -i '336s|#ALGEBRA_DIRS += Distributed2|ALGEBRA_DIRS += Distributed2|g' makefile.algebras
sed -i '337s|#ALGEBRAS += Distributed2Algebra|ALGEBRAS += Distributed2Algebra|g' makefile.algebras
sed -i '338s|#DEFAULTCCFLAGS += -pthread -DTHREAD_SAFE|DEFAULTCCFLAGS += -pthread -DTHREAD_SAFE|g' makefile.algebras
sed -i '339s|#CCFLAGS += -pthread -DTHREAD_SAFE|CCFLAGS += -pthread -DTHREAD_SAFE|g' makefile.algebras
sed -i '340s|#COMMON_LD_FLAGS += -lboost_thread -lboost_system|COMMON_LD_FLAGS += -lboost_thread -lboost_system|g' makefile.algebras

sed -i 's|#ALGEBRA_DIRS  += DFS|ALGEBRA_DIRS  += DFS|g' makefile.algebras
sed -i 's|#ALGEBRAS      += DFSAlgebra|ALGEBRAS      += DFSAlgebra|g' makefile.algebras
sed -i 's|#COMMON_LD_FLAGS += -L$(SECONDO_BUILD_DIR)/Tools/DFS/dfs -ldfs|COMMON_LD_FLAGS += -L$(SECONDO_BUILD_DIR)/Tools/DFS/dfs -ldfs|g' makefile.algebras
sed -i 's|#COMMON_LD_FLAGS += -L$(SECONDO_BUILD_DIR)/Tools/DFS/shared -lshared|COMMON_LD_FLAGS += -L$(SECONDO_BUILD_DIR)/Tools/DFS/shared -lshared|g' makefile.algebras
sed -i 's|#COMMON_LD_FLAGS += -L$(SECONDO_BUILD_DIR)/Tools/DFS/commlayer -lcommlayer|COMMON_LD_FLAGS += -L$(SECONDO_BUILD_DIR)/Tools/DFS/commlayer -lcommlayer|g' makefile.algebras

sed -i 's|#ALGEBRA_DIRS += RobustGeometry|ALGEBRA_DIRS += RobustGeometry|g' makefile.algebras
sed -i 's|#ALGEBRAS   += RobustGeometryAlgebra|ALGEBRAS   += RobustGeometryAlgebra|g' makefile.algebras

sed -i 's|#ALGEBRA_DIRS += MONTree|ALGEBRA_DIRS += MONTree|g' makefile.algebras
sed -i 's|#ALGEBRAS     += MONTreeAlgebra|ALGEBRAS     += MONTreeAlgebra|g' makefile.algebras

sed -i 's|# DEFAULTCCFLAGS += -DSM_FILE_ID|DEFAULTCCFLAGS += -DSM_FILE_ID|g' secondo/makefile.env
cd secondo

# Link our debian files to the package
if [ ! -h debian ]; then
   rmdir debian
   ln -s ../debian debian
fi 

dpkg-buildpackage -rfakeroot -nc -b
