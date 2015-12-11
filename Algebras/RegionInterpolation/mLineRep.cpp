/*
  
see mLineRep.h for documentation
 
\tableofcontents
 
*/
#include "RegionInterpolator.h"

using namespace temporalalgebra;
using namespace std;

namespace RegionInterpol
{
/* 

1 Constructors and Destructor

*/      
mLineRep :: mLineRep()
{
   myMatch = NULL;   
}

mLineRep :: mLineRep(Match *myMatch)
{ 
    this->myMatch = myMatch;    
    for(int i = 0; i < myMatch->getSource()->getNrOfFaces(); i++)
    {
        Face *tmpFace = myMatch->getSource()->getFace(i);
        vector<RegionTreeNode*>  matched = (myMatch->getMatches(tmpFace));
        if(matched.size() == 0)
        {
            addTrianglesFromFaceNull(tmpFace, 0, i);
        }
        else
        {
            if(matched.size() == 1)
            {
                addTrianglesFromFaceFace(tmpFace, (Face*) matched[0],0,1,0,i);
            }
#ifdef MLR_DEBUG                
            else
            {
                cout<<"Problem, mehrere Matches"<<endl;
            }
#endif            
        }
    }
    
    for(int i = 0; i < myMatch->getTarget()->getNrOfFaces(); i++)
    {
        Face *tmpFace = myMatch->getTarget()->getFace(i);
        vector<RegionTreeNode*>  matched = (myMatch->getMatches(tmpFace));
        if(matched.size() == 0)
        {
            addTrianglesFromFaceNull(tmpFace, 1, 
                                     myMatch->getSource()->getNrOfFaces() + i);
        }
    }
}

mLineRep :: ~mLineRep()
{
   delete myMatch;
}
/*

1 Get functions

1.1 getTriangles()

*/   
vector<MSegmentData> mLineRep :: getTriangles()
{
   return(triangles);
}
/*
 
1 Private Methods()
 
1.1 addTrianglesFromFaceFace()

*/
void mLineRep :: addTrianglesFromFaceFace(Face *face1, Face *face2, 
                  int time1, int time2, int time, int facenr)
{
#ifdef MLR_DEBUG
   cout<<"face<<face"<<endl;
#endif
    addTrianglesFromCHTCHT(face1->getCycle(), face2->getCycle(), 
                           time1, time2, facenr, 0);
    for (int i = 0; i < face1->getNrOfHoles(); i++)
    {
        vector<RegionTreeNode*> matchedHole = 
                                 (myMatch->getMatches(face1->getHole(i)));
        if(matchedHole.size() == 0)
        {
          LineWA param1 =face1->getHole(i)->getCenter();
          PointWNL *p3 = new PointWNL(&param1, time2);
          addTrianglesFromCHTPoint(face1->getHole(i), time1, p3, facenr, i+1);
          delete p3;
        }
        else
        {
            if(matchedHole.size() == 1)
            {
               addTrianglesFromCHTCHT(face1->getHole(i), 
                 ((ConvexHullTreeNode*) matchedHole[0]),time1,time2,facenr,i+1);
            }
        }
    }
    for (int i = 0; i < face2->getNrOfHoles(); i++)
    {
        vector<RegionTreeNode*> matchedHole = 
                                (myMatch->getMatches(face2->getHole(i)));
        if(matchedHole.size() == 0)
        {
          LineWA param1 =face2->getHole(i)->getCenter();
          PointWNL *p3 = new PointWNL(&param1, time1);
          addTrianglesFromCHTPoint(face2->getHole(i), time2, p3, facenr, 
                                   i + face1->getNrOfHoles() + 1);
          delete p3;
        }
    }
}
/*
 
1.1 addTrianglesFromCHTNull()

*/
void mLineRep :: addTrianglesFromCHTNull(ConvexHullTreeNode *cht, 
                                          int time, int facenr, int cyclenr)
{     
#ifdef MLR_DEBUG
   cout<<"CHTN<<null"<<endl;
#endif
    vector<LineWA> edgesLine = cht->getOutLine();
    vector<PointWNL> edges = vector<PointWNL>(edgesLine.size());
    for(unsigned int i = 0; i < edgesLine.size(); i++)
    {
        edges[i] = PointWNL(&edgesLine[i],time);        
    }
    for(unsigned int i = 0; i < edges.size(); i++)
    {
        PointWNL *p3 = getCorrespondingPoint(&edges[i], 
                          &edges[(i + 1) % edges.size()], facenr, cyclenr);
        if(p3 != NULL)
        {
            addTrianglesFromCHTPoint(cht, time, p3, facenr, cyclenr);
            break;
        }
    }
}
/*
 
1.1 addTrianglesFromCHTCHT()

*/
void mLineRep :: addTrianglesFromCHTCHT(ConvexHullTreeNode *cht1, 
        ConvexHullTreeNode *cht2, int time1, int time2, int facenr, int cyclenr)
{    
#ifdef MLR_DEBUG
   cout<<"CHTN<<CHTN"<<time1<<time2<<endl;   
#endif
    rotaring_pane(cht1,cht2,time1,time2,facenr,cyclenr);
    if(dynamic_cast<ConvexHullTreeNode*> (cht1->getParentNode()))
   {
      vector<LineWA*> line1 = ((ConvexHullTreeNode*) cht1->getParentNode()) 
                              ->getLineForChild(cht1);
      vector<LineWA*> line2 = ((ConvexHullTreeNode*) cht2->getParentNode()) 
                              ->getLineForChild(cht2);
      PointWNL p1 =PointWNL(line1[0]->getX(),line1[0]->getY(),time1);
      PointWNL p2 =PointWNL(line1[1]->getX(),line1[1]->getY(),time1);
      PointWNL p3 =PointWNL(line2[0]->getX(),line2[0]->getY(),time2);
      PointWNL p4 =PointWNL(line2[1]->getX(),line2[1]->getY(),time2);
      removeTrapezoid(&p1, &p2, &p3, &p4, facenr, cyclenr);
   }
    vector<ConvexHullTreeNode*> cht1Children = cht1->getChildren();
    for(unsigned int i = 0; i < cht1Children.size(); i++)
    {
        vector<RegionTreeNode*> matches = 
                              myMatch->getMatches((cht1Children.at(i)));
        if(matches.size() == 0)
        {
            addTrianglesFromCHTNull(cht1Children.at(i), 
                                    time1, facenr, cyclenr);
        }
        else
        {
            if(matches.size() == 1)
            {
                addTrianglesFromCHTCHT(cht1Children.at(i), 
                   (ConvexHullTreeNode*) matches[0],time1,time2,facenr,cyclenr);
            }
#ifdef MLR_DEBUG            
            else
            {
                cout<<"Problem mehrerer Matches"<<endl;
            }
#endif            
        }
    }    
    vector<ConvexHullTreeNode*> cht2Children = cht2->getChildren();
    for(unsigned int i = 0; i < cht2Children.size(); i++)
    {
        vector<RegionTreeNode*> matches = 
                                myMatch->getMatches(cht2Children.at(i));
        if(matches.size() == 0)
        {
            addTrianglesFromCHTNull(cht2Children.at(i), 
                                    time2, facenr, cyclenr);
        }
    }
}
/*
 
1.1 addTrianglesFromFaceNull()

*/
void mLineRep :: addTrianglesFromFaceNull(Face *face, int time, int facenr)
{
#ifdef MLR_DEBUG  
   cout<<"face<<null"<<time<<endl;
#endif   
   LineWA param1 =face->getCycle()->getCenter();
    PointWNL *p3 = new PointWNL(param1.getX(), param1.getY(), (time + 1) % 2);
    addTrianglesFromCHTPoint(face->getCycle(), time, p3, facenr, 0);
    delete p3;
    for (int i = 0; i < face->getNrOfHoles(); i++)
    {
        addTrianglesFromCHTPoint(face->getHole(i), time, p3, facenr, i + 1);
    }
}
/*
 
1.1 addTrianglesFromCHTPoint()

*/
void mLineRep :: addTrianglesFromCHTPoint(ConvexHullTreeNode *chtn, int time, 
                             double x, double y, int t, int facenr, int cyclenr)
{
    PointWNL *p3 = new PointWNL(x, y, t);
    addTrianglesFromCHTPoint(chtn, time, p3, facenr,cyclenr);
    delete p3;
}
/*
 
1.1 addTrianglesFromCHTPoint()

*/
void mLineRep :: addTrianglesFromCHTPoint(ConvexHullTreeNode *chtn, int time, 
                                          PointWNL *p3, int facenr, int cyclenr)
{
#ifdef MLR_DEBUG  
   cout<<"CHTN<<Point"<<time<<endl;
#endif
    vector<LineWA> tmp = chtn->getOutLine();
    PointWNL *p1;
    PointWNL *p2;
    for(unsigned int i = 0; i <  tmp.size(); i++)
    {
        p1 = new PointWNL(tmp[i].getX(), tmp[i].getY(), time);
        p2 = new PointWNL(tmp[(i + 1) % tmp.size()].getX(), 
                          tmp[(i + 1) % tmp.size()].getY(), time);
        addTriangle(p1, p2, p3, facenr, cyclenr);
        delete p1; delete p2;
    }
    vector<ConvexHullTreeNode*> children = chtn->getChildren();
    for(unsigned int i = 0; i < children.size(); i++)
    {
        addTrianglesFromCHTPoint(children.at(i), time, p3, facenr, cyclenr);
    }
}
/*
 
1.1 addTriangle()

*/
void mLineRep :: addTriangle(PointWNL *p1, PointWNL *p2, PointWNL *p3, 
                              int facenr, int cyclenr)
{
#ifdef MLR_DEBUG
   cout<<"Triangle: "<<*p1<<"; "<<*p2<<"; "<<*p3<<endl;
    if(p1->getT() != p2->getT())
        cout<<"Fehler erste beide Punkte muessen dasselbe t haben."<<endl;
    if(p1->getT()==p3->getT())
        cout<<"Fehler dritter Punkt muss ein anderes t haben."<<endl;
#endif        
    PointWNL *p1s;
    PointWNL *p2s;     
   if(p1->getX() > p2->getX()||
      (AlmostEqual(p1->getX(), p2->getX()) &&
      p1->getY() > p2->getY()) )
   {  
      p1s = p2;
      p2s = p1;
   }        
   else
   {     
      p1s = p1;
      p2s = p2;
   }
    PointWNL *corr = getCorrespondingPoint(p1, p2, facenr, cyclenr);
    if(corr == NULL)
    {
      if(p1->getT() == 0)
      {     
         MSegmentData newmseg = MSegmentData(facenr,cyclenr,triangles.size()+1, 
                     false, p1s->getX(), p1s->getY(), p2s->getX(), p2s->getY(), 
                      p3->getX(), p3->getY(), p3->getX(), p3->getY());
         triangles.push_back(newmseg);
      }
      else
      {     
         MSegmentData newmseg = MSegmentData(facenr,cyclenr,triangles.size()+1, 
                         false, p3->getX(), p3->getY(), p3->getX(), p3->getY(), 
                         p1s->getX(), p1s->getY(), p2s->getX(), p2s->getY());
         triangles.push_back(newmseg);
      }        
    }
    else
    {
        if(corr->equals(p3))
        {         
         cout<<"rem Tri"<<endl;
            removeTriangle(p1s,p2s,facenr,cyclenr);
        }
        else
        {
#ifdef MLR_DEBUG           
            cout<<"Dritte Punkte stimmen nicht"<<endl;
#endif         
           if(p1->getT() == 0)
         {           
           MSegmentData newmseg=MSegmentData(facenr,cyclenr,triangles.size()+1, 
                     false, p1s->getX(), p1s->getY(), p2s->getX(), p2s->getY(),
                     p3->getX(), p3->getY(), p3->getX(), p3->getY());
            triangles.push_back(newmseg);
         }
         else
         {           
          MSegmentData newmseg = MSegmentData(facenr,cyclenr,triangles.size()+1,
                         false, p3->getX(), p3->getY(), p3->getX(), p3->getY(), 
                         p1s->getX(), p1s->getY(), p2s->getX(), p2s->getY());
            triangles.push_back(newmseg);
         }        
        }
    }
}
/*
 
1.1 getCorrespondingPoint()

*/
 PointWNL *mLineRep :: getCorrespondingPoint(PointWNL *p1, PointWNL *p2, 
                                             int facenr, int cyclenr)
{
    
    PointWNL *res = NULL;
    int find = findIndex(p1, p2, facenr, cyclenr);
    if(find != -1)
    {
      if(p1->getT() == 0)
      {
         res = new PointWNL(triangles[find].GetFinalEndX(), 
                            triangles[find].GetFinalEndY(), 1);          
      }
      else
      {
         res = new PointWNL(triangles[find].GetInitialEndX(), 
                            triangles[find].GetInitialEndY(), 0);
      }        
    }
    return(res);
}
/*
 
1.1 findIndex()

*/
int mLineRep :: findIndex(PointWNL *p1, PointWNL *p2, int facenr, int cyclenr)
{    
   //cout<<"Suche: "<<endl<<*p1<<*p2<<endl;
    for (unsigned int i = 0; i < triangles.size(); i++)
    {
      double q1x;
      double q1y;
      double q2x;
      double q2y;
      if(p1->getT() == 0)
      {
         q1x = triangles[i].GetInitialStartX();
         q1y = triangles[i].GetInitialStartY();
         q2x = triangles[i].GetInitialEndX();
         q2y = triangles[i].GetInitialEndY();
         
      }
      else
      {
         q1x = triangles[i].GetFinalStartX();
         q1y = triangles[i].GetFinalStartY();
         q2x = triangles[i].GetFinalEndX();
         q2y = triangles[i].GetFinalEndY();
      }
      //cout<<"Vergleich ("<<q1x<<"; "<<q1y<<")("<<q2x<<"; "<<q2y<<endl;
      if((AlmostEqual(p1->getX(), q1x) && AlmostEqual(p1->getY(), q1y) && 
          AlmostEqual(p2->getX(), q2x) && AlmostEqual(p2->getY(), q2y) && 
          facenr == (int)triangles[i].GetFaceNo() && 
          cyclenr == (int)triangles[i].GetCycleNo()) ||
         (AlmostEqual(p2->getX(), q1x) && AlmostEqual(p2->getY(), q1y) && 
         AlmostEqual(p1->getX(), q2x) && AlmostEqual(p1->getY(), q2y) && 
         facenr == (int)triangles[i].GetFaceNo() && 
         cyclenr == (int)triangles[i].GetCycleNo()))           
         return(i);
      //cout<<"Daneben"<<endl;   
    }
    return(-1);
}
/*
 
1.1 removeTriangle()

*/
void mLineRep :: removeTriangle(PointWNL *p1, PointWNL *p2, 
                                 int facenr, int cyclenr)
{
    int find = findIndex(p1, p2, facenr, cyclenr);
    while(find != -1)
    {
        triangles.erase(triangles. begin() + find);
        find = findIndex(p1, p2, facenr, cyclenr);        
    }
}
/*
 
1.1 removeTrapezoid()

*/
void mLineRep :: removeTrapezoid(PointWNL *p1, PointWNL *p2, PointWNL *p3, 
                                 PointWNL *p4, int facenr, int cyclenr)
{
#ifdef MLR_DEBUG        
      cout << "rem trapezoid" << *p1 << *p2 << *p3 << *p4 << endl;
#endif            
   int find=findIndex(p1, p2, facenr, cyclenr);
   while(find != -1)
   {
#ifdef MLR_DEBUG        
      cout<<"rem tra1"<<triangles[find].ToString()<<endl;
#endif    
      triangles.erase(triangles.begin() + find);
      find = findIndex(p1, p2, facenr, cyclenr);
        
   }
   find=findIndex(p3,p4,facenr,cyclenr);
   while(find!=-1)
   {
#ifdef MLR_DEBUG        
      cout<<"rem tra2"<<triangles[find].ToString()<<endl;
#endif    
      triangles.erase(triangles.begin() + find);
      find = findIndex(p3, p4, facenr, cyclenr);
   }
}
/*
 
1.1 rotaring\_pane()

*/

ostream& operator<<(ostream& o, vector<LineWA> v);


void mLineRep :: rotaring_pane(ConvexHullTreeNode *chtn1, 
       ConvexHullTreeNode *chtn2, int time1, int time2, int facenr, int cyclenr)
{
#ifdef MLR_DEBUG
   cout<<"RP"<<time1<<time2<<endl;
#endif
    vector<LineWA> s1 = chtn1->getOutLine();
    vector<LineWA> s2 = chtn2->getOutLine();
    Utils :: computeLineAngles(&s1);
    Utils :: computeLineAngles(&s2);       
    sort(s1.begin(), s1.end());
    sort(s2.begin(), s2.end());
#ifdef MLR_DEBUG
    cout << s1;
    cout << s2;
#endif
    int jj = 0;
    PointWNL *p1=0, *p2=0, *p3=0;
    for(unsigned int i = 0; i < s1.size(); i++)
    {        
      jj = findMatchingIndex(&s2, jj, s1[i].getAngle(), true);
      p1= new PointWNL(&s1[i], time1);
      p2= new PointWNL(&s1[(i + 1 + s1.size()) % s1.size()], time1);
      p3= new PointWNL(&s2[jj], time2);
      addTriangle(p1, p2, p3, facenr, cyclenr);
      delete p1; delete p2; delete p3;
    } 
    jj = 0;
    for(unsigned int i = 0; i < s2.size(); i++)
    {
      jj = findMatchingIndex(&s1, jj, s2[i].getAngle(), false);
      p1= new PointWNL(&s2[i], time2);
      p2= new PointWNL(&s2[(i + 1 + s2.size()) % s2.size()], time2);
      p3= new PointWNL(&s1[jj], time1);
      addTriangle(p1, p2, p3, facenr, cyclenr);
      delete p1; delete p2; delete p3;
    }
}
/*
 
1.1 findMatchingIndex()

*/
int mLineRep :: findMatchingIndex(vector<LineWA> *s,int j, double angle,bool ka)
{    
    if(AlmostEqual(s->at(0).getAngle(), angle))
    {
      if(ka)
      {
         //cout<<"Para2"<<endl;
         return(0);
      }
      else
      {
         //cout<<"Para3"<<endl;
         return(1);
      }     
    }
    if(angle < s->at(0).getAngle())
    {   
      //cout<<"Para1"<<endl;     
        return(0);
    }
    if(AlmostEqual(s->at(s->size()-1).getAngle(), angle))
    {
      if(ka)
      {
         //cout<<"Para2"<<endl;
         return(0);
      }
      else
      {
         //cout<<"Para2"<<endl;
         return(s->size()-1);
      }
    }
    if(angle > s->at(s->size()-1).getAngle())
    {   
      //cout<<"Para6"<<endl;    
        return(0);
    }   
    while(
      !(AlmostEqual(s->at(j).getAngle(), angle) || 
      (s->at(j).getAngle() > angle)) ||
      !(AlmostEqual(s->at((j - 1 + s->size()) % s->size()).getAngle(), angle)|| 
      (s->at((j - 1 + s->size()) % s->size()).getAngle() < angle))
      )
    {
        if(j != 0 && angle < s->at(j - 1).getAngle())
        {
            j--;
        }
        else
        {                
          j++;            
        }
    }
    
    if(AlmostEqual(s->at(j).getAngle(),angle) && ka)
    {             
//      cout<<"Para5"<<endl;
        j=(j + 1) % s->size();
        
    }
    return(j);
}

}
