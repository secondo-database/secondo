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
        candidates[0]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.1,false);
        candidates[1]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.2,false);
        candidates[2]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.3,false);
        candidates[3]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.4,false);
        candidates[4]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.5,false);
        candidates[5]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.6,false);
        candidates[6]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.7,false);
        candidates[7]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.8,false);
        candidates[8]=new OverlappingMatch((Region)source.clone(),(Region)target.clone(),0.9,false);
        candidates[9]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.1,false);
        candidates[10]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.2,false);
        candidates[11]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.3,false);
        candidates[12]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.4,false);
        candidates[13]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.5,false);
        candidates[14]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.6,false);
        candidates[15]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.7,false);
        candidates[16]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.8,false);
        candidates[17]=new SteinerPointMatch((Region)source.clone(),(Region)target.clone(),0.9,false);
        candidates[18]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.1,false);
        candidates[19]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.2,false);
        candidates[20]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.3,false);
        candidates[21]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.4,false);
        candidates[22]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.5,false);
        candidates[23]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.6,false);
        candidates[24]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.7,false);
        candidates[25]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.8,false);
        candidates[26]=new CentroidMatch((Region)source.clone(),(Region)target.clone(),0.9,false);
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
