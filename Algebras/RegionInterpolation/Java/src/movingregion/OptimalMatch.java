/*
 * OptimalMatch.java
 *
 * Created on 4. November 2007, 01:30
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;

/**
 *
 * @author java
 */
public class OptimalMatch extends Match
{
    /** Creates a new instance of OptimalMatch */
    public OptimalMatch(Region source, Region target,double AreaWeight,double OverlapWeight,double HausdorffWeight,double LinearWeight)
    {
        super((Region)source.clone(),(Region)target.clone(),"","");
        if(TriRepUtil.debugging)
            System.out.println("1-->"+source.getFace(0).getCycle());
        Match best;
        Match[] candidates  =new Match[17];
        candidates[0]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.05,true);
        candidates[1]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.1,true);
        candidates[2]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.15,true);
        candidates[3]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.2,true);
        candidates[4]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.25,true);
        
//        candidates[2]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.3,true);
//        candidates[3]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.4,true);
//        candidates[4]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.5,true);
//        candidates[5]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.6,true);
//        candidates[6]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.7,true);
//        candidates[7]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.8,true);
//        candidates[8]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.9,true);
        candidates[5]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.05,true);
        candidates[6]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.1,true);
        candidates[7]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.15,true);
        candidates[8]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.2,true);
        candidates[9]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.25,true);
        candidates[10]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.3,true);
//        candidates[15]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.7,true);
//        candidates[16]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.8,true);
//        candidates[17]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.9,true);
        candidates[11]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.05,true);
        candidates[12]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.1,true);
        candidates[13]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.15,true);
        candidates[14]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.2,true);
        candidates[15]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.25,true);
        candidates[16]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.3,true);
//        candidates[24]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.7,true);
//        candidates[25]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.8,true);
//        candidates[26]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.9,true);
        best=candidates[0];
        for(int i=1;i<candidates.length;i++)
        {
            if(candidates[i].getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight)==best.getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight))
                best.addName(candidates[i].getName());
            if(candidates[i].getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight)>best.getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight))
                best=candidates[i];
        }
        best.fertig();
        this.setMatch(best);
        if(TriRepUtil.debugging)
            System.out.println("2-->"+source.getFace(0).getCycle());
    }
    
    
    public void matchCHTNs(ConvexHullTreeNode[] chtn1,ConvexHullTreeNode[] chtn2)
    {
        //No use here
    }
    public void matchFaces(Face[] chtn1,Face[] chtn2)
    {
        
    }
    public Face getBestMatch(Face source,Face[] targets)
    {
        return(null);
    }
    public ConvexHullTreeNode getBestMatch(ConvexHullTreeNode source,ConvexHullTreeNode[] targets)
    {
        return(null);
    }
}
