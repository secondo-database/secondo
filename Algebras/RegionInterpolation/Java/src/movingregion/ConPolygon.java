/*
 * ConPolygon.java
 *
 * Created on 11. Juni 2007, 14:22
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;
import java.util.*;
/**
 *
 * @author java
 */
public class ConPolygon
{
    ConVertex[] vertices;
    /** Creates a new instance of ConPolygon */
    public ConPolygon(int[][] vertices)
    {
        int n=vertices.length;
        this.vertices=new ConVertex[n];
        this.vertices[0]=new ConVertex(vertices[0][0],vertices[0][1],vertices[0][2],vertices[n-1][1],vertices[n-1][2],vertices[1][1],vertices[1][2]);
        this.vertices[n-1]=new ConVertex(vertices[n-1][0],vertices[n-1][1],vertices[n-1][2],vertices[n-2][1],vertices[n-2][2],vertices[0][1],vertices[0][2]);
        for(int i=1;i<n-1;i++)
        {
            this.vertices[i]=new ConVertex(vertices[i][0],vertices[i][1],vertices[i][2],vertices[i-1][1],vertices[i-1][2],vertices[i+1][1],vertices[i+1][2]);
        }
        if(this.getArea()<0)
            this.reverse();
    }
    
    public ConPolygon(Vector vertices)
    {
        int n=vertices.size();
        int[][] tmp= new int[n][3];
        this.vertices=new ConVertex[n];
        for (int i=0;i<n;i++)
        {
            int[] tmp2=(int[])vertices.get(i);
            tmp[i]=tmp2;
        }
        this.vertices[0]=new ConVertex(tmp[0][0],tmp[0][1],tmp[0][2],tmp[n-1][1],tmp[n-1][2],tmp[1][1],tmp[1][2]);
        this.vertices[n-1]=new ConVertex(tmp[n-1][0],tmp[n-1][1],tmp[n-1][2],tmp[n-2][1],tmp[n-2][2],tmp[0][1],tmp[0][2]);
        for(int i=1;i<n-1;i++)
        {
            this.vertices[i]=new ConVertex(tmp[i][0],tmp[i][1],tmp[i][2],tmp[i-1][1],tmp[i-1][2],tmp[i+1][1],tmp[i+1][2]);
        }
        if(this.getArea()<0)
            this.reverse();
    }
    
    public ConVertex getVertexAt(int pos)
    {
        return(this.vertices[pos]);
    }
    
    public int getIndexAt(int i)
    {
        return(this.vertices[i].getIndex());
    }
    
    public int getNrOfVertices()
    {
        return(this.vertices.length);
    }
    
    public int getPositionOfIndex(int index)
    {
        int res;
        for (res=0;res<vertices.length;res++)
        {
            if(vertices[res].getIndex()==index)
                return(res);
        }
        return(-1);
    }
    
    public void reverse()
    {
        ConVertex[] revVertices=new ConVertex[this.getNrOfVertices()];
        for(int i=0;i<this.getNrOfVertices();i++)
        {
            ConVertex tmp=this.vertices[this.getNrOfVertices()-i-1];
            ConVertex pretmp=this.vertices[(this.getNrOfVertices()-i)%this.getNrOfVertices()];
            ConVertex foltmp=this.vertices[(2*this.getNrOfVertices()-i-2)%this.getNrOfVertices()];
            revVertices[i]=new ConVertex(tmp.getIndex(),tmp.getX(),tmp.getY(),pretmp.getX(),pretmp.getY(),foltmp.getX(),foltmp.getY());
        }
        this.vertices=revVertices;
    }
    
    public double getArea()
    {
        double res=0.0;
        for(int i=0;i<this.getNrOfVertices();i++)
        {
            int ip=(i+1)%this.getNrOfVertices();
            res=res+((this.vertices[i].getX()-this.vertices[ip].getX())*(this.vertices[i].getY()+this.vertices[ip].getY()))/2.0;
        }
        return(res);
    }
    
