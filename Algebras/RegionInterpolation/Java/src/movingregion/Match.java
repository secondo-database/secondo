

package movingregion;

import java.util.*;
import java.io.*;
import java.awt.*; //Just for debugging

public abstract class Match
{
    Region source;
    Region target;
    HashMap maps;
    String name;
    String description;
    
    public Match(Region source, Region target,String name,String description)
    {
        this.description=description;
        this.name=name;
        this.source=source;
        this.target=target;
        maps=new HashMap();
    }
    
    public String getName()
    {
        return(name);
    }
    
    public String getDescription()
    {
        return(description);
    }
    
    public Region getSource()
    {
        return(source);
    }
    
    public Region getTarget()
    {
        return(target);
    }
    
    public void addMatch(RegionTreeNode source,RegionTreeNode target)
    {
        SingleMatch tmp=(SingleMatch)this.maps.get(source);
        if(tmp==null)
        {
            tmp=new SingleMatch(source,target);
            maps.put(tmp,tmp);
        }
        else
        {
            tmp.addTarget(target);
        }
    }
    
    public RegionTreeNode[] getMatches(RegionTreeNode source)
    {
        
        SingleMatch tmp=(SingleMatch)this.maps.get(source);
        if(tmp==null)
            return null;
        RegionTreeNode[] res=new RegionTreeNode[tmp.getNrTargets()];
        for(int i=0;i<res.length;i++)
        {
            res[i]=tmp.getTargetAt(i);
        }
        return(res);
    }
    
    public ConvexHullTreeNode[] getTargetChildren(RegionTreeNode source)
    {
        RegionTreeNode[] tmp=getMatches(source);
        HashSet tmpSet=new HashSet();
        for(int i=0;i<tmp.length;i++)
        {
            if(tmp[i] instanceof Face)
            {
                Face tmpf=(Face)tmp[i];
                ConvexHullTreeNode[] tmpc=tmpf.getCycle().getChildren();
                for(int j=0;j< tmpc.length;j++)
                {
                    tmpSet.add(tmpc[j]);
                }
                
                for(int j=0;j< tmpf.getNrOfHoles();j++)
                {
                    tmpSet.add(tmpf.getHole(j));
                }
            }
            if(tmp[i] instanceof ConvexHullTreeNode)
            {
                ConvexHullTreeNode tmpf=(ConvexHullTreeNode)tmp[i];
                ConvexHullTreeNode[] tmpc=tmpf.getChildren();
                for(int j=0;j< tmpc.length;j++)
                {
                    tmpSet.add(tmpc[j]);
                }
            }
        }
        Object[] tmpArray=tmpSet.toArray();
        ConvexHullTreeNode[] res=new ConvexHullTreeNode[tmpArray.length];
        for (int i=0;i<tmpArray.length;i++)
        {
            res[i]=(ConvexHullTreeNode)tmpArray[i];
        }
        return(res);
    }
    
    public void fertig()
    {
        Iterator keyIter=maps.keySet().iterator();
        while(keyIter.hasNext())
        {
            SingleMatch tmp=(SingleMatch)maps.get(keyIter.next());
            //System.out.println(tmp);
            if(tmp!=null&&tmp.getNrTargets()>1&&(tmp.getTargetAt(0)instanceof ConvexHullTreeNode))
            {
                Vector Children=new Vector();
                ConvexHullTreeNode Parent=null;
                if(((ConvexHullTreeNode)tmp.getTargetAt(0)).getParentNode()instanceof ConvexHullTreeNode)
                {
                    Parent=(ConvexHullTreeNode)((ConvexHullTreeNode)tmp.getTargetAt(0)).getParentNode();
                }
                
                
                boolean sameParent=true;
                for(int i=0;i<tmp.getNrTargets();i++)
                {
                    if(!(tmp.getTargetAt(i)instanceof ConvexHullTreeNode))
                    {
                        sameParent=false;
                    }
                    else
                    {
                        if(Parent!=null&&Parent.equals(((ConvexHullTreeNode)tmp.getTargetAt(i)).getParentNode()))
                        {
                            Children.add((ConvexHullTreeNode)tmp.getTargetAt(i));
                        }
                        else
                        {
                            sameParent=false;
                        }
                    }
                }
                if(sameParent)
                {
                    ConvexHullTreeNode[] ChildList=new ConvexHullTreeNode[Children.size()];
                    for(int i=0;i<Children.size();i++)
                    {
                        ChildList[i]=(ConvexHullTreeNode)Children.get(i);
                    }
                    Parent.joinChildren(ChildList);
                    tmp.removeTargets();
                    tmp.addTarget(ChildList[0].getParentNode());
                }
            }
        }
        
    }
    
    public double getRating()
    {
        return(0.0);
    }
    
    public String toString()
    {
        String res="";
        Object[] tmp=maps.values().toArray();
        for(int i=0;i<tmp.length;i++)
        {
            res=res+'\n'+((SingleMatch)tmp[i]);
        }
        return(res);
    }
    
    public static void main(String[] args)
    {
        
        LineWA[] fl1=new LineWA[9];
        fl1[0]=new LineWA(197,60);
        fl1[1]=new LineWA(447,98);
        fl1[2]=new LineWA(438,192);
        fl1[3]=new LineWA(353,308);
        fl1[4]=new LineWA(266,216);
        fl1[5]=new LineWA(136,204);
        fl1[6]=new LineWA(47,305);
        fl1[7]=new LineWA(52,128);
        fl1[8]=new LineWA(84,99);
        Face f1=new Face(fl1);
        Region r1=new Region();
        r1.addFace(f1);
        LineWA[] fl2=new LineWA[10];
        fl2[0]=new LineWA(328,70);
        fl2[1]=new LineWA(448,98);
        fl2[2]=new LineWA(437,196);
        fl2[3]=new LineWA(348,310);
        fl2[4]=new LineWA(266,224);
        fl2[5]=new LineWA(180,315);
        fl2[7]=new LineWA(43,306);
        fl2[6]=new LineWA(135,204);
        fl2[8]=new LineWA(37,144);
        fl2[9]=new LineWA(128,81);
        Face f2=new Face(fl2);
        LineWA[] fl3=new LineWA[3];
        fl3[0]=new LineWA(100,100);
        fl3[1]=new LineWA(100,150);
        fl3[2]=new LineWA(150,150);
        LineWA[] fl4=new LineWA[3];
        fl4[0]=new LineWA(101,101);
        fl4[1]=new LineWA(101,151);
        fl4[2]=new LineWA(151,151);
        f1.addHole(fl3);
        f2.addHole(fl4);
        Region r2=new Region();
        r2.addFace(f2);
        //SimpleMatch sm=new SimpleMatch(r1,r2);
        OverlappingMatch sm=new OverlappingMatch(r1,r2,.5);
        System.out.println(sm);
        mLineRep lr=new mLineRep(sm);
        String filename="test";
        String app="dune";
        FileOutputStream filestream;
        OutputStreamWriter fs;
        System.out.println("test");
        MatchViewer testm=new MatchViewer(sm);
        Frame frame=new Frame();
        frame.add(testm);
        frame.setVisible(true);
        try
        {
            lr.saveAsVRML(filename+".vrml",5);
            Runtime.getRuntime().exec(app+" "+filename+".vrml");
        }
        catch(IOException ex)
        {
            System.out.println(ex.getLocalizedMessage());
        }
    }
}
