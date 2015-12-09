/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the BTree Algebra

[TOC]

1 Defines and Includes

*/
#ifndef _BTREE_ALGEBRA_H_
#define _BTREE_ALGEBRA_H_


#include "SecondoSMI.h"
#include "StandardTypes.h"

/*

Commenting out the following line prevents the usage
of prefetching iterators in the btree algebra.

*/
#define BTREE_PREFETCH

#ifdef BTREE_PREFETCH
typedef PrefetchingIterator BTreeFileIteratorT;
#else
typedef SmiKeyedFileIterator BTreeFileIteratorT;
#endif /* BTREE_PREFETCH */

/*
Funcion headers

*/
SmiKey::KeyDataType ExtractKeyTypeFromTypeInfo( ListExpr typeInfo );
void KeyToAttr( Attribute* attr, SmiKey& key,
                SmiKey::KeyDataType keyType);
void AttrToKey( Attribute* attr, SmiKey& key,
                SmiKey::KeyDataType keyType);


/*

3 Class ~BTreeIterator~

Used to iterate over all record ids fulfilling a certain condition.

*/
template<class Valuetype>
class BTreeIterator_t
{
  public:
    BTreeIterator_t( BTreeFileIteratorT* iter );
/*
Constructs a B-Tree iterator given a keyed file iterator.

*/

    ~BTreeIterator_t();
/*
The destructor.

*/

    bool Next();
/*
Moves forward.

*/

    const SmiKey* GetKey() const;
/*
Returns the key of the iterator's current position.

*/

    Valuetype GetId() const;
/*
Returns the tuple id of the iterator's current position.

*/

  private:

    BTreeFileIteratorT* fileIter;
    Valuetype id;
    SmiKey smiKey;
    SmiRecord record;
};

typedef BTreeIterator_t<SmiRecordId> BTreeIterator;

// template<class Valuetype>
// bool ReadRecordId(SmiRecord& record, Valuetype& id);
// 
// template<class Valuetype>
// bool WriteRecordId(SmiRecord& record, Valuetype id);

/*
2.5 Function ~WriteRecordId~

Writes a ~SmiRecordId~ to a ~SmiRecord~.

*/
template<class Valuetype>
bool WriteRecordId(SmiRecord& record, Valuetype id)
{
  SmiSize bytesWritten;
  SmiSize idSize = sizeof(Valuetype);

  bytesWritten = record.Write(&id, idSize);
  return bytesWritten == idSize;
}

/*
2.6 Function ~ReadRecordId~

Reads a ~SmiRecordId~ from a ~SmiRecord~.

*/
template<class Valuetype>
bool ReadRecordId(SmiRecord& record, Valuetype& id)
{
  SmiSize bytesRead;
  Valuetype ids[2];
  SmiSize idSize = sizeof(Valuetype);

  bytesRead = record.Read(ids, 2 * idSize);
  id = ids[0];
  return bytesRead == idSize;
}

/*
2.7 Function ~ReadRecordId~

Reads a ~SmiRecordId~ from a ~PrefetchingIterator~.

*/
template<class Valuetype>
bool ReadRecordId(PrefetchingIterator* iter, Valuetype& id)
{
  SmiSize bytesRead;
  Valuetype ids[2];
  SmiSize idSize = sizeof(Valuetype);

  bytesRead = iter->ReadCurrentData(ids, 2 * idSize);
  id = ids[0];
  return bytesRead == idSize;
}

/*

3 Class ~BTreeIterator~

Used to iterate over all record ids fulfilling a certain condition.

*/
template<class Valuetype>
BTreeIterator_t<Valuetype>::BTreeIterator_t(BTreeFileIteratorT* iter)
  : fileIter(iter)
{
}

template<class Valuetype>
BTreeIterator_t<Valuetype>::~BTreeIterator_t()
{
  delete fileIter;
}

