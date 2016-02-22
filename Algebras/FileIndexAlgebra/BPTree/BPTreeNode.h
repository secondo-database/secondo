/*
class representing nodes in a B+ Tree

*/

#ifndef BPTREENODE_H
#define BPTREENODE_H

#include "Attribute.h"

namespace fialgebra{
  class BPTreeNode{

  public:
    // ctor
    BPTreeNode(char* bytes, size_t pageSize, size_t typeSize,
               size_t pageNumber, ObjectCast valueCast);
    BPTreeNode(size_t pageSize, size_t typeSize,
               size_t pageNumber, ObjectCast valueCast, bool isLeaf);
    ~BPTreeNode();

    void InsertValue(Attribute& value, size_t index);
    void InsertValue(Attribute& value, unsigned long id, size_t index);
    void RemoveValueAt(size_t index);

    void InsertValues(size_t index, Attribute** const values, size_t count);
    void RemoveValues(size_t index, size_t count);

    Attribute& GetValueAt(size_t index);
    void SetValueAt(size_t index, Attribute& value);

    unsigned long GetIDAt(size_t index);
    void SetIDAt(size_t index, unsigned long id);
    void InsertIDAt(size_t index, unsigned long id);
    void RemoveIDAt(size_t index);

    void InsertIds(size_t index, unsigned long* const ids, size_t count);
    void RemoveIds(size_t index, size_t count);

    unsigned long GetPrevLeaf();
    void SetPrevLeaf(unsigned long pageNumber);

    unsigned long GetNextLeaf();
    void SetNextLeaf(unsigned long pageNumber);

    size_t GetValueCount();
    size_t GetMaxValueCount();

    size_t GetTypeSize();
    char* GetBytes();

    bool IsLeaf();

    size_t GetPageNumber();
    void SetPageNumber(size_t pageNumber);

    void Print(std::ostream& o);

    std::string ToString();

  private:
    char* m_bytes,
      * m_values,
      * m_overflow,
      * m_overflowValue;

    size_t* m_valueCount;

    bool* m_isLeaf;

    unsigned long* m_ids,
      * m_prevLeaf,
      * m_nextLeaf,
      * m_overflowID;

    size_t m_typeSize,
      m_maxValueCount,
      m_pageSize,
      m_pageNumber;

    ObjectCast m_valueCast;

    const size_t constantSize = sizeof(size_t) + sizeof(bool) +
                                sizeof(unsigned long) + sizeof(unsigned long);

    std::string ToString(Attribute& value) const;
  };
}

#endif // BPTREENODE_H
