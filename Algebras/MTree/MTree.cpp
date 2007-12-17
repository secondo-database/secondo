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
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

November/December 2007, Mirko Dibbert

5.2 Implementation of class "MTree"[4] (file: MTree.cpp)

*/
#include <stack>
#include "MTree.h"

/*
Constructor (new m-tree):

*/
MT::MTree::MTree()
: initialized( false ), file( true, NODE_PAGESIZE ), header(),
  splitpol( 0 ), nodeMngr( 0 )
{
  file.Create();

  // create header nodeId
  SmiRecordId headerId;
  SmiRecord headerRecord;
  file.AppendRecord( headerId, headerRecord );
  assert( headerId == 1 );
}
/*
Constructor (load m-tree):

*/
MT::MTree::MTree( const SmiFileId fileid )
: initialized( false ), file( true ), header(),
  splitpol( 0 ), nodeMngr( 0 )
{
  assert(file.Open( fileid ));
  readHeader();

  // init metric function
  metric = MetricRegistry::getMetric
      ( header.tcName, header.metricName );

  // init getDistData function
  getDistData = MetricRegistry::getDataFun
      ( header.tcName, header.metricName );

  // init MTreeConfig object
  config = MTreeConfigReg::getMTreeConfig ( header.configName );

  // init node manager
  nodeMngr = new NodeMngr( &file, config.maxNodeEntries );

  // init split policy
  splitpol = new
      Splitpol( config.promoteFun, config.partitionFun, metric );

  initialized = true;
}

/*
Copy constructor

*/
MT::MTree::MTree( const MTree& mtree )
: file( true, NODE_PAGESIZE ), splitpol( 0 ), nodeMngr( 0 )
{
   file.Create();

  // create header nodeId
  SmiRecordId headerId;
  SmiRecord headerRecord;
  file.AppendRecord( headerId, headerRecord );
  assert( headerId == 1 );

  // copy header
  strcpy(header.tcName, mtree.header.tcName);
  strcpy(header.metricName, mtree.header.metricName);
  strcpy(header.configName, mtree.header.configName);
  header.height = mtree.header.height;
  header.entryCount = mtree.header.entryCount;
  header.routingCount = mtree.header.routingCount;
  header.leafCount = mtree.header.leafCount;

  initialized = mtree.initialized;

  if ( initialized )
  {
    // init metric function
    metric = MetricRegistry::getMetric
        ( header.tcName, header.metricName );

    // init getDistData function
    getDistData = MetricRegistry::getDataFun
        ( header.tcName, header.metricName );

    // init MTreeConfig object
    config = MTreeConfigReg::getMTreeConfig ( header.configName );

    // init node manager
    nodeMngr = new NodeMngr( &file, config.maxNodeEntries );

    // init split policy
    splitpol = new
        Splitpol( config.promoteFun, config.partitionFun, metric );

    // copy tree structure
    stack< pair<SmiRecordId,SmiRecordId> > remaining;
    Node* sourceNode;
    Node* targetNode;

    sourceNode = new Node( const_cast<SmiRecordFile*>(&mtree.file),
        config.maxNodeEntries, mtree.header.root );
    targetNode = new Node( &file, config.maxNodeEntries );
    header.root = targetNode->getNodeId();

    vector<Entry*>* sourceEntries = sourceNode->getEntries();
    vector<Entry*>* targetEntries = targetNode->getEntries();

    vector<Entry*>::iterator iter;
    for ( iter = sourceEntries->begin();
          iter != sourceEntries->end();
          iter++)
    {
      targetEntries->push_back( new Entry( **iter ) );
      if ( (*iter)->chield() )
      {
        Node* newChield = new Node( &file, config.maxNodeEntries );
        targetEntries->back()->setChield( newChield->getNodeId() );
        remaining.push( pair< SmiRecordId, SmiRecordId >
            ( (*iter)->chield(), newChield->getNodeId() ) );
        newChield->update( SIZE_CHANGED );
        newChield->deleteIfAllowed();
      }
    }

    targetNode->update( SIZE_CHANGED );

    while ( !remaining.empty() )
    {
      sourceNode->read( remaining.top().first );
      targetNode->read( remaining.top().second );
      remaining.pop();

      vector<Entry*>* sourceEntries = sourceNode->getEntries();
      vector<Entry*>* targetEntries = targetNode->getEntries();

      vector<Entry*>::iterator iter;
      for ( iter = sourceEntries->begin();
            iter != sourceEntries->end();
            iter++)
      {
        targetEntries->push_back( new Entry( **iter ) );
        if ( (*iter)->chield() )
        {
          Node* newChield = new Node( &file, config.maxNodeEntries );
          targetEntries->back()->setChield( newChield->getNodeId() );
          remaining.push( pair< SmiRecordId, SmiRecordId >
              ( (*iter)->chield(), newChield->getNodeId() ) );
          newChield->deleteIfAllowed();
        }
      } // for
      targetNode->update( SIZE_CHANGED );
    } // while

    sourceNode->deleteIfAllowed();
    targetNode->deleteIfAllowed();
  } // if initialized
}

