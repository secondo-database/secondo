/*
class representing nodes in a R Tree

*/

#ifndef RTREENODE_H
#define RTREENODE_H

#include "Attribute.h"
#include <vector>
#include <utility>
#include "RectangleAlgebra.h"
#include "WinUnix.h"


namespace fialgebra{

  template<int dim> class RTreeNode;

  template <int dim>
  class RTreeNode{
  public:
    RTreeNode(size_t pageSize, size_t id, bool isLeaf);
    RTreeNode(char* bytes, size_t pageSize, size_t id);
    ~RTreeNode();

    //Copy constructor (for pair<>)
    RTreeNode(const RTreeNode<dim>& o);

    void AddEntry(const Rectangle<dim>& value, size_t id);
    void RemoveEntryAtEnd();
    void RemoveEntryAt(size_t index);
    void ClearEntries();

    const Rectangle<dim>& GetValueAt(size_t index) const;
    void SetValueAt(size_t index, const Rectangle<dim>& value);

    size_t GetIDAt(size_t index) const;
    void SetIDAt(size_t index, size_t id);

    size_t GetNumberOfEntries() const;

    size_t GetMax();
    static size_t GetMax(size_t pageSize);

    bool IsLeaf();

    char* GetBytes();

    Rectangle<dim>* getRectangle();

    size_t GetNodeID() const;
    void SetNodeID(size_t id);

    const unsigned long GetParentNodeID() const;
    void SetParentNodeID(unsigned long id);

    void Print(std::ostream& o);

    //Assignment operator overload
    RTreeNode<dim>& operator=(const RTreeNode<dim>& o);

    // overloaded < Operator
    bool operator < (const RTreeNode<dim>& o);

    // overloaded == Operator
    bool operator == (const RTreeNode<dim>& o);

    const Rectangle<dim>& GetBox();

    void PrintNodeToString();

  private:
    struct Overflow{
      Rectangle<dim> rectangle;
      size_t id;

      Overflow(const Rectangle<dim>& rectangle, const size_t id);
    };

   static size_t constantSize,
                 typeSize;

   Rectangle<dim>* m_box;

   char* m_bytes,
       * m_values;

   size_t* m_ids;

   bool* m_isLeaf;
   size_t* m_numberOfEntries;

   std::vector<Overflow*> m_overflow;

   unsigned long nodeID,
     parentNodeID;
   size_t m_pageSize;

   size_t m_max;
  };
}

#endif
