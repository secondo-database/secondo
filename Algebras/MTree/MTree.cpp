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

1 Implementation of ~MTree~ datastructure

November 2007 Mirko Dibbert

[TOC]

1 Defines and includes

*/
using namespace std;

#include "MTree.h"

/*
1 Class ~MTreeNode~

1.1 Constructor

*/
MTreeNode::MTreeNode( bool leaf, int max, int sizeOfEntries )
{
  this->isLeaf = leaf;
  this->maxEntries = max;
  this->sizeOfEntries = sizeOfEntries;

  modified = true;
}

/*
1.1 Destructor

*/
MTreeNode::~MTreeNode()
{
 entries.clear();
}

/*
1.1 Method ~SizeOfEmptyNode~

*/
int MTreeNode::SizeOfEmptyNode()
{
  return sizeof(int); // count
}

/*
1.1 Methods ~Read~

*/
void MTreeNode::Read( SmiRecordFile &file, const SmiRecordId page )
{
  SmiRecord record;
  assert( file.SelectRecord( page, record, SmiFile::ReadOnly ) );
  Read( record );
}

void MTreeNode::Read( SmiRecord &record )
{
  int offset = 0;
  char buffer[Size() + 1];
  memset( buffer, 0, Size() + 1 );

  assert( record.Read( buffer, Size(), offset ) );

  // read number of stored entries
  size_t count;
  memcpy( &count, buffer + offset, sizeof( count ) );
  offset += sizeof( count );

  assert( count <= maxEntries );

  // read the entry array.
  for( size_t i = 0; i < count; i++ )
  {
    if( isLeaf )
      entries.push_back(new MTreeLeafEntry( buffer, offset));
    else
      entries.push_back(new MTreeRoutingEntry( buffer, offset));

  }
  modified = false;
}

/*
1.1 Methods ~Write~

*/
void MTreeNode::Write( SmiRecordFile &file, const SmiRecordId page )
{
  if( modified )
  {
    SmiRecord record;
    assert( file.SelectRecord( page, record, SmiFile::Update ) );
    Write( record );
  }
}

void MTreeNode::Write( SmiRecord &record )
{
  if( modified )
  {
    int offset = 0;
    char buffer[Size() + 1];
    memset( buffer, 0, Size() + 1 );

    // write number of stored 
    unsigned int count = entries.size();
    memcpy( buffer + offset, &count, sizeof( count ) );
    offset += sizeof( count );

    assert(count <= maxEntries );

    // write the entry array
    list<MTreeEntry*>::iterator iter;
    for(iter = entries.begin(); iter != entries.end(); iter++ )
    if (isLeaf)
      ((MTreeLeafEntry*)*iter)->Write( buffer, offset );
    else
      ((MTreeRoutingEntry*)*iter)->Write( buffer, offset );

    assert( record.Write( buffer, Size(), 0 ) );

    modified = false;
  }
}

/*
1.1 Methods ~InsertEntry~

*/
void MTreeNode::InsertEntry( MTreeEntry *entry )
{
  entries.push_back(entry);
  modified = true;
}

/*
2 Class ~MTree~

2.1 Constructors

*/
MTree::MTree( size_t pagesize ) :
  file(true, pagesize),
  header(),
  initialized ( false ),
  pagesize ( pagesize )
{
  assert ( pagesize >= MIN_PAGESIZE );

  file.Create();

  // create header page
  SmiRecordId headerId;
  SmiRecord headerRecord;
  assert( file.AppendRecord( headerId, headerRecord ) );
  assert( headerId == 1 );

  // create root node page
  SmiRecordId rootId;
  SmiRecord rootRecord;
  assert( file.AppendRecord( rootId, rootRecord ) );
  header.root = rootId;
}

MTree::MTree( const SmiFileId fileId ) :
  file(true),
  initialized ( false )
{
  file.Open( fileId );
  ReadHeader();

  if (header.storage == EXTERNAL)
  {
    datafile = new SmiRecordFile( true );
    datafile->Open( header.datafileId );
  }

}
/*
2.2 Destructor

*/
MTree::~MTree()
{
  if( file.IsOpen() )
  {
    WriteHeader();
    file.Close();
  }

  if ( header.storage == EXTERNAL )
    if( datafile->IsOpen() )
      datafile->Close();
}

void MTree::DeleteFile()
{
  cout << "deleting file\n";
  if( file.IsOpen() )
    file.Close();

  file.Drop();

  // delete datafile, if external storage had been used
  if ( header.storage == EXTERNAL )
  {
    cout << "deleting datafile\n";
    if (datafile->IsOpen())
      datafile->Close();

    datafile->Drop();
  }
}

