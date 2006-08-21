/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header File of Module Network Algebra

March 2004 Victor Almeida

[TOC]

1 Overview

This file contains the C++ headers necessary for the type constructors ~network~,
~gpoint~, and ~gline~.

A network is composed by routes, junctions, and sections. For that, the ~network~
class contains three relations: ~routes~, ~junctions~, and ~sections~. An operator
for the network creation is provided, namely ~thenetwork~. This operator receives
two relations containing the information about routes and junctions in the following
format:

----    routes( id: int, length: real, geometry: line, dual: bool, 
                startsSmaller: bool )
        junctions( r1id: int, r1meas: real, r2id: int, r2meas: real, 
                   cc: int )
----

The internal representation of the ~routes~ relation is the same as the relation passed
in the creation operator. For the ~junctions~ relation, some attributes are appended.
First, record identifications (~r1rc~ and ~r2rc~) are added for direct access to the 
~routes~ relation, and second, a point (~pos~) containing the (x,y) position of the 
junction is also stored internally. The internal representation of the three relations 
is then:

----    routes( id: int, length: real, curve: line, dual: bool, 
                startsSmaller: bool )
        junctions( r1id: int, r1meas: real, r2id: int, r2meas: real, 
                   cc: int, pos: point, r1rc: int, r2rc: int )
        sections( rid: int, meas1: real, meas2: real, dual: bool, 
                  curve: line, rrc: int )
----

The sections internal structure also contains the record identification for routes (~rrc~).

Three operators are provided to retrieve the information about routes, junctions, and 
sections. They return relations with the following format:

----    routes( id: int, length: real, geometry: line, dual: bool, 
                startsSmaller: bool )
        junctions( r1id: int, r1meas: real, r2id: int, r2meas: real, 
                   cc: int )
        sections( rid: int, meas1: real, meas2: real, dual: bool, 
                  curve: line )
----

Given that, we have internal and external representations of the ~routes~, ~junctions~, 
and ~sections~ relations. A B-Tree on the ~id~ attribute of the ~routes~ relation is
also provided.
 
2 Defines, includes, and constants

*/
#ifndef __NETWORK_ALGEBRA_H__
#define __NETWORK_ALGEBRA_H__

#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 C++ Classes

3.1 Class ~Network~

*/
class Network
{
  public:

    Network();
/*
The simple constructor.

*/

    Network( Relation *routes, Relation *junctions, 
             Relation *sections, BTree *routesBTree );
/*
The constructor that receives all information to create a network.

*/

    ~Network();
/*
The destructor.

*/

    void Destroy();
/*
This function sets all information inside the network to be destroyed when the destructor
is called.

*/

    void Load( const Relation *routes, const Relation *junctions );
/*
Loads a network given two relations containing the routes and junctions. This function corresponds
to the operator ~thenetwork~.

*/

    ListExpr Out( ListExpr typeInfo );
    ListExpr Save( SmiRecord& valueRecord, size_t& offset, 
                   const ListExpr typeInfo );
    static Network *Open( SmiRecord& valueRecord, size_t& offset, 
                          const ListExpr typeInfo );
/*
The ~Out~, ~Save~, and ~Open~ functions of the type constructor ~network~. The ~In~ function is not
provided, given that the creation of the network is only done via the ~thenetwork~ operator.

*/

    Relation *GetRoutes();
/*
Returns a copy of the ~routes~ relation in external representation. This function is used
in the ~routes~ operator.

*/

    Relation *GetRoutesInternal();
/*
Returns the ~routes~ relation (in internal representation).

*/

    Relation *GetJunctions();
/*
Returns a copy of the ~junctions~ relation in external representation. This function is used
in the ~junctions~ operator.

*/  

    Relation *GetJunctionsInternal();
/*
Returns the ~junctions~ relation (in internal representation).

*/

    Relation *GetSections();
/*
Returns a copy of the ~sections~ relation in external representation. This function is used in 
the ~sections~ operator.

*/

