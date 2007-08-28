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
//[TOC] [\tableofcontents]

[1] Implementation of Module Network Algebra

Mai-July 2007 Martin Scheppokat

Parts of the source taken from Victor Almeida

[TOC]

1 Overview


This file contains the implementation of the type ~network~,

2 Defines, includes, and constants

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
                                    SECTION_RRC };


/*
3 Class ConnectivityCode

*/

class ConnectivityCode
{

/*
3.1 Public Parts of the class

*/
  public:

/*
3.1.1 Enum defining binary coding for the possible transitions \\
between to routes.

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

/*
3.1.2 Constructor. 

Constructs the connectivity code given an integer value ~cc~.

*/
  ConnectivityCode( int in_iCc ):
    m_iConnectivityCode( in_iCc )
  {
  }

/*
3.1.2 Constructor for boolean values. 

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
3.1.3 Method isPossible. 

Checks if a transition is possible.

*/
  bool IsPossible( Transition in_xTransition ) const
  {
    return in_xTransition & m_iConnectivityCode;
  }

/*
3.2 Private Parts of the class

*/
  private: 
/*
3.2.1 The connectivity code

*/
    int m_iConnectivityCode;
};



/*
4 Class DirectedSection

This class is needed for the list of sections used in each entry 
in the adjacency-list

*/
class DirectedSection
{
/*
4.1 Public Parts of the class

*/
  public:

/*
4.1.1 Constructor

*/
  DirectedSection() 
  {
  }
  
/*
4.1.2 Constructor giving a section

*/
  DirectedSection( int in_iSectionTid,
                   bool in_bUpDown):
    m_iSectionTid( in_iSectionTid ),
    m_bUpDown( in_bUpDown )
  {
  }
  
/*
4.1.3 Copy-Constructor

*/
  DirectedSection( const DirectedSection& in_xSection ):
    m_iSectionTid( in_xSection.m_iSectionTid),
    m_bUpDown( in_xSection.m_bUpDown )
  {
  }
    

/*
4.1.4 Redefinition of the assignment operator.

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
  
  
/*
4.2 Private parts of the class

*/
  private:

/*
4.2.2 Section-pointer

A pointer to the section.

*/
  int m_iSectionTid;

/*

4.2.1 Direction-flag

A flag indicating the direction: ~true~ means up and ~false~ means down.

*/
  bool m_bUpDown;
};


/*
5 Class AdjacencyListEntry

Used for the adjacency-list of the network

*/
struct AdjacencyListEntry
{
/*
5.1 The simple constructor.

*/
  AdjacencyListEntry() {}

/*
5.2 The constructor.

*/
  AdjacencyListEntry( int in_iLow, 
                      int in_iHigh ):
  m_iLow( in_iLow ),
  m_iHigh( in_iHigh )
  {
  }

/*
5.3 The copy constructor.

*/
  AdjacencyListEntry( const AdjacencyListEntry& in_xEntry ):
  m_iLow( in_xEntry.m_iLow ), 
  m_iHigh( in_xEntry.m_iHigh )
  {
  }

/*
5.4 Lower index

The lower index in the adjacency lists sub-array.

*/
  int m_iLow;

/*
5.5 Higher index

The higher index in the adjacency lists sub-array.

*/
  int m_iHigh;
};

