/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

2012, May Simone Jandt

1 Includes

*/

#ifndef JNETWORK_H
#define JNETWORK_H

#include <map>
#include <string>
#include "NestedList.h"
#include "NList.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "OrderedRelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "RTreeAlgebra.h"
#include "SpatialAlgebra.h"
#include "JList.h"

/*
1 Class ~JNetwork~

Hybrid representation for street networks.

The representation consists of an defined flag (bool), an network id (string),
three relations, one ordered relation, and some BTree and RTree indices on this
relations providing faster access to the data stored in the network.

The defined flag tells if the network data is well defined. The network id
corresponds to the objectname of the network in the secondo database.

The first relation stores the network informations about street crossings, and
dead ends of streets. The relation is called junctions. The attributes of this
relation and their meanings are:

Attributname | Datatype | Meaning
==============================================================================
Id           | int      | unifique identifier of the street crossing
Position     | point    | spatial position of the crossing
ListRoutePos | listrloc | list of network (route) positions of the crossing
ListSectIn   | listint  | list of section identifiers for sections incoming to
             |          | this crossing
ListSectOut  | listint  | list of section identifiers for sections outgoing from
             |          | this crossing

The attribut Id is indexed by an BTree (junctionBTree) and the attribut
Position by an RTree (junctionRTree).

The second relation stores the network informations about the sections of the
street network. A section is the street part between two crossings resp. the
street part between a crossing and the end of a street.

Attributname      | Datatype  | Meaning
==============================================================================
Id                | int       | unifique identifier of the street section
Curve             | SimpleLine| spatial representation of the section
StartJuncId       | int       | identifier of the crossing at the start of the
                  |           | section
EndJuncId         | int       | identifier of the crossing at the end of the
                  |           | section
Side              | jdirection| tells if the section can be used upwards,
                  |           | downwards or in both directions
VMax              | real      | maximum allowed speed on this section in km/h
Length            | real      | length of the section in meter
ListRoadInter     | listjrint | list of road intervals represented by this
                  |           | section (sections may belong to more than one
                  |           | road, for example parts of main roads have often
                  |           | additional street names inside of towns)
ListAdjSectUp     | listint   | list of the identifiers of adjacent sections at
                  |           | the end of the section
ListAdjSectDow    | listint   | list of the identifiers of adjacent sections at
                  |           | the end of the section
ListRevAdjSectUp  | listint   | list of the identifiers of adjacent sections at
                  |           | the end of the section
ListRevAdjSectDown| listint   | list of the identifiers of adjacent sections at
                  |           | the end of the section

The attribut Id is again indexed by an BTree (sectionBTree) and the attribut
Curve by an RTree(sectionRTree).

In fact the complete network data is given in this two relations but men are
used into roads and positions on roads not in sections and junctions therefore
we maintain a third relation, called routes, which connects the junction and
section information to the road definition we normal use.

Attributname      | Datatype  | Meaning
===============================================================================
Id                | int       | unifique identifier of the road
ListJunctions     | listint   | list of junction identifiers for the crossings
                  |           | of this road
ListSections      | listint   | list of section identifiers for the sections
                  |           | belonging to this road
Length            | real      | length of the road in meter

The attribut Id is indexed by BTree routeBTree.

Last we define an ordered relation to be part of jnetwork which is stores the
netdistance values for already computed paths. The attributes are:

Attributename     | Datatype  | Meaning
===============================================================================
Source            | int       | identifier of source junction
Target            | int       | identifier of junction at the end of the path
NextJunction      | int       | identifier of next junction on the path
NextSection       | int       | identifier of section, which leads to the next
                  |           | junction on the path
NetworkDistance   | real      | network distance from source to target

The composite key is defined by source and target identifier.
At the start the relation is empty it is filled step by step at each network
distance computation.

*/


class JNetwork
{

/*
1.1. Public Declarations

*/
public:

/*
1.1.1 Constructors and Deconstructors

The first constructor expects the Relations in internal representation. It just
copies the given relations into the internal data structure and builds the
network indexes over the copied relations.

*/

  explicit JNetwork(const bool def);
  JNetwork(const string nid, const Relation* inJuncRel,
           const Relation* inSectRel, const Relation* inRoutesRel);
  JNetwork(const string nid, const Relation* inJuncRel,
           const Relation* inSectRel, const Relation* inRoutesRel,
           const OrderedRelation* inNetDistRel);
  JNetwork(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);

  ~JNetwork();

/*
1.1.1. ~Destroy~

Destroy all network data from database

*/

 void Destroy();

/*
1.1.1 Get and Set Network Data

1.1.1.1 ~IsDefined~

Returns defined flag.

*/

  bool IsDefined() const;

/*
1.1.1.1 ~GetId~

Returns the network id.

*/

  const STRING_T* GetId() const;

/*
1.1.1.1 Get Relation Type Infos

*/

 static string GetJunctionsRelationType();
 static string GetSectionsRelationType();
 static string GetRoutesRelationType();
 static string GetNetdistancesRelationType();

/*
1.1.1.1 ~SetDefined~

Sets the defined flag.

*/

void SetDefined(const bool def);

/*
1.1.1.1 GetRelationCopy

Returns a pointer to a copy of the original network data relation. The pointer
to the origin is not given to the outside to preserve internal network data from
beeing damaged.

*/

