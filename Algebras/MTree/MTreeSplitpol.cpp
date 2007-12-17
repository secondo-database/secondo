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

//[_] [\_]

November/December 2007, Mirko Dibbert

5.6 Implementation of class "MT::Splitpol"[4] (file: MTreeSplitpol.cpp)

*/
#include "MTreeSplitpol.h"

/*
Constructor :

*/
MT::Splitpol::Splitpol( PROMOTE promId, PARTITION partId,
                        TMetric metric )
: m_distances( 0 )
{
  m_metric = metric;
  srand( time(0) ); // needed for Rand_Prom

  // init promote function
  switch ( promId )
  {
    case RANDOM:
      promFun = &Splitpol::Rand_Prom;
      break;

    case m_RAD:
      promFun = &Splitpol::MRad_Prom;
      break;

    case mM_RAD:
      promFun = &Splitpol::MMRad_Prom;
      break;

    case M_LB_DIST:
      promFun = &Splitpol::MLB_Prom;
      break;
  }

  // init partition function
  switch ( partId )
  {
    case GENERALIZED_HYPERPLANE:
      partFun = &Splitpol::Hyperplane_Part;
      break;

    case BALANCED:
      partFun = &Splitpol::Balanced_Part;
      break;
  }
}

/*
Method ~RandProm~ :

*/
void MT::Splitpol::Rand_Prom()
{
  #ifdef __MTREE_DEBUG
  assert ( m_entries->size() >= 2 );
  #endif

  unsigned pos1 = rand() % m_entries->size();
  unsigned pos2 = rand() % m_entries->size();
  if (pos1 == pos2)
  {
    if ( pos1 == 0 )
      pos1++;
    else
      pos1--;
  }

  m_promLId = pos1;
  m_promRId = pos2;
}

/*
Method ~MRad[_]Prom~ :

*/
void MT::Splitpol::MRad_Prom()
{
  // precompute distances
  m_distances = new double[m_entries->size() * m_entries->size()];
  for (unsigned i=0; i< m_entries->size(); i++)
    m_distances[i*m_entries->size() + i] = 0;

  for (unsigned i=0; i < (m_entries->size()-1); i++)
    for (unsigned j=(i+1); j < m_entries->size(); j++)
    {
      double dist;
      (*m_metric)((*m_entries)[ i ]->data(),
                  (*m_entries)[ j ]->data(), dist);
      m_distances[i*m_entries->size() + j] = dist;
      m_distances[j*m_entries->size() + i] = dist;
    }

  bool first = true;
  double minRadSum;
  unsigned bestPromLId = 0;
  unsigned bestPromRId = 1;

  for (unsigned i=0; i < (m_entries->size()-1); i++)
    for (unsigned j=(i+1); j < m_entries->size(); j++)
    {
      // call partition function with promoted elements i and j
      m_promLId = i;
      m_promRId = j;
      (this->*partFun)();

      if ( first )
      {
        minRadSum = ( m_radL + m_radR );
        first = false;
      }
      else
      {
        if ( ( m_radL + m_radR ) < minRadSum )
        {
          minRadSum = ( m_radL + m_radR );
          bestPromLId = i;
          bestPromRId = j;
        }
      }
    }

  m_promLId = bestPromLId;
  m_promRId = bestPromRId;

  // remove array of precomputed distances
  delete[] m_distances;
  m_distances = 0;
}

/*
Method ~MMRadProm~ :

*/
void MT::Splitpol::MMRad_Prom()
{
  // precompute distances
  m_distances = new double[m_entries->size() * m_entries->size()];
  for (unsigned i=0; i< m_entries->size(); i++)
    m_distances[i*m_entries->size() + i] = 0;

  for (unsigned i=0; i < (m_entries->size()-1); i++)
    for (unsigned j=(i+1); j < m_entries->size(); j++)
    {
      double dist;
      (*m_metric)((*m_entries)[ i ]->data(),
                  (*m_entries)[ j ]->data(), dist);
      m_distances[i*m_entries->size() + j] = dist;
      m_distances[j*m_entries->size() + i] = dist;
    }

  bool first = true;
  double minMaxRad;
  unsigned bestPromLId = 0;
  unsigned bestPromRId = 1;

  for (unsigned i=0; i < (m_entries->size()-1); i++)
    for (unsigned j=(i+1); j < m_entries->size(); j++)
    {
      // call partition function with promoted elements i and j
      m_promLId = i;
      m_promRId = j;
      (this->*partFun)();

      if ( first )
      {
        minMaxRad = max( m_radL, m_radR );
        first = false;
      }
      else
      {
        if ( max( m_radL, m_radR ) < minMaxRad )
        {
          minMaxRad = max( m_radL, m_radR );
          bestPromLId = i;
          bestPromRId = j;
        }
      }
    }

  m_promLId = bestPromLId;
  m_promRId = bestPromRId;

  // remove array of precomputed distances
  delete[] m_distances;
  m_distances = 0;
}

/*
Method ~MLBProm~ :

*/
void MT::Splitpol::MLB_Prom()
{
  m_promLId = 0;
  m_promRId = 1;
  double maxDistToParent = (*m_entries)[ 1 ]->dist();
  for (unsigned i=2; i < m_entries->size(); i++ )
  {
    double dist = (*m_entries)[ i ]->dist();
    if ( dist > maxDistToParent )
    {
      maxDistToParent = dist;
      m_promRId = i;
    }
  }
}

