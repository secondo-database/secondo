/*
 
 see ConvexHullTreeNode.h for documentation
 
 \tableofcontents
 
*/
 
#include "RegionInterpolator.h"

using namespace std;

namespace RegionInterpol 
{

/* 

1 Constructors and Destructor

*/ 
ConvexHullTreeNode :: ConvexHullTreeNode()
{
   level=0;;
   hole=false;
   myParent=NULL;
   m_HashCode=0;
   setDirtyHash();
}

ConvexHullTreeNode :: ConvexHullTreeNode(LineWA linelistParam[], 
    int linelistlength, RegionTreeNode* myParent, 
    int level=0, bool isHole=false)
{  
   ConvexHullTreeNode();
   LineWA *tmplist, *childlist;
   vector<LineWA*> convhull;
   int index1, index2, length, lastindex, noiterations;
   int indexll1, indexll2;
   this->level = level;
   this->hole = isHole;
   this->myParent = myParent; 
   if ((level == 0 && !isHole) || (level == 1 && isHole))
   {
      double area = Utils :: getArea(linelistParam, linelistlength);
#ifdef DECHTN     
      cout << area << endl;
#endif
      if (area < 0)
      {
#ifdef DECHTN        
         cout<<"drehe"<<endl;
#endif         
         LineWA tmp;
         for (int i = 0; i < ((int)(linelistlength /2)); ++i)
         {
           tmp= linelistParam[i];
           linelistParam[i]= linelistParam[linelistlength-1-i];
           linelistParam[linelistlength-1-i]= tmp;
         }
      }
   }
   // Create temporary list so that convexHull() won't change the order of
   // points in the original linelistParam.
   tmplist = new LineWA[linelistlength];  
   for (int a = 0; a < linelistlength; a++)
   {
      tmplist[a] = linelistParam[a];
   }
 
   // Find the convex hull 
   convhull = Utils :: convexHull(tmplist, linelistlength);
   // Find where the first node in linelist is in the convex hull
   // (Or the earliest possible if the first node is not on the hull)
   // This is to preserve the order in the linelist in the ordering of
   // points in the convex hull tree node.
   index1 = 0;
   lastindex = convhull.size();
   for (int a = 0; a < linelistlength; a++)
   {     
      index1 = Utils :: indexOf(convhull, linelistParam[a]);
      index1 = Utils :: indexOf(convhull, linelistParam[a]);
      if (index1 != -1)
         break;
   }
#ifdef DECHTN  
   for (int a=0; a<linelistlength; a++)
   {
      cout<<linelistParam[a];
   }
   cout<<"const CHTN4"<<endl;
#endif   
   for (int a = linelistlength - 1; a >= 0; a--)
   {
      lastindex = Utils :: indexOf(convhull, linelistParam[a]);
      if (lastindex != -1)
         break;
   }  
   if (lastindex > index1)
   {
      for (int a=index1; a <= lastindex; a++)
      {
         insertLine(convhull[a]);
      }
   } 
   
   else
   {
      for (unsigned int a=index1; a < convhull.size(); a++)
      {
         insertLine(convhull[a]);
      }
      for (int a = 0; a <= lastindex; a++)
      {
         insertLine(convhull[a]);
      }
   }
   delete[] tmplist;
   // Check lines in convex hull with lines in line list. Whenever two points
   // which are neighbours in the convex hull are not neighbours in the line
   // list, create a child node from the points between them.
   noiterations = this->getNrLines();
   index1 = 0;
   indexll1 = 0;
   while ( !(linelistParam[indexll1].equals(getLine(index1))) )
   {
      indexll1++;
   }  
   for (int a = 0; a < noiterations; a++)
   {
      index2 = index1+1;
      indexll2 = indexll1+1;     
      while ( !(linelistParam[indexll2 % linelistlength].
         equals(getLine(index2 % noiterations))) )
      {           
         indexll2++;
      }
      if ((indexll2 != indexll1 + 1) && (((level == 0) && !isHole) || 
         ((level == 1) && isHole) || 
         ((indexll2 != linelistlength) && (indexll1 != linelistlength - 1))) && 
         (indexll2 != indexll1 - 1))
      {
         // create child node
         length = indexll2 - indexll1 + 1;
         childlist = new LineWA[length];
         for (int b = 0; b < length; b++)
         {
            if ((b + indexll1) < linelistlength)
            {
               childlist[length-b-1] = linelistParam[b + indexll1];
            } 
            else
            {
               childlist[length-b-1] = linelistParam[b+indexll1-linelistlength];
            }
         }
         insertChild(index1, 
            new ConvexHullTreeNode(childlist, length, this, level + 1, hole));
      }
      index1 = index2;
      indexll1 = indexll2;
   }
   this->dirtyHash=true;   
}

ConvexHullTreeNode :: ~ConvexHullTreeNode()
{
   myParent = NULL;
}

/*

1 Get functions

1.1 getLevel()

*/    

int ConvexHullTreeNode :: getLevel()
{
   return (level);
}
   
/*

1.1 isHole()
 
*/   
bool ConvexHullTreeNode :: isHole()
{
   return(hole);
}
/*

1.1 getNrLines()
 
*/   
int ConvexHullTreeNode :: getNrLines()
{
   return(linelist.size());
}
/*

1.1 getCHLine()
 
*/
CHLine ConvexHullTreeNode :: getCHLine(int i)
{
   return(linelist[i]);
}
/*

1.1 getCenter()
 
*/
LineWA ConvexHullTreeNode :: getCenter()
{
    double resx = 0;
    double resy = 0;
    for(unsigned int i = 0; i < linelist.size(); i++)
    {
        resx += linelist.at(i).getX();
        resy += linelist.at(i).getY();
    }
    resx = resx / (linelist.size());
    resy = resy / (linelist.size());
    LineWA res(resx,resy);
    return res;
}
/*

1.1 getSteinerPoint()
 
*/    
LineWA ConvexHullTreeNode :: getSteinerPoint()
{
    double resx = 0;
    double resy = 0;
    for(unsigned int i = 0; i < linelist.size(); i++)
    {
      CHLine curr = linelist.at(i);
      CHLine prev = linelist.at((i - 1 + linelist.size()) % linelist.size());
      CHLine next = linelist.at((i + 1) % linelist.size());
        double angle = Utils :: getAngleRad(curr.getX(), curr.getY(), 
           prev.getX(), prev.getY(), next.getX(), next.getY());
        double weight = 0.5 - (angle / 2 / M_PI);
        resx += (int)(linelist.at(i).getX() * weight);
        resy += (int)(linelist.at(i).getY() * weight);
    }
    LineWA res(resx, resy);
    return res;
}
/*

1.1 getLines()
 
*/    
vector<CHLine> ConvexHullTreeNode :: getLines()
{
   vector<CHLine> listoflines, tmplist;
   CHLine tmpline;
   int nolines, nolines2;     
    nolines = linelist.size();
   for (int a = 0; a < nolines; a++)
   {
      tmpline = linelist.at(a);
      listoflines.insert(listoflines.begin(), 1, tmpline);
      if (tmpline.getChild() != NULL)
      {        
         tmplist = tmpline.getChild()->getLines();
         nolines2 = tmplist.size() - 1;
         for (int b = 1; b < nolines2; b++)
         {
            listoflines.insert(listoflines.begin(), 1, tmplist.at(b));
         }
      }
   }
   return (listoflines);
}
/*

1.1 getOutLine()
 
*/    
vector<LineWA> ConvexHullTreeNode::getOutLine()
{
    vector<LineWA> result ;
    for(unsigned int i = 0; i < linelist.size(); i++)
    {
      result.push_back( (LineWA) linelist.at(i));
    }
    return(result);
}
/*

1.1 getChildren()
 
*/       
vector<ConvexHullTreeNode*> ConvexHullTreeNode :: getChildren()
{                
    CHLine line;
    vector<ConvexHullTreeNode*> result;
    for (int a = 0; a < this->getNrLines(); a++)
    {
        line = this->getCHLine(a);
        if (line.getChild() != NULL)
        {
            result.push_back(line.getChild());
        }
    }
    return(result);
}
/*

1.1 getParentNode()
 
*/        
RegionTreeNode *ConvexHullTreeNode :: getParentNode()
{
   return(this->myParent);
}
/*

1.1 getLineForChild()
 
*/    
vector<LineWA*> ConvexHullTreeNode :: getLineForChild(ConvexHullTreeNode *child)
{
    int length;
    CHLine line;
    vector<LineWA*> result(2);
    length = linelist.size();
    for (int a = 0; a < length; a++)
    {
        line = linelist[a]; 
        if (line.getChild() != NULL && line.getChild()->equals(child))
        {            
            result[0] = (LineWA*) &linelist[a];
            if (a != length - 1)
            {
                result[1] = (LineWA*) &linelist[a + 1];
            }
            else
            {
                result[1] = (LineWA*) &linelist[0];
            }
            return(result);
        }
    }
    return(result);
}    
/*

1.1 isDirtyHash()
 
*/        
bool ConvexHullTreeNode :: isDirtyHash()
{
   return(dirtyHash);
}    
    
/*

1 Set functions

1.1 setParent()

*/

void ConvexHullTreeNode :: setParent(RegionTreeNode *_myParent)
{  
    myParent = _myParent;
    myParent->setDirtyHash();
}
/*
 
1.1  setLevel()
 
*/    
void ConvexHullTreeNode :: setLevel(int lev)
{
   level = lev;
    vector<ConvexHullTreeNode*> children = this->getChildren();
    for(unsigned int i = 0; i < children.size(); i++)
    {
        children.at(i)->setLevel(lev + 1);
    }
    //delete children;
    setDirtyHash();
}
/*
 
1.1  setHole()
 
*/                
void ConvexHullTreeNode :: setHole(bool _isHole)
{
    hole = _isHole;
    vector<ConvexHullTreeNode*> children = this->getChildren();
    for(unsigned int i = 0; i < children.size(); i++)
    {
        children.at(i)->setHole(_isHole);
    }
    //delete children;;
    setDirtyHash();
}
/*
 
1.1  setDirtyHash()
 
*/    
void ConvexHullTreeNode :: setDirtyHash()
{
   dirtyHash = true;
   if(myParent != NULL)
   {
      myParent->setDirtyHash();
   }
}

/*
 
1 Overrides
 
1.1 hashCode() 

*/ 

unsigned int ConvexHullTreeNode :: hashCode()
{  
   assert( !( !isDirtyHash() && m_HashCode == 0));
   if(isDirtyHash())
   {
      calculateHashCode();
   }
   return(m_HashCode);
}
/*
 
1.1 calculateHashCode()
 
*/ 
void ConvexHullTreeNode::calculateHashCode()
{
   unsigned int start = 5132;
   unsigned int multi = 312;
   unsigned int res = start;
   vector<CHLine> tmp = this->getLines();
   for (unsigned int i = 0; i < tmp.size(); i++)
   {
      res += (unsigned int)(res+tmp[i].getX() * multi + tmp[i].getY() * multi);
   }
   res += level * 201;
   if(hole) 
      res += 113; 
   m_HashCode = res;
   dirtyHash = false;
}
/*
 
1.1 equals()
 
*/    
bool ConvexHullTreeNode :: equals(RegionTreeNode* other)
{     
   if(other->hashCode() != this->hashCode())
   {
      return(false);
   }
    if(dynamic_cast<ConvexHullTreeNode*>(other))
    {
        bool res = true;
        ConvexHullTreeNode *tmp = (ConvexHullTreeNode*) other;
        vector<CHLine> tmp1 = this->getLines();
        vector<CHLine> tmp2 = tmp->getLines();
        if(tmp1.size() != tmp2.size())
            return false;
        for(unsigned int i = 0; i < tmp1.size(); i++)
        {
            if(tmp1[i].getX() != tmp2[i].getX())
                res = false;
            if(tmp1[i].getY() != tmp2[i].getY())
                res = false;
        }
        return(res);
    }
    else
    {
      return(false);
    }
}
   
/*

1 Operators

1.1 $<<$

*/

ostream & operator << (ostream& s, ConvexHullTreeNode chtn)
{        
   if(chtn.getLevel() == 0)
   {        
      if(chtn.isHole())
      {
         s << "CHTN is hole" << endl;
         s << "Hash: " << chtn.hashCode() << endl;
      }
      else
      {
         s << "CHTN is not a hole" << endl;
         s << "Hash: " << chtn.hashCode() << endl;
      }
      for(int i = 0; i < chtn.getNrLines(); i++)
      {           
         s << "I" << setw(3) << setfill('-') << i << 
            setfill(' ') << ": " << chtn.getCHLine(i);
         if(chtn.getCHLine(i).getChild() != NULL)        
         {           
            s << *(chtn.getCHLine(i).getChild());
         }
      }
      s << endl;
   }       
   else
   {
      s << "I" << setw(3 * chtn.getLevel() - 1) << setfill('-') << "" << 
         setw(3) << chtn.hashCode() << endl;
      for(int i = 0; i < chtn.getNrLines(); i++)
      {     
         s << "I" << setw(3 * chtn.getLevel() - 1) << setfill('-') << "" << 
            setw(3) << i << setfill(' ') << ": " << chtn.getCHLine(i);       
         s << setfill(' ');
         if(chtn.getCHLine(i).getChild() != NULL)
         {           
            s << *(chtn.getCHLine(i).getChild());
         }
      }
   }            
   return s;
}

/*

1 Private Methods

1.1 insertChild()

*/
      
void ConvexHullTreeNode :: insertChild(int lineindex, ConvexHullTreeNode *child)
{
    CHLine *line= &linelist.at(lineindex);        
    line->setChild(child);
}
/*

1.1 insertLine()
 
*/    
int ConvexHullTreeNode :: insertLine(LineWA* line)
{
    CHLine newline = (CHLine)line;
    linelist.insert(linelist.end(), newline);
    return(linelist.size() - 1);
}
/*

1.1 getLine()
 
*/    
LineWA* ConvexHullTreeNode :: getLine(int index)
{
    return(&linelist.at(index));
}        
/*

1.1 getSplitLine()
 
*/    
vector<LineWA*>* ConvexHullTreeNode :: getSplitLine(ConvexHullTreeNode *ref1, 
   ConvexHullTreeNode *ref2)
{
    LineWA p1 = ref1->getCenter();
    LineWA p2 = ref2->getCenter();
    vector<LineDist> pdist1;
    vector<LineDist> pdist2;
    vector<LineWA*> *res =new vector<LineWA*>;
    vector<CHLine> ref1lines = ref1->getLines();
    vector<CHLine> ref2lines = ref2->getLines();
    for (unsigned int i = 0; i < ref1lines.size(); i++)
    {
        double dist = Utils :: getRectangularDistance(&p1, &p2, 
           (LineWA*) &ref1lines[i]);        
        if( !isnan(dist))
            pdist1.push_back(LineDist( (LineWA*) &ref1lines[i], dist));
    }    
    for (unsigned int i = 0; i < ref2lines.size(); i++)
    {
        double dist = Utils :: getRectangularDistance(&p1, &p2, 
           (LineWA*) &ref2lines[i]);
        if(!isnan(dist))
            pdist2.push_back( LineDist( (LineWA*) &ref2lines[i], dist));
    }    
    sort(pdist1.begin(), pdist1.end());
    sort(pdist2.begin(), pdist2.end());    
    while(pdist1.size() > 1 || pdist2.size() > 1)
    {
        if(pdist1.size() == 0 || pdist2.size() == 0)
        {            
            return(res);
        }
        LineDist tmp1 = pdist1[0];
        LineDist tmp2 = pdist2[0];
        vector<LineWA*> *inter1=Utils::getIntersections(new LineWA(tmp1.getX(), 
           tmp1.getY()), new LineWA(tmp2.getX(), tmp2.getY()), &ref1lines);
        vector<LineWA*> *inter2=Utils::getIntersections(new LineWA(tmp1.getX(), 
           tmp1.getY()), new LineWA(tmp2.getX(), tmp2.getY()), &ref2lines);
        if(inter1->size() <= 2 && inter2->size() <= 2)
            res->push_back(new LineWA((tmp1.getX() + tmp2.getX()) / 2.0, 
               (tmp1.getY() + tmp2.getY()) / 2.0));        
        if(tmp1.getDistance() > tmp2.getDistance())
        {
            if(pdist2.size() > 1)
            {
                pdist2.erase(pdist2.begin());
            }
            else
            {
                while(pdist1.size() > 1)
                {
                    tmp1=pdist1[0];
                    tmp2=pdist2[0];
                    if(((Utils :: getIntersections(new LineWA(tmp1.getX(), 
                          tmp1.getY()), new LineWA(tmp2.getX(), tmp2.getY()), 
                          &ref1lines)->size()) <= 2)
                       &&(Utils :: getIntersections(new LineWA(tmp1.getX(), 
                          tmp1.getY()), new LineWA(tmp2.getX(), tmp2.getY()), 
                          &ref2lines)->size()) <= 2)
                    {
                        res->push_back(new LineWA((tmp1.getX()+tmp2.getX())/2.0,
                           (tmp1.getY() + tmp2.getY()) / 2.0));
                    }
                    pdist1.erase(pdist1.begin());
                }
            }
        }
        else
        {            
            if(pdist1.size() > 1)
            {
                pdist1.erase(pdist1.begin());
            }
            else
            {
                while(pdist2.size() > 1)
                {
                    tmp1 = pdist1[0];
                    tmp2 = pdist2[0];
                    res->push_back(new LineWA((tmp1.getX()+tmp2.getX())/2.0,
                       (tmp1.getY()+tmp2.getY())/2.0));
                    pdist2.erase(pdist2.begin());
                }
            }
        }
        
    }
    if(pdist1.size()==0 || pdist2.size()==0)
    {
      return(new vector<LineWA*>);
    }
    LineDist tmp1 = pdist1[0];
    LineDist tmp2 = pdist2[0];
    if(((Utils :: getIntersections(new LineWA(tmp1.getX(), tmp1.getY()), 
          new LineWA(tmp2.getX(), tmp2.getY()), &ref1lines)->size()) <= 2) &&
          (Utils :: getIntersections(new LineWA(tmp1.getX(), tmp1.getY()), 
          new LineWA(tmp2.getX(), tmp2.getY()), &ref2lines)->size()) <= 2)
    {
        res->push_back(new LineWA((tmp1.getX() + tmp2.getX()) / 2.0, 
           (tmp1.getY() + tmp2.getY()) / 2.0));
    }
    double sumLinex = 0;
    double sumLiney = 0;
    for(unsigned int i = 0; i < res->size(); i++)
    {
        sumLinex += res->at(i)->getX();
        sumLiney += res->at(i)->getY();
    }
    LineWA centerLine = LineWA(sumLinex/(res->size()), sumLiney/(res->size()));
    double sumRefx = 0;
    double sumRefy = 0;
    for(unsigned int i = 0; i < ref1lines.size(); i++)
    {
        sumRefx += ref1lines[i].getX();
        sumRefy += ref1lines[i].getY();
    }
    for(unsigned int i = 0; i < ref2lines.size(); i++)
    {
        sumRefx += ref2lines[i].getX();
        sumRefy += ref2lines[i].getY();
    }
    LineWA centerRef = LineWA(sumRefx / (ref1lines.size() + ref2lines.size()), 
       sumRefy / (ref1lines.size() + ref2lines.size()));        
    double scaleVector = abs(Utils :: getArea(getLines())) / 
       (abs(Utils :: getArea(ref1lines)) + abs(Utils :: getArea(ref2lines)));
    double distLine = sqrt((centerLine.getX() - (res->at(0))->getX()) * 
       (centerLine.getX() - (res->at(0))->getX()) + (centerLine.getY() - 
       (res->at(0))->getY()) * (centerLine.getY() - (res->at(0))->getY()));
    distLine = max(distLine, sqrt((centerLine.getX() - 
       (res->at(res->size() - 1))->getX()) * (centerLine.getX() - 
       (res->at(res->size() - 1))->getX()) + (centerLine.getY() - 
       (res->at(res->size() - 1))->getY()) * (centerLine.getY() - 
       (res->at(res->size() - 1))->getY())));
    double thismaxdist = 0;
    LineWA centerThis = getCenter();
    vector<CHLine> thisLines = getLines();
    for(unsigned int i = 0; i< thisLines.size(); i++)
    {
        thismaxdist = max(thismaxdist, sqrt((thisLines[i].getX() - 
           centerThis.getX()) * (thisLines[i].getX() - centerThis.getX()) + 
           (thisLines[i].getY() - centerThis.getY()) * (thisLines[i].getY() -
           centerThis.getY())));
    }
    double scale = thismaxdist / distLine * 1.05;    
    for(unsigned int i = 0; i < res->size(); i++)
    {                
        res->at(i)->setX(centerThis.getX() + (centerLine.getX() - 
           centerRef.getX()) * scaleVector + (res->at(i)->getX() - 
           centerLine.getX()) * scale);                
        res->at(i)->setY(centerThis.getY() + (centerLine.getY() - 
           centerRef.getY()) * scaleVector + (res->at(i)->getY() - 
           centerLine.getY()) * scale);        
    }        
    return res;
}

vector<vector<LineWA> > *ConvexHullTreeNode :: getSplitNodes(
   vector<LineWA*> *splitLine)
{
    if(splitLine == NULL || splitLine->size() == 0)
        return(NULL);
   vector<vector<LineWA> > *res = new vector<vector<LineWA> > (2);
    int lowIndexLine, highIndexLine, lowIndexPoly, highIndexPoly;
    vector<LineWA*> IntersectionPoints;
    vector<int> IntersectionIndexPoly;
    vector<int> IntersectionIndexLine;
    vector<CHLine> polyLine = getLines();
    for(unsigned int i = 0; i < polyLine.size(); i++)
    {
        for(unsigned int j = 0; j < (splitLine->size() - 1); j++)
        {
            LineWA *inters = Utils :: getIntersection( (LineWA*) 
               &(polyLine[i]), (LineWA*) &(polyLine[(i + 1)%polyLine.size()]), 
               splitLine->at(j),splitLine->at(j + 1));
            if(inters != NULL)
            {
                IntersectionPoints.push_back(inters);
                IntersectionIndexPoly.push_back(i);
                IntersectionIndexLine.push_back(j);
            }
        }
    }
    if(IntersectionPoints.size() != 2)
    {
        return(NULL);
    }
    else
    {        
        if(IntersectionIndexPoly[0] > IntersectionIndexPoly[1])
        {
            lowIndexPoly = IntersectionIndexPoly[1];
            highIndexPoly = IntersectionIndexPoly[0];
        }
        else
        {
            lowIndexPoly = IntersectionIndexPoly[0];
            highIndexPoly = IntersectionIndexPoly[1];
        }
        if(IntersectionIndexLine[0] > IntersectionIndexLine[1])
        {
            lowIndexLine = IntersectionIndexLine[1];
            highIndexLine = IntersectionIndexLine[0];
        }
        else
        {
            lowIndexLine = IntersectionIndexLine[0];
            highIndexLine = IntersectionIndexLine[1];
        }
        res->at(0).resize(polyLine.size() - highIndexPoly + lowIndexPoly + 
           highIndexLine - lowIndexLine + 2);        
        res->at(1).resize(highIndexPoly - lowIndexPoly + highIndexLine - 
           lowIndexLine + 2);
        int index1 = 0;
        int index2 = 0;
        for(int i = 0; i <= lowIndexPoly; i++ )
        {
            res->at(0)[index1++] = polyLine[i];
        }
        for(int i = highIndexPoly; i > lowIndexPoly; i--)
        {
            res->at(1)[index2++] = polyLine[i];
        }
        int indexindex = -1;
        for(unsigned int i = 0; i < IntersectionIndexPoly.size(); i++)
        {
         if(IntersectionIndexPoly[i] == lowIndexPoly)
         {
            indexindex = i;
            break;
         }
        }        
        res->at(0)[index1++] = IntersectionPoints[indexindex];
        res->at(1)[index2++] = IntersectionPoints[indexindex];
        if(IntersectionIndexLine[indexindex] == lowIndexLine)
        {
            for(int i = lowIndexLine + 1; i <= highIndexLine; i++)
            {
                res->at(0)[index1++] = splitLine->at(i);
                res->at(1)[index2++] = splitLine->at(i);
            }
        }
        else
        {            
            for(int i = highIndexLine; i > lowIndexLine; i--)
            {
                res->at(0)[index1++] = splitLine->at(i);
                res->at(1)[index2++] = splitLine->at(i);
            }
        }       
        res->at(0)[index1++] = IntersectionPoints[(indexindex - 1) * -1];
        res->at(1)[index2++] = IntersectionPoints[(indexindex - 1) * -1];
        for(unsigned int i = highIndexPoly + 1; i < polyLine.size(); i++)
        {
            res->at(0)[index1++] = polyLine[i];
        }
#ifdef DECHTN  
        cout<<"ResultNode1"<<endl;
        for(int i=0;i<res->at(0).size();i++)
        {
         cout<<i<<res->at(0)[i];
        }
        
        cout<<"ResultNode2"<<endl;
        for(int i=0;i<res->at(1).size();i++)
        {
         cout<<i<<res->at(1)[i];
        }
        if(res->at(0)[0].equals(&res->at(0).back()))
        {
         return(NULL);
        }
#endif        
        if(res->at(1)[0].equals(&res->at(1).back()))
        {
         return(NULL);
        }
    }    
    return(res);
}

}

