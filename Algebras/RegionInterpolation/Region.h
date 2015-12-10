/*
 
1 Region.h

*/

#ifndef REGION_H_
//#define REGION_H_
#include <set>

namespace RegionInterpol
{
/*

1.1 class Region

A region contains several faces. The name region is used by a class in the  SpatialAlgebra, 
so that this class musthave been calledRegionForInterpolation.

*/ 
   class RegionForInterpolation:public RegionTreeNode
   {         
      public:
/*
        
1.1.1 Constructors and Destructor
 
The empty constructor

*/       
         RegionForInterpolation();
/*

this constructor converts a SpatialAlgebra--Region in a RegionForInterpolation

*/        
         RegionForInterpolation(Region *newRegion);
/*

The Destructor

*/        
         ~RegionForInterpolation();
/*
          
1.1.1 Get functions

this functions return the number of stored faces, a list of all faces or a special face identified by its index

*/                 
         int getNrOfFaces();     
         Face *getFace(int index);     
         std::vector<Face*> *getFaces();
/*

Is the cached hashcode valid?

*/       
         bool isDirtyHash();
/*
          
1.1.1 Set functions

these functions add a given face or removes one by index

*/        
         void addFace(Face *newFace);
         void removeFace(int index);
/*

set the cached hashcode invalid

*/          
         void setDirtyHash();
/*
          
1.1.1 Public Methods



This function tries to split all faces along the given splitline and adds the new ones to this region.The function delivers a list all added faces, 
 
*/       
         std::vector<Face*> *splitOnLine(std::vector<LineWA*> *splitLine);
/*

get a deep clone of the face

*/ 
         RegionForInterpolation* clone();
               
/*          
 
1.1.1 Overridden Methods

*/                    
         unsigned int hashCode();
         bool equals(RegionTreeNode* other);
/*
         
1.1.1 Operators

*/             
         friend std::ostream & operator <<(std::ostream & os,
            const RegionForInterpolation region);       
                    
       private:
/*
 
 1.1.1. Private Methods
 
*/              
         void calculateHashCode();
/*
         
1.1.1 Attributes
    
The list of faces in the region   

*/
         std::vector<Face*> Faces;
/*

attributes for handling the hashcode

*/           
         bool dirtyHash;
         unsigned int m_HashCode;   

      };
}
#endif
/*

\pagebreak

*/

