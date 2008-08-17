/*
 * Convexer.java
 *
 * Created on 12. Juni 2007, 01:16
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;
import java.util.*;
import java.io.*;

public class Convexer
{
    Vector Polygones=new Vector();
    /** Creates a new instance of Convexer */
    
    
    private void convex()
    {
        int actual=0;
        Vector conVert;
        all:
            do
            {
            ConPolygon actPoly=((ConPolygon)Polygones.elementAt(actual));
            conVert=actPoly.getConcaveVertices();
            if(conVert.size()>0)
            {
                if(conVert.size()>1)
                {
                    for(int i=0;i<conVert.size();i++)
                    {
                        for (int j=i+1;j<conVert.size();j++)
                        {
                            if(actPoly.isSplitable(((Integer)conVert.elementAt(i)).intValue(),((Integer)conVert.elementAt(j)).intValue()))
                            {
                                if(TriRepUtil.debugging)
                                    System.out.println("Splitte an 2 konkaven Ecken:"+actPoly.getIndexAt(((Integer)conVert.elementAt(i)).intValue())+" und "+actPoly.getIndexAt(((Integer)conVert.elementAt(j)).intValue()));
                                ConPolygon[] tmpPoly=ConPolygon.split(actPoly,((Integer)conVert.elementAt(j)).intValue(),((Integer)conVert.elementAt(i)).intValue());
                                if(TriRepUtil.debugging)
                                {
                                    System.out.println("in: \n"+tmpPoly[0]);
                                    System.out.println("und: \n"+tmpPoly[1]);
                                }
                                this.Polygones.add(tmpPoly[1]);
                                this.Polygones.setElementAt(tmpPoly[0],actual);
                                continue all;
                            }
                        }
                    }
                }
                int posCon=((Integer)conVert.elementAt(0)).intValue();
                int postest=(posCon+actPoly.getNrOfVertices()/2)%actPoly.getNrOfVertices();
                for (int i=1;i<actPoly.getNrOfVertices();i++)
                {
                    int split1=((Integer)conVert.elementAt(0)).intValue();
                    int split2=(postest+i/2*(int)Math.pow(-1,i)+actPoly.getNrOfVertices())%actPoly.getNrOfVertices();
                    if(actPoly.isSplitable(split1,split2))
                    {
                        if(TriRepUtil.debugging)
                            System.out.println("Splitte an: "+actPoly.getIndexAt(split1)+" und "+actPoly.getIndexAt(split2)+" index "+split1+" ; "+split2);
                        ConPolygon[] tmp=ConPolygon.split(actPoly,((Integer)conVert.elementAt(0)).intValue(),(postest+i/2*(int)Math.pow(-1,i)+actPoly.getNrOfVertices())%actPoly.getNrOfVertices());
                        this.Polygones.add(tmp[1]);
                        this.Polygones.setElementAt(tmp[0],actual);
                        break;
                    }
                }
            }
            else
            {
                actual++;
            }
            
            }
        while(conVert.size()>0||actual<Polygones.size());
    }
    
    public Convexer(ConPolygon inPoly)
    {
        
        if(inPoly.getArea()<0)
            inPoly.reverse();
        Polygones.add(inPoly);
        this.convex();
        
    }
    
    public Convexer(int[][] vertices)
    {
        ConPolygon test=new ConPolygon(vertices);
        if(test.getArea()<0)
            test.reverse();
        Polygones.add(test);
        this.convex();
    }
    
    public Convexer(int[] ordertlist,Vector points)
    {
        int[][] tmp=new int[ordertlist.length][3];
        for (int i=0;i<ordertlist.length;i++)
        {
            PointWNL point=(PointWNL)points.elementAt(ordertlist[i]);
            tmp[i][0]=ordertlist[i];
            tmp[i][1]=point.x;
            tmp[i][2]=point.y;
        }
        ConPolygon test=new ConPolygon(tmp);
        if(test.getArea()<0)
            test.reverse();
        Polygones.add(test);
        this.convex();
    }
    
    public void writePolygone(OutputStreamWriter fs,Vector points)throws IOException
    {
        ConPolygon actPoly;
        for(int i=0;i<this.Polygones.size();i++)
        {
            fs.write("      ");
            actPoly=((ConPolygon)Polygones.elementAt(i));
            for(int j=0;j<actPoly.getNrOfVertices();j++)
            {
                fs.write(getPositionOfIndex(actPoly.getIndexAt(j),  points)+" ");
            }
            fs.write(getPositionOfIndex(actPoly.getIndexAt(0),  points)+" -1\n");
        }
    }
    
    public int getPositionOfIndex(int index, Vector points)
    {
        int res=0;
        for (int i=0;i<index;i++)
        {
            if(points.elementAt(i)!=null)
                res++;
        }
        return res;
    }
}
