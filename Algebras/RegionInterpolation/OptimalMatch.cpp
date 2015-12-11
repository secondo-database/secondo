/*
  
see OptimalMatch.h for documentation
 
\tableofcontents
 
*/
#include "RegionInterpolator.h"
using namespace std;

namespace RegionInterpol
{
/* 

1 Constructors and Destructor

*/      
OptimalMatch::OptimalMatch(RegionForInterpolation *source, 
                         RegionForInterpolation *target,vector<double> weights):
                            Match(source->clone(),target->clone(),"","")
{          
   double AreaWeight = weights[0];
   double OverlapWeight = weights[0];
   double HausdorffWeight = weights[0];
   double LinearWeight = weights[0];
        Match *best;
        vector<Match*> candidates;
#ifdef USE_OVERLAP
        candidates.push_back(new OverlapingMatch(source->clone(),
           target->clone(),0.1,false));
        candidates.push_back(new OverlapingMatch(source->clone(),
           target->clone(),0.2,false));
        candidates.push_back(new OverlapingMatch(source->clone(),
           target->clone(),0.3,false));
        candidates.push_back(new OverlapingMatch(source->clone(),
           target->clone(),0.4,false));
        candidates.push_back(new OverlapingMatch(source->clone(),
           target->clone(),0.5,false));
        candidates.push_back(new OverlapingMatch(source->clone(),
           target->clone(),0.6,false));
        candidates.push_back(new OverlapingMatch(source->clone(), 
           target->clone(),0.7,false));
        candidates.push_back(new OverlapingMatch(source->clone(),
           target->clone(),0.8,false));
        candidates.push_back(new OverlapingMatch(source->clone(),
           target->clone(),0.9,false));
#endif
        candidates.push_back(new SteinerPointMatch(source->clone(),
           target->clone(),0.1,false));
        candidates.push_back(new SteinerPointMatch(source->clone(),
           target->clone(),0.2,false));
        candidates.push_back(new SteinerPointMatch(source->clone(),
           target->clone(),0.3,false));
        candidates.push_back(new SteinerPointMatch(source->clone(),
           target->clone(),0.4,false));
        candidates.push_back(new SteinerPointMatch(source->clone(),
           target->clone(),0.5,false));
        candidates.push_back(new SteinerPointMatch(source->clone(),
           target->clone(),0.6,false));
        candidates.push_back(new SteinerPointMatch(source->clone(),
           target->clone(),0.7,false));
        candidates.push_back(new SteinerPointMatch(source->clone(), 
           target->clone(),0.8,false));
        candidates.push_back(new SteinerPointMatch(source->clone(),
           target->clone(),0.9,false));
        candidates.push_back(new CentroidMatch(source->clone(),
           target->clone(),0.1,false));
        candidates.push_back(new CentroidMatch(source->clone(),
           target->clone(),0.2,false));
        candidates.push_back(new CentroidMatch(source->clone(), 
           target->clone(),0.3,false));
        candidates.push_back(new CentroidMatch(source->clone(),
           target->clone(),0.4,false));
        candidates.push_back(new CentroidMatch(source->clone(),
           target->clone(),0.5,false));
        candidates.push_back(new CentroidMatch(source->clone(),
           target->clone(),0.6,false));
        candidates.push_back(new CentroidMatch(source->clone(),
           target->clone(),0.7,false));
        candidates.push_back(new CentroidMatch(source->clone(),
           target->clone(),0.8,false));
        candidates.push_back(new CentroidMatch(source->clone(),
           target->clone(),0.9,false));
        best=candidates[0];
        unsigned int bestIndex=0;
        for(unsigned int i=1;i<candidates.size();i++)
        {
         
            if(candidates[i]->
              getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight)==
              best->
              getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight))
            {              
                best->addName(candidates[i]->getName());
            }
            if(candidates[i]->
              getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight) >
              best->
              getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight))
            {
                  best=candidates[i];
                  bestIndex= i;
            }
        }
        best->finalize();
        
        this->name=best->getName();
        this->source=best->getSource();
        this->target=best->getTarget();
        this->description=best->getDescription();
        this->Arearating=best->getAreaRating();
        this->Hausdorffrating=best->getHausdorffRating();
        this->linearRating=best->getLinarRating();
        this->Ovelaprating=best->getOverlapRating();
        this->maps=best->getMaps();
        best->nullify();
        for(unsigned int i=1;i<candidates.size(); ++i)
            delete candidates[i];
}     
/*
 
1 Overridden Methods

1.1 getBestMatch()

*/ 
ConvexHullTreeNode *OptimalMatch::getBestMatch(ConvexHullTreeNode *source,
                                           vector<ConvexHullTreeNode*> *targets)
{
   return(NULL);
}

Face *OptimalMatch::getBestMatch(Face *source,vector<Face*> *targets)
{
   return(NULL);
}  
/*
 
1.1 matchFaces()

*/    
void OptimalMatch::matchFaces(vector<Face*> *faces1,vector<Face*> *faces2)
{
}
/*
 
1.1 matchCHTNs()

*/   
void OptimalMatch::matchCHTNs(vector<ConvexHullTreeNode*> &chtn1,
                               vector<ConvexHullTreeNode*> &chtn2)
{
}  

}

