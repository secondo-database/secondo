

package movingregion;

public class SingleMatch
{
    private RegionTreeNode source;
    private RegionTreeNode[] targets;
    
    public SingleMatch(RegionTreeNode source,RegionTreeNode target)
    {
        this.source=source;
        targets=new RegionTreeNode[1];
        targets[0]=target;
    }
    
    public RegionTreeNode getSource()
    {
        return(source);
    }
    
    public int getNrTargets()
    {
        return(targets.length);
    }
    
    public RegionTreeNode getTargetAt(int i)
    {
        return(targets[i]);
    }
    
    public void removeTargets()
    {
        targets=        targets=new RegionTreeNode[0];
    }
    
    public void addTarget(RegionTreeNode target)
    {
        RegionTreeNode[]tmp=new RegionTreeNode[targets.length+1];
        for(int i=0;i<targets.length;i++)
        {
            tmp[i]=targets[i];
        }
        tmp[targets.length]=target;
        targets=tmp;
    }
    
    public int hashCode()
    {
        return(source.hashCode());
    }
    
    public boolean equals(Object o)
    {
        if(o instanceof SingleMatch)
        {
            SingleMatch tmp=(SingleMatch)o;
            return(this.source.equals(tmp.getSource()));
        }
        else
        {
            if(o instanceof RegionTreeNode)
            {
                RegionTreeNode tmp=(RegionTreeNode)o;
                return(tmp.equals(this.source));
            }
            else
            {
                return false;
            }
        }
    }
    
    public String toString()
    {
        String res=this.source+"==>";
        if(targets==null)
        {
            res=res+"null";
        }
        else
        {
            for(int i=0;i<this.targets.length;i++)
            {
                res=res+'\n'+targets[i];
            }
        }
        return(res);
    }
}