/*
Method ~HyperplanePart~ :

*/
void MT::Splitpol::Hyperplane_Part()
{
  m_entriesL->clear();
  m_entriesR->clear();

  m_entriesL->push_back( (*m_entries)[ m_promLId ] );
  m_entriesR->push_back( (*m_entries)[ m_promRId ] );

  double distL, distR;
  (*m_entries)[ m_promLId ]->setDist( 0 );
  (*m_entries)[ m_promRId ]->setDist( 0 );

  if ( m_isLeaf )
  {
    m_radL = 0;
    m_radR = 0;
  }
  else
  {
    m_radL = (*m_entries)[ m_promLId ]->rad();
    m_radR = m_radR, (*m_entries)[ m_promRId ]->rad();
  }

  for ( size_t i=0; i < m_entries->size(); i++ )
  {
    if ( (i != m_promLId) && (i != m_promRId) )
    {
      // determine distances to promoted elements
      if ( m_distances )
      {
          unsigned distArrOffset = i * m_entries->size();
          distL = m_distances[distArrOffset + m_promLId];
          distR = m_distances[distArrOffset + m_promRId];
      }
      else
      {
        (*m_metric)( (*m_entries)[ i ]->data(),
                     (*m_entries)[ m_promLId ]->data(), distL );

        (*m_metric)( (*m_entries)[ i ]->data(),
                     (*m_entries)[ m_promRId ]->data(), distR );
      }

      /* push entry i to list with nearest promoted entry and update
         distance to parent and covering radius */
      if ( distL < distR )
      {
        if ( m_isLeaf )
          m_radL = max( m_radL, distL );
        else
          m_radL = max( m_radL, distL + (*m_entries)[ i ]->rad() );

        m_entriesL->push_back( (*m_entries)[i] );
        m_entriesL->back()->setDist( distL );
      }
      else
      {
        if ( m_isLeaf )
          m_radR = max( m_radR, distR );
        else
          m_radR = max( m_radR, distR + (*m_entries)[ i ]->rad() );

        m_entriesR->push_back( (*m_entries)[i] );
        m_entriesR->back()->setDist( distR );
      }
    }
  }
}

/*
Method ~BalancedPart~ :

*/
void MT::Splitpol::Balanced_Part()
{
  m_entriesL->clear();
  m_entriesR->clear();

  m_entriesL->push_back( (*m_entries)[ m_promLId ] );
  m_entriesR->push_back( (*m_entries)[ m_promRId ] );

  m_radL = 0;
  m_radR = 0;

  (*m_entries)[ m_promLId ]->setDist( 0 );
  (*m_entries)[ m_promRId ]->setDist( 0 );

  if ( !m_isLeaf )
    m_radL = max( m_radL, (*m_entries)[ m_promLId ]->rad() );

  if ( !m_isLeaf )
    m_radR = max( m_radR, (*m_entries)[ m_promRId ]->rad() );

  /* copy m_entries into entries (the list contains the entries
     together with its distances to the promoted elements */
  list<BalancedPromEntry> entries;
  for ( size_t i=0; i < m_entries->size(); i++ )
  {
    if ( (i != m_promLId) && (i != m_promRId) )
    {
      double distL;
      double distR;

      if ( m_distances )
      {
          unsigned distArrOffset = i * m_entries->size();
          distL = m_distances[distArrOffset + m_promLId];
          distR = m_distances[distArrOffset + m_promRId];
      }
      else
      {
        (*m_metric)( ((*m_entries)[ i ])->data(),
                     ((*m_entries)[ m_promLId ])->data(), distL );
        (*m_metric)( ((*m_entries)[ i ])->data(),
                     ((*m_entries)[ m_promRId ])->data(), distR );
      }

      entries.push_back(
          BalancedPromEntry( ((*m_entries)[ i ]), distL, distR));
    }
  }

  /* Alternately assign the nearest neighbour of m_promL resp.
     m_promR to m_entriesL resp. m_entriesR and remove it from
     entries. */
  bool assignLeft = true;
  while ( !entries.empty() )
  {
    if ( assignLeft )
    {
      list<BalancedPromEntry>::iterator nearestPos = entries.begin();
      list<BalancedPromEntry>::iterator iter = entries.begin();
      while ( iter  != entries.end() )
      {
        if ( (*iter).distToL < (*nearestPos).distToL )
        {
          nearestPos = iter;
        }
        iter++;
      }

      double distL = (*nearestPos).distToL;
      if ( m_isLeaf )
        m_radL = max( m_radL, distL );
      else
        m_radL = max( m_radL, distL + (*nearestPos).entry->rad() );

      m_entriesL->push_back( (*nearestPos).entry );
      m_entriesL->back()->setDist( distL );
      entries.erase ( nearestPos );
    }
    else
    {
      list<BalancedPromEntry>::iterator nearestPos = entries.begin();
      list<BalancedPromEntry>::iterator iter = entries.begin();
      while ( iter  != entries.end() )
      {
        if ( (*iter).distToL < (*nearestPos).distToR )
        {
          nearestPos = iter;
        }
        iter++;
      }
      double distR = (*nearestPos).distToR;
      if ( m_isLeaf )
        m_radR = max( m_radR, distR );
      else
        m_radR = max( m_radR, distR + (*nearestPos).entry->rad() );

      m_entriesR->push_back( (*nearestPos).entry );
      m_entriesR->back()->setDist( distR );

      entries.erase ( nearestPos );
    }
      assignLeft = !assignLeft;
  }
}
