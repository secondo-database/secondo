/*
1 List of Algebra Modules for Secondo

This file contains a list of all currently available algebra modules.

The macros ~ALGEBRA\_INCLUDE~ and ~ALGEBRA\_EXCLUDE~ allow to include
or exclude an algebra to or from the set of loaded algebra modules.
These macros have three parameters:

  1 ~the unique identification number~ which must be a positive integer,
it is recommended but not absolutely necessary to order the entries of the
list in ascending order. No identification number may occur more than once
in the list.

  2 ~the algebra name~ which is used to build the name of the initialization
function: the algebra name is appended to the string "Initialize".

  3 ~the level of the algebra~ which may be one of the following: ~Descriptive~,
~Executable~ or ~Hybrid~.

*/

ALGEBRA_INCLUDE(1,StandardAlgebra,Hybrid)
ALGEBRA_INCLUDE(2,FunctionAlgebra,Executable)
//ALGEBRA_DYNAMIC(2,FunctionAlgebra,Executable)
ALGEBRA_INCLUDE(3,RelationAlgebra,Executable)
ALGEBRA_EXCLUDE(4,PointRectangleAlgebra,Executable)
ALGEBRA_INCLUDE(5,StreamExampleAlgebra,Executable)
ALGEBRA_INCLUDE(6,PolygonAlgebra,Executable)
ALGEBRA_INCLUDE(7,DateAlgebra,Executable)
ALGEBRA_INCLUDE(8,BTreeAlgebra,Executable)
ALGEBRA_INCLUDE(9,RangeAlgebra,Executable)
ALGEBRA_INCLUDE(10,SpatialAlgebra,Executable)
ALGEBRA_INCLUDE(11,FTextAlgebra,Executable)
ALGEBRA_INCLUDE(12,ArrayAlgebra,Executable)
ALGEBRA_INCLUDE(13,RTreeAlgebra,Executable)
//ALGEBRA_INCLUDE(14,JNIExampleAlgebra,Executable)
//ALGEBRA_INCLUDE(15,RoseAlgebra,Executable)
//ALGEBRA_INCLUDE(16,FuzzyAlgebra,Executable)


