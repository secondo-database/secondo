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
    ostream& operator << (ostream& s, RegionTreeNode *RTN)
    {  
      if(RTN == NULL)
      {
         s << "NULL" <<endl;
         return s;
      }     
         if(dynamic_cast<Face*> (RTN))
         {
            Face* tmp2 = (Face*) RTN;
            s << "Face: " << tmp2->hashCode() << endl;            
         }
         else
         { 
            if((dynamic_cast<RegionForInterpolation*> (RTN)))
            {
                  RegionForInterpolation* tmp2 = (RegionForInterpolation*) RTN;
                  s << "Region: " << tmp2->hashCode() << endl;             
            }
            else
            {              
               if((dynamic_cast<ConvexHullTreeNode*> (RTN)))
               {
                     ConvexHullTreeNode* tmp2 = (ConvexHullTreeNode*) RTN;
                     if(!tmp2->isHole())
                        s << "CHTN: " << tmp2->hashCode() << endl;
                     else
                           s << "Hole: " << tmp2->hashCode() << endl;
               }
            }                          
         }
      return s;
    }
}



