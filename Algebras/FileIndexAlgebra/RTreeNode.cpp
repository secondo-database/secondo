/*
class representing nodes in an R Tree

*/

#include "RTreeNode.h"
#include <exception>
#include <utility>

#include "BPTree/BPTree.h"
#include "WinUnix.h"
#include <vector>
#include <stack>
#include <math.h>
#include <cstring>
#include "RTree.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"

using std::out_of_range;
using std::runtime_error;

//#define RTreeNodeMax 2

namespace fialgebra{
  template <int dim>
  size_t RTreeNode<dim>::constantSize = sizeof(bool) + sizeof(size_t);
  template <int dim>
  size_t RTreeNode<dim>::typeSize = SizeOfRectangle<dim>();

  template <int dim>
  RTreeNode<dim>::RTreeNode(size_t pageSize, size_t id, bool isLeaf) :
    m_box(NULL){
    if (pageSize < typeSize + sizeof(size_t) + constantSize){
      throw out_of_range("RTreeNode::RTreeNode(size_t, size_t, bool): "
                         "page size too small for a entry");
    }

    m_box = NULL;
    nodeID = id;
    m_pageSize = pageSize;

    m_max = GetMax(pageSize);

    m_bytes = new char[pageSize];
    m_isLeaf = (bool*)m_bytes;
    m_numberOfEntries = (size_t *)(m_isLeaf + 1);
    m_values = (char*)(m_numberOfEntries + 1);
    m_ids = (size_t*)(m_values + (m_max * typeSize));

    *m_isLeaf = isLeaf;
    *m_numberOfEntries = 0;
  }

  template <int dim>
  RTreeNode<dim>::RTreeNode(char* bytes, size_t pageSize, size_t id) :
    m_box(NULL){
    if (pageSize < typeSize + sizeof(size_t) + constantSize){
      throw out_of_range("RTreeNode::RTreeNode(size_t, size_t, bool): "
                         "page size too small for a entry");
    }

    m_box = NULL;
    nodeID = id;
    m_pageSize = pageSize;

    m_max = GetMax(pageSize);

    m_pageSize = pageSize;
    m_bytes = bytes;
    m_isLeaf = (bool*)m_bytes;
    m_numberOfEntries = (size_t *)(m_isLeaf + 1);
    m_values = (char*)(m_numberOfEntries + 1);
    m_ids = (size_t*)(m_values + (m_max * typeSize));
  }

  template <int dim>
  RTreeNode<dim>::RTreeNode(const RTreeNode<dim>& o){
    m_box = o.m_box != NULL ? new Rectangle<dim>(*o.m_box) : NULL;
    nodeID = o.nodeID;
    m_pageSize = o.m_pageSize;
    m_max = o.m_max;

    m_bytes = new char[m_pageSize];
    memcpy(m_bytes, o.m_bytes, m_pageSize);

    m_isLeaf = (bool*)m_bytes;
    m_numberOfEntries = (size_t *)(m_isLeaf + 1);
    m_values = (char*)(m_numberOfEntries + 1);
    m_ids = (size_t*)(m_values + (m_max * typeSize));

    for(Overflow* overflow : o.m_overflow){
      m_overflow.push_back(new Overflow(overflow->rectangle, overflow->id));
    }
  }

  template <int dim>
  RTreeNode<dim>::~RTreeNode(){
    delete[] m_bytes;
    
    if ( m_box ) {
      delete m_box;
      m_box = 0;
    } // if
  }

  template <int dim>
  void RTreeNode<dim>::AddEntry( const Rectangle<dim>& value, size_t id ){
    if (m_max <= *m_numberOfEntries){
      m_overflow.push_back(new Overflow(value, id));
    }
    else{
      memcpy(m_values + ((*m_numberOfEntries) * typeSize), (char*)&value,
             typeSize);

      m_ids[*m_numberOfEntries] = id;
    }

    (*m_numberOfEntries)++;

    if (m_box != NULL){
      delete(m_box);
      m_box = NULL;
    }
  }