/*
Destructor:

*/
MT::MTree::~MTree()
{
  if ( file.IsOpen() )
  {
    writeHeader();
    file.Close();
  }

  if ( splitpol )
    delete splitpol;

  if ( nodeMngr )
    delete nodeMngr;

#ifdef __MT_DEBUG
  if ( Node::objectsOpen() )
  {
    cmsg.warning() << "*** Memory leak warning: "
                   << Node::objectsOpen()
                   << " <MT::Node> object(s) left open!" << endl;
    cmsg.send();
  }

  if ( Entry::objectsOpen() )
  {
    cmsg.warning() << "*** Memory leak warning: "
                   << Entry::objectsOpen()
                   << " <MT::Entry> object(s) left open!" << endl;
    cmsg.send();
  }

  #ifdef __DISTDATA_DEBUG
  if ( DistData::objectsOpen() )
  {
    cmsg.warning() << "*** Memory leak warning: "
                   << DistData::objectsOpen()
                   << " <DistData> object(s) left open!" << endl;
    cmsg.send();
  }
  #endif
#endif
}

/*
Method ~deleteFile~

*/
void
MT::MTree::deleteFile()
{
  if ( file.IsOpen() )
    file.Close();

  file.Drop();
}

/*
Method ~writeHeader~ :

*/
void
MT::MTree::writeHeader()
{
  SmiRecord record;
  file.SelectRecord( (SmiRecordId)1, record, SmiFile::Update );

  char buffer[record.Size()];
  int offset = 0;

  memcpy( buffer+offset, &(header.tcName), (MAX_STRINGSIZE+1) );
  offset += (MAX_STRINGSIZE+1);

  memcpy( buffer+offset, &(header.metricName), (MAX_STRINGSIZE+1) );
  offset += (MAX_STRINGSIZE+1);

  memcpy( buffer+offset, (&header.configName), (MAX_STRINGSIZE+1) );
  offset += (MAX_STRINGSIZE+1);

  memcpy( buffer+offset, (&header.root), sizeof(SmiRecordId) );
  offset += sizeof(SmiRecordId);

  memcpy( buffer+offset, (&header.height), sizeof(unsigned) );
  offset += sizeof(unsigned);

  memcpy( buffer+offset, (&header.entryCount), sizeof(unsigned) );
  offset += sizeof(unsigned);

  memcpy( buffer+offset, (&header.routingCount), sizeof(unsigned) );
  offset += sizeof(unsigned);

  memcpy( buffer+offset, (&header.leafCount), sizeof(unsigned) );
  offset += sizeof(unsigned);

  record.Write( buffer, offset, 0 );
}

/*
Method ~readHeader~ :

*/
void MT::MTree::readHeader()
{
  SmiRecord record;
  file.SelectRecord( (SmiRecordId)1, record, SmiFile::ReadOnly );

  char buffer[record.Size()];
  int offset = 0;

  record.Read( buffer, record.Size(), 0 );

  memcpy( (&header.tcName), buffer+offset, (MAX_STRINGSIZE+1) );
  offset += (MAX_STRINGSIZE+1);

  memcpy( (&header.metricName), buffer+offset, (MAX_STRINGSIZE+1) );
  offset += (MAX_STRINGSIZE+1);

  memcpy( (&header.configName), buffer+offset, (MAX_STRINGSIZE+1) );
  offset += (MAX_STRINGSIZE+1);

  memcpy( (&header.root), buffer+offset, sizeof(SmiRecordId) );
  offset += sizeof(SmiRecordId);

  memcpy( (&header.height), buffer+offset, sizeof(unsigned) );
  offset += sizeof(unsigned);

  memcpy( (&header.entryCount), buffer+offset, sizeof(unsigned) );
  offset += sizeof(unsigned);

  memcpy( (&header.routingCount), buffer+offset, sizeof(unsigned) );
  offset += sizeof(unsigned);

  memcpy( (&header.leafCount), buffer+offset, sizeof(unsigned) );
  offset += sizeof(unsigned);
}