    public boolean isSplitable(int index1,int index2)
    {
        if(this.getVertexAt(index1).equals(this.getVertexAt(index2)))
            return false;
        if(this.getVertexAt(index1).equals(this.getVertexAt((index2+1)%this.getNrOfVertices())))
            return(false);
        if(this.getVertexAt(index2).equals(this.getVertexAt((index1+1)%this.getNrOfVertices())))
            return(false);
        
        if(this.getVertexAt(index1).equals(this.getVertexAt((index2-1+this.getNrOfVertices())%this.getNrOfVertices())))
            return(false);
        if(this.getVertexAt(index2).equals(this.getVertexAt((index1-1+this.getNrOfVertices())%this.getNrOfVertices())))
            return(false);        
        if(this.nrOfIntersections(index1,index2)!=0)
            return false;        
        int pos1p1=(index1+1)%getNrOfVertices();
        int pos1m1=(index1-1+getNrOfVertices())%getNrOfVertices();
        int pos2p1=(index2+1)%getNrOfVertices();
        int pos2m1=(index2-1+getNrOfVertices())%getNrOfVertices();
        int px1=this.getVertexAt(index1).getX();
        int py1=this.getVertexAt(index1).getY();
        int px2=this.getVertexAt(index2).getX();
        int py2=this.getVertexAt(index2).getY();
        int px1p1=this.getVertexAt(pos1p1).getX();
        int py1p1=this.getVertexAt(pos1p1).getY();
        int px1m1=this.getVertexAt(pos1m1).getX();
        int py1m1=this.getVertexAt(pos1m1).getY();
        int px2p1=this.getVertexAt(pos2p1).getX();
        int py2p1=this.getVertexAt(pos2p1).getY();
        int px2m1=this.getVertexAt(pos2m1).getX();
        int py2m1=this.getVertexAt(pos2m1).getY();
        double angle=ConVertex.getAngle(px1,py1,px1m1,py1m1,px1p1,py1p1);
        double angle1=ConVertex.getAngle(px1,py1,px1m1,py1m1,px2,py2);
        double angle2=ConVertex.getAngle(px1,py1,px2,py2,px1p1,py1p1);
        if(Math.abs(angle-angle1-angle2)>0.001)
            return(false);
        
        angle=ConVertex.getAngle(px2,py2,px2m1,py2m1,px2p1,py2p1);
        angle1=ConVertex.getAngle(px2,py2,px2m1,py2m1,px1,py1);
        angle2=ConVertex.getAngle(px2,py2,px1,py1,px2p1,py2p1);
        if(Math.abs(angle-angle1-angle2)>0.001)
            return(false);
//        int area=(px1m1-px1)*(py1m1+py1)+(px1-px1p1)*(py1p1+py1)+(px1p1-px2)*(py1p1+py2)+(px2-px1m1)*(py1m1+py2);
//        if(area<0)
//            return false;
        
//        /*px1=this.getVertexAt(index1).getX();
//        py1=this.getVertexAt(index1).getY();
//        px2=this.getVertexAt(index2).getX();
//        py2=this.getVertexAt(index2).getY();*/
//        area=(px2m1-px2)*(py2m1+py2)+(px2-px2p1)*(py2p1+py2)+(px2p1-px1)*(py2p1+py1)+(px1-px2m1)*(py2m1+py1);
//        if(area<0)
//            return false;
        
        return true;
    }
    
    public boolean intersects(int p1x, int p1y, int p2x, int p2y, int q1x,  int q1y,int q2x,int q2y)
    {
        if(p1x==q1x&&p1y==q1y)
            return false;
        if(p1x==q2x&&p1y==q2y)
            return false;
        if(p2x==q1x&&p2y==q1y)
            return false;
        if(p2x==q2x&&p2y==q2y)
            return false;
        int d=(p1x-q1x)*(q2y-q1y)+(p1y-q1y)*(q1x-q2x);
        int f=(q2x-q1x)*(p2y-p1y)+(q2y-q1y)*(p1x-p2x);
        int e=(p1x-q1x)*(p2y-p1y)+(p1y-q1y)*(p1x-p2x);

        if ( f  > 0 )
        {
            if(d < 0 || d > f ) return false;
        }
        else
            if (d > 0 || d < f ) return false;
        if ( f  > 0 )
        {
            if(e < 0 || e > f ) return false;
        }
        else
            if (e > 0 || e < f ) return false;
        return true;
    }
    
    public boolean intersects(int p1, int p2, int q1, int q2)
    {
        int p1x=0,p1y=0,q1x=0,q1y=0,p2x=0,p2y=0,q2x=0,q2y=0;
        p1x=this.getVertexAt(p1).getX();
        p1y=this.getVertexAt(p1).getY();
        p2x=this.getVertexAt(p2).getX();
        p2y=this.getVertexAt(p2).getY();
        q1x=this.getVertexAt(q1).getX();
        q1y=this.getVertexAt(q1).getY();
        q2x=this.getVertexAt(q2).getX();
        q2y=this.getVertexAt(q2).getY();
        return(intersects(p1x,p1y,p2x,p2y,q1x,q1y,q2x,q2y));
    }
    
    public int nrOfIntersections(int p1x,int p1y,int p2x,int p2y)
    {
        int count=0;
        for(int i=0;i<vertices.length-1;i++)
        {
            
            if(intersects(vertices[i].getX(),vertices[i].getY(),vertices[i+1].getX(),vertices[i+1].getY(),p1x,p1y,p2x,p2y))
                count++;
        }
        if(intersects(vertices[0].getX(),vertices[0].getY(),vertices[vertices.length-1].getX(),vertices[vertices.length-1].getY(),p1x,p1y,p2x,p2y))
            count++;
        return(count);
    }
    
    public int nrOfIntersections(int p1,int p2)
    {
        int count=0;
        for(int i=0;i<vertices.length-1;i++)
        {
            if(intersects(i,i+1,p1,p2))
                count++;
        }
        if(intersects(0,vertices.length-1,p1,p2))
            count++;
        return(count);
    }
    
