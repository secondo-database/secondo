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
    BTreeIterator(SmiKey::KeyDataType smiKeyType, BTreeFileIteratorT* iter);

    ~BTreeIterator();

    bool Next();
    StandardAttribute* GetKey();
    SmiRecordId GetId();

  private:
    BTreeFileIteratorT* fileIter;
    StandardAttribute* key;
    SmiRecordId id;
    SmiKey::KeyDataType keyType;
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
    BTree(SmiKey::KeyDataType keyType = SmiKey::Unknown, SmiKeyedFile* file = 0);
    BTree(SmiRecord& record, SmiKey::KeyDataType keyType);
    BTree(SmiFileId fileId, SmiKey::KeyDataType keyType);
    ~BTree();

    bool IsInitialized();
    bool WriteTo(SmiRecord& record);
    void SetPermanent();
    void SetTemporary();
    bool SetTypeAndCreate(SmiKey::KeyDataType keyType);
    bool Truncate();
    void DeleteDeep();
    void DeleteFile();
    bool Append(StandardAttribute* attr, SmiRecordId id);
    SmiKeyedFile* GetFile() const;
    SmiFileId GetFileId() const;
    SmiKey::KeyDataType GetKeyType();
    
    BTreeIterator* ExactMatch(StandardAttribute* key);
    BTreeIterator* LeftRange(StandardAttribute* key);
    BTreeIterator* RightRange(StandardAttribute* key);
    BTreeIterator* Range(StandardAttribute* left, StandardAttribute* right);
    BTreeIterator* SelectAll();

  private:
    SmiRecordId id;
    bool isTemporary;
    SmiKey::KeyDataType keyType;
    SmiKeyedFile* file;
    SmiFileId fileId;
};

#endif