template<class Valuetype>
bool BTreeIterator_t<Valuetype>::Next()
{
  bool received;

#ifdef BTREE_PREFETCH
  received = fileIter->Next();
#else
  received = fileIter->Next(smiKey, record);
#endif /* BTREE_PREFETCH */

  if(received)
  {

#ifdef BTREE_PREFETCH
    fileIter->CurrentKey(smiKey);
#endif /* BTREE_PREFETCH */

#ifdef BTREE_PREFETCH
    return ReadRecordId<Valuetype>(fileIter, id);
#else
    return ReadRecordId<Valuetype>(record, id);
#endif /* BTREE_PREFETCH */
  }
  else
  {
    return false;
  }
}

template<class Valuetype>
const SmiKey *BTreeIterator_t<Valuetype>::GetKey() const
{
  return &smiKey;
}

template<class Valuetype>
Valuetype BTreeIterator_t<Valuetype>::GetId() const
{
  return id;
}

/*

4 Class ~BTree~

The key attribute of a btree can be an ~int~, a ~string~, or a ~real~.

*/
template<class Valuetype>
class BTree_t
{
  public:
    BTree_t( SmiKey::KeyDataType keyType, bool temporary = false );
/*
Creates a B-Tree.

*/
    BTree_t( SmiKey::KeyDataType keyType, SmiRecord& record,
              size_t& offset);
/*
Opens a B-Tree given the information in the root record ~record~.

*/
    BTree_t( SmiKey::KeyDataType keyType, SmiFileId fileId );
/*
Opens a B-Tree given the key type and a file id.

*/
    ~BTree_t();
/*
The destructor.

*/

    static BTree_t *Open( SmiRecord& valueRecord,
		        size_t& offset, const ListExpr typeInfo );
/*
Opens a B-Tree. This function corresponds to the ~open~-function of the type constructor.

*/

    bool Save( SmiRecord& record, size_t& offset, const ListExpr typeInfo );
/*
Saves a B-Tree. This function corresponds to the ~save~-function of the type constructor.

*/

    inline bool IsOpened() const
    { return opened; }
/*
Returns if the B-Tree is correctly in opened state.

*/

    bool Truncate();
/*
Truncates a B-Tree by deleting all its entries.

*/

    void DeleteFile();
/*
Deletes de B-Tree file.

*/

    bool Append( const SmiKey& key, Valuetype id );
/*
Inserts in the B-Tree the pair <~key~, ~recordid~>.

*/

    bool Delete(const SmiKey& key, const Valuetype id);
/*
Deletes in the B-Tree the pair <~key~, ~recordid~>.

*/

    SmiBtreeFile* GetFile() const;
/*
Returns the B-Tree file.

*/

    SmiFileId GetFileId() const;
/*
Returns the B-Tree file id.

*/

    SmiKey::KeyDataType GetKeyType() const;
/*
Returns the key data type.

*/

    BTreeIterator_t<Valuetype>* ExactMatch( Attribute* key );
/*
Performs the exact match query returning an iterator.

*/

    BTreeIterator_t<Valuetype>* LeftRange( Attribute* key );
/*
Performs the left range query returning an iterator.

*/

    BTreeIterator_t<Valuetype>* RightRange( Attribute* key );
/*
Performs the right range query returning an iterator.

*/

    BTreeIterator_t<Valuetype>* Range( Attribute* left, Attribute* right );
/*
Performs the range query between ~left~ and ~right~ returning an iterator.

*/

    BTreeIterator_t<Valuetype>* SelectAll();
/*
Performs a complete scan returning an iterator.

*/

    bool getFileStats( SmiStatResultType &result );
/*
Retrieves statsitics on the used file from the storage manager

*/

    inline static const std::string BasicType() { return "btree"; }

    static const bool checkType(const ListExpr list){
      return listutils::isBTreeDescription(list);
    }

  private:

    bool temporary;
    SmiBtreeFile* file;
    bool opened;
};

typedef BTree_t<SmiRecordId> BTree;

#ifdef BTREE_PREFETCH
typedef PrefetchingIterator BTreeFileIteratorT;
#else
typedef SmiKeyedFileIterator BTreeFileIteratorT;
#endif /* BTREE_PREFETCH */