/*
6.6 Private parts - A helper struct

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
  
  float getRouteMeas()
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
6 Class ~Network~

*/
class Network
{
/*
6.2 Public parts of the class

*/
  public:

/*

6.2.1 Constructor

The simple constructor.

*/
  Network();

/*
6.2.2 Relation-Constuctor

The constructor that receives all information to create a network.

*/
  Network(SmiRecord& in_xValueRecord, 
          size_t& inout_iOffset, 
          const ListExpr in_xTypeInfo);

/*
6.2.3 List-Constructor 

The constructor that receives a list containing a network

*/
  Network(ListExpr in_xValue,
          int in_iErrorPos,
          ListExpr& inout_xErrorInfo, 
          bool& inout_bCorrect);

/*
6.2.4 The destructor.

*/
  ~Network();

/*
6.2.4 Destroy-Method

This function sets all information inside the network to be
destroyed when the destructor is called.

*/
    void Destroy();

/*
6.2.5 Load-Method
 
Loads a network given two relations containing the routes and junctions. 
This function corresponds to the operator ~thenetwork~.

*/
    void Load(int in_iId,
              const Relation *in_pRoutes, 
              const Relation *in_pJunctions);

/*
6.2.6 Out-Method
 
Outputs a network

*/
  ListExpr Out(ListExpr typeInfo);
    
/*
6.2.7 Save-Method

Saves all relations of the network

*/
  ListExpr Save( SmiRecord& valueRecord, 
                 size_t& offset, 
                 const ListExpr typeInfo );
                   
/*
6.2.8 Open-Method
 
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
6.3 Public parts - Get-Methods

*/

/*
Returns the id of this network

*/
    int GetId();

/*
6.3.1 GetRoutes
Returns a copy of the ~routes~ relation in external representation.  
This function is used in the ~routes~ operator.

*/
    Relation *GetRoutes();

/*
6.3.2 GetRoutesInternal
Returns the ~routes~ relation (in internal representation).

*/
    Relation *GetRoutesInternal();

/*
6.3.3 GetJunctions

Returns a copy of the ~junctions~ relation in external representation.  
This function is used in the ~junctions~ operator.

*/  
    Relation *GetJunctions();

/*
6.3.4 GetJunctionsInternal
 
Returns the ~junctions~ relation (in internal representation).

*/
    Relation *GetJunctionsInternal();

    void GetJunctionsOnRoute(CcInt* in_pRouteId,
                             vector<JunctionSortEntry>& inout_xJunctions);

    Tuple* GetSectionOnRoute(GPoint* in_xGPoint);

    Point* GetPointOnRoute(GPoint* in_xGPoint);


/*
6.3.5 GetSections
 
Returns a copy of the ~sections~ relation in external representation. 
This function is used in the ~sections~ operator.

*/
    Relation *GetSections();

/*
6.3.6 GetSectionsInternal

Returns the ~sections~ relation (in internal representation).

*/
    Relation *GetSectionsInternal();

/*
6.4 Public parts - Get-Methods for Type-Infos

*/

void GetAdjacentSections(int in_iSectionId,
                         bool in_bUpDown,
                         vector<DirectedSection> &inout_xSections);


/*
6.4.1 GetRoutesTypeInfo

Returns the internal and external (they are equal) ~routes~ relation type info.

*/
    static ListExpr GetRoutesTypeInfo();

/*
6.4.2 GetRoutesBTreeTypeInfo

Returns the B-Tree in the ~routes~ relation type info.

*/
    static ListExpr GetRoutesBTreeTypeInfo();

/*
6.4.2 GetRoutesBTreeTypeInfo

Returns the B-Tree in the ~junctions~ relation type info.

*/
    static ListExpr GetJunctionsBTreeTypeInfo();

/*
6.4.3 GetJunctionsTypeInfo

Returns the external ~junctions~ relation type info.

*/
    static ListExpr GetJunctionsTypeInfo();

/*
6.4.4 GetJunctionsIntTypeInfo

Returns the internal ~junctions~ relation type info.

*/
    static ListExpr GetJunctionsIntTypeInfo();

/*
6.4.5 GetSectionsTypeInfo

Returns the external ~sections~ relation type info.

*/
    static ListExpr GetSectionsTypeInfo();

/*
6.4.6 GetSectionsInternalTypeInfo

Returns the internal ~sections~ relation type info.

*/
    static ListExpr GetSectionsInternalTypeInfo();


/*
6.4 Public parts - Static-Methods supporting the type constructor for 
the network

*/

/*
6.4.1  NetworkProp

*/
  static ListExpr NetworkProp();

/*
6.4.2 OutNetwork

*/
  static ListExpr OutNetwork(ListExpr typeInfo, Word value);

/*
6.4.3 InNetwork

*/
  static Word InNetwork(ListExpr in_xTypeInfo, 
                        ListExpr in_xValue,
                        int in_iErrorPos, 
                        ListExpr& inout_xErrorInfo, 
                        bool& inout_bCorrect);

/*
6.4.4 CreateNetwork

*/
  static Word CreateNetwork(const ListExpr typeInfo);

/*
6.4.5 CloseNetwork

*/
  static void CloseNetwork(const ListExpr typeInfo, Word& w);

/*
6.4.6 CloneNetwork

*/
  static Word CloneNetwork(const ListExpr typeInfo, const Word& w);

/*
6.4.7 DeleteNetwork

*/
  static void DeleteNetwork(const ListExpr typeInfo, Word& w);

/*
6.4.8 CheckNetwork

*/
  static bool CheckNetwork(ListExpr type, ListExpr& errorInfo);

/*
6.4.9 CastNetwork

*/
  static void* CastNetwork(void* addr);

/*
6.4.10 SaveNetwork

*/
  static bool SaveNetwork( SmiRecord& valueRecord,
                           size_t& offset,
                           const ListExpr typeInfo,
                           Word& value );

/*
6.4.11 OpenNetwork

*/
  static bool OpenNetwork( SmiRecord& valueRecord, 
                           size_t& offset, 
                           const ListExpr typeInfo, 
                           Word& value );

/*
6.4.12 SizeOfNetwork

*/
  static int SizeOfNetwork();

/*
6.5 Private parts of the class

*/
  private:

/*
6.5.1 Enumerators

These enumerators are used to store the positions of each attribute in 
the relations to avoid using numbers.

*/

/*
6.5.2 Type-Info

*/

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

