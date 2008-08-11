
package movingregion;

import java.util.*;

public class OverlappingMatch extends Match
{
    
    double threshold;
    /** Creates a new instance of OverlappingMatch */
    
    public OverlappingMatch(Region source, Region target,double threshold,boolean useFinalize)
    {
        super(source,target,"OverlappingMatch "+((int)(threshold*100))+" %","this implements the Fixed theshold overlaping Match for set of cycles");
        this.threshold=threshold;
        this.addMatch(source,target);
        this.matchFaces(source.getFaces(),target.getFaces());
        
        if(useFinalize)
            this.fertig();
        this.generateRatings();
    }
    
    public void matchFaces(Face[] faces1,Face[] faces2)
    {
        if(TriRepUtil.debugging)
            System.out.println("FaceMAtch");
        HashSet unmatched=new HashSet(faces1.length+faces2.length);
        for(int i=0;i< faces1.length;i++)
        {
            for(int j=0;j<faces2.length;j++)
            {
                double overlap= getOverlapp(faces1[i].getCycle(),faces2[j].getCycle());
                if(overlap>threshold)
                {
                    this.addMatch(faces1[i],faces2[j]);
                    this.addMatch(faces2[j],faces1[i]);
                    this.addMatch(faces1[i].getCycle(),faces2[j].getCycle());
                    this.addMatch(faces2[j].getCycle(),faces1[i].getCycle());
                    if(TriRepUtil.debugging)
                        System.out.println("addMatch");
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
        if(chtn1!=null&&chtn1.length>0&&chtn2!=null&&chtn2.length>0)
        {
            if(TriRepUtil.debugging)
            {
                System.out.println("MAtchCHTN");
                System.out.println("source");
                for(int i=0;i< chtn1.length;i++)
                {
                    System.out.println(chtn1[i]);
                }
                System.out.println("target");
                for(int i=0;i< chtn2.length;i++)
                {
                    System.out.println(chtn2[i]);
                }
            }
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
                        if(TriRepUtil.debugging)
                            System.out.println("addMatch");
                    }
                }
                unmatched.add(chtn1[i]);
            }
            for(int i=0;i<chtn2.length;i++)
            {
                unmatched.add(chtn2[i]);
            }
            while(unmatched.isEmpty())
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
                        if(TriRepUtil.debugging)
                            System.out.println("1");
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
                            if(TriRepUtil.debugging)
                                System.out.println("2");
                            this.matchCHTNs(((ConvexHullTreeNode)matches[0]).getChildren(),this.getTargetChildren(matches[0]));
                        }
                        else
                        {
                            unmatched.remove(matches[0]);
                            if(TriRepUtil.debugging)
                                System.out.println("3");
                            this.matchCHTNs(next.getChildren(),((ConvexHullTreeNode)matches[0]).getChildren());
                        }
                    }
                }
            }
        }
    }
    
    public ConvexHullTreeNode getBestMatch(ConvexHullTreeNode source,ConvexHullTreeNode[] targets)
    {
        double best=0;
        ConvexHullTreeNode bestMatch=null;
        for(int i=0;i<targets.length;i++)
        {
            double overl=getOverlapp(source,targets[i]);
            if(overl>best)
            {
                bestMatch=targets[i];
                best=overl;
            }
        }
        return(bestMatch);
    }
    
    public Face getBestMatch(Face source,Face[] targets)
    {
        double best=0;
        Face bestMatch=null;
        for(int i=0;i<targets.length;i++)
        {
            double overl=getOverlapp(source.getCycle(),targets[i].getCycle());
            if(overl>best)
            {
                bestMatch=targets[i];
                best=overl;
            }
        }
        return(bestMatch);
    }
    
    
    public static double getOverlapp(ConvexHullTreeNode chtn1,ConvexHullTreeNode chtn2)
    {
        double [] res=TriRepUtil.findOverlap(chtn1.getOutLine(),chtn2.getOutLine());        
        return((res[0])/100.0);
    }
}
