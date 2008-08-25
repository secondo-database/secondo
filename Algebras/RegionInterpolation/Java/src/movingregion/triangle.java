/*
 * triangle.java
 *
 * Created on 3. September 2007, 18:35
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;

/**
 *
 * @author java
 */
public class triangle
{
    
    
    public triangle(PointWNL p1, PointWNL p2, PointWNL p3, int faceNr, int cycleNr)
    {
        this.p1=p1;
        this.p2=p2;
        this.p3=p3;
        this.faceNr=faceNr;
        this.cycleNr=cycleNr;
    }
    PointWNL p1;
    PointWNL p2;
    PointWNL p3;
    int faceNr;
    int cycleNr;
    
    /** Creates a new instance of triangle */
    public triangle()
    {
    }
    
}