  static string sectionsTypeInfo;
/*
The external ~sections~ relation type info as string.

*/

  static string sectionsInternalTypeInfo;
/*
The internal ~sections~ relation type info as string.

*/



/*
6.6 Private parts - private methods

*/
/*
6.6.1 FillRoutes

Copies the relation given as argument to the ~routes~ relations sorted by 
the ~id~ attribute and creates the B-Tree in this attribute.

*/
  void FillRoutes( const Relation *in_pRoutes );

/*
6.6.2 FillJunctions

Copies the relation given as argument to the ~junctions~ relation appending 
the new attributes.

*/
  void FillJunctions( const Relation *in_pJunctions );

/*
6.6.3 FillSections

Given the two ~routes~ and ~junctions~ relations, the ~sections~ relation 
is retrieved.

*/
  void FillSections();
  
/*
6.6.4 FillAdjacencyLists

Given that all relations are set up, the adjacency lists are created.

*/
  void FillAdjacencyLists();

  void SaveAdjacencyList(SmiRecord& in_xValueRecord, 
                         size_t& inout_iOffset);

  void SaveSubAdjacencyList(SmiRecord& in_xValueRecord, 
                            size_t& inout_iOffset);

/*
Read a flob from a file

*/
  void OpenAdjacencyList(SmiRecord& in_xValueRecord, 
                         size_t& inout_iOffset);

  void OpenSubAdjacencyList(SmiRecord& in_xValueRecord, 
                            size_t& inout_iOffset);

/*
6.6 Private parts - relations holding the network

*/

 
/*
6.6.1 The ID of the network

*/
  int m_iId;

/*
6.6.1 True if all values of this network have been defined

*/
  int m_bDefined;


/*
6.6.1 The ~routes~ relation.

*/
  Relation* m_pRoutes;

/*
6.6.2 The ~junctions~ relation.

*/
  Relation* m_pJunctions;

/*
6.6.3 The ~sections~ relation.

*/
  Relation* m_pSections;

/*
6.6.4 The B-Tree in the ~routes~ relation.

*/
  BTree* m_pBTreeRoutes;

/*
6.6.5 The B-Tree in the ~routes~ relation.

*/
  BTree* m_pBTreeJunctionsByRoute1;

/*
6.6.6 The B-Tree in the ~routes~ relation.

*/
  BTree* m_pBTreeJunctionsByRoute2;

/*
6.6.l The adjacency lists of sections.

*/
  DBArray<AdjacencyListEntry> m_xAdjacencyList;

/*
6.6.8 The adjacency lists of sections.

*/
  DBArray<DirectedSection> m_xSubAdjacencyList;
};

#endif /*NETWORK_H_*/
