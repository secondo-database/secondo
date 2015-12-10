/*
 
1 SingleMatch.h

*/

#ifndef SINGLEMATCH_H_
#define SINGLEMATCH_H_


namespace RegionInterpol
{  
/*

1.1 SingleMatch

This class represents a single part of a match. It contains a source and a target which are both RegionTreeNodes
    
*/
   class SingleMatch
   {
      public:
/*
      
1.1.1 Constructors and Destructor

This constructor sets the source and the target
 
*/             
         SingleMatch(RegionTreeNode *source,RegionTreeNode *target);
/*

1.1.1 Get Functions

Functions to get the source, the number of targets and a special target identified by its index.

*/             
         RegionTreeNode *getSource();
         int getNrTargets();
         RegionTreeNode *getTargetAt(int i);
/*
         
1.1.1 Set functions

functions to add a target ti the singlematch, remove a special target, remove all targets and a function to remove the targets that are null

*/
         void addTarget(RegionTreeNode *target);
         void removeTarget(int index);
         void removeTargets();
         void removeNulls();
/*
         
1.1.1 Public Methods 

this function returns the hashcode of the source

*/          
         int hashCode();               
         bool equals(SingleMatch* o);
/*

1.1.1 Operators

*/          
         friend std::ostream & operator <<(std::ostream & os,
                      const SingleMatch sm);
      private:
/*
      
1.1.1 Attributes

the source RegionTreeNode

*/    
         RegionTreeNode *source;
/*
 
 a field of RegionTreeNodes that are the targets
 
*/        
         std::vector<RegionTreeNode*> targets;       
   };
}
#endif 
/*

\pagebreak

*/

