package movingregion;

import java.util.*;
import java.awt.*;
import java.io.*;
public class Face implements RegionTreeNode,Cloneable, Serializable 
{
    static final long serialVersionUID = -6107459580342391919L; 
    ConvexHullTreeNode Cycle;
    Vector Holes;
    Region parent;
    int sourceOrTarget;
    
    public Face(LineWA[] linelist,Region parent, int sourceOrTarget)
    {
        Cycle=new ConvexHullTreeNode(linelist,0,this, sourceOrTarget);
        Holes=new Vector();
        this.parent=parent;
    }
    public void removeHole(ConvexHullTreeNode toDelete)
    {
        this.Holes.remove(toDelete);
    }
    public Region getParent()
    {
        return(parent);
    }
    
    public void setParent(Region parent)
    {
        this.parent=parent;
    }

    public void setSourceOrTarget(int sourceOrTarget)
    {
        this.sourceOrTarget=sourceOrTarget;
        this.Cycle.setSourceOrTarget(sourceOrTarget);
        for(int i=0;i<this.getNrOfHoles();i++)
        {
            this.getHole(i).setSourceOrTarget(sourceOrTarget);
        }
    }
    
    protected Object clone() 
    {
        Face res=new Face(Cycle.getLines(),this.getParent(), sourceOrTarget);
        //res.setCycle((ConvexHullTreeNode)this.Cycle.clone());
        for(int i=0;i<this.getNrOfHoles();i++)
        {
            res.addHole(this.getHole(i).getLines());
        }
        return(res);
    }
   
    public void concavity2Hole(ConvexHullTreeNode con)
    {
        this.Cycle.removeChild(con);
        con.setHole(true);
        this.Holes.add(con);
    }
    
    public Face splitOnLine(LineWA[] splitLine)
    {
        LineWA[][]resSplit=this.Cycle.getSplitNodes(splitLine);
        if(resSplit==null||resSplit[1]==null)
            return(null);
        
        Polygon cycle1=new Polygon();
        for(int i=0;i<resSplit[0].length;i++)
        {
            cycle1.addPoint(resSplit[0][i].x,resSplit[0][i].y);
        }
        
        Polygon cycle2=new Polygon();
        for(int i=0;i<resSplit[1].length;i++)
        {
            cycle2.addPoint(resSplit[1][i].x,resSplit[1][i].y);
        }
        Vector stillHoles=new Vector();
        Vector newCons=new Vector();
        for(int i=0;i< this.getNrOfHoles();i++)
        {
            LineWA[][] tmpHole=this.getHole(i).getSplitNodes(splitLine);
            if(tmpHole==null||tmpHole[1]==null)
                stillHoles.add(this.getHole(i));
            else
            {
                newCons.add(tmpHole[0]);
                newCons.add(tmpHole[1]);
            }
        }        
        for(int i=0;i<newCons.size();i++)
        {
            LineWA[] holePoints=(LineWA[])newCons.elementAt(i);
            int j=0;
            while(TriRepUtil.PointOnBoundary(resSplit[0],holePoints[j]))
            {
                j++;
            }
            if(cycle1.contains(new Point(holePoints[j].x,holePoints[j].y)))
            {
                try
                {
                    resSplit[0]=TriRepUtil.joinLinelists(resSplit[0],holePoints);       
                }
                catch(Exception e)
                {
                    if(TriRepUtil.debuggingWarnings)
                        System.out.println("Problem bei joinLineList1 "+e.getMessage());
                    return(null);
                }
            }
            else
            {
                try
                {
                    resSplit[1]=TriRepUtil.joinLinelists(resSplit[1],holePoints);       
                }
                catch(Exception e)
                {
                    if(TriRepUtil.debuggingWarnings)
                        System.out.println("Problem bei joinLineList2 "+e.getMessage());
                    return(null);
                }
            }
        }
        this.Holes=new Vector();
        this.Cycle=new ConvexHullTreeNode(resSplit[0],0,this, sourceOrTarget);        
        Face res=new Face(resSplit[1],this.getParent(), sourceOrTarget);
        for(int i=0;i<stillHoles.size();i++)
        {
            LineWA[] holePoints=((ConvexHullTreeNode)stillHoles.elementAt(i)).getLines();
            int j=0;
            while(TriRepUtil.PointOnBoundary(resSplit[0],holePoints[j]))
            {
                j++;
            }
            if(cycle1.contains(new Point(holePoints[j].x,holePoints[j].y)))
            {
                this.addHole((ConvexHullTreeNode)stillHoles.elementAt(i));                
            }
            else
            {
                res.addHole((ConvexHullTreeNode)stillHoles.elementAt(i));
            }
        }
        return(res);
    }
    public int hashCode()
    {
        int start=5132;
        int modu=7536;
        int res=start;
        LineWA[] tmp=Cycle.getOutLine();
        for(int i=0;i<tmp.length;i++)
        {
            res=res+tmp[i].x+tmp[i].y;
        }
        res=res * (this.getSourceOrTarget() + 1);
        res=res%modu;
        return (res);
    }
    
    public boolean equals(Object o)
    {
        if(o instanceof Face)
        {
            Face tmp=(Face)o;
            if(this.getSourceOrTarget()!=tmp.getSourceOrTarget())
                return(false);
            if(this.getNrOfHoles()!=tmp.getNrOfHoles())
                return false;
            boolean res=Cycle.equals(tmp.getCycle());
            for(int i=0;i<this.getNrOfHoles();i++)
            {
                res=res&&this.getHole(i).equals(tmp.getHole(i));
            }
            return(res);
        }
        else
        {
            if(o instanceof SingleMatch)
            {
                SingleMatch tmp2=(SingleMatch)o;
                return(this.equals(tmp2.getSource()));
            }
            else
            {
                return false;
            }
        }
    }
    
