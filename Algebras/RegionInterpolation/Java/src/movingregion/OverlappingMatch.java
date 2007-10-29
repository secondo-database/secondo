
package movingregion;

import java.util.*;

public class OverlappingMatch extends Match
{
    
    /** Creates a new instance of OverlappingMatch */
    
    public OverlappingMatch(Region source, Region target,double threshold)
    {
        super(source,target,"OverlapingMatch "+threshold,"this implements the Fixed theshold overlaping Match for set of cycles");
        this.addMatch(source,target);
        HashSet unmatched=new HashSet(source.getNrOfFaces()+target.getNrOfFaces());
        for(int i=0;i< source.getNrOfFaces();i++)
        {
            for(int j=0;j<target.getNrOfFaces();j++)
            {
                
                double overlap= getOverlapp(source.getFace(i).getCycle(),target.getFace(j).getCycle());
                if(overlap>threshold)
                {
                    this.addMatch(source.getFace(i),target.getFace(j));
                    this.addMatch(target.getFace(j),source.getFace(i));
                    this.addMatch(source.getFace(i).getCycle(),target.getFace(j).getCycle());
                    this.addMatch(target.getFace(j).getCycle(),source.getFace(i).getCycle());
                    System.out.println("addMatch");
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
                    this.matchCHTNs(next.getHolesAndConcavities(),this.getTargetChildren(next),threshold);
                }
                else
                {
                    if(getMatches(matches[0]).length>1)
                    {
                        for(int i=0;i<getMatches(matches[0]).length;i++)
                        {
                            unmatched.remove(getMatches(matches[0])[i]);
                        }
                        this.matchCHTNs(((Face)matches[0]).getHolesAndConcavities(),this.getTargetChildren(matches[0]),threshold);
                    }
                    else
                    {
                        unmatched.remove(matches[0]);
                        this.matchCHTNs(next.getHolesAndConcavities(),this.getTargetChildren(next),threshold);
                    }
                }
            }
        }
        System.out.println("Fertig2");
        System.out.println(this);
        System.out.println("Fertig");
        
     //   this.fertig();
        System.out.println("Fertig");
        System.out.println(this);
        System.out.println("Fertig2");
    }
    
    public void matchCHTNs(ConvexHullTreeNode[] chtn1,ConvexHullTreeNode[] chtn2,double threshold)
    {
        HashSet unmatched=new HashSet(source.getNrOfFaces()+target.getNrOfFaces());
        for(int i=0;i< chtn1.length;i++)
        {
            for(int j=0;j<chtn2.length;j++)
            {
                
                double overlap= getOverlapp(chtn1[i],chtn2[j]);
                if(overlap>threshold)
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
    
    
    public static double getOverlapp(ConvexHullTreeNode chtn1,ConvexHullTreeNode chtn2)
    {
        double [] res=TriRepUtil.findOverlap(chtn1.getOutLine(),chtn2.getOutLine());
        System.out.println("OVER"+res[0]);
        return((res[0])/100.0);        
    }
}
