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

using namespace std;

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

3 Class ~BTreeIterator~

Used to iterate over all record ids fulfilling a certain condition.

*/
class BTreeIterator
{
  public:
    BTreeIterator( BTreeFileIteratorT* iter );
/*
Constructs a B-Tree iterator given a keyed file iterator.

*/

    ~BTreeIterator();
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

    SmiRecordId GetId() const;
/*
Returns the tuple id of the iterator's current position.

*/

  private:

    BTreeFileIteratorT* fileIter;
    SmiRecordId id;
    SmiKey smiKey;
    SmiRecord record;
};

/*

4 Class ~BTree~

The key attribute of a btree can be an ~int~, a ~string~, or a ~real~.

*/
class BTree
{
  public:
    BTree( SmiKey::KeyDataType keyType, bool temporary = false );
/*
Creates a B-Tree.

*/
    BTree( SmiKey::KeyDataType keyType, SmiRecord& record,
              size_t& offset);
/*
Opens a B-Tree given the information in the root record ~record~.

*/
    BTree( SmiKey::KeyDataType keyType, SmiFileId fileId );
/*
Opens a B-Tree given the key type and a file id.

*/
    ~BTree();
/*
The destructor.

*/

    static BTree *Open( SmiRecord& valueRecord,
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

    bool Append( const SmiKey& key, SmiRecordId id );
/*
Inserts in the B-Tree the pair <~key~, ~recordid~>.

*/

    bool Delete(const SmiKey& key, const SmiRecordId id);
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

    BTreeIterator* ExactMatch( Attribute* key );
/*
Performs the exact match query returning an iterator.

*/

    BTreeIterator* LeftRange( Attribute* key );
/*
Performs the left range query returning an iterator.

*/

    BTreeIterator* RightRange( Attribute* key );
/*
Performs the right range query returning an iterator.

*/

    BTreeIterator* Range( Attribute* left, Attribute* right );
/*
Performs the range query between ~left~ and ~right~ returning an iterator.

*/

    BTreeIterator* SelectAll();
/*
Performs a complete scan returning an iterator.

*/

    bool getFileStats( SmiStatResultType &result );
/*
Retrieves statsitics on the used file from the storage manager

*/

    inline static const string BasicType() { return "btree"; }

  private:

    bool temporary;
    SmiBtreeFile* file;
    bool opened;
};

/*
Funcion headers

*/
bool ReadRecordId(SmiRecord& record, SmiRecordId& id);
bool WriteRecordId(SmiRecord& record, SmiRecordId id);
SmiKey::KeyDataType ExtractKeyTypeFromTypeInfo( ListExpr typeInfo );
void KeyToAttr( Attribute* attr, SmiKey& key,
                SmiKey::KeyDataType keyType);
void AttrToKey( Attribute* attr, SmiKey& key,
                SmiKey::KeyDataType keyType);



#endif