/*

4 Class ~BTree~

The key attribute of a btree can be an ~int~, a ~string~, ~real~, or a
composite one.

*/
template<class Valuetype>
BTree_t<Valuetype>::BTree_t( SmiKey::KeyDataType keyType, bool temporary ):
temporary( temporary ),
file( 0 ),
opened( false )
{
  if( keyType != SmiKey::Unknown )
  {
    file = new SmiBtreeFile( keyType, false, temporary );
    if( file->Create() )
      opened = true;
    else
    {
      delete file; file = 0;
    }
  }
}

template<class Valuetype>
BTree_t<Valuetype>::BTree_t( SmiKey::KeyDataType keyType, SmiRecord& record,
                             size_t& offset):
temporary( false ),
file( 0 ),
opened( false )
{
  SmiFileId fileId;
  if( record.Read( &fileId, sizeof(SmiFileId), offset ) != sizeof(SmiFileId) )
    return;

  this->file = new SmiBtreeFile( keyType, false );
  if( file->Open( fileId ) )
  {
    opened = true;
    offset += sizeof(SmiFileId);
  }
  else
  {
    delete file; file = 0;
  }
}

template<class Valuetype>
BTree_t<Valuetype>::BTree_t( SmiKey::KeyDataType keyType, SmiFileId fileId ):
temporary( false ),
file( 0 ),
opened( false )
{
  if( keyType != SmiKey::Unknown )
  {
    file = new SmiBtreeFile( keyType );
    if( file->Open( fileId ) )
      opened = true;
    else
    {
      delete file; file = 0;
    }
  }
}

template<class Valuetype>
BTree_t<Valuetype>::~BTree_t()
{
  if( opened )
  {
    assert( file != 0 );
    if( temporary )
      file->Drop();
    else
      file->Close();
    delete file;
  }
}

template<class Valuetype>
BTree_t<Valuetype> *BTree_t<Valuetype>::Open( SmiRecord& valueRecord, 
                                       size_t& offset, const ListExpr typeInfo )
{
  return new BTree_t<Valuetype>( ExtractKeyTypeFromTypeInfo( typeInfo ), 
                                 valueRecord, offset );
}

template<class Valuetype>
bool BTree_t<Valuetype>::Truncate()
{
  if( opened )
    return file->Truncate();
  return true;
}

template<class Valuetype>
void BTree_t<Valuetype>::DeleteFile()
{
  if( opened )
  {
    assert( file != 0 );
    file->Close();
    file->Drop();
    delete file; file = 0;
    opened = false;
  }
}

template<class Valuetype>
bool BTree_t<Valuetype>::Save(SmiRecord& record, size_t& offset, 
                              const ListExpr typeInfo)
{
  if( !opened )
    return false;

  assert( file != 0 );
  const size_t n = sizeof(SmiFileId);
  SmiFileId fileId = file->GetFileId();
  if( record.Write( &fileId, n, offset) != n )
    return false;

  offset += n;
  return true;
}

template<class Valuetype>
bool BTree_t<Valuetype>::Append( const SmiKey& smiKey, Valuetype id )
{
  if( opened )
  {
    assert( file != 0 );
    assert( smiKey.GetType() == GetKeyType() );

    SmiRecord record;
    if( file->InsertRecord( smiKey, record ) )
    {
      if( WriteRecordId( record, id ) )
        return true;
      file->DeleteRecord(smiKey);
    }
  }
  return false;
}

template<class Valuetype>
bool BTree_t<Valuetype>::Delete( const SmiKey& smiKey, const Valuetype id )
{
  if( opened )
  {
    assert(file != 0);
    return file->DeleteRecord( smiKey, false, id );
  }
  return false;
}

template<class Valuetype>
SmiBtreeFile* BTree_t<Valuetype>::GetFile() const
{
  return this->file;
}

