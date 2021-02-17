Association Analysis Algebra
============================

Author: Paul Fedorow <fedorow.paul@gmail.com>

Work in Progress

Build
-----

Add the Association Analysis algebra to
the `Algebras/Management/AlgebraList.i.cfg` file like this:

    ALGEBRA_INCLUDE(180,AssociationAnalysisAlgebra)

Replace the number 180 with an another unique number in case this identification
number is already taken.

Make sure at least the following algebras are included in
your `makefile.algebras` file to compile this algebra correctly:

    ALGEBRA_DIRS := Standard-C++
    ALGEBRAS     := StandardAlgebra
    ALGEBRA_DEPS :=
    ALGEBRA_LINK_FLAGS :=
    ALGEBRA_DEP_DIRS := $(SECONDO_SDK)/lib

    ALGEBRA_DIRS += FText
    ALGEBRAS     += FTextAlgebra

    ALGEBRA_DIRS += Relation-C++
    ALGEBRAS     += RelationAlgebra

    ALGEBRA_DIRS += ExtRelation-C++
    ALGEBRAS     += ExtRelationAlgebra

    ALGEBRA_DIRS += OrderedRelation
    ALGEBRAS     += OrderedRelationAlgebra

    ALGEBRA_DIRS += BTree
    ALGEBRAS     += BTreeAlgebra

    ALGEBRA_DIRS += TupleIdentifier
    ALGEBRAS     += TupleIdentifierAlgebra

    ALGEBRA_DIRS += Stream
    ALGEBRAS     += StreamAlgebra

    ALGEBRA_DIRS += Function-C++
    ALGEBRAS     += FunctionAlgebra

    ALGEBRA_DIRS += Hash
    ALGEBRAS     += HashAlgebra

    ALGEBRA_DIRS += DateTime
    ALGEBRAS     += DateTimeAlgebra

    ALGEBRA_DIRS += Collection
    ALGEBRAS     += CollectionAlgebra

    ALGEBRA_DIRS += AssociationAnalysis
    ALGEBRAS     += AssociationAnalysisAlgebra
