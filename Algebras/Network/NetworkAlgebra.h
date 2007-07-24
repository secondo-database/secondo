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
First, record identifications (~r1rc~, ~r2rc~, ~s1rc~, ~s2rc~) are added for 
direct access to the ~routes~ and ~sections~ relations. Second, a point (~pos~) containing 
the (x,y) position of the junction is also stored internally. Finally, as the junctions
are duplicated in this internal representation with the routes mixed, a pointer to the
partner junction ~partrc~ is added. The internal representation of the three relations 
is then:

----    routes( id: int, length: real, curve: line, dual: bool, 
                startsSmaller: bool )
        junctions( r1id: int, r1meas: real, r2id: int, r2meas: real, 
                   cc: int, pos: point, r1rc: tid, r2rc: tid,
                   s1rc: tid, s2rc: tid, partrc: tid )
        sections( rid: int, meas1: real, meas2: real, dual: bool, 
                  curve: line, rrc: tid )
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

#ifndef NESTED_LIST_H
#error NestedList.h is needed by NetworkAlgebra.h. Please include in *.cpp-File.
#endif

#ifndef DBARRAY_H
#error DBArray.h is needed by Network.h. Please include in *.cpp-File.
#endif




#endif // __NETWORK_ALGEBRA_H__
