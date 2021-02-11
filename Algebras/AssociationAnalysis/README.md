Association Analysis Algebra
============================

Author: Paul Fedorow <fedorow.paul@gmail.com>

Work in Progress

Build
-----

Ensure the following is included in your makefile.algebras to compile this algebra correctly:

    ALGEBRA_DIRS := Standard-C++
    ALGEBRAS     := StandardAlgebra
    ALGEBRA_DEPS :=
    ALGEBRA_LINK_FLAGS :=
    ALGEBRA_DEP_DIRS := $(SECONDO_SDK)/lib

    ALGEBRA_DIRS += Relation-C++
    ALGEBRAS     += RelationAlgebra

    ALGEBRA_DIRS += FText
    ALGEBRAS     += FTextAlgebra

    ALGEBRA_DIRS += OrderedRelation
    ALGEBRAS     += OrderedRelationAlgebra

    ALGEBRA_DIRS += TupleIdentifier
    ALGEBRAS     += TupleIdentifierAlgebra

    ALGEBRA_DIRS += Stream
    ALGEBRAS     += StreamAlgebra

    ALGEBRA_DIRS += DateTime
    ALGEBRAS     += DateTimeAlgebra

    ALGEBRA_DIRS += Function-C++
    ALGEBRAS     += FunctionAlgebra

    ALGEBRA_DIRS += Collection
    ALGEBRAS     += CollectionAlgebra

    ALGEBRA_DIRS += AssociationAnalysis
    ALGEBRAS     += AssociationAnalysisAlgebra
