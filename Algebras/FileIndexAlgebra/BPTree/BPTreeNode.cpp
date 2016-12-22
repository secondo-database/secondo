/*
class representing nodes in a B+ Tree

*/

#include "BPTreeNode.h"

#include <exception>
#include <string>
#include <cstring>

#include "IndexableAttribute.h"

using std::memcpy;
using std::memmove;
using std::out_of_range;
using std::runtime_error;
using std::string;

//#define BPTreeNodeMax 9

namespace fialgebra{
  string BPTreeNode::ToString(Attribute& value) const {
    std::stringstream s;
    value.Print(s);
    return s.str();
  }

  BPTreeNode::BPTreeNode(char* bytes, size_t pageSize, size_t typeSize,
                         size_t pageNumber, ObjectCast valueCast ) :
    m_bytes( bytes ) {
    if (pageSize < typeSize + sizeof(unsigned long) + constantSize){
      throw out_of_range("BPTreeNode::BPTreeNode(const char*, size_t,"
                         " size_t, size_t): Too small pageSize.");
    }

    m_pageSize = pageSize;
    m_typeSize = typeSize;
    m_pageNumber = pageNumber;
    m_valueCast = valueCast;

    m_isLeaf = (bool*)bytes;
    m_valueCount = (size_t*)(m_isLeaf + 1);
    m_ids = (unsigned long*)(m_valueCount + 1);

#ifdef BPTreeNodeMax
    m_maxValueCount = BPTreeNodeMax;
#else
    m_maxValueCount = (m_pageSize - constantSize)
                      / (typeSize + sizeof(unsigned long));
#endif

    m_prevLeaf = m_ids + m_maxValueCount;
    m_nextLeaf = m_prevLeaf + 1;
    m_values = (char*)(m_nextLeaf + 1);

    m_overflow = new char[sizeof(unsigned long) + typeSize];
    m_overflowID = (unsigned long*)m_overflow;
    m_overflowValue = (char*)(m_overflowID + 1);
  }

  BPTreeNode::BPTreeNode(size_t pageSize, size_t typeSize,
                         size_t pageNumber, ObjectCast valueCast, bool isLeaf) {
    if (pageSize < typeSize + sizeof(unsigned long) + constantSize){
      throw out_of_range("BPTreeNode::BPTreeNode(size_t, size_t, size_t, bool):"
                         " Too small pageSize.");
    }

    m_pageSize = pageSize;
    m_typeSize = typeSize;
    m_pageNumber = pageNumber;
    m_valueCast = valueCast;

    m_bytes = new char[pageSize];
    m_isLeaf = (bool*)m_bytes;
    *m_isLeaf = isLeaf;
    m_valueCount = (size_t*)(m_isLeaf + 1);
    *m_valueCount = 0;
    m_ids = (unsigned long*)(m_valueCount + 1);

#ifdef BPTreeNodeMax
    m_maxValueCount = BPTreeNodeMax;
#else
    m_maxValueCount = (m_pageSize - constantSize)
                      / (typeSize + sizeof(unsigned long));
#endif

    m_prevLeaf = m_ids + m_maxValueCount;
    *m_prevLeaf = 0;
    m_nextLeaf = m_prevLeaf + 1;
    *m_nextLeaf = 0;
    m_values = (char*)(m_nextLeaf + 1);

    m_overflow = new char[sizeof(unsigned long) + typeSize];
    m_overflowID = (unsigned long*)m_overflow;
    m_overflowValue = (char*)(m_overflowID + 1);
  }

  BPTreeNode::~BPTreeNode(){
    delete[](m_bytes);
    delete[](m_overflow);
  }

