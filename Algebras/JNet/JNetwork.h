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

2011, May Simone Jandt

1 Includes

*/

#ifndef JNETWORK_H
#define JNETWORK_H

#include <map>
#include <string>
#include "NestedList.h"
#include "NList.h"
#include "StandardTypes.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../BTree/BTreeAlgebra.h"
#include "../RTree/RTreeAlgebra.h"
#include "../Spatial/SpatialAlgebra.h"
#include "ListPTIDRLoc.h"
#include "JListTID.h"
#include "ListNetDistGrp.h"
#include "ListPTIDRInt.h"

/*
1 Class ~JNetwork~

*/


class JNetwork
{

public:

/*
1.1 Constructors and Deconstructors

The default constructor should only be used in the cast function.

*/
  JNetwork();
  JNetwork(const bool def);
  JNetwork(const JNetwork& net);
  JNetwork(const string nid, Relation* injunctions, Relation* insections,
           Relation* inroutes);
  JNetwork(SmiRecord& valueRecord, size_t& offset,
           const ListExpr typeInfo);
  JNetwork(const ListExpr instance, const int errorPos,
           ListExpr& errorInfo, bool& correct);
  ~JNetwork();

/*
1.2 Getter and Setter for private Attributes

*/

  bool IsDefined() const;
  string GetId() const;
  Relation* GetJunctionsCopy() const;
  Relation* GetRoutesCopy() const;
  Relation* GetSectionsCopy() const;
  static const string GetRoutesTypeInfo();
  static const string GetJunctionsTypeInfo();
  static const string GetSectionsTypeInfo();

  void SetDefined(const bool def);
  void SetId(const string nid);

/*
1.3 Secondo Integration

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
  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );
  static ListExpr Property();

/*
1.4 StandardOperations

*/

  JNetwork& operator=(const JNetwork& net) ;
  bool operator==(const JNetwork& net) const;
  int Compare(const JNetwork& net) const;
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.5 Network Operations

1.5.1 CreateNetwork

Creates a network object from an string and two input relations.

The string value should be the name of the network object in the database.

The first relation defines the nodes of the network by four values of type
~int~, ~point~, ~int~, ~real~, whereas the meaning is JUNC\_ID, JUNC\_POS,
the id of the road the junctions belongs to and the position of the junction on
that road, it should be sorted by the jid and rid.

The second relation defines the roads of the network by six value of type
~int~, ~int~, ~point~, ~real~, ~real~, ~sline~, whereas the meaning is
ROUTE\_ID, junction id, position of junction on that road, the maximum allowed
speed on the road and the route curve.

*/

  void CreateNetwork(const string netid, const Relation* juncRel,
                     const Relation* routesRel);

/*
1.5.2 Save

Saves the Network Object to Secondo

*/

  bool Save(SmiRecord& valueRecord, size_t& offset, const ListExpr  typeInfo);

/*
1.1.1 Open

Opens the Network Object in Secondo

*/

  static JNetwork* Open(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo);

private:

/*
1.6 Private Attributes of Network Object

*/
  bool nDef;           //defined Flag
  string id;              //network identifier
  Relation* junctions;    //data of street crossings and death ends
  Relation* sections;     //data of connections between junctions
  Relation* routes;       //semantic connection of sections and junctions for
                          //street networks
  BTree* sectionsBTree;   //supports fast access to sections by sections id
  R_Tree<2,TupleId>* sectionsRTree;   //supports fast access to sections by
                                      //spatial input
  BTree* junctionsBTree;  //supports fast access to junctions by junction id
  R_Tree<2,TupleId>* junctionsRTree;  //supports fast access to junction by
                                      //spatial input
  BTree* routesBTree;     //supports fast access to routes by route id

/*
1.1 Relation Descriptors

*/
static string sectionsTypeInfo;
static string junctionsTypeInfo;
static string routesTypeInfo;
static string sectionsBTreeTypeInfo;
static string sectionsRTreeTypeInfo;
static string junctionsBTreeTypeInfo;
static string junctionsRTreeTypeInfo;
static string routesBTreeTypeInfo;

/*
1.7.1 Enumerations of coloumns of internal relations

*/
  enum PositionsJunctionsRelation {
    JUNC_ID = 0,
    JUNC_POS,
    JUNC_LIST_ROUTEPOSITIONS,
    JUNC_LIST_INSECTIONS,
    JUNC_LIST_OUTSECTIONS,
    JUNC_LIST_NETDISTANCES
  };

