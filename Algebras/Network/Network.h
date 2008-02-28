/*
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

1.1 Declaration of Network

Mai-September 2007 Martin Scheppokat

Parts of the source taken from Victor Almeida

This file contains the implementation of the type ~network~,

Defines, includes, and constants

*/

#ifndef NETWORK_H_
#define NETWORK_H_

#ifndef NESTED_LIST_H
#error NestedList.h is needed by Network.h. \
Please include in *.cpp-File.
#endif

#ifndef GPOINT_H_
#error GPoint.h is needed by Network.h. \
Please include in *.cpp-File.
#endif

#ifndef _RELATION_ALGEBRA_H_
#error RelationAlgebra.h is needed by Network.h. \
Please include in *.cpp-File.
#endif

#ifndef _BTREE_ALGEBRA_H_
#error BTreeAlgebra.h is needed by Network.h. \
Please include in *.cpp-File.
#endif


#ifndef DBARRAY_H
#error DBArray.h is needed by Network.h. \
Please include in *.cpp-File.
#endif

/*
Enumerations of columns for relations

*/

    enum PositionRoutesRelation { ROUTE_ID = 0,
                                  ROUTE_LENGTH,
                                  ROUTE_CURVE,
                                  ROUTE_DUAL,
                                  ROUTE_STARTSSMALLER };
    enum PositionJunctionsRelation { JUNCTION_ROUTE1_ID = 0,
                                     JUNCTION_ROUTE1_MEAS,
                                     JUNCTION_ROUTE2_ID,
                                     JUNCTION_ROUTE2_MEAS,
                                     JUNCTION_CC,
                                     JUNCTION_POS,
                                     JUNCTION_ROUTE1_RC,
                                     JUNCTION_ROUTE2_RC,
                                     JUNCTION_SECTION_AUP_RC,
                                     JUNCTION_SECTION_ADOWN_RC,
                                     JUNCTION_SECTION_BUP_RC,
                                     JUNCTION_SECTION_BDOWN_RC};
    enum PositionSectionsRelation { SECTION_RID = 0,
                                    SECTION_MEAS1,
                                    SECTION_MEAS2,
                                    SECTION_DUAL,
                                    SECTION_CURVE,
                                    SECTION_CURVE_STARTS_SMALLER,
                                    SECTION_RRC };


/*
Class ConnectivityCode

*/
/*
Enum for transitions

Enum defining binary coding for the possible transitions between to routes.

*/
  enum Transition
  {
    AUP_AUP     = 0x0001,
    AUP_ADOWN   = 0x0002,
    AUP_BUP     = 0x0004,
    AUP_BDOWN   = 0x0008,
    ADOWN_AUP   = 0x0010,
    ADOWN_ADOWN = 0x0020,
    ADOWN_BUP   = 0x0040,
    ADOWN_BDOWN = 0x0080,
    BUP_AUP     = 0x0100,
    BUP_ADOWN   = 0x0200,
    BUP_BUP     = 0x0400,
    BUP_BDOWN   = 0x0800,
    BDOWN_AUP   = 0x1000,
    BDOWN_ADOWN = 0x2000,
    BDOWN_BUP   = 0x4000,
    BDOWN_BDOWN = 0x8000
  };

class ConnectivityCode
{

