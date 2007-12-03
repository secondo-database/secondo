/*
/newpage

2.2.3 Implementation part (file: MTree.cpp)

*/
#include "MTree.h"

/*
Constructor (new m-tree):

*/
MT::MTree::MTree()
: initialized( false ),
  file( true, NODE_PAGESIZE ),
  header(),
  splitpol( 0 ),
  nodeMngr( 0 )
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
: initialized( false ),
  file( true ),
  header(),
  splitpol( 0 ),
  nodeMngr( 0 )
{
  assert(file.Open( fileid ));
  readHeader();

  // get metric function
  metric = MetricRegistry::getMetric
      ( header.tcName, header.metricName );

  // get MTreeConfig object
  config = MTreeConfigReg::getMTreeConfig ( header.configName );

  // initialize node manager
  nodeMngr = new NodeMngr( &file, config.maxNodeEntries );

  // initialize split policy
  splitpol = new
      Splitpol( config.promoteFun, config.partitionFun, metric );

  initialized = true;
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

  delete splitpol;
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
  record.Write( &header, sizeof( Header ), 0 );
}

/*
Method ~readHeader~ :

*/
void MT::MTree::readHeader()
{
  SmiRecord record;
  file.SelectRecord( (SmiRecordId)1, record, SmiFile::ReadOnly );
  record.Read( &header, sizeof( Header ), 0 );
}

