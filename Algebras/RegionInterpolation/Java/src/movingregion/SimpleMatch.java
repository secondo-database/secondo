
package movingregion;

public class SimpleMatch extends Match
{
    
    public SimpleMatch(Region source, Region target)
    {
        super(source,target,"SimpleMatch","matches each CHTN  1:1 according to the inner representation or 1:Null if noone is there. Only for debugging usefull ");
        this.addMatch(source,target);
        for(int i=0;i<Math.min(source.getNrOfFaces(),target.getNrOfFaces());i++)
        {
            Face[] s=new Face[1];
            Face[] t=new Face[1];
            s[0]=source.getFace(i);
            t[0]=target.getFace(i);
            this.matchFaces(s,t);
        }
        if(source.getNrOfFaces()<target.getNrOfFaces())
        {
            for(int i=source.getNrOfFaces();i<target.getNrOfFaces();i++)
            {
                Face[] t=new Face[1];
                t[0]=target.getFace(i);
                this.matchFaces(t,null);
            }
        }
        
        if(source.getNrOfFaces()>target.getNrOfFaces())
        {
            for(int i=target.getNrOfFaces();i<source.getNrOfFaces();i++)
            {
                Face[] s=new Face[1];
                s[0]=source.getFace(i);
                this.matchFaces(s,null);
            }
        }
        fertig();
    }
    
    public void matchCHTNs(ConvexHullTreeNode[] chtn1,ConvexHullTreeNode[] chtn2)
    {
        if(chtn2!=null&&chtn2[0]!=null)
        {
            ConvexHullTreeNode s=chtn1[0];
            ConvexHullTreeNode t=chtn2[0];
            this.addMatch(s,t);
            this.addMatch(t,s);
            for(int i=0;i< Math.min(s.getChildren().length,t.getChildren().length);i++)
            {
                this.matchCHTNs(s.getChildren()[i],t.getChildren()[i]);
            }
            if(s.getChildren().length>t.getChildren().length)
            {
                for(int i=t.getChildren().length;i<s.getChildren().length;i++)
                {
                    this.matchCHTNs(s.getChildren()[i],null);
                }
            }
            
            if(s.getChildren().length<t.getChildren().length)
            {
                for(int i=s.getChildren().length;i<t.getChildren().length;i++)
                {
                    this.matchCHTNs(t.getChildren()[i],null);
                }
            }
        }
        else
        {
            ConvexHullTreeNode s=chtn1[0];
            this.addMatch(s,null);
            for(int i=0;i<s.getChildren().length;i++)
            {
                this.matchCHTNs(s.getChildren()[i],null);
            }
        }
    }
    
    
    public void matchCHTNs(ConvexHullTreeNode s,ConvexHullTreeNode t)
    {
        ConvexHullTreeNode[]chtn1=new ConvexHullTreeNode[1];
        ConvexHullTreeNode[]chtn2=new ConvexHullTreeNode[1];
        chtn1[0]=s;
        chtn2[0]=t;
        matchCHTNs(chtn1,chtn2);
    }
    
    
    
    public void matchFaces(Face[] face1,Face[] face2)
    {
        Face s=face1[0];
        Face t=face2[0];
        if(t!=null)
        {
            this.addMatch(s,t);
            this.addMatch(t,s);
            this.matchCHTNs(s.getCycle(),t.getCycle());
            for(int i=0;i< Math.min(s.getNrOfHoles(),t.getNrOfHoles());i++)
            {
                this.matchCHTNs(s.getHole(i),t.getHole(i));
            }
            if(s.getNrOfHoles()>t.getNrOfHoles())
            {
                for(int i=t.getNrOfHoles();i<s.getNrOfHoles();i++)
                {
                    this.matchCHTNs(s.getHole(i),null);
                }
            }
            
            if(s.getNrOfHoles()<t.getNrOfHoles())
            {
                for(int i=s.getNrOfHoles();i<t.getNrOfHoles();i++)
                {
                    this.matchCHTNs(t.getHole(i),null);
                }
            }
        }
        else
        {
            this.addMatch(s,null);
            this.matchCHTNs(s.getCycle(),null);
            for(int i=0;i<s.getNrOfHoles();i++)
            {
                this.matchCHTNs(s.getHole(i),null);
            }
        }
        
    }
    
    public ConvexHullTreeNode getBestMatch(ConvexHullTreeNode source,ConvexHullTreeNode[] targets)
    {
        return(targets[0]);
    }
    
    public Face getBestMatch(Face source,Face[] targets)
    {
        return(targets[0]);
    }
    
}
