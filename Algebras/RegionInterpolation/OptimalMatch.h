/*
 
1 OptimalMatch.h

*/

#ifndef OPTIMALMATCH_H_
#define OPTIMALMATCH_H_
namespace RegionInterpol
{
/*

1.1 Class Optimal--Match

This class implements a match by calculating several matches and using the best one.  

*/ 
   class OptimalMatch:public Match 
   {   
      public:     
/*
 
1.1.1 Constructor

This construtor sets the sourceregion, the targetregion, and a vector of weights that are used to calculate the ratings of the various matches. 

*/    
         OptimalMatch(RegionForInterpolation *source, 
            RegionForInterpolation *target, std::vector<double> weights);    
/*

1.1.1 Overridden Methods
 
this Methods are not needed here.
  
*/
         ConvexHullTreeNode *getBestMatch(ConvexHullTreeNode *source,
            std::vector<ConvexHullTreeNode*> *targets);
         Face *getBestMatch(Face *source,std::vector<Face*> *targets);          
         void matchFaces(std::vector<Face*> *faces1,std::vector<Face*> *faces2);
         void matchCHTNs(std::vector<ConvexHullTreeNode*> &chtn1,
            std::vector<ConvexHullTreeNode*> &chtn2);            
   };
}
#endif 

/*

\pagebreak

*/
