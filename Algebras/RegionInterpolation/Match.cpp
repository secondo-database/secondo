/*
  
see Lines.h for documentation
 
\tableofcontents
 
*/
#include "RegionInterpolator.h"

using namespace std;
    
namespace RegionInterpol
{
/* 

1 Constructors and Destructor

*/      
Match :: Match (RegionForInterpolation *src, RegionForInterpolation *trgt, 
                string nam,string dscrpton)
{
    description = dscrpton;
    name = nam;
    source = src;
    target = trgt;
    vector <vector <LineWA*> > tmp;  
    tmp.resize (source->getNrOfFaces() + target->getNrOfFaces());
    for(int i = 0; i<source->getNrOfFaces(); i++)
    {
      vector<LineWA> tmpline = source->getFace(i)->getCycle()->getOutLine();;
      tmp[i].resize(tmpline.size());
      for(unsigned int j = 0; j < tmpline.size(); j++)
      {
         tmp[i][j] = new LineWA(&tmpline[j]);         
      }             
    }
    for(int i = 0; i < target->getNrOfFaces(); i++)
    {
      vector<LineWA> tmpline = target->getFace(i)->getCycle()->getOutLine();;
      tmp[i + source->getNrOfFaces()].resize(tmpline.size());
      for(unsigned int j = 0; j < tmpline.size(); j++)
      {
         tmp[i + source->getNrOfFaces()][j] = new LineWA(&tmpline[j]);        
      }
    }
    
    greatestDist = Utils::getMaxDistance(tmp);
    LineWA* del;
    for(vector <vector <LineWA*> >::iterator it1= tmp.begin(); it1!= 
      tmp.end(); ++it1)
    {
      for(vector<LineWA*>::iterator it2= (*it1).begin(); it2 != 
        (*it1).end(); ++it2)
      {
        del= (*it2);
        delete del;
      }
    }
}  
/*

1 Get functions

1.1 getName()

*/        
string Match :: getName()
{
    return(name);
}
/*
 
1.1 getDescription()

*/
string Match :: getDescription()
{
    return(description);
}
/*
 
1.1 getAreaRating()

*/   
double Match :: getAreaRating()
{
    return(Arearating);
}
/*
 
1.1 getOverlapRating()

*/
double Match :: getOverlapRating()
{
    return(Ovelaprating);
}
/*
 
1.1 getHausdorffRating()

*/
double Match :: getHausdorffRating()
{
    return(Hausdorffrating);
}
/*
 
1.1 getLinarRating()

*/
double Match :: getLinarRating()
{
    return(linearRating);
}
/*
 
1.1 getRating()

*/
double Match::getRating(double AreaWeight, double OverlapWeigth,
                         double HausdorffWeight, double LinearWeigth)
{
    return(Arearating * AreaWeight + Ovelaprating * OverlapWeigth +
            linearRating * LinearWeigth + Hausdorffrating * HausdorffWeight);
}
/*
 
1.1 getSource()

*/
RegionForInterpolation *Match :: getSource()
{
    return(source);
}
/*
 
1.1 getTarget()

*/
RegionForInterpolation *Match :: getTarget()
{  
    return(target);
} 

void Match::setSource(RegionForInterpolation *arg)
{
  source= arg;
}
void Match::setTarget(RegionForInterpolation *arg)
{
  target= arg;
}
void Match::nullify()
{
  source=0;
  target=0;
  for(map<int, SingleMatch*>::iterator it= maps.begin(); it!=
    maps.end(); ++it)
    (*it).second=0;
}
/*
 
1.1 getMatches()

*/       
vector<RegionTreeNode*> Match :: getMatches(RegionTreeNode *source)
{   
   int sourceHash = findIndex(source);
   vector <RegionTreeNode*> res;
   if(maps.find(sourceHash) == maps.end())
   {
      return(res);
   } 
   SingleMatch* tmp = maps[sourceHash];
   tmp->removeNulls();          
    for(int i = 0; i < tmp->getNrTargets(); i++)
    {
        res.push_back(tmp->getTargetAt(i));
    }
    return(res);
}
/*
 
1.1 getMaps()

*/
map<int, SingleMatch*> Match :: getMaps()
{
   return(maps);
}   
/*
 
1.1 getTargetChildren()

*/
vector<ConvexHullTreeNode*> Match :: getTargetChildren(RegionTreeNode *source)
{  
    vector<RegionTreeNode*> tmp = getMatches(source); 
    vector<ConvexHullTreeNode*> res;
    for(unsigned int i = 0; i < tmp.size(); i++)
    {
        if(dynamic_cast <Face*> (tmp[i]))
        {
            Face* tmpFace = (Face*) tmp[i];
            vector <ConvexHullTreeNode*> tmpCHTN = 
                                          tmpFace->getCycle()->getChildren();
            for(unsigned int j = 0; j < tmpCHTN.size(); j++)
            {
                res.push_back(tmpCHTN.at(j));
            }
            
            for(int j = 0; j < tmpFace->getNrOfHoles(); j++)
            {
                res.push_back(tmpFace->getHole(j));
            }
        }
        if(dynamic_cast <ConvexHullTreeNode*> (tmp[i]))
        {
            ConvexHullTreeNode* tmpCHTN = (ConvexHullTreeNode*)tmp[i];
            vector<ConvexHullTreeNode*> tmpChildren = tmpCHTN->getChildren();
            for(unsigned int j = 0 ; j< tmpChildren.size(); j++)
            {
                res.push_back(tmpChildren.at(j));
            }
        }
    }
    return(res);
}
/*

1 Set functions

1.1 addMatch()

*/
void Match :: addMatch(RegionTreeNode *source, RegionTreeNode *target)
{    
   int sourceHash = findIndex(source);
#ifdef DEMATCH 
   cout<< "addMatch: "<< source->hashCode()<< " " << target->hashCode()<< endl;
#endif   
    if(maps.find(sourceHash) == maps.end())
    {
        SingleMatch *tmp = new SingleMatch(source,target);
        maps.insert(map<int, SingleMatch*> :: value_type(sourceHash, tmp));
    }
    else
    {
      if(maps[sourceHash]->getSource()->equals(source))
      {
         maps[sourceHash]->addTarget(target);
      }
      else
      {
#ifdef DEMATCH          
         cout << "echtes Problem" << endl;
#endif         
      }
    }
}
/*
 
1.1 addName()

*/
void Match :: addName(string newName)
{  
    name += '\n' + newName;    
}
/*
 
1.1 removeMatches()

*/
void Match :: removeMatches(RegionTreeNode *toDelete)
{
   if(toDelete != NULL)
   {
       removeSingleMatch(toDelete);
       if(dynamic_cast <ConvexHullTreeNode*> (toDelete))
       {
           vector <ConvexHullTreeNode*> children = ((ConvexHullTreeNode*) 
                                                    toDelete)->getChildren();
           for(unsigned int i = 0; i < children.size(); i++)
           {
               removeMatches(children.at(i));
           }
       }     
       if(dynamic_cast <Face*> (toDelete))
       {
           Face* deleteFace=( (Face*) toDelete);
           removeMatches(deleteFace->getCycle());
           for(int i = 0; i<deleteFace->getNrOfHoles(); i++)
           {
               removeMatches(deleteFace->getHole(i));
           }
       }     
       if(dynamic_cast <Region*> (toDelete))
       {
           RegionForInterpolation* deleteRegion = ((RegionForInterpolation*) 
                                                  toDelete);
           for(int i = 0; i < deleteRegion->getNrOfFaces(); i++)
           {
               removeMatches(deleteRegion->getFace(i));
           }
      }
   }
}
/*

1 Public Methods

1.1 finalize()

*/    
void Match :: finalize()
{    
    map <int, SingleMatch*> :: iterator iter = maps.begin();       
    while(iter != maps.end())
    {          
        SingleMatch *tmp = (*iter).second;
        tmp->removeNulls();
        iter++;                        
        if(tmp->getNrTargets() == 1 && 
           dynamic_cast <ConvexHullTreeNode*> (tmp->getTargetAt(0)))
        //1 to 1 Match of ConvexHullTrees search and delete huge rotatings
        {         
            ConvexHullTreeNode *stmp = (ConvexHullTreeNode*) tmp->getSource();
            ConvexHullTreeNode *ttmp=(ConvexHullTreeNode*) tmp->getTargetAt(0);
                   
            if( (dynamic_cast <ConvexHullTreeNode*> (stmp->getParentNode())) &&
                (dynamic_cast <ConvexHullTreeNode*> (ttmp->getParentNode())) )
            {
                ConvexHullTreeNode *parentS = ((ConvexHullTreeNode*) 
                                              stmp->getParentNode());
                ConvexHullTreeNode *parentT = ((ConvexHullTreeNode*) 
                                              ttmp->getParentNode());
                vector<LineWA*> linesS = parentS->getLineForChild(stmp);
                vector<LineWA*> linesT = parentT->getLineForChild(ttmp);
                if(linesS.size() == 2 && linesT.size() == 2)
                {                                   
                    vector<LineWA> *linesNeiS = new vector <LineWA> (4);
                    vector<LineWA> *linesNeiT = new vector <LineWA> (4);
                    linesNeiS->at(1) = linesS[0];
                    linesNeiS->at(2) = linesS[1];
                    linesNeiT->at(1) = linesT[0];
                    linesNeiT->at(2) = linesT[1];
                    vector <LineWA> parentSLL = parentS->getOutLine();
                    vector <LineWA> parentTLL = parentT->getOutLine();
                    for(unsigned int i = 0; i < parentSLL.size(); i++)
                    {
#ifdef DEMATCH                      
                     cout << parentSLL[i] << " vs. " << *linesS[0] << endl;
                     cout << parentSLL[(i + 1) % parentSLL.size()] <<
                     " vs. " << linesS[1] << endl;
#endif                     
                        if(parentSLL[i].equals(linesS[0]) && 
                           parentSLL[(i+1)%parentSLL.size()].equals(linesS[1]))
                        {           
                           linesNeiS->at(0) = parentSLL[(i-1+parentSLL.size())%
                                              parentSLL.size()];
                            linesNeiS->at(3)=parentSLL[(i+2)%parentSLL.size()];
                        }
                    }
                    for(unsigned int i = 0; i < parentTLL.size(); i++)
                    {
                        if(parentTLL[i].equals(linesT[0]) && parentTLL[ (i+1) %
                           parentTLL.size()] .equals(linesT[1]))
                        {        
                           linesNeiT->at(0)=parentTLL[(i-1+parentTLL.size()) % 
                                      parentTLL.size()];
                            linesNeiT->at(3)=parentTLL[(i+2)%parentTLL.size()];
                        }
                    }
                    Utils :: computeLineAngles(linesNeiS);
                    Utils :: computeLineAngles(linesNeiT);
                    double angleT = linesNeiT->at(1).getAngle();
                    double angleS = linesNeiS->at(1).getAngle();
                    if(linesNeiS->at(0).getAngle()>linesNeiS->at(1).getAngle())
                    {
                        linesNeiS->at(1).setAngle
                                   (linesNeiS->at(1).getAngle() + 2 * M_PI);
                    }
                    if(linesNeiS->at(1).getAngle()>linesNeiS->at(2).getAngle())
                    {
                        linesNeiS->at(2).setAngle
                                   (linesNeiS->at(2).getAngle() + 2 * M_PI);
                    }
                    if(linesNeiT->at(0).getAngle()>linesNeiT->at(1).getAngle())
                    {
                        linesNeiT->at(1).setAngle
                                   (linesNeiT->at(1).getAngle() + 2 * M_PI);
                    }
                    if(linesNeiT->at(1).getAngle()>linesNeiT->at(2).getAngle())
                    {
                        linesNeiT->at(2).setAngle
                                   (linesNeiT->at(2).getAngle() + 2 * M_PI);
                    }               
                    if( (angleS<linesNeiT->at(0).getAngle() || 
                         angleS > linesNeiT->at(2).getAngle() ) && 
                        (angleS + 2 * M_PI < linesNeiT->at(0).getAngle() || 
                         angleS + 2 * M_PI > linesNeiT->at(2).getAngle() ) )
                    {
                        tmp->removeTargets();
                    }
                    if( (angleT < linesNeiS->at(0).getAngle() ||
                      angleT > linesNeiS->at(2).getAngle() ) &&
                     (angleT + 2 * M_PI < linesNeiS->at(0).getAngle() ||
                      angleT + 2 * M_PI > linesNeiS->at(2).getAngle() ) )
                    {      
                        tmp->removeTargets();
                    }
                }
            }
        }
        
        if(tmp!=NULL&&tmp->getNrTargets()>1)
        //1 to N Match
        {            
            if(dynamic_cast<Face*>(tmp->getSource()))
            //1 to N Match of Faces try to split them
            {
                Face *sou = (Face*) tmp->getSource();
                Face *t1 = (Face*) tmp->getTargetAt(0);
                Face *t2 = (Face*) tmp->getTargetAt(1);
                RegionForInterpolation *parentsou = sou->getParent();
            removeMatches(sou);
              removeMatches(t1);
              removeMatches(t2);
                vector<Face*> *newSources = parentsou->splitOnLine(sou->
                    getCycle()->getSplitLine(t1->getCycle() , t2->getCycle()));
                if( newSources->size() == 2 && 
                    newSources->at(0) != NULL && 
                    newSources->at(1) != NULL)
                {
                  vector<Face*> *oldtargets = new vector<Face*> (2);
                    oldtargets->at(0) = t1;
                    oldtargets->at(1) = t2;
                    vector<Face*> new1;
                    vector<Face*> new2;
                    vector<Face*> old1;
                    vector<Face*> old2;                    
                    new1.push_back(newSources->at(0));
                    new2.push_back(newSources->at(1));
                    old1.push_back(getBestMatch(newSources->at(0), oldtargets));
                    old2.push_back(getBestMatch(newSources->at(1), oldtargets));
                    matchFaces(&new1, &old1);
                    matchFaces(&new2, &old2);
                    for(int i = 2; i< tmp->getNrTargets(); i++)
                    {
                        vector<Face*> tmpTar;
                        tmpTar.push_back(( (Face*) tmp->getTargetAt(i)));
                        vector<Face*> tmpSour;
                        tmpSour.push_back(getBestMatch(tmpTar[0], newSources));
                        matchFaces(&tmpSour, &tmpTar); 
                    }
              }
               else
               //Split failed match each face with best matched
               {
                  vector<Face*> tmpSour;
                  tmpSour.push_back(sou);
                  vector<Face*> tmpTar;
                  vector<Face*> tmpTar2;
                  for(int i = 0; i < tmp->getNrTargets(); i++)
                  {
                     tmpTar.push_back( (Face*) tmp->getTargetAt(i));
                  }
                  tmpTar2.push_back(getBestMatch(sou, &tmpTar));
                  matchFaces(&tmpSour, &tmpTar2);             
                }
                iter =maps.begin();                               
                continue;
            }   
         if(dynamic_cast<ConvexHullTreeNode*>(tmp->getSource()))
           {         
            vector<ConvexHullTreeNode*> tmpSour;
             tmpSour.push_back( (ConvexHullTreeNode*) tmp->getSource());
             removeMatches((ConvexHullTreeNode*) tmp->getSource());
             vector<ConvexHullTreeNode*> tmpTar;
             vector<ConvexHullTreeNode*> tmpTar2;
             for(int i = 0; i < tmp->getNrTargets(); i++)
             {
               tmpTar.push_back( (ConvexHullTreeNode*) tmp->getTargetAt(i));
             }
             tmpTar2.push_back(getBestMatch( (ConvexHullTreeNode*) 
                               tmp->getSource(), &tmpTar));
             matchCHTNs(tmpSour, tmpTar2);            
               iter = maps.begin();                                             
               continue;         
           }                   
        }               
    }
}
/*

1 Operators

1.1 $<<$

*/
ostream& operator << (ostream& s, Match *match)
{
   s << match->getName() << endl 
     << match->getDescription() << " Ratings: " <<endl
     << "AR: " << match->getAreaRating() << " OR: " << match->getOverlapRating()
     << " HR: " << match->getHausdorffRating() << " LR: " 
     << match->getLinarRating() << endl;  
#ifdef   DEMATCH
   map <int, SingleMatch*> :: iterator iter = match->maps.begin();   
   while(iter != match->maps.end())
   {
      s << "Key" << (*iter).first;
      s << (*(*iter).second);    
      iter++;
   }
#endif      
   return s;
}
/*

1 Protected Methods

1.1 generateRatings()

*/
void Match :: generateRatings()
{
#ifdef DEMATCH    
   cout<<"gr"<<endl;;
   cout<<this<<endl;
   cout<<*source;
   cout<<*target;
#endif   
    Hausdorffrating = 0.0;
    Ovelaprating = 0.0;
    Arearating = 0.0;
    linearRating = 0;
    NrOfRatings = 0;
    for(int i = 0; i < source->getNrOfFaces(); i++)
    {       
        vector<RegionTreeNode*> tmp = getMatches(source->getFace(i));
#ifdef DEMATCH        
        cout << tmp.size() << endl;
#endif        
        if(tmp.size() == 0)
        {        
            Hausdorffrating += Utils::getDiameter(Utils :: convertCHLine2LineWA
                               (source->getFace(i)->getCycle()->getLines()));
            Ovelaprating += 0.0;
            Arearating += 0.0;
            linearRating += 0.5;
            NrOfRatings++;
        }
        else
        {                        
         rateFace(source->getFace(i), tmp);            
        }         
    }
    for(int i = 0; i<target->getNrOfFaces(); i++)
    {           
        vector<RegionTreeNode*> tmp = getMatches(target->getFace(i));
#ifdef DEMATCH        
        cout<<tmp.size()<<endl;
#endif        
        if(tmp.size() == 0)
        {
            Hausdorffrating += Utils::getDiameter(Utils :: convertCHLine2LineWA
                               (target->getFace(i)->getCycle()->getLines()));
            Ovelaprating += 0.0;
            Arearating += 0.0;
            linearRating += 0.5;
            NrOfRatings++;
        }
        else
        {                  
         rateFace(target->getFace(i),tmp);         
        }
    }
#ifdef DEMATCH    
    cout<<"gr2"<<endl;
    cout<<source->getNrOfFaces()<<" "<<target->getNrOfFaces()<<endl;
    cout<<"gr2"<<NrOfRatings<<" "<<greatestDist<<endl;;
#endif    
    Arearating = Arearating / NrOfRatings;
    Ovelaprating = Ovelaprating / NrOfRatings;
    Hausdorffrating = 1 - Hausdorffrating / NrOfRatings / greatestDist;
    linearRating = linearRating / NrOfRatings;            
}
/*
 
1.1 rateFace()

*/
void Match :: rateFace(Face *source, vector<RegionTreeNode*> targets)
{
#ifdef DEMATCH 
   cout<<"grF"<<endl;
#endif   
    double sourceArea = abs(Utils :: getArea(source->getCycle()->getLines()));
    double targetArea = 0.0;
    double sumOverlaps = 0.0;
    double HausdorffTmp = 0.0;
    vector<CHLine> sourcel = source->getCycle()->getLines();
    for(unsigned int i = 0; i < targets.size(); i++)
    {       
        vector<CHLine> targetl = ( (Face*) targets[i])->getCycle()->getLines();
        if(Utils :: getHausdorfDistance(source->getCycle()->getLines(), 
          ((Face*) targets[i])->getCycle()->getLines()) > HausdorffTmp)
        {
            HausdorffTmp = Utils :: getHausdorfDistance(sourcel, targetl);
        }
        targetArea += abs(Utils :: getArea(targetl));        
        sumOverlaps += Utils :: getOverlap(&sourcel, &targetl);
    }
#ifdef DEMATCH    
    cout<<"grF1 "<<sourceArea<<" "<<targetArea<<endl;
#endif    
    NrOfRatings++;
    Arearating += min(sourceArea, targetArea) / max(sourceArea, targetArea);
    Ovelaprating += sumOverlaps / sourceArea;
    Hausdorffrating += HausdorffTmp / targets.size();
    linearRating += 1.0 / targets.size();
    vector <ConvexHullTreeNode*> children; 
    children = source->getCycle()->getChildren();
#ifdef DEMATCH    
    cout<<"grF2"<<endl;
#endif    
    for(unsigned int i = 0; i<children.size(); i++)
    {
        vector <RegionTreeNode*> tmp = getMatches(children.at(i));
#ifdef DEMATCH            
        cout << "CHTN :" << children.at(i)->hashCode() << " hat "
             << tmp.size() << " Matches" << endl;
#endif        
        if(tmp.size() == 0)
        {
            Hausdorffrating += Utils :: getDiameter(sourcel);
            Ovelaprating += 0.0;
            Arearating += 0.0;
            linearRating += 0.5;
            NrOfRatings++;
        }
        else
        {
         rateCHTN(children.at(i),tmp);            
        }
    }  
#ifdef DEMATCH      
    cout << "GRFace oH AR: " << Arearating << " HR: " << Hausdorffrating 
         << " lR: " << linearRating << " Nr: " << NrOfRatings<<endl;
#endif    
    for(int i = 0; i<source->getNrOfHoles(); i++)
    {
        vector <RegionTreeNode*> tmp = getMatches(source->getHole(i));
#ifdef DEMATCH           
        cout << "Hole :" << source->getHole(i)->hashCode()
             << " hat " << tmp.size() << " Matches" << endl;
#endif        
        if(tmp.size() == 0)
        {
            Hausdorffrating += Utils :: getDiameter
                               (Utils :: convertCHLine2LineWA(sourcel));
            Ovelaprating += 0.0;
            Arearating += 0.0;
            linearRating += 0.5;
            NrOfRatings++;
        }
        else
        {
            rateCHTN(source->getHole(i),tmp);            
        }
    }
#ifdef DEMATCH    
    cout << "GRFace AR: " << Arearating << " HR: " << Hausdorffrating 
         << " lR: " << linearRating << " Nr: " << NrOfRatings << endl;
#endif    
}
/*
 
1.1 rateCHTN()

*/
void Match::rateCHTN(ConvexHullTreeNode *source,vector<RegionTreeNode*> targets)
{
#ifdef DEMATCH 
   cout<<"grCHTN"<<endl;
   cout << "GRCHTN AR: " << Arearating << " HR: " << Hausdorffrating 
        << " lR: " << linearRating << " Nr: " << NrOfRatings << endl;
   cout<<*source;
   //cout<<targets[0];
   cout<<targets.size()<<endl;;
   //cout<<source->getLines().size()<<endl;
   //cout<<((ConvexHullTreeNode*)targets[0])->getLines().size();
#endif   
    double sourceArea = abs(Utils :: getArea(source->getLines()));
    double targetArea = 0.0;
    double sumOverlaps = 0.0;
    double HausdorffTmp = 0.0;    
    for(unsigned int i = 0;i < targets.size(); i++)
    {       
        if(Utils :: getHausdorfDistance(source->getLines(), 
          ((ConvexHullTreeNode*) targets[i])->getLines()) > HausdorffTmp)
        {
            HausdorffTmp = Utils :: getHausdorfDistance(source->getLines(), 
                          ((ConvexHullTreeNode*) targets[i])->getLines());
        }
        targetArea += abs(Utils :: getArea(((ConvexHullTreeNode*) 
                                           targets[i])->getLines()));
        vector<CHLine> sourcel = source->getLines();
        vector<CHLine> targetl =((ConvexHullTreeNode*) targets[i])->getLines();
        sumOverlaps += Utils :: getOverlap(&sourcel, &targetl);
    }    
    NrOfRatings++;
    Arearating += min(sourceArea, targetArea) / max(sourceArea, targetArea);
    Ovelaprating += sumOverlaps / (sourceArea + targetArea);
    Hausdorffrating += HausdorffTmp / targets.size();
    linearRating += 1.0 / targets.size();    
    vector<ConvexHullTreeNode*> children = source->getChildren();
    for(unsigned int i = 0; i<children.size(); i++)
    {
        vector<RegionTreeNode*> tmp = getMatches(children.at(i));
        if(tmp.size() == 0)
        {
            Hausdorffrating += Utils :: getDiameter
                            (Utils :: convertCHLine2LineWA(source->getLines()));
            Ovelaprating += 0.0;
            Arearating += 0.0;
            linearRating += 0.5;
            NrOfRatings++;
        }
        else
        {
            rateCHTN(children.at(i), tmp);
        }
    }
#ifdef DEMATCH    
    cout << "GRCHTN AR: " << Arearating << " HR: " << Hausdorffrating 
         << " lR: " << linearRating << " Nr: " << NrOfRatings << endl;
#endif    
}
/*

1 Private Methods

1.1 findIndex()

*/
int Match :: findIndex(RegionTreeNode *source)
{
   int sourceHash = source->hashCode();
   while(maps.find(sourceHash) != maps.end() && 
         !maps[sourceHash]->getSource()->equals(source))
    {
#ifdef DEMATCH       
      cout << "Konflikt zwischen " << maps.size() << " " << sourceHash << endl;
#endif      
      sourceHash++;
    }
#ifdef DEMATCH    
    cout << "<geloest " << maps.size() << " " << sourceHash << endl;
#endif    
    return(sourceHash);
}
/*

1.1 removeSingleMatch()

*/    
void Match :: removeSingleMatch(RegionTreeNode *toDelete)
{
   if(toDelete!=NULL)
   {
      int sourceHash = findIndex(toDelete);
      delete maps.find(sourceHash)->second;
      maps.erase(sourceHash);           
       map<int,SingleMatch*> :: iterator iter = maps.begin();
       while(iter != maps.end())
       {          
           SingleMatch *tmp = (*iter).second;
           for(int i = 0; i < tmp->getNrTargets(); i++)
           {
               if(toDelete->equals(tmp->getTargetAt(i)))
                   tmp->removeTarget(i);
           }
           iter++;
       }
   }
}
   
}