  void BPTreeNode::InsertValue(Attribute& value, size_t index){
    if(index > *m_valueCount || index > m_maxValueCount){
      throw out_of_range("BPTreeNode::InsertValue(Attribute&, size_t): "
                         "Index is out of range.");
    }
    else{
      //Append overflow?
      if (index == m_maxValueCount){
        memcpy(m_overflowValue, (char*)&value, m_typeSize);
      }
      //Append element?
      else if (index == *m_valueCount){
        memcpy(m_values + (index * m_typeSize), (char*)&value, m_typeSize);
      }
      else{
        char* begin,
            * end;

        //Move overflow?
        if (*m_valueCount >= m_maxValueCount){
          begin = m_values + (index * m_typeSize) + m_typeSize - 1;
          end = m_values + ((m_maxValueCount - 1) * m_typeSize) +
                m_typeSize - 1;

          memcpy(m_overflowValue, end - m_typeSize + 1, m_typeSize);
        }
        //Normal case
        else{
          begin = m_values + (index * m_typeSize) + m_typeSize - 1,
          end = m_values + ((*m_valueCount) * m_typeSize) + m_typeSize - 1;
        }

        //Move values
        for (char* current = end; current > begin; current--){
          *current = *(current - m_typeSize);
        }

        //Insert value
        memcpy(begin - m_typeSize + 1, (char*)&value, m_typeSize);
      }
    }

    (*m_valueCount)++;
  }
  void BPTreeNode::InsertValue(Attribute& value, unsigned long id,
                               size_t index){
    InsertValue(value, index);
    InsertIDAt(index, id);
  }
  void BPTreeNode::InsertValues(size_t index, Attribute** const values,
                                size_t count){
  if(index > *m_valueCount || index > m_maxValueCount){
    throw out_of_range("BPTreeNode::InsertValue(Attribute&, size_t): "
                       "index is out of range.");
  }
  else if((*m_valueCount) + count > m_maxValueCount + 1){
    throw out_of_range("BPTreeNode::InsertValue(Attribute&, size_t): "
                       "count is out of range.");
  }
  else if (count > 0){
    //Move values if neccessary
    if (index < *m_valueCount){
      //How many values are we supposed to move?
      size_t moveCount = (*m_valueCount) - index;

      //Move overflow
      if (*m_valueCount + count > m_maxValueCount){
        memcpy(m_overflowValue, m_values + (((*m_valueCount) - 1) * m_typeSize),
               m_typeSize);
        moveCount--;
      }

      //Move whats left
      memmove(m_values + ((index + count) * m_typeSize),
              m_values + (index * m_typeSize),
              moveCount * m_typeSize);
    }

    //Copy all except the last value
    char* target = m_values + (index * m_typeSize);
    for (size_t i = 0; i < count - 1; i++){
      memcpy(target, (char*)values[i], m_typeSize);
      target += m_typeSize;
    }

    //Copy the last value
    if (index + count > m_maxValueCount){
      memcpy(m_overflowValue, (char*)values[count - 1], m_typeSize);
    }
    else{
      memcpy(target, (char*)values[count - 1], m_typeSize);
    }

    (*m_valueCount) += count;
  }
}

  void BPTreeNode::RemoveValueAt(size_t index){
    if (index >= *m_valueCount){
      throw out_of_range("BPTreeNode::RemoveValueAt(size_t): "
                         "Index is out of range.");
    }
    else{
      (*m_valueCount)--;

      //Move values if not at end
      if (index < *m_valueCount){
        char* begin = m_values + (index * m_typeSize),
          * end;

        //Overflow?
        if (*m_valueCount == m_maxValueCount){
          end = m_values + ((m_maxValueCount - 1) * m_typeSize);

          for (char* current = begin; current < end; current++){
            *current = *(current + m_typeSize);
          }

          memcpy(end, m_overflowValue, m_typeSize);
        }
        //Normal case
        else{
          end = m_values + ((*m_valueCount) * m_typeSize);

          for (char* current = begin; current < end; current++){
            *current = *(current + m_typeSize);
          }
        }
      }
    }
  }

  void BPTreeNode::RemoveValues(size_t index, size_t count){
    if (index >= *m_valueCount){
      throw out_of_range("BPTreeNode::RemoveValues(size_t, size_t): "
                         "index is out of range.");
    }
    else if(index + count > *m_valueCount){
      throw out_of_range("BPTreeNode::RemoveValues(size_t, size_t): "
                         "count is out of range.");
    }
    else if (count > 0){
      //How many values are we supposed to move?
      size_t moveCount = (*m_valueCount) - (index + count);

      //Is moving necessary?
      if (moveCount > 0){
        //Move all except the last value
        memmove(m_values + (index * m_typeSize),
                m_values + ((index + count) * m_typeSize),
                (moveCount - 1) * m_typeSize);

        //Copy the last value
        if (*m_valueCount > m_maxValueCount){
          memcpy(m_values + ((index + moveCount - 1) * m_typeSize),
                 m_overflowValue, m_typeSize);
        }
        else{
          memcpy(m_values + ((index + moveCount - 1) * m_typeSize),
                 m_values + (((*m_valueCount) - 1) * m_typeSize), m_typeSize);
        }
      }

      (*m_valueCount) -= count;
    }
  }

