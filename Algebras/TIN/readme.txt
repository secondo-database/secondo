To activate the TinAlgebra please activate the following entries in file .../secondo/makefile.algebra :

ALGEBRA_DIRS += TIN
ALGEBRAS     += TinAlgebra
ALGEBRA_DEPS += quadmath gmp

Besides the activation of the algebra, this will ensure, that the library quadmath is linked. 
For more information on quadmath library goto:
https://gcc.gnu.org/onlinedocs/libquadmath/

