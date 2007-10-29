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
public class SteinerPointMatch extends Match
{
    
    /** Creates a new instance of CentroidMatch */
    public SteinerPointMatch(Region source, Region target,double thresholdRel)
    {
        
        super(source,target,"Steiner-Point Match","this implements a Matching according to the Steiner Point");
        
        LineWA[][] tmp=new LineWA[source.getNrOfFaces()+target.getNrOfFaces()][];
        for(int i=0;i<source.getNrOfFaces();i++)
        {
            tmp[i]=source.getFace(i).getCycle().getLines();
        }
        for(int i=0;i<target.getNrOfFaces();i++)
        {
            tmp[i+source.getNrOfFaces()]=target.getFace(i).getCycle().getLines();
        }
        double greatestDist=TriRepUtil.getMaxDistance2(tmp);
        int threshold=(int)(greatestDist*thresholdRel);
        System.out.println("HALLO"+greatestDist+" "+threshold);
        this.addMatch(source,target);
        HashSet unmatched=new HashSet(source.getNrOfFaces()+target.getNrOfFaces());
        for(int i=0;i< source.getNrOfFaces();i++)
        {
            for(int j=0;j<target.getNrOfFaces();j++)
            {
                
                double distance= getDistance(source.getFace(i).getCycle(),target.getFace(j).getCycle());
                if(distance<threshold)
                {
                    this.addMatch(source.getFace(i),target.getFace(j));
                    this.addMatch(target.getFace(j),source.getFace(i));
                    this.addMatch(source.getFace(i).getCycle(),target.getFace(j).getCycle());
                    this.addMatch(target.getFace(j).getCycle(),source.getFace(i).getCycle());
                }
            }
            unmatched.add(source.getFace(i));
        }
        for(int i=0;i<target.getNrOfFaces();i++)
        {
            unmatched.add(target.getFace(i));
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
                    this.matchCHTNs(next.getCycle().getChildren(),this.getTargetChildren(next),threshold);
                }
                else
                {
                    if(getMatches(matches[0]).length>1)
                    {
                        for(int i=0;i<getMatches(matches[0]).length;i++)
                        {
                            unmatched.remove(getMatches(matches[0])[i]);
                        }
                        this.matchCHTNs(((Face)matches[0]).getCycle().getChildren(),this.getTargetChildren(matches[0]),threshold);
                    }
                    else
                    {
                        unmatched.remove(matches[0]);
                        this.matchCHTNs(next.getCycle().getChildren(),((Face)matches[0]).getCycle().getChildren(),threshold);
                    }
                }
            }
        }
        this.fertig();
    }
    
    public void matchCHTNs(ConvexHullTreeNode[] chtn1,ConvexHullTreeNode[] chtn2,int threshold)
    {
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
                    System.out.println("addMatch");
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
                    this.matchCHTNs(next.getChildren(),this.getTargetChildren(next),threshold);
                }
                else
                {
                    if(getMatches(matches[0]).length>1)
                    {
                        for(int i=0;i<getMatches(matches[0]).length;i++)
                        {
                            unmatched.remove(getMatches(matches[0])[i]);
                        }
                        this.matchCHTNs(((ConvexHullTreeNode)matches[0]).getChildren(),this.getTargetChildren(matches[0]),threshold);
                    }
                    else
                    {
                        unmatched.remove(matches[0]);
                        this.matchCHTNs(next.getChildren(),((ConvexHullTreeNode)matches[0]).getChildren(),threshold);
                    }
                }
            }
        }
    }
    
    public static double getDistance(ConvexHullTreeNode chtn1,ConvexHullTreeNode chtn2)
    {
        LineWA center1=chtn1.getSteinerPoint();
        LineWA center2=chtn2.getSteinerPoint();
        return(Math.sqrt((center1.x-center2.x)*(center1.x-center2.x)+(center1.y-center2.y)*(center1.y-center2.y)));
    }
}