  public:


/*
Constructor.

Constructs the connectivity code given an integer value ~cc~.

*/
  ConnectivityCode( int in_iCc ):
    m_iConnectivityCode( in_iCc )
  {
  }

/*
Constructor for boolean values.

Constructs the connectivity code given the Boolean values for
all possibilities.

*/
  ConnectivityCode( bool in_bAup_Aup,
                    bool in_bAup_Adown,
                    bool in_bAup_Bup,
                    bool in_bAup_Bdown,
                    bool in_bAdown_Aup,
                    bool in_bAdown_Adown,
                    bool in_bAdown_Bup,
                    bool in_bAdown_Bdown,
                    bool in_bBup_Aup,
                    bool in_bBup_Adown,
                    bool in_bBup_Bup,
                    bool in_bBup_Bdown,
                    bool in_bBdown_Aup,
                    bool in_bBdown_Adown,
                    bool in_bBdown_Bup,
                    bool in_bBdown_Bdown ):
    m_iConnectivityCode( (in_bAup_Aup && AUP_AUP) |
                         (in_bAup_Adown && AUP_ADOWN) |
                         (in_bAup_Bup && AUP_BUP) |
                         (in_bAup_Bdown && AUP_BDOWN) |
                         (in_bAdown_Aup && ADOWN_AUP) |
                         (in_bAdown_Adown && AUP_ADOWN) |
                         (in_bAdown_Bup && ADOWN_BUP) |
                         (in_bAdown_Bdown && AUP_BDOWN) |
                         (in_bBup_Aup && BUP_AUP) |
                         (in_bBup_Adown && BUP_ADOWN) |
                         (in_bBup_Bup && BUP_BUP) |
                         (in_bBup_Bdown && BUP_BDOWN) |
                         (in_bBdown_Aup && BDOWN_AUP) |
                         (in_bBdown_Adown && BUP_ADOWN) |
                         (in_bBdown_Bup && BDOWN_BUP) |
                         (in_bBdown_Bdown && BUP_BDOWN) )
  {
  }



/*
Method isPossible

Checks if a transition is possible.

*/
  bool IsPossible( Transition in_xTransition ) const
  {
    return in_xTransition & m_iConnectivityCode;
  }

  private:
/*
Field ConnectivtyCode

The connectivity code

*/
    int m_iConnectivityCode;
};



/*
Class DirectedSection

This class is needed for the list of sections used in each entry
in the adjacency-list

*/
class DirectedSection
{
  public:

/*
Constructor

*/
  DirectedSection()
  {
  }

/*
Constructor giving a section

*/
  DirectedSection( int in_iSectionTid,
                   bool in_bUpDown):
    m_iSectionTid( in_iSectionTid ),
    m_bUpDown( in_bUpDown )
  {
  }

/*
Copy-Constructor

*/
  DirectedSection( const DirectedSection& in_xSection ):
    m_iSectionTid( in_xSection.m_iSectionTid),
    m_bUpDown( in_xSection.m_bUpDown )
  {
  }


/*
Redefinition of the assignment operator.

*/
  DirectedSection& operator=( const DirectedSection& in_xSection )
  {
    m_bUpDown = in_xSection.m_bUpDown;
    m_iSectionTid = in_xSection.m_iSectionTid;
    return *this;
  }

  bool getUpDownFlag()
  {
    return m_bUpDown;
  }

  int getSectionTid()
  {
    return m_iSectionTid;
  }

  private:

/*
Field with section-pointer

A pointer to the section.

*/
  int m_iSectionTid;

/*
Field Direction-flag

A flag indicating the direction: ~true~ means up and ~false~ means down.

*/
  bool m_bUpDown;
};

/*
Class DirectedSection

This class is needed for the list of sections used in each entry
in the adjacency-list

*/
class DirectedSectionPair
{
  public:

/*
Constructor

*/
  DirectedSectionPair()
  {
  }

/*
Constructor giving a section

*/
  DirectedSectionPair(int in_iFirstSectionTid,
                      bool in_bFirstUpDown,
                      int in_iSecondSectionTid,
                      bool in_bSecondUpDown):
    m_iFirstSectionTid( in_iFirstSectionTid ),
    m_bFirstUpDown( in_bFirstUpDown ),
    m_iSecondSectionTid( in_iSecondSectionTid ),
    m_bSecondUpDown( in_bSecondUpDown )
  {
  }

/*
Copy-Constructor

*/
  DirectedSectionPair( const DirectedSectionPair& in_xSection ):
    m_iFirstSectionTid(in_xSection.m_iFirstSectionTid ),
    m_bFirstUpDown(in_xSection.m_bFirstUpDown ),
    m_iSecondSectionTid(in_xSection.m_iSecondSectionTid ),
    m_bSecondUpDown(in_xSection.m_bSecondUpDown )
  {
  }

