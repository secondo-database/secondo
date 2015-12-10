
/*
  
1 SteinerPointMatch.h 
 
*/
#ifndef STEINERPOINTMATCH_H_
#define STEINERPOINTMATCH_H_
namespace RegionInterpol
{
/*

1.1 Class Steiner--Point Match

This class is threshold match, using the Steiner--Point as a reference point  

*/ 
   class SteinerPointMatch:public Match 
   {   
      public:     
/*
 
1.1.1 Constructor

This construtor sets the sourceregion, the targetregion, and calculates an absolute threshold by the given relative one.  

*/       
         SteinerPointMatch(RegionForInterpolation *source, 
            RegionForInterpolation *target,
            double thresholdRel, bool useFinalize);
/*

1.1.1 Overridden Methods
 
this Methods return the one $target$ that matches $source$ best
  
*/          
         ConvexHullTreeNode *getBestMatch(ConvexHullTreeNode *source,
            std::vector<ConvexHullTreeNode*> *targets);
         Face *getBestMatch(Face *source,std::vector<Face*> *targets);
/*

this Methods match a set of faces or ConVexHullTreeNodes to an other set of those 
 
*/  
         void matchFaces(std::vector<Face*> *faces1, 
                         std::vector<Face*> *faces2);
         void matchCHTNs(std::vector<ConvexHullTreeNode*> &chtn1,
            std::vector<ConvexHullTreeNode*> &chtn2);
         
      private:
/*

1.1.1 Private Methods
 
this Method calculates the distance of the centroids of the given ConvexHullTreeNodes

*/  
         static double getDistance(ConvexHullTreeNode *chtn1,
            ConvexHullTreeNode *chtn2);
/*

1.1.1 Attributes

the $name$ and the $description$ of the Match not usefulll in SECONDO but only for cout 

The absolute threshold the Match uses

*/                         
         double threshold;       
   };
}
#endif 
/*

\pagebreak

*/