/*
Method ~initialize~ :

*/
void MT::MTree::initialize( const Attribute* attr,
                            const string tcName,
                            const string metricName,
                            const string configName )
{
  if ( initialized )
    return;

  // get metric function
  metric = MetricRegistry::getMetric ( tcName, metricName );

  // get MTreeConfig object
  config = MTreeConfigReg::getMTreeConfig( configName );

  // initialize node manager
  nodeMngr = new NodeMngr( &file, config.maxNodeEntries );

  // initialize split policy
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
Method ~getDistData~ :

*/
DistData*
MT::MTree::getDistData( Attribute* attr )
{
  DistData* data;
  string tcName ( header.tcName );
  if ( tcName == "int" )
  {
    int value = static_cast<CcInt*>(attr)->GetValue();
    char buffer[sizeof(int)];
    memcpy( buffer, &value, sizeof(int) );
    data = new DistData( sizeof(int), buffer );
  }
  else if  ( tcName == "real" )
  {
    SEC_STD_REAL value =
        static_cast<CcReal*>(attr)-> GetValue();
    char buffer[sizeof(SEC_STD_REAL)];
    memcpy( buffer, &value, sizeof(SEC_STD_REAL) );
    data = new DistData( sizeof(SEC_STD_REAL), buffer );
  }
  else if  ( tcName == "string" )
  {
    string value = static_cast<CcString*>( attr )-> GetValue();
    data = new DistData( value );
  }
  else
  {
    data = static_cast<MetricalAttribute*>( attr )->
        getDistData( header.metricName );
  }
  return data;
}

/*
Method ~split~ :

*/
void
MT::MTree::split( Entry* entry )
{
  unsigned char deepth = header.height - 1;

  while ( true )
  {
    bool isLeaf = ( deepth == (header.height-1) );

    // create new node
    Node* newNode;
    if ( isLeaf )
    {
      header.leafCount++;
      newNode = nodeMngr->createNode();
    }
    else
    {
      header.routingCount++;
      newNode = nodeMngr->createNode();
    }

    // get current entries, store new entry vector to node
    // (will be filled in splitpol->apply)
    vector<Entry*>* entries = new vector<Entry*>();
    nodePtr->swapEntries( entries );
    entries->push_back( entry );

    /* apply splitpol: this will split the entries given in the first
       vector to the second and third vector by using the promote and
       partition function defined in the current MTreeConfig object.
    */
    splitpol->apply( entries, nodePtr->getEntries(),
                     newNode->getEntries(), isLeaf );
    delete entries;

    #ifdef __MT_PRINT_SPLIT_INFO
    cmsg.info() << "\nsplit: splitted nodes contain "
                << nodePtr->getEntryCount() << " / "
                << newNode->getEntryCount() << " entries." << endl;
    cmsg.send();
    #endif

    // set modified flag to true and recompute node size
    nodePtr->modified( SIZE_CHANGED );
    newNode->modified( SIZE_CHANGED );

    // retrieve promote entries
    Entry* promL = splitpol->getPromL();
    Entry* promR = splitpol->getPromR();

    // update chield pointers
    promL->setChield( nodePtr->getNodeId() );
    promR->setChield( newNode->getNodeId() );

    newNode->deleteIfAllowed();

    // insert new root
    if ( deepth == 0 )
    {
      header.routingCount++;
      Node* newRoot = nodeMngr->createNode();
      newRoot->insert( promL );
      newRoot->insert( promR );
      header.root = newRoot->getNodeId();
      header.height++;
      newRoot->deleteIfAllowed();
      return;
    }
    // insert promoted entries into routing nodes
    else
    {
      deepth--;
      nodePtr->deleteIfAllowed();
      nodePtr = nodeMngr->getNode( path[deepth] );

      // update distances to parent
      if ( deepth > 0)
      {
        double distL, distR;
        Node* parent = nodeMngr->getNode( path[deepth-1] );
        Entry* parentEntry =
            (*parent->getEntries())[indizes[deepth-1]];
        (*metric)( promL->data(), parentEntry->data(), distL );
        (*metric)( promR->data(), parentEntry->data(), distR );
        promL->setDist( distL );
        promR->setDist( distR );
        parent->deleteIfAllowed();
      }

      // replace old promoted entry with promL
      nodePtr->update( indizes[deepth], promL );

      // insert promR
      if (!nodePtr->insert( promR ))
        entry = promR;
      else
        return;
    } // else
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

  #ifdef __MT_PRINT_INSERT_INFO
  if ((header.entryCount % 5000) == 0)
  {
    cmsg.info() << endl
                << "routing nodes: " << header.routingCount
                << "\tleaves: " << header.leafCount
                << "\theight: " << header.height
                << "\tentries: " << header.entryCount << "\t";
    cmsg.send();
  }
  else if ((header.entryCount % 100) == 0)
  {
    cmsg.info() << ".";
    cmsg.send();
  }
  #endif

  unsigned char deepth = 0;

  // init path vector
  path.clear();
  path.reserve( header.height + 1 );
  path.push_back( header.root );

  // init index vector
  indizes.clear();
  indizes.reserve( header.height );

  // init node pointer
  nodePtr = nodeMngr->getNode( header.root );

  // create new entry
  Entry* entry = new Entry( tupleId, getDistData(attr) );

  // descent tree until leaf level
  while ( deepth < header.height - 1 )
  { /* find best path (follow the entry with the nearest dist to
        new entry or the smallest covering radius increase) */
      list<SearchBestPathEntry> entriesIn;
      list<SearchBestPathEntry> entriesOut;

      vector<Entry*>* entries = nodePtr->getEntries();
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
      } // for

      list<SearchBestPathEntry>::iterator best;
      if (!entriesIn.empty())
      { // select entry with nearest dist to new entry
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
        } // for
      } // if
      else
      { // select entry with minimal radius increase
        best = entriesOut.begin();
        double minIncrease =
            (*best).dist - entriesOut.front().entry->rad();

        list<SearchBestPathEntry>::iterator iter;
        for ( iter = entriesIn.begin();
              iter != entriesIn.end();
              iter++ )
        {
          double increase = (*iter).dist - (*iter).entry->rad();
          if ( increase < minIncrease )
          {
            minIncrease = increase;
            best = iter;
          }
        }

        // update increased covering radius
        (*best).entry->setRad( (*best).dist );
        nodePtr->modified( !SIZE_CHANGED );
      }

      // update path/indizes vector
      path.push_back( (*best).entry->chield() );
      indizes.push_back( (*best).index );

      //load chield node
      deepth++;
      nodePtr->deleteIfAllowed();
      nodePtr = nodeMngr->getNode( (*best).entry->chield() );
  }

  // nodePtr points to a leaf node

  // compute distance to parent node, if exist
  if ( deepth > 0 )
  {
    double dist;
    Node* parent = nodeMngr->getNode( path[deepth-1] );
    Entry* parentEntry = (*parent->getEntries())[indizes[deepth-1]];
    (*metric)( entry->data(), parentEntry->data(), dist );
    entry->setDist( dist );
    parent->deleteIfAllowed();
  }

  // insert entry into leaf, split if neccesary
  if ( !nodePtr->insert( entry ) )
  {
    split( entry );
  }

  nodePtr->deleteIfAllowed();
  header.entryCount++;
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
  DistData* data = getDistData( attr );

  stack<RemainingNodesEntry> remainingNodess;
  remainingNodess.push( RemainingNodesEntry(header.root, 0, 0 ) );

  size_t count = 0;
  while( !remainingNodess.empty() )
  {
    RemainingNodesEntry parent = remainingNodess.top();
    nodePtr = nodeMngr->getNode( remainingNodess.top().nodeId );
    unsigned char deepth = remainingNodess.top().deepth;
    double distQueryParent = remainingNodess.top().dist;
    remainingNodess.pop();

    if ( deepth < (header.height - 1) )
    { // routing node
      vector<Entry*>* entries = nodePtr->getEntries();
      for ( size_t i=0; i<entries->size(); i++ )
      {
        Entry* curEntry = (*entries)[i];
        double dist = curEntry->dist();
        double radSum = searchRad + curEntry->rad();
        if ( abs( distQueryParent - dist ) <= radSum )
        {
          double newDistQueryParent;
          (*metric)( data, curEntry->data(), newDistQueryParent );
          if ( newDistQueryParent <= radSum )
          {
            remainingNodess.push( RemainingNodesEntry(
                curEntry->chield(), deepth+1, newDistQueryParent ) );
          }
        }
      }
    }
    else
    { // leaf
      vector<Entry*>* entries = nodePtr->getEntries();
      for ( size_t i=0; i<entries->size(); i++ )
      {
        Entry* curEntry = (*entries)[i];
        double dist = curEntry->dist();
        if ( abs( distQueryParent - dist ) <= searchRad )
        {
          count++;
          double distQueryCurrent;
          (*metric)( data, curEntry->data(), distQueryCurrent );
          if ( distQueryCurrent <= searchRad )
          {
            results->push_back( curEntry->tid() );
          }
        } // if
      } // for
    } // else
    nodePtr->deleteIfAllowed();
  } // while

  data->deleteIfAllowed();

#ifdef __MT_PRINT_SEARCH_INFO
  cmsg.info() << "Tried " << count << " out of " << header.entryCount
              << " elements..." << endl << endl;
  cmsg.send();
#endif
}

/*
Method ~rangeSearch~ :

*/
void MT::MTree::nnSearch( Attribute* attr, int nncount,
                          list<TupleId>* results )
{
// not yet implemented
}