/*
Method ~initialize~ :

*/
void MT::MTree::initialize( const string& tcName,
                            const string& metricName,
                            const string& configName )
{
  if ( initialized )
    return;

  // init metric function
  metric = MetricRegistry::getMetric ( tcName, metricName );

  // init getDistData function
  getDistData = MetricRegistry::getDataFun
      ( tcName, metricName );

  // init MTreeConfig object
  config = MTreeConfigReg::getMTreeConfig( configName );

  // init node manager
  nodeMngr = new NodeMngr( &file, config.maxNodeEntries );

  // init split policy
  splitpol = new
      Splitpol( config.promoteFun, config.partitionFun, metric );

  //create root node
  Node* root = nodeMngr->createNode();
  header.leafCount++;
  header.height++;

  // update header
  strcpy( header.tcName, tcName.c_str() );
  strcpy( header.metricName, metricName.c_str() );
  strcpy( header.configName, configName.c_str() );
  header.root = root->getNodeId();

  root->deleteIfAllowed();

  initialized = true;
}

/*
Method ~printMTreeConfig~ :

*/
void
MT::MTree::printMTreeConfig()
{
  string promFunStr, partFunStr;
  switch ( config.promoteFun )
  {
    case RANDOM:
      promFunStr = "random";
      break;
    case m_RAD:
      promFunStr = "minmal sum of covering radii";
      break;
    case mM_RAD:
      promFunStr = "minimal maximum of covering radii";
      break;
    case M_LB_DIST:
      promFunStr = "maximum lower bound on distance";
      break;
    default:
      promFunStr = "unknown";
      break;
  }

  switch ( config.partitionFun )
  {
    case GENERALIZED_HYPERPLANE:
      partFunStr = "generalized hyperplane";
      break;
    case BALANCED:
      partFunStr = "balanced";
      break;
    default:
      partFunStr = "unknown";
      break;
  }

  string configName( header.configName );
  string metricName( header.metricName );
  cmsg.info() << endl
      << "using metric:       \"" << metricName << "\"" << endl
      << "using mtree-config: \"" << configName << "\"" << endl
      << "----------------------------------------" << endl
      << "max entries per node: " << config.maxNodeEntries << endl
      << "promote function: " << promFunStr << endl
      << "partition function: " << partFunStr << endl
      << endl;
  cmsg.send();
}

/*
Method ~split~ :

*/
void
MT::MTree::split( Entry* entry )
{
  bool done = false;
  while ( !done )
  {
    bool isLeaf = ( !nodeMngr->hasChield() );

    // create new node
    Node* newNode = nodeMngr->createNode();
    if ( isLeaf )
      header.leafCount++;
    else
      header.routingCount++;

    // get current entries, store new entry vector to node
    // (will be filled in splitpol->apply)
    vector<Entry*>* entries = new vector<Entry*>();
    nodeMngr->curNode()->swapEntries( entries );
    entries->push_back( entry );

    /* apply splitpol: this will split the entries given in the first
       vector to the second and third vector by using the promote and
       partition function defined in the current MTreeConfig object.
    */
    splitpol->apply( entries,
                     nodeMngr->curNode()->getEntries(),
                     newNode->getEntries(),
                     isLeaf );
    delete entries;

    #ifdef __MT_PRINT_SPLIT_INFO
    cmsg.info() << "\nsplit: splitted nodes contain "
                << nodeMngr->curNode()->getEntryCount() << " / "
                << newNode->getEntryCount() << " entries." << endl;
    cmsg.send();
    #endif

    // set modified flag to true and recompute node size
    nodeMngr->update( nodeMngr->curNode(), SIZE_CHANGED );
    nodeMngr->update( newNode, SIZE_CHANGED );

    // retrieve promote entries
    Entry* promL = splitpol->getPromL();
    Entry* promR = splitpol->getPromR();

    // update chield pointers
    promL->setChield( nodeMngr->curNode()->getNodeId() );
    promR->setChield( newNode->getNodeId() );

    newNode->deleteIfAllowed();

    // insert new root
    if ( !nodeMngr->hasParent() )
    {
      header.routingCount++;
      Node* newRoot = nodeMngr->createNode();
      nodeMngr->insert( newRoot, promL );
      nodeMngr->insert( newRoot, promR );
      header.root = newRoot->getNodeId();
      header.height++;
      newRoot->deleteIfAllowed();
      done = true;
    }
    // insert promoted entries into routing nodes
    else
    {
      nodeMngr->getParent();

      if ( nodeMngr->hasParent() )
      {
        // update dist from distL/distR to parent of distL/distR
        double distL, distR;
        Entry* parentEntry = nodeMngr->parentEntry();
        (*metric)( promL->data(), parentEntry->data(), distL );
        (*metric)( promR->data(), parentEntry->data(), distR );
        promL->setDist( distL );
        promR->setDist( distR );
      }

      // replace old promoted entry with promL
      nodeMngr->replacePromotedEntry( promL );

      // insert promR
      if (!nodeMngr->insert( nodeMngr->curNode(), promR ))
        entry = promR;
      else
        done = true;
    }
  } // while
}

