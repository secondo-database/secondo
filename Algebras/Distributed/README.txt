Installation of ZThread Library:

1. Get the patched(*) version of this library, currently from Algebras/Distributed.
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





(*) Content of the patch:


The patch concerns two files:

ZThread-2.3.2/include/zthread/Guard.h
ZThread-2.3.2/src/MutexImpl.h

In prinziple, this patch replaces function calls by their qualified counterparts.
The patch is located within the ZThread.patch file. To patch the library by 
yourself, do the following:
Download the ZThread library version 2.3.2 and unpack it.  Go into the
newly created directory and enter the following line

patch -p1 < <path_to_patch>/ZThread.patch