template<class Valuetype>
SmiFileId BTree_t<Valuetype>::GetFileId() const
{
  return this->file->GetFileId();
}

template<class Valuetype>
SmiKey::KeyDataType BTree_t<Valuetype>::GetKeyType() const
{
  return this->file->GetKeyType();
}

template<class Valuetype>
BTreeIterator_t<Valuetype>* BTree_t<Valuetype>::ExactMatch( Attribute* key )
{
  if( !opened )
    return 0;

  assert( file != 0 );

  SmiKey smiKey;
  BTreeFileIteratorT* iter;

  AttrToKey( key, smiKey, file->GetKeyType() );

#ifdef BTREE_PREFETCH
  iter = file->SelectRangePrefetched( smiKey, smiKey );
  if( iter == 0 )
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectRange( smiKey, smiKey, *iter, SmiFile::ReadOnly, true ) )
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator_t<Valuetype>( iter );
}

template<class Valuetype>
BTreeIterator_t<Valuetype>* BTree_t<Valuetype>::LeftRange( Attribute* key )
{
  if( !opened )
    return 0;

  assert( file != 0 );
  SmiKey smiKey;
  BTreeFileIteratorT* iter;

  AttrToKey( key, smiKey, file->GetKeyType() );

#ifdef BTREE_PREFETCH
  iter = file->SelectLeftRangePrefetched( smiKey );
  if(iter == 0)
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectLeftRange( smiKey, *iter, SmiFile::ReadOnly, true ) )
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator_t<Valuetype>( iter );
}

template<class Valuetype>
BTreeIterator_t<Valuetype>* BTree_t<Valuetype>::RightRange( Attribute* key )
{
  if( !opened )
    return 0;

  assert( file != 0 );
  SmiKey smiKey;
  BTreeFileIteratorT* iter;

  AttrToKey( key, smiKey, file->GetKeyType() );

#ifdef BTREE_PREFETCH
  iter = file->SelectRightRangePrefetched( smiKey );
  if( iter == 0 )
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if(!file->SelectRightRange( smiKey, *iter, SmiFile::ReadOnly, true ) )
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator_t<Valuetype>( iter );
}

template<class Valuetype>
BTreeIterator_t<Valuetype>* BTree_t<Valuetype>::Range( Attribute* left, 
                                                       Attribute* right)
{
  if( !opened )
    return 0;

  assert( file != 0 );
  SmiKey leftSmiKey;
  SmiKey rightSmiKey;
  BTreeFileIteratorT* iter;

  AttrToKey( left, leftSmiKey, file->GetKeyType() );
  AttrToKey( right, rightSmiKey, file->GetKeyType() );

#ifdef BTREE_PREFETCH
  iter = file->SelectRangePrefetched( leftSmiKey, rightSmiKey );
  if( iter == 0 )
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectRange( leftSmiKey, rightSmiKey,
                          *iter, SmiFile::ReadOnly, true))
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator_t<Valuetype>( iter );
}

template<class Valuetype>
BTreeIterator_t<Valuetype>* BTree_t<Valuetype>::SelectAll()
{
  BTreeFileIteratorT* iter;

#ifdef BTREE_PREFETCH
  iter = file->SelectAllPrefetched();
  if(iter == 0)
    return 0;
#else
  iter = new SmiKeyedFileIterator( true );
  if( !file->SelectAll( *iter, SmiFile::ReadOnly, true ) )
  {
    delete iter;
    return 0;
  }
#endif /* BTREE_PREFETCH */

  return new BTreeIterator_t<Valuetype>( iter );
}

template<class Valuetype>
bool BTree_t<Valuetype>::getFileStats( SmiStatResultType &result )
{
  if ( (file == 0) || !opened ){
    return false;
  }
  result = file->GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << file->GetFileId();
  result.push_back(std::pair<std::string,std::string>("FilePurpose",
            "SecondaryBtreeIndexFile"));
  result.push_back(std::pair<std::string,std::string>("FileId",fileid.str()));
  return true;
}

#endif