  bool operator<(const DirectedSectionPair& in_xOther) const
  {
    if(m_iFirstSectionTid != in_xOther.m_iFirstSectionTid)
    {
      return m_iFirstSectionTid < in_xOther.m_iFirstSectionTid;
    }
    if(m_bFirstUpDown != in_xOther.m_bFirstUpDown)
    {
      return m_bFirstUpDown;
    }
    if(m_iSecondSectionTid != in_xOther.m_iSecondSectionTid)
    {
      return m_iSecondSectionTid < in_xOther.m_iSecondSectionTid;
    }
    return m_bSecondUpDown;
  }



/*
Field with section-pointer

A pointer to the section.

*/
  int m_iFirstSectionTid;

/*

Field Direction-flag

A flag indicating the direction: ~true~ means up and ~false~ means down.

*/
  bool m_bFirstUpDown;

/*
Field with section-pointer

A pointer to the section.

*/
  int m_iSecondSectionTid;

/*
Field Direction-flag

A flag indicating the direction: ~true~ means up and ~false~ means down.

*/
  bool m_bSecondUpDown;
};


/*
Class AdjacencyListEntry

Used for the adjacency-list of the network

*/
struct AdjacencyListEntry
{
/*
The simple constructor.

*/
  AdjacencyListEntry() {}

/*
The constructor.

*/
  AdjacencyListEntry( int in_iLow,
                      int in_iHigh ):
  m_iLow( in_iLow ),
  m_iHigh( in_iHigh )
  {
  }

/*
The copy constructor.

*/
  AdjacencyListEntry( const AdjacencyListEntry& in_xEntry ):
  m_iLow( in_xEntry.m_iLow ),
  m_iHigh( in_xEntry.m_iHigh )
  {
  }

/*
Lower index

The lower index in the adjacency lists sub-array.

*/
  int m_iLow;

/*
Higher index

The higher index in the adjacency lists sub-array.

*/
  int m_iHigh;
};

/*
Class JunctionSortEntry - A helper struct

*/
struct JunctionSortEntry
{
  JunctionSortEntry()
  {
  }

  JunctionSortEntry(bool in_bFirstRoute,
                    Tuple* in_pJunction):
    m_bFirstRoute(in_bFirstRoute),
    m_pJunction(in_pJunction)
  {
  }

  bool m_bFirstRoute;

  Tuple* m_pJunction;

  bool operator<(const JunctionSortEntry& in_xOther) const
  {
    CcReal* xMeas1;
    if(m_bFirstRoute)
    {
      xMeas1 = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    }
    else
    {
      xMeas1 = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE2_MEAS);
    }

    CcReal* xMeas2;
    if(in_xOther.m_bFirstRoute)
    {
      xMeas2 =
          (CcReal*)in_xOther.m_pJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    }
    else
    {
      xMeas2 =
          (CcReal*)in_xOther.m_pJunction->GetAttribute(JUNCTION_ROUTE2_MEAS);
    }

    return (xMeas1->GetRealval() < xMeas2->GetRealval());
  }

  double getRouteMeas()
  {
    CcReal* pMeas;
    if(m_bFirstRoute)
    {
      pMeas = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    }
    else
    {
      pMeas = (CcReal*)m_pJunction->GetAttribute(JUNCTION_ROUTE2_MEAS);
    }
    return pMeas->GetRealval();

  }

  int getUpSectionId()
  {
    TupleIdentifier* pTid;
    if(m_bFirstRoute)
    {
      pTid =
      (TupleIdentifier*)m_pJunction->GetAttribute(JUNCTION_SECTION_AUP_RC);
    }
    else
    {
      pTid =
      (TupleIdentifier*)m_pJunction->GetAttribute(JUNCTION_SECTION_BUP_RC);
    }
    return pTid->GetTid();
  }

  int getDownSectionId()
  {
    TupleIdentifier* pTid;
    if(m_bFirstRoute)
    {
      pTid =
      (TupleIdentifier*)m_pJunction->GetAttribute(JUNCTION_SECTION_ADOWN_RC);
    }
    else
    {
      pTid =
      (TupleIdentifier*)m_pJunction->GetAttribute(JUNCTION_SECTION_BDOWN_RC);
    }
    return pTid->GetTid();
  }
};

