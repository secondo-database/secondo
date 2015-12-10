/*
 
1 OverlapingMatch.h 
 
*/

#ifndef OVERLAPINGMATCH_H_
#define OVERLAPINGMATCH_H_
namespace RegionInterpol
{
/*

1.1 Class Centroid Match

This class is threshold match, using the overlapings of polygones  

*/ 
   class OverlapingMatch:public Match 
   {   
      public:     
/*
 
1.1.1 Constructor

This construtor sets the sourceregion, the targetregion, and the threshold. 

*/       
         OverlapingMatch(RegionForInterpolation *source, 
                         RegionForInterpolation *target, 
                         double thresholdRel, bool useFinalize);
/*

1.1.1 Overridden Methods
 
this Methods return the one $target$ that matches $source$ best
  
*/                
         ConvexHullTreeNode *getBestMatch(ConvexHullTreeNode *source, 
                              std::vector<ConvexHullTreeNode*> *targets);
         Face *getBestMatch(Face *source, std::vector<Face*> *targets);
/*

this Methods match a set of faces or ConVexHullTreeNodes to an other set of those 
 
*/  

         void matchFaces(std::vector<Face*> *faces1, 
                         std::vector<Face*> *faces2);       
         void matchCHTNs(std::vector<ConvexHullTreeNode*> &chtn1, 
                          std::vector<ConvexHullTreeNode*> &chtn2);
      
      private:
/*

1.1.1 Attributes

The absolute threshold the Match uses

*/       
         double threshold;
   };
}
#endif 

/*

\pagebreak

*/

