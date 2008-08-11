/*
 * CentroidMatch.java
 *
 * Created on 29. September 2007, 23:34
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;

import java.util.*;

/**
 *
 * @author java
 */
public class CentroidMatch extends Match
{
    int threshold;
    /** Creates a new instance of CentroidMatch */
    public CentroidMatch(Region source, Region target,double thresholdRel, boolean useFinalize)
    {
        
        super(source,target,"CentroidMatch "+((int)(thresholdRel*100))+" %","this implements the position of centroids Match, known from the paper of Erlend Tøssebro (5.2 1)");
        
        LineWA[][] tmp=new LineWA[source.getNrOfFaces()+target.getNrOfFaces()][];   //Calculate the threshold for matching
        for(int i=0;i<source.getNrOfFaces();i++)
        {
            tmp[i]=source.getFace(i).getCycle().getLines();
        }
        for(int i=0;i<target.getNrOfFaces();i++)
        {
            tmp[i+source.getNrOfFaces()]=target.getFace(i).getCycle().getLines();
        }
        double greatestDist=TriRepUtil.getMaxDistance2(tmp);
        threshold=(int)(greatestDist*thresholdRel);                                 //Calculate the threshold for matching
        this.addMatch(source,target);
        this.matchFaces(source.getFaces(),target.getFaces());
        
        if(useFinalize)
            this.fertig();
        this.generateRatings();
    }
    
    public void matchFaces(Face[] faces1,Face[] faces2)
    {
        HashSet unmatched=new HashSet(faces1.length+faces2.length);
        for(int i=0;i< faces1.length;i++)
        {
            for(int j=0;j<faces2.length;j++)
            {
                
                double distance= getDistance(faces1[i].getCycle(),faces2[j].getCycle());
                if(distance<threshold)
                {
                    this.addMatch(faces1[i],faces2[j]);
                    this.addMatch(faces2[j],faces1[i]);
                    this.addMatch(faces1[i].getCycle(),faces2[j].getCycle());
                    this.addMatch(faces2[j].getCycle(),faces1[i].getCycle());
                }
            }
            unmatched.add(faces1[i]);
        }
        for(int i=0;i<faces2.length;i++)
        {
            unmatched.add(faces2[i]);
        }
        while(!unmatched.isEmpty())
        {
            Face next=(Face)(unmatched.iterator().next());
            unmatched.remove(next);
            RegionTreeNode[] matches=this.getMatches(next);
            if(matches!=null&&matches[0]!=null)
            {
                if(matches.length>1)
                {
                    int dimMatch=0;
                    for(int i=0;i<matches.length;i++)
                    {
                        unmatched.remove(matches[i]);
                        dimMatch+=((Face)matches[i]).getCycle().getChildren().length;
                    }
                    this.matchCHTNs(next.getHolesAndConcavities(),this.getTargetChildren(next));
                }
                else
                {
                    if(getMatches(matches[0]).length>1)
                    {
                        for(int i=0;i<getMatches(matches[0]).length;i++)
                        {
                            unmatched.remove(getMatches(matches[0])[i]);
                        }
                        this.matchCHTNs(((Face)matches[0]).getHolesAndConcavities(),this.getTargetChildren(matches[0]));
                    }
                    else
                    {
                        unmatched.remove(matches[0]);
                        
                        this.matchCHTNs(next.getHolesAndConcavities(),this.getTargetChildren(next));
                    }
                }
            }
        }
    }
    
    public void matchCHTNs(ConvexHullTreeNode[] chtn1,ConvexHullTreeNode[] chtn2)
    {
        if(chtn1==null||chtn1.length==0||chtn2==null||chtn2.length==0)
        {
            return;
        }
        HashSet unmatched=new HashSet(source.getNrOfFaces()+target.getNrOfFaces());
        for(int i=0;i< chtn1.length;i++)
        {
            for(int j=0;j<chtn2.length;j++)
            {
                
                double distance= getDistance(chtn1[i],chtn2[j]);
                if(distance<threshold)
                {
                    this.addMatch(chtn1[i],chtn2[j]);
                    this.addMatch(chtn2[j],chtn1[i]);
                    if(TriRepUtil.debugging)
                    {
                        System.out.println("addMatch");
                        System.out.println(chtn1[i]);
                        System.out.println(chtn2[j]);
                    }
                }
            }
            unmatched.add(chtn1[i]);
        }
        for(int i=0;i<chtn2.length;i++)
        {
            unmatched.add(chtn2[i]);
        }
        while(!unmatched.isEmpty())
        {
            ConvexHullTreeNode next=(ConvexHullTreeNode)(unmatched.iterator().next());
            unmatched.remove(next);
            RegionTreeNode[] matches=this.getMatches(next);
            if(matches!=null&&matches[0]!=null)
            {
                if(matches.length>1)
                {
                    for(int i=0;i<matches.length;i++)
                    {
                        unmatched.remove(matches[i]);
                    }
                    this.matchCHTNs(next.getChildren(),this.getTargetChildren(next));
                }
                else
                {
                    if(getMatches(matches[0]).length>1)
                    {
                        for(int i=0;i<getMatches(matches[0]).length;i++)
                        {
                            unmatched.remove(getMatches(matches[0])[i]);
                        }
                        this.matchCHTNs(((ConvexHullTreeNode)matches[0]).getChildren(),this.getTargetChildren(matches[0]));
                    }
                    else
                    {
                        unmatched.remove(matches[0]);
                        this.matchCHTNs(next.getChildren(),((ConvexHullTreeNode)matches[0]).getChildren());
                    }
                }
            }
        }
    }
    
    public Face getBestMatch(Face source,Face[] targets)
    {
        double best=Double.MAX_VALUE;
        Face bestMatch=null;
        for(int i=0;i<targets.length;i++)
        {
            double overl=getDistance(source.getCycle(),targets[i].getCycle());
            if(overl<best)
            {
                bestMatch=targets[i];
                best=overl;
            }
        }
        return(bestMatch);
    }
    
    public ConvexHullTreeNode getBestMatch(ConvexHullTreeNode source,ConvexHullTreeNode[] targets)
    {
        double best=Double.MAX_VALUE;
        ConvexHullTreeNode bestMatch=null;
        for(int i=0;i<targets.length;i++)
        {
            double overl=getDistance(source,targets[i]);
            if(overl<best)
            {
                bestMatch=targets[i];
                best=overl;
            }
        }
        return(bestMatch);
    }
    
    private static double getDistance(ConvexHullTreeNode chtn1,ConvexHullTreeNode chtn2)
    {
        if(chtn1==null || chtn2==null) return(1000000);
        LineWA center1=chtn1.getCenter();
        LineWA center2=chtn2.getCenter();
        return(Math.sqrt((center1.x-center2.x)*(center1.x-center2.x)+(center1.y-center2.y)*(center1.y-center2.y)));
    }
}