/*
Method ~insert~ :

*/
void
MT::MTree::insert( Attribute* attr, TupleId tupleId )
{
  #ifdef __MT_DEBUG
  assert( initialized );
  #endif

  // create new entry
  Entry* entry;
  try
  {
    entry = new Entry( tupleId, (*getDistData)(attr) );
  }
  catch ( bad_alloc& )
  {
    cmsg.warning() << "Not enough memory to create new entry, "
                   << "disabling node cache... "
                   << endl;
    cmsg.send();
    nodeMngr->flush();
    nodeMngr->disableCache();
    try
    {
      entry = new Entry( tupleId, (*getDistData)(attr) );
    }
    catch (bad_alloc&)
    {
      cmsg.error() << "Not enough memory to create new entry!"
                   << endl;
      cmsg.send();
    }
  }

  nodeMngr->initPath( header.root );

  // descent tree until leaf level
  while ( nodeMngr->hasChield() )
  { /* find best path (follow the entry with the nearest dist to
        new entry or the smallest covering radius increase) */
      list<SearchBestPathEntry> entriesIn;
      list<SearchBestPathEntry> entriesOut;

      vector<Entry*>* entries = nodeMngr->curNode()->getEntries();
      vector<Entry*>::iterator iter;

      unsigned index = 0;
      for ( iter = entries->begin();
            iter != entries->end();
            iter++, index++ )
      {
        double dist;
        (*metric)( (*iter)->data(), entry->data(), dist );
        if ( dist <= (*iter)->rad() )
        {
          entriesIn.push_back(
              SearchBestPathEntry( *iter, dist, index ) );
        }
        else
        {
          entriesOut.push_back(
              SearchBestPathEntry( *iter, dist, index ) );
        }
      }

      list<SearchBestPathEntry>::iterator best;

      if ( !entriesIn.empty() )
      { // select entry with nearest dist to new entry
        // (covering radius must not be increased)
        best = entriesIn.begin();
        list<SearchBestPathEntry>::iterator iter;
        for ( iter = entriesIn.begin();
              iter != entriesIn.end();
              iter++ )
        {
          if ( (*iter).dist < (*best).dist )
          {
            best = iter;
          }
        }
      }
      else
      { // select entry with minimal radius increase
        double dist;
        (*metric)( entriesOut.front().entry->data(),
                   entry->data(), dist );
        double minIncrease = dist - entriesOut.front().entry->rad();
        double minDist = dist;

        best = entriesOut.begin();
        list<SearchBestPathEntry>::iterator iter;
        for ( iter = entriesIn.begin();
              iter != entriesIn.end();
              iter++ )
        {
          (*metric)( (*iter).entry->data(), entry->data(), dist );
          double increase = dist - (*iter).entry->rad();
          if ( increase < minIncrease )
          {
            minIncrease = increase;
            best = iter;
            minDist = dist;
          }
        }

        // update increased covering radius
        (*best).entry->setRad( minDist );
        nodeMngr->update( nodeMngr->curNode(), !SIZE_CHANGED );
      }

      nodeMngr->getChield( (*best).index );
  }

  //   compute distance from entry to parent node, if exist
  if ( nodeMngr->hasParent() )
  {
    double dist;
    (*metric)( entry->data(),
               nodeMngr->parentEntry()->data(), dist );
    entry->setDist( dist );
  }

  // insert entry into leaf, split if neccesary
  if ( !nodeMngr->insert( nodeMngr->curNode(), entry ) )
  {
    split( entry );
  }

  header.entryCount++;

  #ifdef __MT_PRINT_INSERT_INFO
  if ((header.entryCount % 25) == 0)
  {
    cmsg.info() << "nodes: "
                << header.routingCount + header.leafCount
                << "\tentries: " << header.entryCount
                << "\tcache used: "
                << nodeMngr->usedCacheSize()/1024 << " kb" << endl;
    cmsg.send();
  }
  #endif
}

