
package movingregion;

public class SimpleMatch extends Match
{
    
    public SimpleMatch(Region source, Region target)
    {
        super(source,target,"SimpleMatch","matches each CHTN  1:1 according to the inner representation or 1:Null if noone is there. Only for debugging usefull ");
        this.addMatch(source,target);
        for(int i=0;i<Math.min(source.getNrOfFaces(),target.getNrOfFaces());i++)
        {
            this.addFaceMatch(source.getFace(i),target.getFace(i));            
        }
        if(source.getNrOfFaces()<target.getNrOfFaces())
        {
            for(int i=source.getNrOfFaces();i<target.getNrOfFaces();i++)
            {
                this.addFaceMatch(target.getFace(i),null);
            }
        }
        
        if(source.getNrOfFaces()>target.getNrOfFaces())
        {
            for(int i=target.getNrOfFaces();i<source.getNrOfFaces();i++)
            {
                this.addFaceMatch(source.getFace(i),null);
            }
        }
        
    }
    
    private void addFaceMatch(Face s, Face t)
    {
        if(t!=null)
        {
            this.addMatch(s,t);
            this.addMatch(t,s);
            this.addCHTNMatch(s.getCycle(),t.getCycle());
            for(int i=0;i< Math.min(s.getNrOfHoles(),t.getNrOfHoles());i++)
            {
                this.addCHTNMatch(s.getHole(i),t.getHole(i));
            }
            if(s.getNrOfHoles()>t.getNrOfHoles())
            {
                for(int i=t.getNrOfHoles();i<s.getNrOfHoles();i++)
                {
                    this.addCHTNMatch(s.getHole(i),null);
                }
            }
            
            if(s.getNrOfHoles()<t.getNrOfHoles())
            {
                for(int i=s.getNrOfHoles();i<t.getNrOfHoles();i++)
                {
                    this.addCHTNMatch(t.getHole(i),null);
                }
            }
        }
        else
        {
            this.addMatch(s,null);
            this.addCHTNMatch(s.getCycle(),null);
            for(int i=0;i<s.getNrOfHoles();i++)
            {
                this.addCHTNMatch(s.getHole(i),null);
            }
        }                
        
    }
    
    private void addCHTNMatch(ConvexHullTreeNode s,ConvexHullTreeNode t)
    {
        
        if(t!=null)
        {
            this.addMatch(s,t);
            this.addMatch(t,s);
            for(int i=0;i< Math.min(s.getChildren().length,t.getChildren().length);i++)
            {
                this.addCHTNMatch(s.getChildren()[i],t.getChildren()[i]);
            }
            if(s.getChildren().length>t.getChildren().length)
            {
                for(int i=t.getChildren().length;i<s.getChildren().length;i++)
                {
                    this.addCHTNMatch(s.getChildren()[i],null);
                }
            }
            
            if(s.getChildren().length<t.getChildren().length)
            {
                for(int i=s.getChildren().length;i<t.getChildren().length;i++)
                {
                    this.addCHTNMatch(t.getChildren()[i],null);
                }
            }
        }
        else
        {
            this.addMatch(s,null);
            for(int i=0;i<s.getChildren().length;i++)
            {
                this.addCHTNMatch(s.getChildren()[i],null);
            }
        }                        
    }    
    
}