    public void addHole(ConvexHullTreeNode newHole)
    {
        
        Holes.add(newHole);
    }
    
    public void addHole(LineWA[] linelist)
    {
        Holes.add(new ConvexHullTreeNode(linelist,true,this, sourceOrTarget));
    }
    
    public int getSourceOrTarget()
    {
        return(this.sourceOrTarget);
    }
            
    public int getNrOfHoles()
    {
        return(Holes.size());
    }
    
    public ConvexHullTreeNode getCycle()    
    {
        return(Cycle);
    }
    
    public ConvexHullTreeNode getHole(int index)
    {
        return((ConvexHullTreeNode)Holes.elementAt(index));
    }
    
    public ConvexHullTreeNode[] getHolesAndConcavities()
    {
        ConvexHullTreeNode[] res=new ConvexHullTreeNode[this.getCycle().getChildren().length+this.getNrOfHoles()];
        int index=0;
        for(;index<this.getCycle().getChildren().length;index++)
        {
            res[index]=this.getCycle().getChildren()[index];
        }
        //index++;
        for(int i=0;i< this.getNrOfHoles();i++)
        {
            res[index+i]=this.getHole(i);
        }
        return(res);
    }
    
    public void paintFace(Graphics g, boolean isactive)
    {
        if(isactive)
            g.setColor(Color.BLUE);
        else
            g.setColor(new Color(128,128,255));
        LineWA[] cycle=Cycle.getLines();
        for(int i=0;i<cycle.length;i++)
        {
            g.fillOval(cycle[i].x-4,cycle[i].y-4,8,8);
            g.drawLine(cycle[i].x,cycle[i].y,cycle[(i+1)%cycle.length].x,cycle[(i+1)%cycle.length].y);
        }
        if(isactive)
            g.setColor(Color.RED);
        else
            g.setColor(new Color(255,128,128));
        for(int j=0;j<this.getNrOfHoles();j++)
        {
            LineWA[] hole=this.getHole(j).getLines();
            for(int i=0;i<hole.length;i++)
            {
                g.fillOval(hole[i].x-4,hole[i].y-4,8,8);
                g.drawLine(hole[i].x,hole[i].y,hole[(i+1)%hole.length].x,hole[(i+1)%hole.length].y);
            }
        }
    }
    
    public void writeFaceToVRML( OutputStreamWriter fs,
            double time, String color,String shininess, String transparency) throws IOException
    {
        
        fs.write("Shape {\n");
        fs.write("   appearance Appearance {\n");
        fs.write("      material Material { diffuseColor "+color+" shininess "+shininess+" transparency "+transparency+"}\n");
        fs.write("   }");
        fs.write("   geometry IndexedFaceSet {\n");
        fs.write("      coord Coordinate {\n");
        fs.write("         point [\n");
        LineWA[] pointList=Cycle.getLines();
        Vector tmp=new Vector();
        int[][] convertlist=new int[pointList.length][3];
        int counter=0;
        for (int a=0;a<pointList.length;a++)
        {
            tmp.add(new Integer(a));
            fs.write("                ");
            fs.write(Float.toString((((float)pointList[a].x)/20)-5));
            convertlist[a][0]=a;
            convertlist[a][1]=pointList[a].x;
            convertlist[a][2]=pointList[a].y;
            fs.write(" ");
            fs.write(Float.toString((((float)pointList[a].y)/-20)-6));
            fs.write(" ");
            fs.write(Float.toString((float)time));
            fs.write("\n");
            counter++;
        }
        ConPolygon polycycle=new ConPolygon(convertlist);
        ConPolygon[] holes=new ConPolygon[this.getNrOfHoles()];
        for(int j=0;j<this.getNrOfHoles();j++)
        {
            LineWA[] pointListH=this.getHole(j).getLines();
            int[][] convertlistH=new int[pointListH.length][3];
            for(int i=0;i<pointListH.length;i++)
            {
                tmp.add(new Integer(counter));
                convertlistH[i][0]=counter;
                convertlistH[i][1]=pointListH[i].x;
                convertlistH[i][2]=pointListH[i].y;
                fs.write(Float.toString((((float)pointListH[i].x)/20)-5));
                fs.write(" ");
                fs.write(Float.toString((((float)pointListH[i].y)/-20)-6));
                fs.write(" ");
                fs.write(Float.toString((float)time));
                fs.write("\n");
                counter++;
            }
            holes[j]=new ConPolygon(convertlistH);
            
        }
        polycycle.addHoles(holes);
        fs.write("               ]\n");
        fs.write("         }\n");
        fs.write("      coordIndex [\n");
        Convexer convexer=new Convexer(polycycle);
        convexer.writePolygone(fs,tmp);
        fs.write("      ]\n");
        fs.write("      solid FALSE\n");
        fs.write("   }\n");
        fs.write("}\n");
    }

    public String toString()
    {
        String res="";        
        res=res+this.getCycle();
        for(int i=0;i< this.getNrOfHoles();i++)
        {
            res=res+"Holenr."+i+'\n';
            res=res+this.getHole(i);
        }
        return(res);
    }
    
}