  void BPTreeNode::InsertIDAt(size_t index, unsigned long id){
    size_t maxIndex = *m_isLeaf ? (*m_valueCount) -1 : *m_valueCount;
    if (index > maxIndex){
      throw out_of_range("BPTreeNode::InsertIDAt(size_t, unsigned long): "
                         "Index is out of range.");
    }
    else{
      unsigned long* begin,
        * end;

      if (*m_valueCount > m_maxValueCount){
        if (index == maxIndex){
          //Insert value
          begin = m_overflowID;
          end = begin;
        }
        else{
          begin = m_ids + index;
          end = *m_isLeaf ? m_ids + m_maxValueCount - 1 :
                            m_ids + m_maxValueCount;

          *m_overflowID = *end;
        }
      }
      else{
        begin = m_ids + index,
        end = m_ids + maxIndex;
      }

      for (unsigned long * current = end; current > begin; current--){
        *current = *(current - 1);
      }

      *begin = id;
    }
  }
  void BPTreeNode::RemoveIDAt(size_t index){
    size_t maxIndex = *m_isLeaf ? (*m_valueCount) -1 : *m_valueCount;
    if (index > maxIndex){
      throw out_of_range("BPTreeNode::RemoveIDAt(size_t): "
                         "Index is out of range.");
    }
    else if (index < maxIndex){
      unsigned long* begin = m_ids + index,
        * end;

      //Overflow?
      if (*m_valueCount > m_maxValueCount){
        end = m_ids + maxIndex - 1;

        for (unsigned long* current = begin; current < end; current++){
          *current = *(current + 1);
        }

        *end = *m_overflowID;
      }
      //Normal case
      else{
        end = m_ids + maxIndex;

        for (unsigned long* current = begin; current < end; current++){
          *current = *(current + 1);
        }
      }
    }
  }

  void BPTreeNode::InsertIds(size_t index, unsigned long* const ids,
                             size_t count){
    size_t maxIndex = *m_isLeaf ? *m_valueCount : *m_valueCount + 1,
           maxIDCount = (*m_isLeaf) ? m_maxValueCount : m_maxValueCount + 1;
    if(index > maxIndex || index > maxIDCount){
      throw out_of_range("BPTreeNode::InsertValue(Attribute&, size_t): "
                         "index is out of range.");
    }
    else if(index + count > maxIDCount + 1){
      throw out_of_range("BPTreeNode::InsertValue(Attribute&, size_t): "
                         "count is out of range.");
    }
    else if (count > 0){
      //Move values if neccessary
      if (index < maxIndex){
        //How many values are we supposed to move?
        size_t moveCount = maxIndex - index;

        //Is there enough space for all of them?
        if (index + count + moveCount > maxIDCount + 1){
          moveCount = maxIDCount + 1 - (index + count);
        }

        //Move the last value
        //Overflow?
        if (index + count + moveCount > maxIDCount){
          //Move overflow
          *m_overflowID = m_ids[index + moveCount - 1];
        }
        else{
          m_ids[index + count + moveCount - 1] = m_ids[index + moveCount - 1];
        }

        //Move all except the last value
        memmove(m_ids + index + count, m_ids + index,
                (moveCount - 1) * sizeof(unsigned long));
      }

      //Copy all except the last value
      memcpy(m_ids + index, ids, (count - 1) * sizeof(unsigned long));

      //Copy the last value
      //Overflow?
      if (index + count > maxIDCount){
        *m_overflowID = ids[count - 1];
      }
      else{
        m_ids[index + count - 1] = ids[count - 1];
      }
    }
  }
  void BPTreeNode::RemoveIds(size_t index, size_t count){
    size_t maxIndex = *m_isLeaf ? (*m_valueCount) - 1 : *m_valueCount;
    if (index > maxIndex){
      throw out_of_range("BPTreeNode::RemoveValues(size_t, size_t): "
                         "Index is out of range.");
    }
    else if(index + count - 1 > maxIndex){
      throw out_of_range("BPTreeNode::RemoveValues(size_t, size_t): "
                         "Count is out of range.");
    }
    else if (count > 0 && index + count - 1 < maxIndex){
      size_t moveCount = maxIndex - (index + count - 1);

      //Move all except the last id
      memmove(m_ids + index, m_ids + index + count,
              (moveCount - 1) * sizeof(unsigned long));

      if (*m_valueCount > m_maxValueCount){
        *(m_ids + index + moveCount - 1) = *m_overflowID;
      }
      else{
        *(m_ids + index + moveCount - 1) = *(m_ids + maxIndex);
      }
    }
  }

  Attribute& BPTreeNode::GetValueAt(size_t index){
    if (index >= *m_valueCount){
      throw out_of_range("BPTreeNode::GetValueAt(size_t): "
                         "Index is out of range.");
    }
    else{
      //Overflow
      if (index == m_maxValueCount){
        return *(Attribute*)m_valueCast((void*)m_overflowValue);
      }
      //Normal case
      else{
        return *(Attribute*)m_valueCast((void*)&m_values[index * m_typeSize]);
      }
    }
  }
  void BPTreeNode::SetValueAt(size_t index, Attribute& value){
    if (index >= *m_valueCount){
      throw out_of_range("BPTreeNode::SetValueAt(size_t, Attribute&): "
                         "Index is out of range.");
    }
    else{
      char* sourceBytes = (char*)&value,
        * targetBytes;

      //Overflow?
      if (index == m_maxValueCount){
        targetBytes = m_overflowValue;
      }
      //Normal case
      else{
        targetBytes = m_values + (index * m_typeSize);
      }

      for (size_t i = 0; i < m_typeSize; i++){
        *targetBytes = *sourceBytes;
        targetBytes++;
        sourceBytes++;
      }
    }
  }