  template <int dim>
  void RTreeNode<dim>::RemoveEntryAtEnd(){
    if (*m_numberOfEntries <= 0){
      throw out_of_range("RTreeNode::RemoveEntryAtEnd()");
    }
    else{
      if (m_max < *m_numberOfEntries){
        delete(m_overflow.back());
        m_overflow.pop_back();
      }

      (*m_numberOfEntries)--;

      if (m_box != NULL){
        delete(m_box);
        m_box = NULL;
      }
    }
  }

  template <int dim>
  void RTreeNode<dim>::RemoveEntryAt(size_t index){
    if (index >= *m_numberOfEntries){
      throw out_of_range("RTreeNode::RemoveEntryAt(size_t)");
    }
    else{
      if (*m_numberOfEntries <= m_max){
        if (index < (*m_numberOfEntries - 1)){
          memmove(m_values + (index * typeSize),
                  m_values + ((index + 1) * typeSize),
                  ((*m_numberOfEntries) - index - 1) * typeSize);

          for (size_t i = index; i < *m_numberOfEntries - 1; i++){
            m_ids[i] = m_ids[i + 1];
          }
        }
      }
      else{
        if (index < m_max){
          memmove(m_values + (index * typeSize),
                  m_values + ((index + 1) * typeSize),
                  (m_max - index - 1) * typeSize);

          for (size_t i = index; i < m_max - 1; i++){
            m_ids[i] = m_ids[i + 1];
          }

          Overflow* overflow = m_overflow[0];
          m_overflow.erase(m_overflow.begin());

          memcpy(m_values + ((m_max - 1) * typeSize),
                 (char*)&overflow->rectangle, typeSize);
          m_ids[m_max - 1] = overflow->id;

          delete(overflow);
          overflow = NULL;
        }
        else{
          delete(m_overflow[index - m_max]);
          m_overflow.erase(m_overflow.begin() + index - m_max);
        }
      }

      (*m_numberOfEntries)--;

      if (m_box != NULL){
        delete(m_box);
        m_box = NULL;
      }
    }
  }

  template <int dim>
  void RTreeNode<dim>::ClearEntries(){
    *m_numberOfEntries = 0;

    for(Overflow* overflow : m_overflow){
      delete(overflow);
    }

    m_overflow.clear();

    if (m_box != NULL){
      delete(m_box);
      m_box = NULL;
    }
  }

  template <int dim>
  const Rectangle<dim>& RTreeNode<dim>::GetValueAt(size_t index) const{
    if (index >= *m_numberOfEntries){
      throw out_of_range("RTreeNode::GetValueAt(size_t)");
    }
    else{
      if (index >= m_max){
        return m_overflow[index - m_max]->rectangle;
      }
      else{
        return *(Rectangle<dim>*)CastRectangle<dim>(m_values +
                                                    (index * typeSize));
      }
    }
  }

  template <int dim>
  void RTreeNode<dim>::SetValueAt(size_t index, const Rectangle<dim>& value){
    if (index >= *m_numberOfEntries){
      throw out_of_range("RTreeNode::SetValueAt(size_t, "
                         "const Rectangle<dim>&)");
    }
    else{
      if (index >= m_max){
        m_overflow[index - m_max]->rectangle = value;
      }
      else{
        memcpy(m_values + (index * typeSize), (char*)&value, typeSize);
      }

      if (m_box != NULL){
        delete(m_box);
        m_box = NULL;
      }
    }
  }


  template <int dim>
  size_t RTreeNode<dim>::GetIDAt(size_t index) const{
    if (index >= *m_numberOfEntries){
      throw out_of_range("RTreeNode::GetIDAt(size_t)");
    }
    else{
      return index >= m_max ? m_overflow[index - m_max]->id : m_ids[index];
    }
  }

  template <int dim>
  void RTreeNode<dim>::SetIDAt(size_t index, size_t id){
    if (index >= *m_numberOfEntries){
      throw out_of_range("RTreeNode::SetIDAt(size_t, size_t)");
    }
    else{
      if (index >= m_max){
        m_overflow[index - m_max]->id = id;
      }
      else{
        m_ids[index] = id;
      }
    }
  }


