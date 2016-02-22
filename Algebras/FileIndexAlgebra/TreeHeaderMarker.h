/*
enum representing the header marker of a Tree-Index file

*/

#ifndef TREEHEADERMARKER_H
#define TREEHEADERMARKER_H

namespace fialgebra{

  /* marker showing the type of tree
   * within an index file*/
  enum TreeHeaderMarker
  {
    BPlusTree = 0,
    Rtree = 1
  }; 
}

#endif // TREEHEADERMARKER_H