  unsigned long BPTreeNode::GetIDAt(size_t index){
    if (*m_isLeaf){
      if (index >= *m_valueCount){
        throw out_of_range("BPTreeNode::GetIDAt(size_t): "
                           "Index is out of range.");
      }
      else{
        return index == m_maxValueCount ? *m_overflowID : *(m_ids + index);
      }
    }
    else{
      if (index > *m_valueCount){
        throw out_of_range("BPTreeNode::GetIDAt(size_t): "
                           "Index is out of range.");
      }
      else{
        return index > m_maxValueCount ? *m_overflowID : *(m_ids + index);
      }
    }
  }
  void BPTreeNode::SetIDAt(size_t index, unsigned long id){
    if (*m_isLeaf){
      if (index >= *m_valueCount){
        throw out_of_range("BPTreeNode::SetIDAt(size_t, unsigned long): "
                           "Index is out of range.");
      }
      else if(index == m_maxValueCount){
        *m_overflowID = id;
      }
      else{
        m_ids[index] = id;
      }
    }
    else{
      if (index > *m_valueCount){
        throw out_of_range("BPTreeNode::SetIDAt(size_t, unsigned long): "
                           "Index is out of range.");
      }
      else if(index > m_maxValueCount){
        *m_overflowID = id;
      }
      else{
        m_ids[index] = id;
      }
    }
  }

  unsigned long BPTreeNode::GetPrevLeaf(){
    return *m_prevLeaf;
  }
  void BPTreeNode::SetPrevLeaf(unsigned long pageNumber){
    (*m_prevLeaf) = pageNumber;
  }

  unsigned long BPTreeNode::GetNextLeaf(){
    return *m_nextLeaf;
  }
  void BPTreeNode::SetNextLeaf(unsigned long pageNumber){
    (*m_nextLeaf) = pageNumber;
  }

  size_t BPTreeNode::GetValueCount(){
    return *m_valueCount;
  }
  size_t BPTreeNode::GetMaxValueCount(){
    return m_maxValueCount;
  }

  size_t BPTreeNode::GetTypeSize(){
    return m_typeSize;
  }
  char* BPTreeNode::GetBytes(){
    return m_bytes;
  }

  bool BPTreeNode::IsLeaf(){
    return *m_isLeaf;
  }
  size_t BPTreeNode::GetPageNumber(){
      return m_pageNumber;
  }
  void BPTreeNode::SetPageNumber(size_t pageNumber){
      m_pageNumber = pageNumber;
  }

  void BPTreeNode::Print(std::ostream& o) {
    o << "' (values:(";
    for(size_t i = 0; i < GetValueCount(); i++) {
      if(i != 0) {
         o << ", ";
      }
      // Datentypen, die von Attribute erben, haben die Methode Print, die
      // sie als Objekt vom Typ ostream (= mit Typ char parametrisierte
      // Template-Klasse basic_ostream) zurückgibt.
      // Annahme: der korrekte Cast (mittels Klasse Algebramanager) wurde
      // bereits in der Methode GetValueAt durchgeführt.
      GetValueAt(i).Print(o);
    }
    o << "), ids:(";
    for(size_t i = 0; i < GetValueCount(); i++) {
      if(i != 0) {
         o << ", ";
      }
      // Datentypen, die von Attribute erben, haben die Methode Print, die
      // sie als Objekt vom Typ ostream (= mit Typ char parametrisierte
      // Template-Klasse basic_ostream) zurückgibt.
      // Annahme: der korrekte Cast (mittels Klasse Algebramanager) wurde
      // bereits in der Methode GetValueAt durchgeführt.
      o << GetIDAt(i);
    }
    if (!IsLeaf()){
      o << ", ";
      o << GetIDAt(GetValueCount());
    }
    o << ")) '";
  }

  string BPTreeNode::ToString(){
    std::ostringstream o;

    o << "(";

    Print(o);

    o << "(Id: " << GetPageNumber();

    if (IsLeaf()){
      o << " Prev: " << GetPrevLeaf() << " Next: " << GetNextLeaf();
    }

    o << " ValueCount: " << GetValueCount() << "))";

    return o.str();
  }
}
