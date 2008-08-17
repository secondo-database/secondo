/*
 * ConVertex.java
 *
 * Created on 10. Juni 2007, 22:08
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;

/**
 *
 * @author java
 */
public class ConVertex
{
    private int index;
    private int xCoord;
    private int yCoord;
    private boolean isConcave=false;
    
    
    /** Creates a new instance of ConVertex */
    public ConVertex(int index, int x, int y, int preX,int preY,int folX,int folY)
    {
        this.index=index;
        this.xCoord=x;
        this.yCoord=y;
        if(((x-preX)*(folY-preY)-(folX-preX)*(y-preY))<0)
            isConcave=true;
    }
    
    
    public static double getAngleRad(int x, int y, int preX,int preY,int folX,int folY)
    {
        
        double a=Math.sqrt((preX-folX)*(preX-folX)+(preY-folY)*(preY-folY));
        double b=Math.sqrt((preX-x)*(preX-x)+(preY-y)*(preY-y));
        double c=Math.sqrt((folX-x)*(folX-x)+(folY-y)*(folY-y));
        double angle=Math.acos((a*a-b*b-c*c)/(-2*b*c));
        if(((x-preX)*(folY-preY)-(folX-preX)*(y-preY))<0)            
            angle=Math.PI*2*-angle;
        return (angle);
    }
    
    public static double getAngle(int x, int y, int preX,int preY,int folX,int folY)
    {
        
        double a=Math.sqrt((preX-folX)*(preX-folX)+(preY-folY)*(preY-folY));
        double b=Math.sqrt((preX-x)*(preX-x)+(preY-y)*(preY-y));
        double c=Math.sqrt((folX-x)*(folX-x)+(folY-y)*(folY-y));
        double angle=Math.toDegrees(Math.acos((a*a-b*b-c*c)/(-2*b*c)));
        if(((x-preX)*(folY-preY)-(folX-preX)*(y-preY))<0)            
            angle=360-angle;
        return (angle);
    }
    public ConVertex(ConVertex in,ConVertex pre, ConVertex fol)
    {
        this(in.getIndex(),in.getX(),in.getY(),pre.getX(),pre.getY(),fol.getX(),fol.getY());
    }
    
    public int getIndex()
    {
        return(index);
    }
    
    public int getX()
    {
        return(this.xCoord);
    }
    
    public int getY()
    {
        return(this.yCoord);
    }
    
    public boolean isConcave()
    {
        return(this.isConcave);
    }
    
    public String toString()
    {
        return(index+": "+"("+xCoord+";"+yCoord+")\n");
    }
    
    public int hashCode()
    {
        return((456122 * this.getX() + 45168 * this.getY() + 4513 * this.index) % 451357821);        
    }
    
    public boolean equals(Object o)
    {
        if(!(o instanceof ConVertex))
            return false;
        ConVertex other=(ConVertex)o;
        boolean res =true;
        if(this.getX()!=other.getX())
            res=false;
        if(this.getY()!=other.getY())
            res=false;
        if(this.getIndex()!=other.getIndex())
            res=false;
        return(res);
    }
    
}
