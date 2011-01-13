Installation of ZThread Library:

1. Get the patched version of this library, currently from Algebras/Distributed.
2. Unpack it into some directory of choice.
3. Within that directory, execute the steps
	configure
	make
	make install
4. On MacOS, in the file secondo.config.mac_osx located in secondo-sdk, modify 
   the line

	export SECONDO_LDFLAGS="-L$SECONDO_SDK/auxtools/lib

   to

	 SECONDO_LDFLAGS="-L$SECONDO_SDK/auxtools/lib -framework CoreServices"

Activation of Distributed Algebra:

5. In makefile.algebras, use the three lines

ALGEBRA_DIRS 	+= Distributed 
ALGEBRAS 	+= DistributedAlgebra
ALGEBRA_DEPS 	+= ZThread