/*
Method ~rangeSearch~ :

*/
void MT::MTree::rangeSearch( Attribute* attr,
                             const double& searchRad,
                             list<TupleId>* results )
{
  #ifdef __MT_DEBUG
  assert( initialized );
  #endif

  results->clear();
  list< pair<double, TupleId> > resultList;
  DistData* data = getDistData( attr );

  stack<RemainingNodesEntry> remainingNodes;
  remainingNodes.push( RemainingNodesEntry(header.root, 0, 0 ) );

  #ifdef __MT_PRINT_SEARCH_INFO
  unsigned entryCount = 0;
  unsigned nodeCount = 0;
  unsigned distComputations = 0;
  #endif

  Node* nodePtr = new Node(
      &file, config.maxNodeEntries, header.root );

  while( !remainingNodes.empty() )
  {
    #ifdef __MT_PRINT_SEARCH_INFO
    nodeCount++;
    #endif

    nodePtr->read( remainingNodes.top().nodeId );
    unsigned char deepth = remainingNodes.top().deepth;
    double distQueryParent = remainingNodes.top().dist;
    remainingNodes.pop();

    if ( deepth < (header.height - 1) )
    { // routing node
      vector<Entry*>* entries = nodePtr->getEntries();
      vector<Entry*>::iterator iter;
      for ( iter = entries->begin(); iter != entries->end(); iter++)
      {
        double dist = (*iter)->dist();
        double radSum = searchRad + (*iter)->rad();
        if ( abs( distQueryParent - dist ) <= radSum )
        {
          #ifdef __MT_PRINT_SEARCH_INFO
          distComputations++;
          #endif

          double newDistQueryParent;
          (*metric)( data, (*iter)->data(), newDistQueryParent );
          if ( newDistQueryParent <= radSum )
          {
            remainingNodes.push( RemainingNodesEntry(
                (*iter)->chield(), deepth+1, newDistQueryParent ) );
          }
        }
      }
    }
    else
    { // leaf
      vector<Entry*>* entries = nodePtr->getEntries();
      vector<Entry*>::iterator iter;
      for ( iter = entries->begin(); iter != entries->end(); iter++)
      {
        double dist = (*iter)->dist();
        if ( abs( distQueryParent - dist ) <= searchRad )
        {
          #ifdef __MT_PRINT_SEARCH_INFO
          entryCount++;
          distComputations++;
          #endif

          double distQueryCurrent;
          (*metric)( data, (*iter)->data(), distQueryCurrent );
          if ( distQueryCurrent <= searchRad )
          {
            resultList.push_back( pair<double, TupleId>(
                distQueryCurrent, (*iter)->tid()) );
          }
        } // if
      } // for
    } // else
  } // while

  nodePtr->deleteIfAllowed();
  data->deleteIfAllowed();

  resultList.sort();
  list< pair<double, TupleId> >::iterator iter = resultList.begin();
  while ( iter != resultList.end() )
  {
    results->push_back( iter->second );
    iter++;
  }

#ifdef __MT_PRINT_SEARCH_INFO
  unsigned maxNodes = header.routingCount + header.leafCount;
  unsigned maxEntries = header.entryCount;
  unsigned maxDistComputations = maxNodes + maxEntries - 1;
  cmsg.info()
      << "Distance computations : " << distComputations << "\t(max "
      << maxDistComputations << ")" << endl
      << "Nodes analyzed        : " << nodeCount << "\t(max "
      << maxNodes << ")" << endl
      << "Entries analyzed      : " << entryCount << "\t(max "
      << maxEntries << ")" << endl << endl;
  cmsg.send();
#endif
}

/*
Method ~rangeSearch~ :

*/
void MT::MTree::nnSearch( Attribute* attr, int nncount,
                          list<TupleId>* results )
{
  #ifdef __MT_DEBUG
  assert( initialized );
  #endif

  // not yet implemented
}
