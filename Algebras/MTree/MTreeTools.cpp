/*
----
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
----

//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

1 Implementation of ~MTreeTools~

November 2007 Mirko Dibbert

[TOC]

1 Defines and includes

*/
using namespace std;

#include "MTree.h"
#include <time.h>

/*
2 Class ~SplitPolicy~

2.1 Constructor

*/
SplitPolicy::SplitPolicy( DistanceFunction *distFun,
                          PROMFUN promFunId, PARTFUN partFunId )
{
  this->distFun = distFun;

  // init promote function
  switch ( promFunId )
  {
    case RANDOM:
      promFun = RAND_Prom; break;

    case m_RAD: 
      promFun = MRAD_Prom; break;

    case mM_RAD:
      promFun = MMRAD_Prom; break;

    case M_LB_DIST:
      promFun = MLB_Prom; break;
  }

  // init partition function
  switch ( partFunId )
  {
    case GENERALIZED_HYPERPLANE:
      partFun = HyperplanePart; break;

    case BALANCED:
      partFun = BalancedPart; break;
  }
}

/*
2.2 Promote functions

2.2.1 Method ~RANDProm~

*/
void SplitPolicy::RAND_Prom(
      list<MTreeEntry*> &entries,
      MTreeEntry *prom1, MTreeEntry *prom2 )
{
  srand ((unsigned) time(NULL));
  int pos1 = rand() % entries.size();
  int pos2 = rand() % entries.size();

  // set another random value for pos2, until it is different form pos1
  while ( pos1 != pos2 )
    pos2 = rand() % entries.size();

  // sort positions
  if (pos1 > pos2)
  {
    int buf = pos1;
    pos1 = pos2;;
    pos2 = buf;
  }

  list<MTreeEntry*>::iterator iter = entries.begin();

  // select pos1
  for (int pos = 0; pos < pos1; pos++)
    iter++;

  // get prom1 and delete it from entries
  prom1 = *iter;
  iter = entries.erase( iter );

  // select pos2
  for (int pos = ( pos1 + 1 ); pos < pos2; pos++)
    iter++;

  // get prom2 and delete it from entries
  prom2 = *iter;
  iter = entries.erase( iter );
}

/*
2.2.2 Method ~MRADProm~

*/
void SplitPolicy::MRAD_Prom(
      list<MTreeEntry*> &entries,
      MTreeEntry *prom1, MTreeEntry *prom2 )
{
  // TODO not implemented yet
}


/*
2.2.3 Method ~MMRADProm~

*/
void SplitPolicy::MMRAD_Prom(
      list<MTreeEntry*> &entries,
      MTreeEntry *prom1, MTreeEntry *prom2 )
{
  // TODO not implemented yet
}

/*
2.2.4 Method ~MLBProm~

*/
void SplitPolicy::MLB_Prom( 
      list<MTreeEntry*> &entries,
      MTreeEntry *prom1, MTreeEntry *prom2 )
{
  // TODO not implemented yet
}

/*
2.3 Partition functions

2.3.1 Method ~HyperplanePart~

*/
void SplitPolicy::HyperplanePart(
      list<MTreeEntry*> &entries,
      MTreeEntry *prom1, MTreeEntry *prom2,
      list<MTreeEntry*> &entries1, double &rad1,
      list<MTreeEntry*> &entries2, double &rad2,
      DistanceFunction *distFun )
{
  entries1.push_back( prom1 );
  entries2.push_back( prom2 );

  list<MTreeEntry*>::iterator iter = entries.begin();
  while (iter != entries.end())
  {
    double dist1 = distFun->Distance( *iter,prom1 );
    double dist2 = distFun->Distance( *iter,prom2 );
    if ( dist1 < dist2 )
      entries1.push_back( *iter );
    else 
      entries2.push_back( *iter );
  }
}

/*
2.3.2 Method ~BalancedPart~

*/
void SplitPolicy::BalancedPart(
      list<MTreeEntry*> &entries,
      MTreeEntry *prom1, MTreeEntry *prom2,
      list<MTreeEntry*> &entries1, double &rad1,
      list<MTreeEntry*> &entries2, double &rad2,
      DistanceFunction *distFun )
{
  // TODO not implemented yet
}

/*
3 Struct ~MTreeLeafEntry~

3.1 Method ~Read~

*/
void MTreeLeafEntry::Read(char *buffer, int &offset )
{
  memcpy( &dist, buffer+offset, sizeof(double) );
  offset += sizeof(double);

  memcpy( &tupleId, buffer+offset, sizeof(TupleId) );
  offset += sizeof(TupleId);

  data->Read( buffer, offset );
};

/*
3.2 Method ~Write~

*/
void MTreeLeafEntry::Write( char *buffer, int &offset )
{
  memcpy( buffer+offset, &dist, sizeof(double) );
  offset += sizeof(double);

  memcpy( buffer+offset, &tupleId, sizeof(TupleId) );
  offset += sizeof(TupleId);

  data->Write( buffer, offset );
}

/*
4 Struct ~MTreeRoutingEntry~

4.1 Method ~Read~

*/
void MTreeRoutingEntry::Read( char *buffer, int &offset )
{
  memcpy( &dist, buffer+offset, sizeof(double) );
  offset += sizeof(double);

  memcpy( &r, buffer+offset, sizeof(double) );
  offset += sizeof(double);

  memcpy( &chield, buffer+offset, sizeof(SmiRecordId) );
  offset += sizeof(SmiRecordId);

  data->Read( buffer, offset );
}

/*
4.2 Method ~Write~

*/
void MTreeRoutingEntry::Write( char *buffer, int &offset )
{
  memcpy( buffer+offset, &dist, sizeof(double) );
  offset += sizeof(double);

  memcpy( buffer+offset, &r, sizeof(double) );
  offset += sizeof(double);

  memcpy( buffer+offset, &chield, sizeof(SmiRecordId) );
  offset += sizeof(SmiRecordId);

  data->Write( buffer, offset );
}
