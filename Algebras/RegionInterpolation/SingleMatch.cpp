/*
 
 see SingleMatch.h for documentation
 
 \tableofcontents
 
*/

#include "RegionInterpolator.h"

namespace RegionInterpol
{
/*
 
1 Constructors and Destructor
 
*/    
SingleMatch :: SingleMatch(RegionTreeNode *src, RegionTreeNode *target)
{
   source = src;
   targets.push_back(target);
}
/*

1 Get functions

1.1 getSource()

*/   
RegionTreeNode *SingleMatch :: getSource()
{
    return(source);
}
/*

1.1 getNrTargets()

*/  
int SingleMatch :: getNrTargets()
{
    int res = 0;
    for(unsigned int i = 0; i < targets.size(); i++)
    {
        if(targets[i] != NULL)
        {
            res++;
        }
    }
    return(res);
}
/*

1.1 getTargetAt()

*/  
RegionTreeNode *SingleMatch :: getTargetAt(int i)
{
    return(targets[i]);
}
/*

1 Set functions

1.1 addTarget()

*/     
void SingleMatch :: addTarget(RegionTreeNode *target)
{
   bool found = false;
   for(unsigned int i = 0; i < targets.size(); i++)
   {     
      if(target->equals(targets[i]))
      {
         found = true;
         break;
      }
   }
   if(!found)
      targets.push_back(target);
}
/*

1.1 removeTarget()

*/     
void SingleMatch :: removeTarget(int index)
{
   targets.erase(targets.begin() + index);  
}
/*

1.1 removeTargets()

*/
void SingleMatch :: removeTargets()
{
   targets.clear();
}
/*

1.1 removeNulls()

*/
void SingleMatch :: removeNulls()
{
    for(unsigned int i = 0; i < targets.size(); i++)
    {
        if(targets[i] == NULL)
        {
            removeTarget(i);
        }
    }
}
/*

1 Public Methods

1.1 hashCode()

*/
int SingleMatch :: hashCode()
{
    return(source->hashCode());
}
/*

1.1 equals()

*/
bool SingleMatch :: equals(SingleMatch* o)
{            
   return(source->equals(o->getSource()));        
}
/*

1 Operators

1.1 $<<$

*/  
std::ostream& operator << (std::ostream& s, SingleMatch sm)
{
   s << "Match:" << std::endl;  
   s << sm.getSource() << "===============>" << std::endl;
   for(int i = 0; i < sm.getNrTargets(); i++)
   {
      s << sm.getTargetAt(i);
      if(i != sm.getNrTargets() - 1)
         s << "... ... ..." << std::endl;         
   }        
   s << std::endl;
   return s;
}  

}