/*
Public Methods of Class ~Network~

*/
class Network
{
  public:

  static string routesTypeInfo;
/*
The internal and external (they are equal) ~routes~ relation type info
as string.

*/

  static string routesBTreeTypeInfo;
/*
The B-Tree in the ~routes~ relation type info as string.

*/

  static string junctionsBTreeTypeInfo;
/*
The B-Tree in the ~junctions~ relation type info as string.

*/

  static string junctionsTypeInfo;
/*
The external ~junctions~ relation type info as string.

*/

  static string junctionsInternalTypeInfo;
/*
The internal ~junctions~ relation type info as string.

*/

  static string sectionsInternalTypeInfo;
/*
The internal ~sections~ relation type info as string.

*/


/*

Constructor

The simple constructor.

*/
  Network();

/*
Relation-Constuctor

The constructor that receives all information to create a network.

*/
  Network(SmiRecord& in_xValueRecord,
          size_t& inout_iOffset,
          const ListExpr in_xTypeInfo);

/*
List-Constructor

The constructor that receives a list containing a network

*/
  Network(ListExpr in_xValue,
          int in_iErrorPos,
          ListExpr& inout_xErrorInfo,
          bool& inout_bCorrect);

/*
The destructor.

*/
  ~Network();

/*
Destroy-Method

This function sets all information inside the network to be
destroyed when the destructor is called.

*/
    void Destroy();

/*
Load-Method

Loads a network given two relations containing the routes and junctions.
This function corresponds to the operator ~thenetwork~.

*/
    void Load(int in_iId,
              const Relation *in_pRoutes,
              const Relation *in_pJunctions);

/*
Out-Method

Outputs a network

*/
  ListExpr Out(ListExpr typeInfo);

/*
Save-Method

Saves all relations of the network

*/
  ListExpr Save( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo );

/*
Open-Method

Opens a network

*/
  static Network* Open( SmiRecord& valueRecord,
                        size_t& offset,
                        const ListExpr typeInfo );

/*
The ~Out~, ~Save~, and ~Open~ functions of the type constructor ~network~.
The ~In~ function is not provided, given that the creation of the network
is only done via the ~thenetwork~ operator.

*/


/*
Method GetId

Returns the id of this network

*/
    int GetId();

/*
GetRoutes

Returns a copy of the ~routes~ relation in external representation.
This function is used in the ~routes~ operator.

*/
    Relation *GetRoutes();

/*
GetJunctions

Returns a copy of the ~junctions~ relation in external representation.
This function is used in the ~junctions~ operator.

*/
    Relation *GetJunctions();

/*
GetJunctionsOnRoute


*/
    void GetJunctionsOnRoute(CcInt* in_pRouteId,
                             vector<JunctionSortEntry>& inout_xJunctions);

/*
GetSectionOnRoute


*/
    Tuple* GetSectionOnRoute(GPoint* in_xGPoint);

/*
GetPointOnRoute

*/
    Point* GetPointOnRoute(GPoint* in_xGPoint);


/*
GetSections

Returns a copy of the ~sections~ relation in external representation.
This function is used in the ~sections~ operator.

*/
    Relation *GetSections();

/*
GetSectionsInternal

Returns the ~sections~ relation (in internal representation).

*/
    Relation *GetSectionsInternal();

/*
Method GetAdjacentSections

*/

