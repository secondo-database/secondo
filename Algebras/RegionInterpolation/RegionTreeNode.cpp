/*
  
see RegionTreeNode.h for documentation
 
\tableofcontents
 
*/
#include "RegionInterpolator.h"

namespace RegionInterpol 
{
/*

1 Operators

1.1 $<<$

*/
    std::ostream& operator << (std::ostream& s, RegionTreeNode *RTN)
    {  
      if(RTN == NULL)
      {
         s << "NULL" <<std::endl;
         return s;
      }     
         if(dynamic_cast<Face*> (RTN))
         {
            Face* tmp2 = (Face*) RTN;
            s << "Face: " << tmp2->hashCode() << std::endl;            
         }
         else
         { 
            if((dynamic_cast<RegionForInterpolation*> (RTN)))
            {
                  RegionForInterpolation* tmp2 = (RegionForInterpolation*) RTN;
                  s << "Region: " << tmp2->hashCode() << std::endl;             
            }
            else
            {              
               if((dynamic_cast<ConvexHullTreeNode*> (RTN)))
               {
                     ConvexHullTreeNode* tmp2 = (ConvexHullTreeNode*) RTN;
                     if(!tmp2->isHole())
                        s << "CHTN: " << tmp2->hashCode() << std::endl;
                     else
                           s << "Hole: " << tmp2->hashCode() << std::endl;
               }
            }                          
         }
      return s;
    }
}