  Relation* GetJunctionsCopy() const;
  Relation* GetRoutesCopy() const;
  Relation* GetSectionsCopy() const;
  OrderedRelation* GetNedistancesRelationCopy() const;

/*
1.1.1 Secondo Integration

*/
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct);
  static Word Create(const ListExpr typeInfo);
  static void Delete( const ListExpr typeInfo, Word& w );
  static void Close( const ListExpr typeInfo, Word& w );
  static Word Clone( const ListExpr typeInfo, const Word& w );
  static void* Cast( void* addr );
  static bool KindCheck ( ListExpr type, ListExpr& errorInfo );
  static int SizeOf();
  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );
  bool Save(SmiRecord& valueRecord, size_t& offset,
       const ListExpr  typeInfo);
  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );
  static ListExpr Property();

/*
1.1.1 Standard Operations

*/

  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1.1 Operations for other network data types

1.1.1.1 ~Contains~
Returns true if the given position(s) exist in the network.

*/

  bool Contains(const RouteLocation* rloc) const;
  bool Contains(const JRouteInterval* rint) const;


/*
1.1 Private declarations

*/

private:

/*
1.1.1 Attributes of Network Object

*/
  bool defined;           //defined Flag
  STRING_T id;              //network identifier
  Relation* junctions;    //data of street crossings and death ends
  Relation* sections;     //data of connections between junctions
  Relation* routes;       //semantic connection of sections and junctions for
                          //street networks
  OrderedRelation* netdistances; //stores the network distances between the
                          //junctions
  BTree* junctionsBTree;  //supports fast access to junctions by junction id
  R_Tree<2,TupleId>* junctionsRTree;  //supports fast access to junction by
                                      //spatial input
  BTree* sectionsBTree;   //supports fast access to sections by sections id
  R_Tree<2,TupleId>* sectionsRTree;   //supports fast access to sections by
                                      //spatial input
  BTree* routesBTree;     //supports fast access to routes by route id

/*
1.1.1 Default Constructor

The default constructor should only be used in the cast function and is
therefore declared to be private.

*/

JNetwork();

/*
1.1.1 Descriptors of tuples, relations and indexes

*/
static string sectionsTupleTypeInfo;
static string junctionsTupleTypeInfo;
static string routesTupleTypeInfo;
static string netdistancesTupleTypeInfo;
static string sectionsRelationTypeInfo;
static string junctionsRelationTypeInfo;
static string routesRelationTypeInfo;
static string netdistancesRelationTypeInfo;
static string sectionsBTreeTypeInfo;
static string sectionsRTreeTypeInfo;
static string junctionsBTreeTypeInfo;
static string junctionsRTreeTypeInfo;
static string routesBTreeTypeInfo;

/*
1.1.1 Enumerations of coloumns of internal relations

*/
  enum PositionsJunctionsRelation {
    JUNC_ID = 0,
    JUNC_POS,
    JUNC_LIST_ROUTEPOSITIONS,
    JUNC_LIST_INSECTIONS,
    JUNC_LIST_OUTSECTIONS
  };

  enum PositionsSectionsRelation {
    SEC_ID = 0,
    SEC_CURVE,
    SEC_STARTNODE_ID,
    SEC_ENDNODE_ID,
    SEC_DIRECTION,
    SEC_VMAX,
    SEC_LENGTH,
    SEC_LIST_ROUTEINTERVALS,
    SEC_LIST_ADJ_SECTIONS_UP,
    SEC_LIST_ADJ_SECTIONS_DOWN,
    SEC_LIST_REV_ADJ_SECTIONS_UP,
    SEC_LIST_REV_ADJ_SECTIONS_DOWN
  };

  enum PositionsRoutesRelation {
    ROUTE_ID = 0,
    ROUTE_LIST_JUNCTIONS,
    ROUTE_LIST_SECTIONS,
    ROUTE_LENGTH
  };

  enum PositionsNetdistancesRelation {
    SOURCE_JID = 0,
    TARGET_JID,
    NEXT_JUNCTION_JID,
    NEXT_SECTIION_JID,
    NETWORKDISTANCE
  };

/*
1.1.1 Private Helper Functions

1.1.1.1 ListRepresentation of internal relations

*/

  ListExpr JunctionsToList() const;
  ListExpr SectionsToList() const;
  ListExpr RoutesToList() const;
  ListExpr NetdistancesToList() const;

/*
1.1.1.1 Create Internal Trees

*/

  void CreateTrees();

/*
1.1.1.1 Initialize ordered relation of netdistances by section length between
        junctions

*/

  void InitNetdistances();

/*
1.1.1.1 Tuple Access by Identifier

The returned tuple must be deleted by the caller.

*/

  Tuple* GetRouteTupleWithId(const int rid) const;

/*
1.1.1.1 Access to (tuple) attributes for identifiers

*/

  double GetRouteLength(const int rid) const;


};
/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const JNetwork& n);

#endif // JNETWORK_H