  template <int dim>
  size_t RTreeNode<dim>::GetNumberOfEntries() const{
    return *m_numberOfEntries;
  }

  template <int dim>
  size_t RTreeNode<dim>::GetMax(){
    return m_max;
  }

  template <int dim>
  size_t RTreeNode<dim>::GetMax(size_t pageSize){
#ifdef RTreeNodeMax
    return RTreeNodeMax;
#else
    return (pageSize - constantSize)/(typeSize + sizeof(size_t));
#endif
  }

  template <int dim>
  char* RTreeNode<dim>::GetBytes(){
    return m_bytes;
  }

  template <int dim>
  bool RTreeNode<dim>::IsLeaf(){
    return *m_isLeaf;
  }

  template <int dim>
  size_t RTreeNode<dim>::GetNodeID() const{
    return nodeID;
  }

  template <int dim>
  void RTreeNode<dim>::SetNodeID(size_t id){
    nodeID = id;
  }

  template <int dim>
  const unsigned long RTreeNode<dim>::GetParentNodeID() const{
    return parentNodeID;
  }

  template <int dim>
  void RTreeNode<dim>::SetParentNodeID(unsigned long id){
     parentNodeID = id;
  }

  template <int dim>
  const Rectangle<dim>& RTreeNode<dim>::GetBox(){
    if (m_box == NULL){
      if (*m_numberOfEntries > 0){
        m_box = new Rectangle<dim>(GetValueAt(0));

        for(size_t i = 1; i < *m_numberOfEntries; i++){
          *m_box = m_box->Union(GetValueAt(i));
        }
      }
      else{
        m_box = new Rectangle<dim>(false);
      }
    }

    return *m_box;
  }


  template<int dim>
  RTreeNode<dim>& RTreeNode<dim>::operator=(const RTreeNode<dim>& o){
    if (m_box != NULL){
      delete(m_box);
    }
    m_box = o.m_box != NULL ? new Rectangle<dim>(*o.m_box) : NULL;
    m_pageSize = o.m_pageSize;
    m_max = o.m_max;

    m_bytes = new char[m_pageSize];
    memcpy(m_bytes, o.m_bytes, m_pageSize);

    m_isLeaf = (bool*)m_bytes;
    m_numberOfEntries = (size_t *)(m_isLeaf + 1);
    m_values = (char*)(m_numberOfEntries + 1);
    m_ids = (size_t*)(m_values + (m_max * typeSize));

    for(Overflow* overflow : o.m_overflow){
      m_overflow.push_back(new Overflow(overflow->rectangle, overflow->id));
    }

    return *this;
  }

  template<int dim>
  bool RTreeNode<dim>::operator<(const RTreeNode<dim>& o){
    return *m_numberOfEntries < *o.m_numberOfEntries;
  }

  template<int dim>
  bool RTreeNode<dim>::operator==(const RTreeNode<dim>& o){
    return *m_numberOfEntries == *o.m_numberOfEntries;
  }

  template<int dim>
  RTreeNode<dim>::Overflow::Overflow(const Rectangle<dim>& rectangle,
                                     const size_t id){
    this->rectangle = rectangle;
    this->id = id;
  }

  template<int dim>
  void RTreeNode<dim>::PrintNodeToString(){
    if(!IsLeaf()){
      cout<<"Interne Knote: ";
      for(size_t i = 0; i < GetNumberOfEntries(); i++){
         cout<<"EntryNumber: "<<i+1<<"; EntryPageNumber: "<<GetNodeID()<<
         "; Entry: "<<GetValueAt(i)<<"   ";
       }
       cout<<"\n";
    }
    else{
      cout<<"Blatt: ";
      for(size_t i = 0; i < GetNumberOfEntries(); i++){
         cout<<"IDNumber: "<<i+1<<"; EntryPageNumber: "<<GetNodeID()<<
         "; ID: "<<GetIDAt(i)<<"   ";
       }
       cout<<"\n";
    }
}

  template class RTreeNode<1>;
  template class RTreeNode<2>;
  template class RTreeNode<3>;
  template class RTreeNode<4>;
  template class RTreeNode<8>;
}//end of namespace fialgebra