    void GetAdjacentSections(int in_iSectionId,
                             bool in_bUpDown,
                             vector<DirectedSection> &inout_xSections);


/*
Static methods of Class ~Network~ supporting the type constructor

Public parts - Static-Methods supporting the type constructor for
the network

*/

/*
NetworkProp

*/
  static ListExpr NetworkProp();

/*
OutNetwork

*/
  static ListExpr OutNetwork(ListExpr typeInfo, Word value);

/*
InNetwork

*/
  static Word InNetwork(ListExpr in_xTypeInfo,
                        ListExpr in_xValue,
                        int in_iErrorPos,
                        ListExpr& inout_xErrorInfo,
                        bool& inout_bCorrect);

/*
CreateNetwork

*/
  static Word CreateNetwork(const ListExpr typeInfo);

/*
CloseNetwork

*/
  static void CloseNetwork(const ListExpr typeInfo, Word& w);

/*
CloneNetwork

*/
  static Word CloneNetwork(const ListExpr typeInfo, const Word& w);

/*
DeleteNetwork

*/
  static void DeleteNetwork(const ListExpr typeInfo, Word& w);

/*
CheckNetwork

*/
  static bool CheckNetwork(ListExpr type, ListExpr& errorInfo);

/*
CastNetwork

*/
  static void* CastNetwork(void* addr);

/*
SaveNetwork

*/
  static bool SaveNetwork( SmiRecord& valueRecord,
                           size_t& offset,
                           const ListExpr typeInfo,
                           Word& value );

/*
OpenNetwork

*/
  static bool OpenNetwork( SmiRecord& valueRecord,
                           size_t& offset,
                           const ListExpr typeInfo,
                           Word& value );

/*
SizeOfNetwork

*/
  static int SizeOfNetwork();

/*
isDefined

*/

 int isDefined();

/*
Computes the junction of the two given routes and returns the routemeasvalues
of the junction on this route.

*/

 void GetJunctionMeasForRoutes(CcInt *pLastRouteId, CcInt *pCurrentSectionRid,
                              double& rid1meas, double& rid2meas);

  private:


/*
Private methods of class ~Network~

*/

/*
FillRoutes

Copies the relation given as argument to the ~routes~ relations sorted by
the ~id~ attribute and creates the B-Tree in this attribute.

*/
  void FillRoutes( const Relation *in_pRoutes );

/*
FillJunctions

Copies the relation given as argument to the ~junctions~ relation appending
the new attributes.

*/
  void FillJunctions( const Relation *in_pJunctions );

/*
FillSections

Given the two ~routes~ and ~junctions~ relations, the ~sections~ relation
is retrieved.

*/
  void FillSections();

/*
FillAdjacencyLists

Given that all relations are set up, the adjacency lists are created.

*/
  void FillAdjacencyLists();

  void FillAdjacencyPair(Tuple* in_pFirstSection,
                         bool in_bFirstUp,
                         Tuple* in_pSecondSection,
                         bool in_bSecondUp,
                         ConnectivityCode in_xCc,
                         Transition in_xTransition,
                         vector<DirectedSectionPair> &inout_xPairs);


  void SaveAdjacencyList(SmiRecord& in_xValueRecord,
                         size_t& inout_iOffset,
                         SmiFileId& fileId);

  void SaveSubAdjacencyList(SmiRecord& in_xValueRecord,
                            size_t& inout_iOffset,
                            SmiFileId& fileId);

/*
Read a flob from a file

*/
  void OpenAdjacencyList(SmiRecord& in_xValueRecord,
                         size_t& inout_iOffset);

  void OpenSubAdjacencyList(SmiRecord& in_xValueRecord,
                            size_t& inout_iOffset);

/*
Private fields of Class ~Network~

*/



/*
The ID of the network

*/
  int m_iId;

/*
True if all values of this network have been defined

*/
  int m_bDefined;


/*
The ~routes~ relation.

*/
  Relation* m_pRoutes;

/*
The ~junctions~ relation.

*/
  Relation* m_pJunctions;

/*
The ~sections~ relation.

*/
  Relation* m_pSections;

/*
The B-Tree in the ~routes~ relation.

*/
  BTree* m_pBTreeRoutes;

/*
The B-Tree in the ~routes~ relation.

*/
  BTree* m_pBTreeJunctionsByRoute1;

/*
The B-Tree in the ~routes~ relation.

*/
  BTree* m_pBTreeJunctionsByRoute2;

/*
The adjacency lists of sections.

*/
  DBArray<AdjacencyListEntry> m_xAdjacencyList;

/*
The adjacency lists of sections.

*/
  DBArray<DirectedSection> m_xSubAdjacencyList;
};

#endif /*NETWORK_H_*/
