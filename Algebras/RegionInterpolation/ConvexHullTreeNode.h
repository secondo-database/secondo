/*

1 ConvexHullTreeNode.h

*/

#ifndef CONVEXHULLTREENODE_H_
#define CONVEXHULLTREENODE_H_

namespace RegionInterpol
{
/*

1.1 class ConvexHullTreeNode

this class represents a node in a tree of convex hulls
 
*/ 
   class ConvexHullTreeNode:public RegionTreeNode
   {
      public:  
      
/*
1.1.1 Constructors and Destructor

empty constructor

*/   
         ConvexHullTreeNode();
/*

A constructor that gets a list of LineWAs an the node this one will be child of. Level and Hole are optional.

*/       
         ConvexHullTreeNode(LineWA linelist[], int linelistlength, 
            RegionTreeNode *myParent, int level, bool isHole);
/*

the destructor

*/                
         ~ConvexHullTreeNode();
/*
         
1.1.1 Get functions

functions that are used to return the stored Attributes

*/       
         int getLevel(void);
         bool isHole(void);
         RegionTreeNode *getParentNode();       
         CHLine getCHLine(int i);         
         int getNrLines();
         std::vector<CHLine> getLines();
/*
 
returns the coentroid or the Steiner--Point of the convex hull
 
*/       
         LineWA getCenter();
         LineWA getSteinerPoint();
/*
 
 returns the repesented polygone as a vector of LineWAs
 
*/                               
         std::vector<LineWA> getOutLine();
/*

returns all the children as a vector of ConvexHullTreeNodes

*/                      
         std::vector<ConvexHullTreeNode*> getChildren();
/*

returns the line the given $child$ has in comon with this node as a two elemet vector 

*/       
         std::vector<LineWA*> getLineForChild(ConvexHullTreeNode *child); 
/*

Is the cached hashcode valid?

*/       
         bool isDirtyHash();
/*             
1.1.1 Set functions

functions to set the attributes of the node

*/       
         void setParent(RegionTreeNode *myParent);
         void setLevel(int lev);
         void setHole(bool isHole);
/*

set the cached hashcode invalid

*/       
         void setDirtyHash();
/*          
 
1.1.1 Overridden Methods

*/    
         unsigned int hashCode(void);        
         bool equals(RegionTreeNode* other);
         
/* 

1.1.1 Public Methods

this Method calculates a splitline for this node using $ref12$ and $ref2$ as a hint
 
*/
         
         std::vector<LineWA*> *getSplitLine(ConvexHullTreeNode *ref1, 
            ConvexHullTreeNode *ref2);
/*
 
this method splits the node into two pieces

*/          
         std::vector<std::vector<LineWA> > 
                     *getSplitNodes(std::vector<LineWA*> *splitLine);
         
/*

Operators

*/
         friend std::ostream & operator <<(std::ostream & os, 
            const ConvexHullTreeNode chtn);    
      
      private:
/*

1.1.1 Private Methods

This function links a particular line in this object to a given
convex hull tree node, thereby making it a child of this node.

*/    
         void insertChild(int lineindex, ConvexHullTreeNode *child);
/*
This function inserts a new line at the end of the list of lines
defining the polygon represented by this object.

*/       
                      
          int insertLine(LineWA *line);          
          LineWA *getLine(int index);
/*

calculate the hashcode of the Node

*/        
          void calculateHashCode();
/*
1.1.1 Attributes

attributes for handling the hashcode

*/        
         bool dirtyHash;                     
         unsigned int m_HashCode;
/*

the level the Node has in the ConvexHullTree

*/       
         int level;
/*
 * 
the convex hull, represented by a list of CHLines

*/                                    
         std::vector<CHLine> linelist;
/*

is the node part of a hole?

*/          
         bool hole;
/*

the RegionTreeNode this node is child of

*/          
         RegionTreeNode *myParent;        
   };
}

#endif 

/*

\pagebreak

*/
