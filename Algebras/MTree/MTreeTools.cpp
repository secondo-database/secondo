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

1.1 Defines and includes

*/
using namespace std;

#include "MTree.h"
#include <time.h>

/*
1.2 Class ~SplitPolicy~

1.2.1 Constructor

*/
SplitPolicy::SplitPolicy( MetricWrapper* metric,
                          PROMFUN promFunId, PARTFUN partFunId )
{
  this->metric = metric;

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
1.2.2 Method ~RANDProm~

*/
void SplitPolicy::RAND_Prom( list<MTreeEntry*> &entries,
                              MTreeEntry *prom1, MTreeEntry *prom2,
                              MetricWrapper *metric )
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
1.2.3 Method ~MRADProm~

*/
void SplitPolicy::MRAD_Prom( list<MTreeEntry*> &entries,
                              MTreeEntry *prom1, MTreeEntry *prom2,
                              MetricWrapper *metric )
{
  // TODO not yet implemented
  assert( false );
}


/*
1.2.4 Method ~MMRADProm~

*/
void SplitPolicy::MMRAD_Prom( list<MTreeEntry*> &entries,
                              MTreeEntry *prom1, MTreeEntry *prom2,
                              MetricWrapper *metric )
{
  // TODO not yet implemented
  assert( false );
}

/*
1.2.5 Method ~MLBProm~

*/
void SplitPolicy::MLB_Prom( list<MTreeEntry*> &entries,
                            MTreeEntry *prom1, MTreeEntry *prom2,
                            MetricWrapper *metric )
{
  // TODO not yet implemented
  assert( false );
}

/*
1.2.6 Method ~HyperplanePart~

*/
void SplitPolicy::HyperplanePart( list<MTreeEntry*> &entries,
                                  MTreeEntry *prom1, MTreeEntry *prom2,
                                  list<MTreeEntry*> &entries1, double &rad1,
                                  list<MTreeEntry*> &entries2, double &rad2,
                                  MetricWrapper *metric )
{
  entries1.clear();
  entries2.clear();

  entries1.push_back( prom1 );
  entries2.push_back( prom2 );

  list<MTreeEntry*>::iterator iter = entries.begin();
  while (iter != entries.end())
  {
    double dist1, dist2; 
    metric->Distance( *iter,prom1, dist1 );
    metric->Distance( *iter,prom2, dist2 );

    if ( dist1 < dist2 )
      entries1.push_back( *iter );
    else 
      entries2.push_back( *iter );
  }
}

/*
1.2.7 Method ~BalancedPart~

*/
void SplitPolicy::BalancedPart( list<MTreeEntry*> &entries,
                                MTreeEntry *prom1, MTreeEntry *prom2,
                                list<MTreeEntry*> &entries1, double &rad1,
                                list<MTreeEntry*> &entries2, double &rad2,
                                MetricWrapper *metric )
{
  // TODO not yet implemented
  assert( false );
}

/*
1.2 Class ~MetricWrapper~

1.2.1 Method ~DistRef~

*/
  void MetricWrapper::DistRef( const MTreeEntry* e1, const MTreeEntry* e2,
                               double& result, MWSupp* supp )
  {
    TupleId tid1 = ((MTreeData_Ref*)e1->data)->tupleId;
    TupleId tid2 = ((MTreeData_Ref*)e2->data)->tupleId;

    // retrieve obj1 from relation rel
    StandardMetricalAttribute *obj1 = ( StandardMetricalAttribute* )
         ( supp->rel->GetTuple(tid1) )->GetAttribute( supp->attrIndex );

    // retrieve obj2 from relation rel
    StandardMetricalAttribute *obj2 = ( StandardMetricalAttribute* )
         ( supp->rel->GetTuple(tid2) )->GetAttribute( supp->attrIndex );

    // call metric with objects
    supp->metric( obj1, obj2, result );
  }

/*
1.2.2 Method ~DistInf~

*/
  void MetricWrapper::DistInt( const MTreeEntry* e1, const MTreeEntry* e2,
                               double& result, MWSupp* supp )
  {
    // call data strings from internal representation
    char* data1 = ((MTreeData_Int*)e1->data)->datastr;
    char* data2 = ((MTreeData_Int*)e2->data)->datastr;

    // call metric with data strings
    supp->metric( data1, data2, result );
  }

/*
1.2.3 Method ~DistExt~

*/
  void MetricWrapper::DistExt( const MTreeEntry* e1, const MTreeEntry* e2,
                               double& result, MWSupp* supp )
  {
    SmiRecordId rec1 = ((MTreeData_Ext*)e1->data)->dataRec;
    SmiRecordId rec2 = ((MTreeData_Ext*)e2->data)->dataRec;

    int size = supp->file->GetRecordLength();
    SmiRecord record;
    char* data1[size];
    char* data2[size];

    // retrieve data1 from record rec1
    assert( supp->file->SelectRecord( rec1, record, SmiFile::ReadOnly ));
    assert( record.Read( data1, size, 0 ));

    // retrieve data2 from record rec2
    assert( supp->file->SelectRecord( rec2, record, SmiFile::ReadOnly ));
    assert( record.Read( data2, size, 0 ));

    // call metric with data strings
    supp->metric( data1, data2, result );
  }
