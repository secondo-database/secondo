/*
//[_] [\_]

\newpage

2.6.3 Implementation part (file: MTSplitpol.cpp)

*/
#include "MTSplitpol.h"

/*
Constructor :

*/
MT::Splitpol::Splitpol( PROMOTE promId, PARTITION partId,
                        TMetric metric )
{
  m_metric = metric;
  srand( time(0) ); // needed for Rand_Prom
  unsigned maxEntries =
      ((NODE_PAGESIZE - Node::emptySize()) / Entry::minSize()) + 1;
  m_distances.reserve( maxEntries );
  for (unsigned i=0; i<maxEntries; i++)
  {
    m_distances.push_back( vector<double>() );
    m_distances.back().reserve( maxEntries - i );
  }
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
//   assert ( m_entries->size() >= 2 );
  unsigned pos1 = rand() % m_entries->size();
  unsigned pos2 = rand() % m_entries->size();
  if (pos1 == pos2)
  {
    if ( pos1 == 0 )
      pos1++;
    else
      pos1--;
  }

  if ( pos1 > pos2 )
    std::swap( pos1, pos2 );

  m_promLId = pos1;
  m_promRId = pos2;
}

/*
Method ~MRad[_]Prom~ :

*/
void MT::Splitpol::MRad_Prom()
{
  bool first = true;
  unsigned bestProm1 = 0;
  unsigned bestProm2 = 1;
  double minRadSum;

  vector<Entry*>::iterator prom1Iter;
  vector<Entry*>::iterator prom2Iter;
  vector<Entry*>::iterator last = (m_entries->end())--;

  unsigned i = 0;
  prom1Iter = m_entries->begin();
  while ( prom1Iter != last )
  {
    unsigned j = i + 1;
    prom2Iter = prom1Iter;
    prom2Iter++;
    m_distances[ i ].clear();
    while ( prom2Iter != m_entries->end() )
    {
      double dist;
      (*m_metric)
          ((*m_entries)[ i ]->data(),
           (*m_entries)[ j ]->data(), dist);
      m_distances[ i ].push_back( dist );

      j++;
      prom2Iter++;
    }

    i++;
    prom1Iter++;
  }
  m_distancesDefined = true;

  i = 0;
  prom1Iter = m_entries->begin();
  while ( prom1Iter != last )
  {
    unsigned j = i + 1;
    prom2Iter = prom1Iter;
    prom2Iter++;
    while ( prom2Iter != m_entries->end() )
    {
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
          bestProm1 = i;
          bestProm2 = j;
        }
      }

      j++;
      prom2Iter++;
    }

    i++;
    prom1Iter++;
  }
  m_promLId = bestProm1;
  m_promRId = bestProm2;
}

/*
Method ~MMRadProm~ :

*/
void MT::Splitpol::MMRad_Prom()
{
  bool first = true;
  unsigned bestProm1 = 0;
  unsigned bestProm2 = 1;
  double minMaxRad;

  vector<Entry*>::iterator prom1Iter;
  vector<Entry*>::iterator prom2Iter;
  vector<Entry*>::iterator last = (m_entries->end())--;

  unsigned i = 0;
  prom1Iter = m_entries->begin();
  while ( prom1Iter != last )
  {
    unsigned j = i + 1;
    prom2Iter = prom1Iter;
    prom2Iter++;
    m_distances[ i ].clear();
    while ( prom2Iter != m_entries->end() )
    {
      double dist;
      (*m_metric) (
          (*m_entries)[ i ]->data(),
          (*m_entries)[ j ]->data(), dist );
      m_distances[ i ].push_back( dist );

      j++;
      prom2Iter++;
    }

    i++;
    prom1Iter++;
  }
  m_distancesDefined = true;

  i = 0;
  prom1Iter = m_entries->begin();
  while ( prom1Iter != last )
  {
    unsigned j = i + 1;
    prom2Iter = prom1Iter;
    prom2Iter++;
    while ( prom2Iter != m_entries->end() )
    {
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
          bestProm1 = i;
          bestProm2 = j;
        }
      }

      j++;
      prom2Iter++;
    }

    i++;
    prom1Iter++;
  }
  m_promLId = bestProm1;
  m_promRId = bestProm2;
}

/*
Method ~MLBProm~ :

*/
void MT::Splitpol::MLB_Prom()
{
  // TODO : not yet implemented
  assert( false );
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

  m_radL = 0;
  m_radR = 0;

  double distL, distR;
  (*m_entries)[ m_promLId ]->setDist( 0 );
  (*m_entries)[ m_promRId ]->setDist( 0 );

  if ( !m_isLeaf )
    m_radL = max( m_radL, (*m_entries)[ m_promLId ]->rad() );

  if ( !m_isLeaf )
    m_radR = max( m_radR, (*m_entries)[ m_promRId ]->rad() );

  for ( size_t i=0; i<m_entries->size(); i++ )
  {
    if ( (i != m_promLId) && (i != m_promRId) )
    {
      // TODO wenn vorhanden, zuvor berechnete Distanzen nutzen!
      (*m_metric)( (*m_entries)[ i ]->data(),
                         (*m_entries)[ m_promLId ]->data(), distL );

      (*m_metric)( (*m_entries)[ i ]->data(),
                         (*m_entries)[ m_promRId ]->data(), distR );

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

  list<BalancedPromEntry> entries;
  for ( size_t i=0; i<m_entries->size(); i++ )
  {
    if ( (i != m_promLId) && (i != m_promRId) )
    {
      double distL;
      double distR;

      if ( m_distancesDefined )
      {
        if ( i < m_promLId )
          distL = m_distances[ i ][ m_promLId-(i+1) ];
        else
          distL = m_distances[ m_promLId ][ i-(m_promLId+1) ];

        if ( i < m_promRId )
          distR = m_distances[ i ][ m_promRId-(i+1) ];
        else
          distR = m_distances[ m_promRId ][ i-(m_promRId+1) ];

      }
      else
      {
        (*m_metric)( ((*m_entries)[ i ])->data(),
                     ((*m_entries)[ m_promLId ])->data(), distL );
        (*m_metric)( ((*m_entries)[ i ])->data(),
                     ((*m_entries)[ m_promRId ])->data(), distR );
      }
      entries.push_back(
          BalancedPromEntry(
              ((*m_entries)[ i ]), distL, distR));
//         (*m_metric)( (*m_entries)[ i ]->data(),
//                           (*m_entries)[ m_promLId ], distL );
//         (*m_metric)( (*m_entries)[ i ]->data(),
//                           (*m_entries)[ m_promRId ], distR );
//         assert ( distL == entries.back().distToL );
//         assert ( distR == entries.back().distToR );
    }
  }

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