/*
2.3 Method ~Initialize~

*/
void MTree::Initialize(
        Relation *rel, Tuple *tuple, int attrIndex,
        PROMFUN promFun, PARTFUN partFun )
{
  this->rel = rel;

  // get attribute object from tuple
  StandardMetricalAttribute *attr = (StandardMetricalAttribute*)
                             tuple->GetAttribute( attrIndex );

  // get attribute type
  AttributeType type = tuple->GetTupleType()->GetAttributeType( attrIndex );

  // update header
  header.promFun = promFun;
  header.partFun = partFun;
  header.attrIndex = attrIndex;
  header.algebraId = type.algId;
  header.typeId = type.typeId;
  header.dataLength = attr->GetMDataSize();

  // calculate max entries per node
  int emptySize = pagesize - MTreeNode::SizeOfEmptyNode();
  int maxLeaf, maxRouting;
  int leafEntrySize, routingEntrySize;

  if ( header.dataLength == 0 )
  {
    cout << "REFERENCE storage\n";
    header.storage = REFERENCE;
    leafEntrySize = MTreeLeafEntry::StaticSize() + MTreeDataRef::Size();
    routingEntrySize = MTreeRoutingEntry::StaticSize() + MTreeDataRef::Size();
    maxLeaf = emptySize / leafEntrySize;
    maxRouting = emptySize / routingEntrySize;
  }
  else
  {
    leafEntrySize = MTreeLeafEntry::StaticSize() + 
                    MTreeDataInternal::StaticSize() + header.dataLength;
    routingEntrySize = MTreeRoutingEntry::StaticSize() + 
                       MTreeDataInternal::StaticSize() + header.dataLength;
    maxLeaf = emptySize / leafEntrySize;
    maxRouting = emptySize / routingEntrySize;

    if ( ( maxLeaf >= MIN_NODE_CAPACITY ) && 
         ( maxRouting >= MIN_NODE_CAPACITY ))
    {
      cout << "INTERNAL storage\n";
      header.storage = INTERNAL;
    }
    else
    {
      cout << "EXTERNAL storage\n";
      header.storage = EXTERNAL;
      leafEntrySize = MTreeLeafEntry::StaticSize() + 
                      MTreeDataExternal::Size();
      routingEntrySize = MTreeRoutingEntry::StaticSize() + 
                         MTreeDataExternal::Size();
      maxLeaf = emptySize / leafEntrySize;
      maxRouting = emptySize / routingEntrySize;

      // create datafile for external storage
      datafile = new SmiRecordFile(true, header.dataLength);
      datafile->Create();
      header.datafileId = datafile->GetFileId();
    }
  }
  header.maxLeafEntries = maxLeaf;
  header.maxRoutingEntries = maxRouting;
  header.leafEntrySize = leafEntrySize;
  header.routingEntrySize = routingEntrySize;
  header.headerInitialized = true;
  cout << "MaxEntries : [ " << maxLeaf << " | " << maxRouting << " ]\n";
  cout << "Sizes      : [ " << leafEntrySize << " | " 
       << routingEntrySize << " ]\n";

  InitializeFromHeader( rel );
}

/*
2.4 Method ~InitializeFromHeader~

*/
void MTree::InitializeFromHeader(Relation *rel)
{
  assert (header.headerInitialized);

  Metric metric = MetricRegistry::GetMetric( header.algebraId, header.typeId );
  assert (metric != 0); 
/*
This assertion fails, in no metric had been defined in MetricRegistry.

*/

  // create distance function
  switch( header.storage )
  {
    case REFERENCE:
      distFun = new DFReference( rel, header.attrIndex, metric );
      break;
    case INTERNAL:
      distFun = new DFInternal( metric );
      break;
    case EXTERNAL:
      distFun = new DFExternal( metric, datafile );
      break;
  }

  // create split policy
  splitPol = new SplitPolicy( distFun, header.promFun, header.partFun );

  //create root node and write it into root page
  MTreeNode *root = new MTreeNode( true, header.maxLeafEntries, 
                                   header.leafEntrySize );
  root->Write( file, header.root );
  header.height = 1;
  header.nodeCount++;

  initialized = true;
}

/*
2.5 Method ~ReadHeader~

*/
void MTree::ReadHeader()
{
  SmiRecord record;
  assert( file.SelectRecord( (SmiRecordId)1, record, SmiFile::ReadOnly ) );
  header.Read( record );
}

/*
2.6 Method ~WriteHeader~

*/
void MTree::WriteHeader()
{
  SmiRecord record;
  assert( file.SelectRecord( (SmiRecordId)1, record, SmiFile::Update ) );
  header.Write( record );
}

/*
1.1 Method Split

*/
void MTree::Split()
{
//  MTreeEntry *prom1;
//  MTreeEntry *prom2;
//  splitPol->Promote(entries, prom1, prom2);
//  list<MTreeEntry*> entries1, entries2;
//  double rad1, rad2;
//  splitPol->Partition(entries, prom1, prom2, entries1, rad1, entries2,rad2);
//  SmiRecordId* chield = ???
//  MTreeRoutingEntry *p1 = new MTreeRoutingEntry(prom1, chield, rad1);
//  MTreeRoutingEntry *p2 = new MTreeRoutingEntry(prom2, chield, rad2);
//  ...
}

/*
1.1 Method ~Insert~

*/
void MTree::Insert( TupleId tupleId  )
{
cout << "MTree::Insert( " << tupleId << " )\n";

   StandardMetricalAttribute *attr = (StandardMetricalAttribute*)
         (rel->GetTuple( tupleId ))->GetAttribute( header.attrIndex );
   MTreeData* data;

/*
creating test object to test distFun:

*/

  // create data object
  switch( header.storage )
  {
    case REFERENCE:
      data = new MTreeDataRef( tupleId );
      break;

    case INTERNAL:
      data = new MTreeDataInternal(attr->GetMData(), attr->GetMDataSize());
      break;

    case EXTERNAL:
      SmiRecordId dataRecId;
      SmiRecord record;
      datafile->AppendRecord(dataRecId, record);
      data = new MTreeDataExternal(dataRecId);

      // write data string into record
      char* datastr = attr->GetMData();
      assert( datafile->SelectRecord( dataRecId, record, SmiFile::Update ) );
      assert( record.Write( datastr, header.dataLength, 0 ) 
              == header.dataLength );
      break;
  }
  MTreeLeafEntry* entry = new MTreeLeafEntry(data, tupleId, 0);
  distFun->Distance(entry,entry);

}
