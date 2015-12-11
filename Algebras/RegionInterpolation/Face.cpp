/*
 
see Face.h for documentation
 
\tableofcontents
 
*/
#include "RegionInterpolator.h"
using namespace std;
    
namespace RegionInterpol
{   
   
/*

1 Constructors and Destructor

*/
   
Face :: Face()
{     
}
   
Face ::Face(LineWA linelist[],int linelistlength,RegionForInterpolation* parent)
{  
    Cycle = new ConvexHullTreeNode( (LineWA*) linelist, linelistlength, 
       (RegionTreeNode*) this, 0, false);        
    this->parent = parent;
    setDirtyHash();
}

Face  :: ~Face()
{
  delete Cycle;
  for(vector<ConvexHullTreeNode*>::iterator it=Holes.begin(); it!=
    Holes.end(); ++it)
    delete (*it);
}
    
/*

1 Get functions

1.1 getParent()

*/     
    
RegionForInterpolation *Face :: getParent()
{
    return(parent);
}
/*

1.1 getNrOfHoles()
 
*/    
int Face :: getNrOfHoles()
{
    return(Holes.size());
}
/*

1.1 getCycle()
 
*/        
ConvexHullTreeNode *Face :: getCycle()
{
    return(Cycle);
}
/*

1.1 getHole()
 
*/        
ConvexHullTreeNode *Face :: getHole(int index)
{
    return(Holes[index]);
}
/*

1.1 getHolesAndConcavities()
 
*/    
vector<ConvexHullTreeNode*> Face :: getHolesAndConcavities()
{
    vector<ConvexHullTreeNode*> res;
    vector<ConvexHullTreeNode*> cycleChildrean= getCycle()->getChildren(); 
    for(unsigned int i = 0; i < cycleChildrean.size(); i++)
    {
      res.push_back(cycleChildrean.at(i));        
    }
    for(int i = 0; i < getNrOfHoles(); i++)
    {
        res.push_back(getHole(i));
    }
    return(res);
}
/*

1.1 isDirtyHash()
 
*/    
bool Face :: isDirtyHash()
{
   return(dirtyHash);
}
/*

1.1 clone()
 
*/    
Face *Face :: clone()
{
   vector<CHLine> cyclelines = Cycle->getLines();
   LineWA cycle[cyclelines.size()];
   for(unsigned int i = 0; i < cyclelines.size(); i++)
   {
      cycle[i] = LineWA(cyclelines[i].getX(), cyclelines[i].getY());
   }
    Face *res = new Face(cycle, cyclelines.size(), getParent());
    for(int i = 0; i < getNrOfHoles(); i++)
    {
      vector<CHLine> holelines = getHole(i)->getLines();
      LineWA hole[holelines.size()];
      for(unsigned int i = 0; i < holelines.size(); i++)
      {
         hole[i] = LineWA(holelines[i].getX(), holelines[i].getY());
      }     
        res->addHole(hole, holelines.size());
    }
    return(res);
}

/*

1 Set functions

1.1 setParent()

*/
             
void Face :: setParent(RegionForInterpolation *parent)
{
    this->parent = parent;
    parent->setDirtyHash();
}
/*

1.1 removeHole()
 
*/ 
void Face :: removeHole(ConvexHullTreeNode *toDelete)
{
   for (unsigned int i = 0; i < Holes.size(); i++)
   {
      if(toDelete->equals(( (RegionTreeNode*) Holes[i])))
      {
         Holes.erase(Holes.begin() + i);
      }
   }
   setDirtyHash();
}
/*

1.1 addHole()
 
*/    
void Face :: addHole(ConvexHullTreeNode *newHole)
{
   if(!newHole->isHole())        
      newHole->setHole(true);
   if(newHole->getLevel() != 1)
      newHole->setLevel(1);
   newHole->setParent(this);
    Holes.push_back(newHole);
    setDirtyHash();
}

void Face :: addHole(LineWA linelist[], int linelistlength)
{
   ConvexHullTreeNode *newHole= new ConvexHullTreeNode(linelist,linelistlength,
      (RegionTreeNode*) this, 1, true);
    Holes.push_back(newHole);
    setDirtyHash();
}
/*

1.1 setDirtyHash()
 
*/     
void Face :: setDirtyHash()
{
   if(parent != NULL)
   {
      parent->setDirtyHash();
   }
   dirtyHash = true;
}
/*

1.1 splitOnLine()
 
*/    
Face *Face :: splitOnLine(vector<LineWA*> *splitLine)
{
    vector<vector<LineWA> > *resSplit = getCycle()->getSplitNodes(splitLine);
    if(resSplit == NULL || resSplit->at(1).size() == 0)
        return(NULL);
    Region cycle1 = *Utils :: convert2Region(
       Utils :: convertLineWA2CHLine(&resSplit->at(0)));
    Region cycle2 = *Utils :: convert2Region(
       Utils :: convertLineWA2CHLine(&resSplit->at(1)));
    vector<ConvexHullTreeNode*> stillHoles;
    vector<vector<LineWA> > newCons;
    for(int i = 0; i <  getNrOfHoles(); i++)
    {
        vector<vector<LineWA> > *tmpHole = getHole(i)->getSplitNodes(splitLine);
        if(tmpHole == NULL || tmpHole->at(1).size() == 0)
            stillHoles.push_back(getHole(i));
        else
        {
            newCons.push_back(tmpHole->at(0));
            newCons.push_back(tmpHole->at(1));
        }
    }           
    for(unsigned int i = 0; i < newCons.size(); i++)
    {
        vector<LineWA> holePoints = newCons[i];
        int j = 0;
        while(cycle1.OnBorder(Point(true, holePoints[j].getX(), 
           holePoints[j].getY())))
        {
            j++;
        }
        Point test = Point(true, holePoints[j].getX(), holePoints[j].getY());
        if(cycle1.Contains(Point(true, holePoints[j].getX(), 
           holePoints[j].getY())))
        {
            resSplit->at(0)=*Utils::joinLinelists(&resSplit->at(0),&holePoints);
        }
        else
        {
            resSplit->at(1)=*Utils::joinLinelists(&resSplit->at(1),&holePoints);
        }
    }    
    Holes.clear();
    LineWA *ll1 = new LineWA[resSplit->at(0).size()];
    LineWA *ll2 = new LineWA[resSplit->at(1).size()];
    for(unsigned int i = 0; i < resSplit->at(0).size(); i++)
    {
      ll1[i] = resSplit->at(0)[i];
    }
    for(unsigned int i = 0; i < resSplit->at(1).size(); i++)
    {
      ll2[i] = resSplit->at(1)[i];
    }    
    Cycle = new ConvexHullTreeNode(ll1, resSplit->at(0).size(), this, 0, false);
    Face *res = new Face(ll2, resSplit->at(1).size(), getParent());
    for(unsigned int i = 0; i < stillHoles.size(); i++)
    {
        vector<CHLine> holePoints = stillHoles[i]->getLines();
        int j = 0;
        while(cycle1.OnBorder(Point(true, holePoints[j].getX(), 
           holePoints[j].getY())))
        {
            j++;
        }
        if(cycle1.Contains(Point(true, holePoints[j].getX(), 
           holePoints[j].getY())))
        {
         stillHoles[i]->setParent(this);
            addHole(stillHoles[i]);                
        }
        else
        {
         stillHoles[i]->setParent(res);
            res->addHole(stillHoles[i]);
        }
    }    
    return(res);
}    
/*

1 Overrides

1.1 hashCode()

*/    
     
unsigned int Face :: hashCode()
{  
   if(this->isDirtyHash())
   {
      this->calculateHashCode();
   }
   return(m_HashCode);
}
/*

1.1 calculateHashCode()
 
*/     
void Face :: calculateHashCode()
{
    unsigned int start = 13424;
    unsigned int res = start;    
    res += Cycle->hashCode();
    for(int i = 0; i < this->getNrOfHoles(); i++)
    {
        res += this->getHole(i)->hashCode();
    }
    m_HashCode = res;
    dirtyHash = false;
}
/*

1.1 equals()
 
*/      
bool Face :: equals(RegionTreeNode* o)
{
   if(o->hashCode() != this->hashCode())
   {
      return(false);
   }
    if(dynamic_cast<Face*>(o))
    {
        Face *tmp = (Face*) o;
        if(getNrOfHoles() != tmp->getNrOfHoles())
            return(false);
        bool res = Cycle->equals(tmp->getCycle());
        for(int i = 0; i < getNrOfHoles(); i++)
        {
            res = res && getHole(i)->equals(tmp->getHole(i));
        }
        return(res);
    }
    else
    {
      return false;
    }     
}
    
/*

1 Operators

1.1 $<<$

*/    
    

ostream& operator << (ostream& s,Face face)
{
   s << "Face:" << endl;
   s << "Hash: " << face.hashCode() << endl;
   s << "Cycle:" << endl;
   s << *(face.getCycle()) << endl;;
   for(int i = 0; i < face.getNrOfHoles(); i++)
   {
      s << "Hole " << (i + 1) << " von " << face.getNrOfHoles() << endl;
      s << *(face.getHole(i)) << endl;
   }  
   return s;
}

}