    public Vector getConcaveVertices()
    {
        Vector res=new Vector();
        for(int i=0;i<this.vertices.length;i++)
        {
            if(this.vertices[i].isConcave())
                res.add(new Integer(i));
        }
        return(res);
    }
    
    public static ConPolygon[] split(ConPolygon inPoly,int index1,int index2)
    {
        Vector tmp=new Vector();
        Vector tmpb=new Vector();
        ConPolygon[]outPoly=new ConPolygon[2];
        boolean found=false;
        for(int i=0;i<inPoly.vertices.length;i++)
        {
            if(i==index1||i==index2)
            {
                int[]tmp2=new int[3];
                tmp2[0]=inPoly.vertices[i].getIndex();
                tmp2[1]=inPoly.vertices[i].getX();
                tmp2[2]=inPoly.vertices[i].getY();
                tmp.add(tmp2);
                tmpb.add(tmp2);
                found=found^true;
            }
            else
            {
                if(found)
                {
                    int[]tmp2=new int[3];
                    tmp2[0]=inPoly.vertices[i].getIndex();
                    tmp2[1]=inPoly.vertices[i].getX();
                    tmp2[2]=inPoly.vertices[i].getY();
                    tmp.add(tmp2);
                }
                else
                {
                    int[]tmp2=new int[3];
                    tmp2[0]=inPoly.vertices[i].getIndex();
                    tmp2[1]=inPoly.vertices[i].getX();
                    tmp2[2]=inPoly.vertices[i].getY();
                    tmpb.add(tmp2);
                }
            }
        }
        outPoly[0]=new ConPolygon(tmpb);
        outPoly[1]=new ConPolygon(tmp);
        return(outPoly);
    }
    
    public int getNrOfIndex(int index)
    {
        int res=0;
        for(int i=0;i<this.vertices.length;i++)
        {
            if(this.getIndexAt(i)==index)
                res++;
        }
        return(res);
    }
    
    public void addHoles(ConPolygon[] holes)
    {
        for(int k=0;k<holes.length;k++)
        {
            all:
                for(int i=0;i<this.getNrOfVertices();i++)
                {
                    for( int j=0;j<holes[k].getNrOfVertices();j++)
                    {
                        if(this.nrOfIntersections(this.getVertexAt(i).getX(),this.getVertexAt(i).getY(),holes[k].getVertexAt(j).getX(),holes[k].getVertexAt(j).getY())==0)
                        {
                            int intersections=0;
                            for(int kk=0;kk<holes.length;kk++)
                            {
                                intersections+=holes[kk].nrOfIntersections(this.getVertexAt(i).getX(),this.getVertexAt(i).getY(),holes[k].getVertexAt(j).getX(),holes[k].getVertexAt(j).getY());
                            }
                            if(intersections==0)
                            {
                                if(this.getNrOfIndex(getIndexAt(i))<2)
                                {
                                    this.addHoleAt(holes[k],i,j);
                                    break all;
                                }
                            }
                        }
                    }
                }
        }
    }
    
    
    public void addHoleAt(ConPolygon hole, int atPoly,int atHole)
    {
        if(hole.getArea()>0)
        {
            int index=hole.getIndexAt(atHole);
            hole.reverse();
            atHole=hole.getPositionOfIndex(index);
        }
        ConVertex[] newVertices=new ConVertex[vertices.length+hole.getNrOfVertices()+2];
        int counter=0;
        for(int i=0;i<=atPoly;i++)
        {
            newVertices[i]=vertices[i];
            counter++;
        }
        for(int i=0;i<=hole.getNrOfVertices();i++)
        {
            newVertices[counter]=hole.getVertexAt((i+atHole)%hole.getNrOfVertices());
            counter++;
        }
        for(int i=atPoly;i<this.getNrOfVertices();i++)
        {
            newVertices[counter]=vertices[i];
            counter++;
        }
        newVertices[atPoly]=new ConVertex(newVertices[atPoly],newVertices[(atPoly-1+newVertices.length)%newVertices.length],newVertices[(atPoly+1)%newVertices.length]);
        newVertices[(atPoly+1)%newVertices.length]=new ConVertex(newVertices[(atPoly+1)%+newVertices.length],newVertices[atPoly],newVertices[(atPoly+2)%newVertices.length]);
        int nextindex=atPoly+hole.getNrOfVertices()+1;
        newVertices[nextindex]=new ConVertex(newVertices[nextindex],newVertices[(nextindex-1+newVertices.length)%newVertices.length],newVertices[(nextindex+1)%newVertices.length]);
        newVertices[(nextindex+1)%newVertices.length]=new ConVertex(newVertices[(nextindex+1)%+newVertices.length],newVertices[nextindex],newVertices[(nextindex+2)%newVertices.length]);
        vertices=newVertices;
        
    }
    
    public String toString()
    {
        String res="";
        for (int i=0;i<this.vertices.length;i++)
        {
            res=res+vertices[i];
        }
        return res;
    }
}
