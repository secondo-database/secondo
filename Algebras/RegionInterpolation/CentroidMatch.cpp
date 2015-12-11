/*
   
see CentroidMatch.h for documentation

\tableofcontents
 
*/
#include "RegionInterpolator.h"

using namespace std;

namespace RegionInterpol
{

/*
 
1 Constructor

*/ 
   
CentroidMatch :: CentroidMatch(RegionForInterpolation *source, 
   RegionForInterpolation *target, double thresholdRel, bool useFinalize=true) :
   
   Match(source, target, "CentroidMatch " + 
         Utils :: toString( (int) (thresholdRel * 100)) + " %" , 
         "this implements the position of centroids Match")
{
    threshold = greatestDist * thresholdRel;
    addMatch(source, target);
    addMatch(target, source);
    matchFaces(source->getFaces(), target->getFaces());
    this->generateRatings();
    if(useFinalize)      
      finalize();
}
/*
 
1 Overridden Methods

1.1 getBestMatch()

*/

ConvexHullTreeNode *CentroidMatch :: getBestMatch(ConvexHullTreeNode *source, 
                                     vector<ConvexHullTreeNode*> *targets)
{
    double best = numeric_limits<double> :: max();
    ConvexHullTreeNode* bestMatch;
    for(unsigned int i = 0; i < targets->size(); i++)
    {
        double dist = getDistance(source, targets->at(i));
        if (dist < best)
        {
            bestMatch = targets->at(i);
            best = dist;
        }
    }
    return(bestMatch);
}
   
Face *CentroidMatch :: getBestMatch(Face *source, vector<Face*> *targets)
{
    double best = numeric_limits<double> :: max();
    Face* bestMatch = NULL;    
    for(unsigned int i = 0; i < targets->size(); i++)
    {             
        double dist =getDistance(source->getCycle(),targets->at(i)->getCycle());
        if(dist < best)
        {
            bestMatch = targets->at(i);
            best = dist;
        }
    }    
    return(bestMatch);
}
         
/*

1.1 matchCHTNs()
 
*/
void CentroidMatch :: matchCHTNs(vector<ConvexHullTreeNode*> &chtn1, 
                                  vector<ConvexHullTreeNode*> &chtn2)
{
    vector<ConvexHullTreeNode*> unmatched;
    for(unsigned int i = 0; i < chtn1.size(); i++)
    {
        for(unsigned int j = 0; j < chtn2.size(); j++)
        {            
            double distance = getDistance(chtn1.at(i), chtn2.at(j));
            if(distance < threshold)
            {
                addMatch(chtn1.at(i), chtn2.at(j));
                addMatch(chtn2.at(j), chtn1.at(i));                
            }
        }
        unmatched.push_back(chtn1.at(i));
    }
    for(unsigned int i = 0; i < chtn2.size(); i++)
    {
        unmatched.push_back(chtn2.at(i));
    }     
    while(!unmatched.empty())
    {
        ConvexHullTreeNode *next = unmatched.back();
        unmatched.pop_back();
        vector<RegionTreeNode*> matches = getMatches(next);
        if (!matches.empty())
        {
            if(matches.size() > 1)
            {
                for(unsigned int i = 0; i < matches.size(); i++)
                {
                  for(unsigned int j = 0; j < unmatched.size(); j++)
                  {
                     if(unmatched[j]->equals(matches[i]))
                     {
                        unmatched.erase(unmatched.begin() + j);
                        break;
                     }
                  }                        
                }
                vector<ConvexHullTreeNode*> param1= next->getChildren();
                vector<ConvexHullTreeNode*> param2= getTargetChildren(next);
                matchCHTNs(param1, param2);
            }
            else
            {
                if(getMatches(matches[0]).size() > 1)
                {
                    for(unsigned int i=0; i<getMatches(matches[0]).size(); i++)
                    {
                     for(unsigned int j = 0; j<unmatched.size(); j++)
                     {
                        if(unmatched[j]->equals(getMatches(matches[0])[i]))
                        {
                           unmatched.erase(unmatched.begin() + j);
                           break;
                        }
                     }                                       
                                           
                    }
                    vector<ConvexHullTreeNode*> param1= 
                      ((ConvexHullTreeNode*)matches[0])->getChildren();
                    vector<ConvexHullTreeNode*> param2=
                      getTargetChildren(matches[0]);
                    matchCHTNs(param1, param2);
                }
                else
                {          
                  for(unsigned int j = 0; j<unmatched.size(); j++)
                  {
                     if(unmatched[j]->equals(matches[0]))
                     {
                        unmatched.erase(unmatched.begin() + j);
                        break;
                     }
                  } 
                  vector<ConvexHullTreeNode*> param1= next->getChildren();
                  vector<ConvexHullTreeNode*> param2= 
                    ((ConvexHullTreeNode*) matches[0])->getChildren();
                  matchCHTNs(param1, param2);
                }
            }
        }
    }
}

/*

1.1 matchFaces()
 
*/

void CentroidMatch :: matchFaces(vector<Face*> *faces1, vector<Face*> *faces2)
{
   vector<Face*> unmatched;
    for(unsigned int i = 0; i < faces1->size(); i++)
    {
        for(unsigned int j = 0; j<faces2->size(); j++)
        {            
            double distance = getDistance(faces1->at(i)->getCycle(), 
                               faces2->at(j)->getCycle());
            if(distance < threshold)
            {
                addMatch(faces1->at(i), faces2->at(j));
                addMatch(faces2->at(j), faces1->at(i));
                addMatch(faces1->at(i)->getCycle(), faces2->at(j)->getCycle());
                addMatch(faces2->at(j)->getCycle(), faces1->at(i)->getCycle());
            }
        }
        unmatched.push_back(faces1->at(i));
    }
    for(unsigned int i = 0; i<faces2->size(); i++)
    {
        unmatched.push_back(faces2->at(i));
    }    
    while(!unmatched.empty())
    {
        Face *next = unmatched.back();
        unmatched.pop_back();
        vector<RegionTreeNode*> matches = getMatches(next);
        if(!matches.empty())
        {
            if(matches.size() > 1)
            {
                int dimMatch = 0;
                for(unsigned int i = 0; i < matches.size(); i++)
                {
                  for(unsigned int j = 0; j<unmatched.size(); j++)
                  {
                     if(unmatched[j]->equals(matches[i]))
                     {
                        unmatched.erase(unmatched.begin() + j);
                        break;
                     }
                  }                                       
                    dimMatch += ( (Face*) matches[i])->getCycle()->
                                  getChildren().size();
                }      
               vector<ConvexHullTreeNode*> param1= 
                 next->getHolesAndConcavities();
               vector<ConvexHullTreeNode*> param2= getTargetChildren(next);
               matchCHTNs(param1, param2);                 
           }
            else
            {
                if(getMatches(matches[0]).size() > 1)
                {
                    for(unsigned int i=0; i<getMatches(matches[0]).size();i++)
                    {
                     for(unsigned int j = 0; j < unmatched.size(); j++)
                     {
                        if(unmatched[j]->equals(getMatches(matches[0])[i]))
                        {
                           unmatched.erase(unmatched.begin() + j);
                           break;
                        }
                     }                        
                    }      
                    vector<ConvexHullTreeNode*> param1=
                      ((Face*) matches[0])->getHolesAndConcavities();
                    vector<ConvexHullTreeNode*> param2=
                      getTargetChildren(matches[0]);
                    matchCHTNs(param1, param2);
                }
                else
                {
                  for(unsigned int j = 0; j<unmatched.size(); j++)
                  {
                     if(unmatched[j]->equals(matches[0]))
                     {
                        unmatched.erase(unmatched.begin() + j);
                        break;
                     }
                  }     
                  vector<ConvexHullTreeNode*> param1= 
                    next->getHolesAndConcavities(); 
                  vector<ConvexHullTreeNode*> param2= 
                    getTargetChildren(next); 
                  matchCHTNs(param1, param2);
                }
            }
        }
    }
}
         
/*
 
1 Private Methods()
 
1.1 getDistance()

*/
         
double CentroidMatch :: getDistance(ConvexHullTreeNode *chtn1, 
                                     ConvexHullTreeNode *chtn2)
{
    LineWA center1 = chtn1->getCenter();
    LineWA center2 = chtn2->getCenter();
    return(sqrt(Utils :: getSquareDistance(&center1, &center2)));
}

}
