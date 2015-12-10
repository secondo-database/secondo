/*

1 RegionTreeNode.h

*/

#ifndef REGIONTREENODE_H_
#define REGIONTREENODE_H_

namespace RegionInterpol
{
/*

1.1 class RegionTreeNode


This interface serves to be able to treat region, Faces and ConvexHullTreeNodes the same.

*/
   class RegionTreeNode
   {
      public :
         virtual ~RegionTreeNode(){};
/*
 
1.1.1  Virtal Functions 

this functions set the hashcode invalid or answers the question if the hashvalue is valid

*/             
         virtual void setDirtyHash()=0;   
         virtual bool isDirtyHash()=0; 
/*
 
returns the hashvalue of a region tree. If the caches hashvalue is valid then it is returned, otherwise it is calculated.   
 
*/       
         virtual unsigned int hashCode()=0;        
         virtual bool equals(RegionTreeNode* other)=0;
         friend std::ostream& operator <<(std::ostream & os,
                                          RegionTreeNode *RTN);       
   };
}
#endif 
/*

\pagebreak

*/
