Installation of ZThread Library:

1. Get the patched version of this library, currently from Algebras/Distributed.
2. Unpack it into some directory of choice.
3. Within that directory, execute the steps
	configure
	make
	make install

Activation of Distributed Algebra:

4. In makefile.algebras, use the lines

ALGEBRA_DIRS += Distributed
ALGEBRAS     += DistributedAlgebra
ALGEBRA_DEPS += ZThread
# on max os X the framework CoreServices must be added
# uncomment the next line if you use mac os
#ALGEBRA_LINK_FLAGS += -framework CoreServices







