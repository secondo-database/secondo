Installation of ZThread Library:

1. Get the patched version of this library, currently from Algebras/Distributed.
2. Unpack it into some directory of choice.
   ATTENTION: The directory must be outside of the scondo directory strcuture.
   (e.g. use ~/tmp/zthread/ )
3. Within that directory, execute the steps
	configure
	make
	make install (with root permissions, e.g. "sudo make install")

Activation of Distributed Algebra:

4. In makefile.algebras, use the lines

ALGEBRA_DIRS += Distributed
ALGEBRAS     += DistributedAlgebra
ALGEBRA_DEPS += ZThread
# on max os X the framework CoreServices must be added
# uncomment the next line if you use mac os
#ALGEBRA_LINK_FLAGS += -framework CoreServices

5. Add the following line to your .secondorc (or .bashrc) file:

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

if /usr/local/lib/ is the place where make install has put libZThread.a







