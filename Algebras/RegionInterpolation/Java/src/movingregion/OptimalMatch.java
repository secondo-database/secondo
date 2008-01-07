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
        System.out.println("1-->"+source.getFace(0).getCycle());
        Match best;
        Match[] candidates  =new Match[27];
        candidates[0]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.1);
        candidates[1]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.2);
        candidates[2]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.3);
        candidates[3]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.4);
        candidates[4]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.5);
        candidates[5]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.6);
        candidates[6]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.7);
        candidates[7]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.8);
        candidates[8]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.9);
        candidates[9]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.1);
        candidates[10]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.2);
        candidates[11]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.3);
        candidates[12]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.4);
        candidates[13]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.5);
        candidates[14]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.6);
        candidates[15]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.7);
        candidates[16]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.8);
        candidates[17]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.9);
        candidates[18]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.1);
        candidates[19]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.2);
        candidates[20]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.3);
        candidates[21]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.4);
        candidates[22]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.5);
        candidates[23]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.6);
        candidates[24]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.7);
        candidates[25]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.8);
        candidates[26]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.9);
        best=candidates[0];
        for(int i=1;i<candidates.length;i++)
        {
            if(candidates[i].getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight)==best.getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight))
                best.addName(candidates[i].getName());
            if(candidates[i].getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight)>best.getRating(AreaWeight,OverlapWeight,HausdorffWeight,LinearWeight))
                best=candidates[i];
        }
        this.setMatch(best);
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
