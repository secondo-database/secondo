/*
 
1 Match.h

*/

#ifndef MATCH_H_
#define MATCH_H_

namespace RegionInterpol
{

/*

1.1 abstract class Match

This class provides the basic mechanism to simply create Matches of Regions 

*/    
   
   class Match
   {
      public:
/*
 
1.1.1 Constructor

This construtor sets the sourceregion, the targetregion , the name and a short description of the match. The greatest distance in $source$ and $target$ is calculated. 

*/       
         Match(RegionForInterpolation *source, 
            RegionForInterpolation *target, string name, string description);

         virtual ~Match()
         {
           delete source;
           delete target;
           for(map<int, SingleMatch*>::iterator it= maps.begin(); it!=
             maps.end(); ++it)
             delete (*it).second;
         }

/*
           
1.1.1 Get functions

return the name or the description of the Match

*/
         string getName();
         string getDescription();
/*

return the ratings of the Match
 
*/ 
         double getAreaRating();
         double getOverlapRating();
         double getHausdorffRating();
         double getLinarRating();
/*

return the weighted sum of the ratings

*/             
         double getRating(double AreaWeight, double OverlapWeigth, 
            double HausdorffWeight, double LinearWeigth);
/*

return the source or target region

*/           
         RegionForInterpolation *getSource();
         RegionForInterpolation *getTarget();
         void setSource(RegionForInterpolation *);
         void setTarget(RegionForInterpolation *);
         void nullify();
/*

return all RegionTreeNodes that are matches to $source$

*/           
         vector<RegionTreeNode*> getMatches(RegionTreeNode *source);
/*
 
return the table of all $SingleMatches$ in the $Match$
 
*/             
         map<int,SingleMatch*> getMaps();
/*

return the children of the target that is matches with $source$

*/             
         vector<ConvexHullTreeNode*> getTargetChildren(RegionTreeNode *source);
/*
 
1.1.1 Set functions
 
 add a SingleMatch from $source$ to $target$
 
*/                                      
         void addMatch(RegionTreeNode *source,RegionTreeNode *target);
/*

Concatenate the old name of the $Mathc$ with $newName$

*/           
         void addName(string newName);
/*

removes the $toDelete$ from the Match   
 
*/           
         void removeMatches(RegionTreeNode *toDelete);

/*
           
1.1.1 Public Methods

this method delets huge rotation matches and cares for 1 to N Matches

*/  

         void finalize();            
/*

1.1.1 Virtual Methods
 
this Methods return the one $target$ that matches $source$ best
  
*/             
         virtual ConvexHullTreeNode *getBestMatch(ConvexHullTreeNode *source, 
            vector<ConvexHullTreeNode*> *targets) = 0;
         virtual Face *getBestMatch(Face *source, vector<Face*> *targets) = 0;
            
/*

this Methods match a set of faces or ConVexHullTreeNodes to an other set of those 
 
*/             
            
         virtual void matchFaces(vector<Face*> *faces1,vector<Face*> *faces2)=0;
         virtual void matchCHTNs(vector<ConvexHullTreeNode*> &chtn1, 
            vector<ConvexHullTreeNode*> &chtn2) = 0;
/*
  
1.1.1 Operators

this operator prints the $Matches$ name, description and the ratings 

*/                       
         friend ostream & operator <<(ostream & os,Match *match);
                                 
      protected:
/*
 
1.1.1 Protected Methods

$generateRating$ generate the ratings of this Match. It uses $rateFace$ and $rateCHTN$ to go throu the $RegionTree$

*/       
                     
         void generateRatings();                      
         void rateFace(Face *source, vector<RegionTreeNode*> targets);
         void rateCHTN(ConvexHullTreeNode *source, vector<RegionTreeNode*> );
/*

1.1.1 Protected Attributes

the $name$ and the $description$ of the Match not usefulll in SECONDO but only for cout 

*/                         
         string name;
         string description;
/*
 
the ratings of the Match

*/             
            
         double Hausdorffrating;
         double Ovelaprating;
         double Arearating;
         double linearRating;
/*
 
$NrOfRating$ and $greatesDist$ are usefull for genertaing the ratings

*/
         int NrOfRatings;   
         double greatestDist;             
/*

the source and the target region of the Match

*/             
         RegionForInterpolation *source;
         RegionForInterpolation *target;
/*

a table of SingleMatches, where the SingleMatches are stored as a hashtable
 
*/                                           
         map<int,SingleMatch*> maps;
            
      private:

/*

1.1.1 Private Methods
 
this Method gets the code where source can be stored in the hashtable. It cares for conflicts

*/       
         int findIndex(RegionTreeNode *source);
/*

removes only the SingleMatch that has $toDelete$ as source
 
*/                      
         void removeSingleMatch(RegionTreeNode *toDelete);
   }; 
}

#endif
 
/*

\pagebreak

*/
