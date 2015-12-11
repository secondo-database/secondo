/*
 
 see OverlapingMatch.h for documentation
 
 \tableofcontents
 
*/
#include "RegionInterpolator.h"

using namespace std;

namespace RegionInterpol
{  
/*
 
1 Constructor

*/   
OverlapingMatch::OverlapingMatch(RegionForInterpolation *source, 
  RegionForInterpolation *target, double thresholdRel, bool useFinalize = true):
  Match(source, target, "Steiner-Point Match " + 
  Utils :: toString( (int) (thresholdRel * 100)) + " %" , 
  "this implements a Matching according to Overlap of polygons")
   
{
    threshold = thresholdRel;
    addMatch(source, target);
    addMatch(target, source);
    matchFaces(source->getFaces(),target->getFaces());
    generateRatings();
    if(useFinalize)      
      finalize();
}
/*
 
1 Overridden Methods

1.1 getBestMatch()

*/
ConvexHullTreeNode *OverlapingMatch :: getBestMatch(ConvexHullTreeNode *source, 
                                          vector<ConvexHullTreeNode*> *targets)
{
    double best = numeric_limits<double> :: min();
    ConvexHullTreeNode* bestMatch;
    for(unsigned int i = 0; i < targets->size(); i++)
    {
        vector<CHLine> l1 = source->getLines();
        vector<CHLine> l2 = targets->at(i)->getLines();
        double overlap = Utils :: getOverlap(&l1, &l2);
        if(overlap > best)
        {
            bestMatch = targets->at(i);
            best = overlap;
        }
    }
    return(bestMatch);
}

Face *OverlapingMatch :: getBestMatch(Face *source, vector<Face*> *targets)
{
    double best = numeric_limits<double> :: min();
    Face* bestMatch;
    for(unsigned int i = 0; i < targets->size(); i++)
    {
        vector<CHLine> l1 = source->getCycle()->getLines();
        vector<CHLine> l2 = targets->at(i)->getCycle()->getLines();
        double overlap = Utils :: getOverlap(&l1, &l2);
        if(overlap > best)
        {
            bestMatch = targets->at(i);
            best = overlap;
        }
    }
    return(bestMatch);
}


/*

1.1 matchCHTNs()
 
*/          
void OverlapingMatch :: matchCHTNs(vector<ConvexHullTreeNode*> &chtn1, 
                                    vector<ConvexHullTreeNode*> &chtn2)
{
   vector<ConvexHullTreeNode*> unmatched;
    for(unsigned int i = 0; i < chtn1.size(); i++)
    {
        for(unsigned int j = 0; j < chtn2.size(); j++)
        {
            vector<CHLine> l1 = chtn1.at(i)->getLines(); 
            vector<CHLine> l2 = chtn2.at(j)->getLines();
            double overlap = Utils :: getOverlap(&l1, &l2);
            if(overlap > threshold)
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
        if(!matches.empty())
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
                      ((ConvexHullTreeNode*)matches[0])->getChildren();
                    vector<ConvexHullTreeNode*> param2= 
                      getTargetChildren(matches[0]);
                    matchCHTNs(param1, param2);
                }
                else
                {          
                  for(unsigned int j = 0; j < unmatched.size(); j++)
                  {
                     if(unmatched[j]->equals(matches[0]))
                     {
                        unmatched.erase(unmatched.begin() + j);
                        break;
                     }
                  }                          
                  vector<ConvexHullTreeNode*> param1= next->getChildren();
                  vector<ConvexHullTreeNode*> param2=
                    ((ConvexHullTreeNode*)matches[0])->getChildren();
                  matchCHTNs(param1, param2);
                }
            }
        }
    }
}
/*

1.1 matchFaces()
 
*/
void OverlapingMatch :: matchFaces(vector<Face*> *faces1, vector<Face*> *faces2)
{  
    vector<Face*> unmatched;
    for(unsigned int i = 0; i < faces1->size(); i++)
    {
        for(unsigned int j = 0; j < faces2->size(); j++)
        {
            vector<CHLine> l1 = faces1->at(i)->getCycle()->getLines();
            vector<CHLine> l2 = faces2->at(j)->getCycle()->getLines(); 
            double overlap = Utils :: getOverlap( &l1,&l2);
            if(overlap > threshold)
            {
                addMatch(faces1->at(i), faces2->at(j));
                addMatch(faces2->at(j), faces1->at(i));
                addMatch(faces1->at(i)->getCycle(), faces2->at(j)->getCycle());
                addMatch(faces2->at(j)->getCycle(), faces1->at(i)->getCycle());
            }
        }
        unmatched.push_back(faces1->at(i));
    }
    for(unsigned int i = 0; i < faces2->size(); i++)
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
                  for(unsigned int j = 0; j < unmatched.size(); j++)
                  {
                     if(unmatched[j]->equals(matches[i]))
                     {
                        unmatched.erase(unmatched.begin() + j);
                        break;
                     }
                  }                                       
                    dimMatch += 
                        ((Face*) matches[i])->getCycle()->getChildren().size();
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
                    for(unsigned int i=0; i<getMatches(matches[0]).size(); i++)
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
                  for(unsigned int j = 0; j < unmatched.size(); j++)
                  {
                     if(unmatched[j]->equals(matches[0]))
                     {
                        unmatched.erase(unmatched.begin() + j);
                        break;
                     }
                  }     
                  vector<ConvexHullTreeNode*> param1= 
                    next->getHolesAndConcavities();
                  vector<ConvexHullTreeNode*> param2= getTargetChildren(next); 
                  matchCHTNs(param1, param2 );
                }
            }
        }
    }
}
    
}
