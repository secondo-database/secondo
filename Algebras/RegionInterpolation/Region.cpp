/*
 
 see Region.h for documentation
 
\tableofcontents
 
*/

#include "RegionInterpolator.h"

using namespace std;

namespace RegionInterpol
{

/*
   
1 Constructors and Destructor
 
*/    
   
RegionForInterpolation :: RegionForInterpolation()
{     
}

RegionForInterpolation :: RegionForInterpolation(Region *newRegion)
{
   vector<vector<vector<HalfSegment*> > > tmpvec;
   newRegion->SelectFirst();
   HalfSegment *tmp;
   while(!newRegion->EndOfHs())
   {
      tmp= new HalfSegment();       
      newRegion->GetHs(*tmp);
      newRegion->SelectNext();
      if(tmp->GetAttr().faceno >= (int)tmpvec.size())
      {
         tmpvec.resize(tmp->GetAttr().faceno + 1);          
      }
      
      if(tmp->GetAttr().cycleno >= (int)tmpvec[tmp->GetAttr().faceno].size())
      {
         tmpvec[tmp->GetAttr().faceno].resize((tmp->GetAttr().cycleno + 1));
      }           
      if(tmp->GetAttr().edgeno >= (int) tmpvec[tmp->GetAttr().faceno] 
         [tmp->GetAttr().cycleno].size())
      {
         tmpvec[tmp->GetAttr().faceno] [tmp->GetAttr().cycleno].resize(
            (tmp->GetAttr().edgeno + 1));
      }
      if(tmpvec[tmp->GetAttr().faceno] [tmp->GetAttr().cycleno] 
         [tmp->GetAttr().edgeno] != NULL) 
      {
        delete tmpvec[tmp->GetAttr().faceno] [tmp->GetAttr().cycleno] 
                     [tmp->GetAttr().edgeno];
      }
      tmpvec[tmp->GetAttr().faceno] [tmp->GetAttr().cycleno] 
               [tmp->GetAttr().edgeno] = (HalfSegment*) tmp;
   }    

   for(unsigned int i = 0; i < tmpvec.size(); i++)
   {       
     Face *newFace;
     for(unsigned int j = 0; j < tmpvec[i].size(); j++)
     {
        LineWA linelist[tmpvec[i] [j].size()];
        linelist[0] = LineWA(tmpvec[i] [j] [0]->GetDomPoint().GetX(), 
            tmpvec[i] [j] [0]->GetDomPoint().GetY());
        Point nextP(tmpvec[i] [j] [0]->GetSecPoint());
        for(unsigned int k = 1; k < tmpvec[i] [j].size(); k++)
        {            
           if(nextP == tmpvec[i] [j] [k]->GetDomPoint())     
           {
             linelist[k] = LineWA(tmpvec[i] [j] [k]->GetDomPoint().GetX(), 
                 tmpvec[i] [j] [k]->GetDomPoint().GetY());
             nextP = tmpvec[i] [j] [k]->GetSecPoint();
//             cerr<<endl;
//             tmpvec[i] [j] [k]->GetDomPoint().Print(cerr);
           }
           else
           {
             linelist[k] = LineWA(tmpvec[i] [j] [k]->GetSecPoint().GetX(), 
                 tmpvec[i] [j] [k]->GetSecPoint().GetY());
             nextP = tmpvec[i] [j] [k]->GetDomPoint();
//             cerr<<endl;
//             tmpvec[i] [j] [k]->GetSecPoint().Print(cerr); 
           }
        }
#ifdef REG_DEBUG        
         cout<<"Cycle "<<j<<" has "<<tmpvec[i][j].size()<<" vertices"<<endl;
#endif         
         if(j == 0)        
            newFace = new Face( (LineWA*) linelist, 
               (int) tmpvec[i] [j].size(), this);
         else
         {
            newFace->addHole( (LineWA*) linelist, (int) tmpvec[i] [j].size());
         }              
      }
      this->addFace(newFace);
    }    
   
   for(unsigned int i = 0; i < tmpvec.size(); i++)
     for(unsigned int j = 0; j < tmpvec[i].size(); j++)
        for(unsigned int k = 0; k < tmpvec[i] [j].size(); k++)
           delete tmpvec[i] [j] [k];   
  setDirtyHash();    
}

RegionForInterpolation :: ~RegionForInterpolation()
{    
  for(vector<Face*>::iterator it=Faces.begin(); it!= Faces.end(); ++it)
    delete (*it);
}

/*

1 Get functions

1.1 getNrOfFaces()

*/              

int RegionForInterpolation :: getNrOfFaces()
{
    return(Faces.size());
}
/*
 
1.1 getFace()

*/     
Face *RegionForInterpolation :: getFace(int index)
{
    return(Faces.at(index));
}
/*
 
1.1 getFaces()

*/
vector<Face*> *RegionForInterpolation :: getFaces()
{
    return(&Faces);
}
/*
 
1.1 isDirtyHash()

*/
bool RegionForInterpolation :: isDirtyHash()
{
   return(dirtyHash);
}

/*

1 Set functions

1.1 addFace()

*/              
void RegionForInterpolation :: addFace(Face *newFace)
{
   newFace->setParent(this);
   Faces.push_back(newFace);
    setDirtyHash();
}
/*
 
1.1 removeFace()

*/
void RegionForInterpolation :: removeFace(int index)
{           
   Faces.erase(Faces.begin() + index);
   setDirtyHash();     
}
/*
 
1.1 setDirtyHash()

*/
void RegionForInterpolation :: setDirtyHash()
{
   dirtyHash = true;
}
   
    
/*

1 Public Methods

1.1 splitOnLine()

*/
  
vector<Face*> *RegionForInterpolation :: splitOnLine(vector<LineWA*> *splitLine)
{
    vector<Face*> *resv = new vector<Face*>;
    for(int i = 0; i < getNrOfFaces(); i++)
    {
        Face *tmp = getFace(i)->splitOnLine(splitLine);
        if(tmp != NULL)
        {            
            addFace(tmp);
            resv->push_back(tmp);
            resv->push_back(getFace(i));            
        }
    }    
    return(resv);        
} 
/*

1.1 clone()

*/ 
RegionForInterpolation *RegionForInterpolation :: clone()
{
   RegionForInterpolation *res = new RegionForInterpolation();
    for(int i = 0; i < getNrOfFaces(); i++)
    {
      Face *tmp = getFace(i)->clone();
      tmp->setParent(res);
      res->addFace(tmp);
    }
    return(res);  
}
/*
 
1 Overridden Methods

1.1 hashCode()

*/     
unsigned int RegionForInterpolation :: hashCode()
{
   if(isDirtyHash())
   {
      calculateHashCode();
   }  
   return(m_HashCode);
}
/*

1.1 equals()

*/ 
bool RegionForInterpolation :: equals(RegionTreeNode* other)
{     
   if(other->hashCode() != this->hashCode())
   {
      return(false);
   }
   if( !(dynamic_cast<RegionForInterpolation*>(other)))
   {
      return(false);
   }
   RegionForInterpolation* o = (RegionForInterpolation*) other;
   if(o->getNrOfFaces() != this->getNrOfFaces())
   {
      return(false);
   }
   for(int i = 0; i < this->getNrOfFaces(); i++)
   {
      if( !this->getFace(i)->equals( (RegionTreeNode*) o->getFace(i)))
      {
         return(false);
      }
   }
   return(true);
}
   
/*

1 Operators

1.1 $<<$

*/    

ostream& operator << (ostream& s,RegionForInterpolation region)
{
   s << "Region:" << endl;             
   for(int i = 0; i < region.getNrOfFaces(); i++)
   {
      s << "Face " << (i + 1) << " of " << region.getNrOfFaces();
      s << *(region.getFace(i));
   }  
   return s;
}
/*

1 Private Methods

1.1 calculateHashCode()

*/ 
void RegionForInterpolation :: calculateHashCode()
{
    unsigned int start = 44621;
    unsigned int res = start;                
    for(int i = 0; i < this->getNrOfFaces(); i++)
    {
        res += this->getFace(i)->hashCode();
    }  
    m_HashCode = res;
    dirtyHash = false;
}   

}
