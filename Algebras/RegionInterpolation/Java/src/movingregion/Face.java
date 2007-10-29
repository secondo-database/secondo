package movingregion;

import java.util.*;
import java.awt.*;
import java.io.*;
public class Face implements RegionTreeNode
{
    
    ConvexHullTreeNode Cycle;
    Vector Holes;
    
    public Face(LineWA[] linelist)
    {
        Cycle=new ConvexHullTreeNode(linelist,0,this);
        Holes=new Vector();
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
        res=res%modu;
        return (res);
    }
    
    public boolean equals(Object o)
    {
        if(o instanceof Face)
        {
            Face tmp=(Face)o;
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
    
    
    
    public void addHole(LineWA[] linelist)
    {
        Holes.add(new ConvexHullTreeNode(linelist,true,this));
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
//        System.out.println("Nächstaes Polygon:");
        convexer.writePolygone(fs,tmp);
        fs.write("      ]\n");
        fs.write("      solid FALSE\n");
        fs.write("   }\n");
        fs.write("}\n");
    }
}
