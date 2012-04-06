For this algebra, installation of library libxml2 is necessary.

1. For example, on Ubuntu 10.04:

sudo apt-get install libxml2
sudo apt-get install libxml2-dev


Activation of algebra:

2. In makefile.algebras, use the lines

ALGEBRA_DIRS += OSM
ALGEBRAS     += OsmAlgebra
ALGEBRA_DEPS += xml2
ALGEBRA_INCLUDE_DIRS += /usr/include/libxml2

# on max os X the framework CoreServices must be added
# uncomment the next line if you use mac os
#ALGEBRA_LINK_FLAGS += -framework CoreServices


3. Check that $LD_LIBRARY_PATH contains the directory where installation has put the file libxml2.a. On Ubuntu, it is /usr/lib.

If it is missing, add the following line to your .secondorc (or .bashrc) file:

export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH

