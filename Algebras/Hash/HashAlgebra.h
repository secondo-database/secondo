/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the Hash Algebra

[TOC]

1 Defines and Includes

*/
#ifndef _HASH_ALGEBRA_H_
#define _HASH_ALGEBRA_H_

#include "SecondoSMI.h"
#include "StandardTypes.h"
#include "ListUtils.h"

/*

3 Class ~HashIterator~

Used to iterate over all record ids fulfilling a certain condition.

*/
class HashIterator
{
  public:
    HashIterator( SmiKeyedFileIterator* iter );
/*
Constructs a Hash iterator given a keyed file iterator.

*/

    ~HashIterator();
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

    SmiKeyedFileIterator* fileIter;
    SmiRecordId id;
    SmiKey smiKey;
    SmiRecord record;
};

/*

4 Class ~Hash~

The key attribute of a hash can be an ~int~, a ~string~, or a ~real~.

*/
class Hash
{
  public:
    Hash( SmiKey::KeyDataType keyType, bool temporary = false );
/*
Creates a Hash.

*/
    Hash( SmiKey::KeyDataType keyType, SmiRecord& record,
              size_t& offset);
/*
Opens a Hash given the information in the root record ~record~.

*/
    Hash( SmiKey::KeyDataType keyType, SmiFileId fileId );
/*
Opens a Hash given the key type and a file id.

*/
    ~Hash();
/*
The destructor.

*/

    static Hash *Open( SmiRecord& valueRecord,
                       size_t& offset, const ListExpr typeInfo );
/*
Opens a Hash. This function corresponds to the ~open~-function of the type constructor.

*/

    bool Save( SmiRecord& record, size_t& offset, const ListExpr typeInfo );
/*
Saves a Hash. This function corresponds to the ~save~-function of the type constructor.

*/

    inline bool IsOpened() const
    { return opened; }
/*
Returns if the Hash is correctly in opened state.

*/

    bool Truncate();
/*
Truncates a Hash by deleting all its entries.

*/

    void DeleteFile();
/*
Deletes de Hash file.

*/

    bool Append( const SmiKey& key, SmiRecordId id );
/*
Inserts in the Hash the pair <~key~, ~recordid~>.

*/

    bool Delete(const SmiKey& key, const SmiRecordId id);
/*
Deletes in the Hash the pair <~key~, ~recordid~>.

*/

    SmiHashFile* GetFile() const;
/*
Returns the Hash file.

*/

    SmiFileId GetFileId() const;
/*
Returns the Hash file id.

*/

    SmiKey::KeyDataType GetKeyType() const;
/*
Returns the key data type.

*/

    HashIterator* ExactMatch( Attribute* key );
/*
Performs the exact match query returning an iterator.

*/

    bool getFileStats( SmiStatResultType &result );
/*
Retrieves statsitics on the used file from the storage manager

*/

/*
Return the Secondo type name

*/
  inline static const std::string BasicType() { return "hash"; }

  static const bool checkType(ListExpr list){
    return listutils::isHashDescription(list);
  }

  private:

    bool temporary;
    SmiHashFile* file;
    bool opened;
};

#endif
