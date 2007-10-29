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
//        System.out.println((ConPolygon)Polygones.elementAt(0));
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
//                                System.out.println("Splitte an 2 konkaven Ecken:"+actPoly.getIndexAt(((Integer)conVert.elementAt(i)).intValue())+" und "+actPoly.getIndexAt(((Integer)conVert.elementAt(j)).intValue()));
                                ConPolygon[] tmpPoly=ConPolygon.split(actPoly,((Integer)conVert.elementAt(j)).intValue(),((Integer)conVert.elementAt(i)).intValue());
//                                System.out.println("in: \n"+tmpPoly[0]);
//                                System.out.println("und: \n"+tmpPoly[1]);
                                this.Polygones.add(tmpPoly[1]);
                                this.Polygones.setElementAt(tmpPoly[0],actual);
                          //      actPoly=((ConPolygon)Polygones.elementAt(actual));
                           //     conVert=actPoly.getConcaveVertices();
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
//                        System.out.println("Splitte an: "+actPoly.getIndexAt(split1)+" und "+actPoly.getIndexAt(split2)+" index "+split1+" ; "+split2);
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
//        for (int i=0;i<Polygones.size();i++)
//        {
//            System.out.println((ConPolygon)Polygones.elementAt(i));
//        }
    }
    
    public Convexer(ConPolygon inPoly)
    {
        
        if(inPoly.getArea()<0)
            inPoly.reverse();
//        System.out.println(inPoly);
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
                fs.write(getPositionOfIndex(actPoly.getIndexAt(j),  points)+" ");;
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
    
    public static void main(String[] args)
    {
        //int[][] poly={{2, 420, 140 },{1, 530, 350 },{0, 300, 350 },{5, 420, 220  },{3,390, 310  },{4, 460, 300  },{15, 420, 220  },{10, 300, 350 }};
        //Convexer test=new Convexer(poly);
        
//        int[][]poly={
//        {2, 564,197},
//        {1, 456,59},
//        {0, 567,40}};
//
//        int[][]hole1={
//        {9, 557,103},
//        {3, 547,66},
//        {4, 544,88},
//        {5, 516,84},
//        {6, 499,66},
//        {7, 506,93},
//        {8, 549,133}};
//POlygon mit zwei Löchern
        int[][]poly={{6, 290,71},
        {5, 433,127},
        {4, 516,288},
        {3, 316,365},
        {2, 136,366},
        {1, 76,261},
        {0, 114,101}};
        int[][]hole1={
            {7, 141,123},
            {8, 116,235},
            {9, 167,345},
            {10,278,328}};
        int[][]hole2={
            {11, 363,154},
            {12, 331,173},
            {13, 266,113},
            {14, 323,281},
            {15, 455,262},
            {16, 403,147}};
        ConPolygon test=new ConPolygon(poly);
        ConPolygon[] holes=new ConPolygon[2];
        holes[0]=new ConPolygon(hole1);
        holes[1]=new ConPolygon(hole2);
        test.addHoles(holes);
        //test.addHole(new ConPolygon(hole2));
        Convexer test2=new Convexer(test);
        
        
//        ConPolygon test2=new ConPolygon(poly);
//        System.out.println(test2.getArea());
//        test2.reverse();
//        System.out.println(test2.getArea());
////        ConPolygon[] test2=split(test,1,4);
////        System.out.println(test.intersects(1,2,3,4));
////        System.out.println(test.intersects(1,2,2,4));
////        System.out.println(test.nrOfIntersections(1,4));
////        System.out.println(test.nrOfIntersections(1,3));
////        System.out.println(test.nrOfIntersections(2,5));
////        System.out.println(test.nrOfIntersections(1,5));
    }
}