    Relation *GetSectionsInternal();
/*
Returns the ~sections~ relation (in internal representation).

*/

    static ListExpr GetRoutesTypeInfo();
/*
Returns the internal and external (they are equal) ~routes~ relation type info.

*/

    static ListExpr GetRoutesBTreeTypeInfo();
/*
Returns the B-Tree in the ~routes~ relation type info.

*/

    static ListExpr GetJunctionsTypeInfo();
/*
Returns the external ~junctions~ relation type info.

*/

    static ListExpr GetJunctionsIntTypeInfo();
/*
Returns the internal ~junctions~ relation type info.

*/

    static ListExpr GetJunctionsAppTypeInfo();
/*
Returns the appended part of the ~junctions~ relation type info. The appended part corresponds 
to the difference between the external and the internal representations.

*/

    static ListExpr GetSectionsTypeInfo();
/*
Returns the external ~sections~ relation type info.

*/

    static ListExpr GetSectionsInternalTypeInfo();
/*
Returns the internal ~sections~ relation type info.

*/

    static ListExpr GetSectionsAppendTypeInfo();
/*
Returns the appended part of the ~sections~ relation type info. The appended part corresponds 
to the difference between the external and the internal representations.

*/

  private:

    enum PositionRoutesRelation { POS_RID = 0, POS_RLENGTH, POS_RCURVE, 
                                  POS_RDUAL, POS_RSTARTSSMALLER };
    enum PositionJunctionsRelation { POS_JR1ID = 0, POS_JMEAS1, POS_JR2ID, 
                                     POS_JMEAS2, POS_JCC, POS_JPOS, 
                                     POS_JR1RC, POS_JR2RC };
    enum PositionSectionsRelation { POS_SRID = 0, POS_SMEAS1, POS_SMEAS2, 
                                    POS_SDUAL, POS_SCURVE, POS_SRRC };
    enum PositionAppendJunctionsRelation { POS_APPJPOS = 0, POS_APPJR1RC, 
                                           POS_APPJR2RC };
    enum PositionAppendSectionsRelation { POS_APPSRRC = 0 };
/*
These enumerators are used to store the positions of each attribute in the relations to avoid
using numbers.

*/

    void FillRoutes( const Relation *routes );
/*
Copies the relation given as argument to the ~routes~ relations sorted by the ~id~ attribute
and creates the B-Tree in this attribute.

*/

    void FillJunctions( const Relation *junctions );
/*
Copies the relation given as argument to the ~junctions~ relation appending the new attributes.

*/

    void FillSections();
/*
Given the two ~routes~ and ~junctions~ relations, the ~sections~ relation is retrieved.

*/

    static string routesTypeInfo;
/*
The internal and external (they are equal) ~routes~ relation type info as string.

*/

    static string routesBTreeTypeInfo;
/*
The B-Tree in the ~routes~ relation type info as string.

*/

    static string junctionsTypeInfo;
/*
The external ~junctions~ relation type info as string.

*/

    static string junctionsInternalTypeInfo;
/*
The internal ~junctions~ relation type info as string.

*/

    static string junctionsAppendTypeInfo;
/*
The appended part of the ~junctions~ relation type info as string. The appended part corresponds
to the difference between the external and the internal representations.

*/

    static string sectionsTypeInfo;
/*
The external ~sections~ relation type info as string.

*/

    static string sectionsInternalTypeInfo;
/*
The internal ~sections~ relation type info as string.

*/

    static string sectionsAppendTypeInfo;
/*
The appended part of the ~sections~ relation type info as string. The appended part corresponds
to the difference between the external and the internal representations.

*/

    Relation *routes;
/*
The ~routes~ relation.

*/

    Relation *junctions;
/*
The ~junctions~ relation.

*/

    Relation *sections;
/*
The ~sections~ relation.

*/

    BTree *routesBTree;
/*
The B-Tree in the ~routes~ relation.

*/

};

#endif // __NETWORK_ALGEBRA_H__
