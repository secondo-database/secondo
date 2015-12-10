/*
 
1 Face.h

*/

#ifndef FACE_H_
#define FACE_H_
namespace RegionInterpol
{
   
/*


~just a forward declaration~

*/ 
   class RegionForInterpolation;
   
/*
 
1.1 class Face

The Face-Class is a representation of a face witch uses $ConvexHulTrees$. A Face has got one $cycle$ and $holes$.
The parent of a $face$ is a $RegionForInterpolation$. 


*/ 
   class Face : public RegionTreeNode 
   {   
      public:
/*
 
1.1.1 Constructors and Destructor

empty constructor

*/    
         Face();
/*
 
constructor that sets the $parent$-Region and the $Cycle$, given by a $linelist$

*/ 
         Face(LineWA linelist[], int linelistlength, 
            RegionForInterpolation *parent);
/*
 
The destructor

*/
         ~Face();
/*
           
1.1.1 Get functions

returns a pointer on the $Parent-Region$

*/
         RegionForInterpolation *getParent();
/*

returns the Number of $holes$ stored in this $face$         
                        
*/                      
         int getNrOfHoles();
/*

returns the cycle or a special hole by the given index 
 
*/       
         ConvexHullTreeNode *getCycle();
         ConvexHullTreeNode *getHole(int index);
/*
 * 
returns all the Concavities of the cycle and the Holes of this face

*/       
         std::vector<ConvexHullTreeNode*> getHolesAndConcavities();
/*

Is the cached hashcode valid?

*/              
         bool isDirtyHash();
/*

get a deep clone of the face

*/       
         Face *clone();
/*

1.1.1 Set functions

set the parent region
 
*/
               
         void setParent(RegionForInterpolation *parent);
/*
  
remove or add a hole given as ConvexHullTreeNode or as list of LineWAs and the length of this list. 
 
*/                   
         void removeHole(ConvexHullTreeNode *toDelete);
         void addHole(ConvexHullTreeNode *newHole);
         void addHole(LineWA linelist[],int linelistlength);
/*
 
splits the face in two new faces using the given linelist. The one new face ist given back, the other takes the place of the old one.
 If the split fails, $null$ will be given back an the face remains unchanged.

*/       
         
         Face *splitOnLine(std::vector<LineWA*> *splitLine);
/*

set the cached hashcode invalid

*/ 
         
            void setDirtyHash();
/*          
 
1.1.1 Overridden Methods

*/                         
         unsigned int hashCode();
         bool equals(RegionTreeNode* other);
/*
         
1.1.1 Operators

*/          
      friend std::ostream & operator << (std::ostream & os,const Face face);
          
       private:
/*
 
 1.1.1 Private Methods
 
 Calculate the hashcode of the region
 
*/     
       void calculateHashCode();                
/*
  
 1.1.1 Attributes
 
 the cycle and the holes of the face both given as ConvexHullTrees
 
*/  
         ConvexHullTreeNode *Cycle;
         std::vector<ConvexHullTreeNode*> Holes;
/*
 
 the region the face is child of
  
*/       
         RegionForInterpolation *parent;
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
