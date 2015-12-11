
/*
3.10 ~LinearInt9mMove~ 

This class provides the unit type for a periodic moving 9 intersection matrix.

*/
#include <iostream>
#include <string>

#include "NestedList.h"
#include "PeriodicTypes.h"
#include "StandardTypes.h"
#include "DateTime.h"
#include "TopRel.h"


extern NestedList* nl;
using namespace std;
using namespace toprel;

namespace periodic{
/*
~ToConstantList~

This function converts a 9 intersection matrix to its external representation.

*/
ListExpr ToConstantListExpr(const Int9M value){
    return value.ToListExpr(nl->TheEmptyList());
}

/*
~ReadFrom~

This function reads a 9 intersection matrix from its 
external representation.

*/
bool ReadFromListExpr(ListExpr le, Int9M& value){
    return value.ReadFrom(le,::nl->TheEmptyList());
}

/*
~Constructor~

*/
LinearInt9MMove::LinearInt9MMove(){}


/*
~Constructor~

This constructor calls the constructor of the superclass.

*/
LinearInt9MMove::LinearInt9MMove(int dummy):LinearConstantMove<Int9M>(dummy){
}

/*
~Transpose~

This operator transposes the matrix managed by this unit.

*/
void LinearInt9MMove::Transpose(){
  value.Transpose();
}

} // end of namespace periodic