  enum PositionsSectionsRelation {
    SEC_ID = 0,
    SEC_CURVE,
    SEC_TID_STARTNODE,
    SEC_TID_ENDNODE,
    SEC_LIST_ROUTEINTERVALS,
    SEC_LIST_ADJSECTIONS_UP,
    SEC_LIST_ADJSECTIONS_DOWN,
    SEC_LIST_REV_ADJSECTIONS_UP,
    SEC_LIST_REV_ADJSECTIONS_DOWN,
    SEC_LENGTH,
    SEC_VMAX,
    SEC_DIRECTION
  };

  enum PositionsRoutesRelation {
    ROUTE_ID = 0,
    ROUTE_LIST_JUNCTIONS,
    ROUTE_LIST_SECTIONS,
    ROUTE_LENGTH
  };

/*
1.8 ListRepresentation of internal relations

*/

  ListExpr JunctionsToList() const;
  ListExpr SectionsToList() const;
  ListExpr RoutesToList() const;

/*
1.2 Access to private relations for copy constructors

*/

  Relation* GetJunctions() const;
  Relation* GetSections() const;
  Relation* GetRoutes() const;
  BTree* GetSectionsBTree() const;
  BTree* GetJunctionsBTree() const;
  BTree* GetRoutesBTree() const;
  R_Tree<2, TupleId>* GetSectionsRTree() const;
  R_Tree<2, TupleId>* GetJunctionsRTree() const;

/*
1.1 Create Network Relations

1.3.1 Initialize relations

*/

  void InitJunctions(const Relation* inJuncRel);
  void InitRoutesAndSections(const Relation* inRoutesRel);

/*
1.3.2 Update relations

*/

  void UpdateJunctions();
  void UpdateSections();

/*
1.8.4 Create Relations from type string

*/

  Relation* CreateRelation(const string descriptor, ListExpr& numType);

/*
1.8.5 Create Internal Trees

*/

  void CreateTrees();

/*
1.8.5.1 Creates BTree

Creates the BTree over the route ids of the given relation.

*/

  BTree* CreateBTree(const Relation* rel, const string descriptor,
                     const string attr);

/*
1.8.5.1 Create RTree

Creates the RTree over the spatial attribute of the given relation.

*/

  R_Tree<2,TupleId>* CreateRTree(const Relation* rel, const string descriptor,
                                 const string attr);


/*
1.1 OpenTrees and Relations

*/

  Relation* OpenRelation(const string descriptor, SmiRecord& valueRecord,
                       size_t& offset);
  BTree* OpenBTree(const string descriptor, SmiRecord& valueRecord,
                 size_t& offset);

/*
1.1 Return TupleId for id

*/

  TupleId GetTupleId( BTree* tree, const int id) const;
  TupleId GetJunctionTupleId(const int jid) const;
  TupleId GetRoutesTupleId(const int rid) const;
  TupleId GetSectionsTupleId(const int sid) const;

/*
1.1 Write Tuples to Relations

*/

  void WriteJunctionTuple(const int jid, Point* pos,
                        ListPTIDRLoc* listRLoc,
                        JListTID* listinsect,
                        JListTID* listoutsect,
                        ListNetDistGrp* listdist,
                        const ListExpr& juncNumType);

  void WriteRoutesTuple(const int rid,
                      const double length,
                      ListPTIDRLoc* listjunc,
                      ListPTIDRInt* listsect,
                      const ListExpr routesNumType);

  void WriteSectionTuple(const int sectId,
                       SimpleLine* curve,
                       const TupleId& curJunTID,
                       const TupleId& actJunTID,
                       const int actRouteId,
                       const double curJuncPosOnRoute,
                       const double actJuncPosOnRoute,
                       const JSide dir,
                       const double curMaxSpeed,
                       const ListExpr& sectionsNumType);

/*
1 Access to internal relations

Some helpful tools for access  to internal relations.

1.1. Copy

*/

  Relation* GetRelationCopy(const string relTypeInfo,
                            Relation* relPointer) const;

/*
1.1 ToList

*/

ListExpr RelationToList(Relation* rel, const string relTypeInfo) const;

};
#endif // JNETWORK_H
