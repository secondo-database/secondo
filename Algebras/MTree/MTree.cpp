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

#define isLeaf(level)\
(header.height == level) ? true : false

/*
3 Struct ~MTreeLeafEntry~

3.1 Method ~Read~

*/
void MTreeLeafEntry::Read(char *buffer, int &offset, STORAGE_TYPE storageType )
{
  memcpy( &dist, buffer+offset, sizeof(double) );
  offset += sizeof(double);

  memcpy( &tupleId, buffer+offset, sizeof(TupleId) );
  offset += sizeof(TupleId);

// cout << "..." << ((MTreeLeafEntry*)entries.back())->tupleId << "...\n";
  switch ( storageType )
  {
    case REFERENCE:
      data = new MTreeData_Ref( buffer, offset );
      break;

    case INTERNAL:
      data = new MTreeData_Int( buffer, offset );
      break;

    case EXTERNAL:
      data = new MTreeData_Ext( buffer, offset );
      break;
  }
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
void MTreeRoutingEntry::Read( char *buffer, int &offset,
                              STORAGE_TYPE storageType )
{
  memcpy( &dist, buffer+offset, sizeof(double) );
  offset += sizeof(double);

  memcpy( &r, buffer+offset, sizeof(double) );
  offset += sizeof(double);

  memcpy( &chield, buffer+offset, sizeof(SmiRecordId) );
  offset += sizeof(SmiRecordId);

  switch ( storageType )
  {
    case REFERENCE:
      data = new MTreeData_Ref( buffer, offset );
      break;

    case INTERNAL:
      data = new MTreeData_Int( buffer, offset );
      break;

    case EXTERNAL:
      data = new MTreeData_Ext( buffer, offset );
      break;
  }
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

/*
1 Class ~MTreeNode~

1.1 Constructor

*/
MTreeNode::MTreeNode( bool leaf, size_t maxEntries,
                      STORAGE_TYPE storageType ) :
  entries()
{
  this->isLeaf = leaf;
  this->maxEntries = maxEntries;
  this->storageType = storageType;
  modified = true;

}

/*
1.1 Destructor

*/
MTreeNode::~MTreeNode()
{
  list<MTreeEntry*>::iterator iter = entries.begin();
  while (iter != entries.end())
  {
    delete *iter;
    iter++;
  }
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
  char buffer[record.Size()];

  assert( record.Read( buffer, record.Size(), offset ) );

  // read number of stored entries
  size_t count;
  memcpy( &count, buffer + offset, sizeof( count ) );
  offset += sizeof( count );

  assert( count <= maxEntries );

  // read the entry array.
  entries.clear();
  for( size_t i = 0; i < count; i++ )
  {
    if( isLeaf )
    {
      MTreeLeafEntry* e = new MTreeLeafEntry( buffer, offset, storageType );
      entries.push_back(e);
    }
    else
    {
      entries.push_back(new MTreeRoutingEntry( buffer, offset, storageType ));
    }
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
    char buffer[record.Size()];
    memset( buffer, 0, record.Size() );

    // write number of stored 
    size_t count = entries.size();
    memcpy( buffer + offset, &count, sizeof( count ) );
    offset += sizeof( count );

    assert(count <= maxEntries );

    // write the entry array
    list<MTreeEntry*>::iterator iter;
    for(iter = entries.begin(); iter != entries.end(); iter++ )
    {
      if (isLeaf)
      {
        ((MTreeLeafEntry*)*iter)->Write( buffer, offset );
      }
      else
      {
        ((MTreeRoutingEntry*)*iter)->Write( buffer, offset );
      }
    }

    assert( record.Write( buffer, record.Size(), 0 ) );

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
MTree::MTree() :
  file( true, PAGESIZE ),
  header(),
  initialized ( false )
{
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
  {
    if( datafile->IsOpen() )
      datafile->Close();

    delete datafile;
  }

  delete metric;
  delete splitpol;
}

void MTree::DeleteFile()
{
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
  int emptySize = PAGESIZE - MTreeNode::SizeOfEmptyNode();
  int maxLeaf, maxRouting;
  int leafEntrySize, routingEntrySize;

  if ( header.dataLength == 0 )
  {
    cout << "REFERENCE storage\n";
    header.storage = REFERENCE;
    leafEntrySize = MTreeLeafEntry::StaticSize() + MTreeData_Ref::Size();
    routingEntrySize = MTreeRoutingEntry::StaticSize() + MTreeData_Ref::Size();
    maxLeaf = emptySize / leafEntrySize;
    maxRouting = emptySize / routingEntrySize;
  }
  else
  {
    leafEntrySize = MTreeLeafEntry::StaticSize() + 
                    MTreeData_Int::StaticSize() + header.dataLength;
    routingEntrySize = MTreeRoutingEntry::StaticSize() + 
                       MTreeData_Int::StaticSize() + header.dataLength;
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
                      MTreeData_Ext::Size();
      routingEntrySize = MTreeRoutingEntry::StaticSize() + 
                         MTreeData_Ext::Size();
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
  header.headerInitialized = true;

  cout << "MaxEntries [leaf | internal ] : [ "
       << maxLeaf << " | " << maxRouting << " ]\n";
  cout << "Sizes                         : [ " << leafEntrySize << " | "
       << routingEntrySize << " ]\n\n";

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
  this->metric = new MetricWrapper( metric, rel, header.attrIndex,
                                    datafile, header.storage);

  // create split policy
  splitpol = new SplitPolicy( this->metric, header.promFun, header.partFun );

  //create root node and write it into root page
  MTreeNode *root = new MTreeNode( true, header.maxLeafEntries,
                                   header.storage );
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
//  splitpol->Promote(entries, prom1, prom2);
//  list<MTreeEntry*> entries1, entries2;
//  double rad1, rad2;
//  splitpol->Partition(entries, prom1, prom2, entries1, rad1, entries2,rad2);
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

  assert( initialized );

   MTreeData* data;
   StandardMetricalAttribute *attr = (StandardMetricalAttribute*)
         (rel->GetTuple( tupleId ))->GetAttribute( header.attrIndex );

  // create new entry
  switch( header.storage )
  {
    case REFERENCE:
      data = new MTreeData_Ref( tupleId );
      break;

    case INTERNAL:
      data = new MTreeData_Int(attr->GetMData(), attr->GetMDataSize());
      break;

    case EXTERNAL:
      SmiRecordId dataRecId;
      SmiRecord record;
      datafile->AppendRecord(dataRecId, record);
      data = new MTreeData_Ext(dataRecId);

      // write data string into record
      char* datastr = attr->GetMData();
      assert( datafile->SelectRecord( dataRecId, record, SmiFile::Update ) );
      assert( record.Write( datastr, header.dataLength, 0 ) 
              == header.dataLength );
      break;
  }
  MTreeLeafEntry* entry = new MTreeLeafEntry(data, tupleId, 0);

  int currLevel;
  SmiRecordId currRec;
  MTreeNode* currNode;
  list<MTreeEntry*>::iterator iter;

  // load root
  currLevel = 1;
  currRec = header.root;
  currNode = new MTreeNode( isLeaf(currLevel), header.maxLeafEntries,
                            header.storage );
  currNode->Read( file, currRec );
  iter = currNode->GetEntryIterator();

cout << " Current entries in root: " << currNode->GetEntryCount() << "\n";
  while( true )
  {
    if ( !(currNode->IsLeaf()) )
    { // routing entry
      // find best node to continue
      return;
    }
    else
    { // insert node into leaf
      if(currNode->IsFull() )
      { // split, if nesccesary
        Split();
        cout << "trying to split";
        return;
      }
      else
      { // currNode is not full, new entry will be stored
        double dist;

        if ( currLevel == 1) // current node is the root node
        {
          dist = 0;
        }
        else
        {
          metric->Distance( *iter, entry, dist );
        }

        currNode->InsertEntry( entry );
        currNode->Write( file, currRec );
        delete currNode;
        currNode = 0;
        return;
      }
    }
  }
}
