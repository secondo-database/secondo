#include <string>
using namespace std;

#include "AlgebraManager.h"

/*
1.2 List of available Algebras

*/

/*
Creation of the prototypes of the initilization functions of all requested
algebra modules.

*/
#define ALGEBRA_INCLUDE ALGEBRA_PROTO_INCLUDE
#define ALGEBRA_EXCLUDE ALGEBRA_PROTO_EXCLUDE
#define ALGEBRA_DYNAMIC ALGEBRA_PROTO_DYNAMIC
#include "AlgebraList.i"

/*
Creation of the list of all requested algebra modules.
The algebra manager uses this list to initialize the algebras and
to access the type constructor and operator functions provided by
the algebra modules.

*/

#undef ALGEBRA_INCLUDE
#undef ALGEBRA_EXCLUDE
#undef ALGEBRA_DYNAMIC
#define ALGEBRA_INCLUDE ALGEBRA_LIST_INCLUDE
#define ALGEBRA_EXCLUDE ALGEBRA_LIST_EXCLUDE
#define ALGEBRA_DYNAMIC ALGEBRA_LIST_DYNAMIC

AlgebraListEntry& GetAlgebraEntry( const int j )
{
ALGEBRA_LIST_START
#include "AlgebraList.i"
ALGEBRA_LIST_END
/*
is the static list of all available algebra modules.

*/
  return (algebraList[j]);
}

