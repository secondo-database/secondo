package viewer.rtree.secondotypes;
import java.awt.Graphics;
import viewer.rtree.datatypes.Point2D;
import viewer.rtree.datatypes.TypeBase;
import sj.lang.*;

/**
 * The Point class renders the Secondo datatype point from the SpatialAlgebra.
 * A Secondo point consists of a single two dimensional point.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.0
 * @since 23.01.2010
 */
public class Point extends TypeBase 
{
	private Point2D point;
	
	// constructors
	
	/**
	 * Creates a new Point object from a given Point2D object.
	 * @param point A Point2D point
	 */
	public Point(Point2D point)
	{
		this.point = point;
	}
	
	// public methods
	
	/**
	 * Returns the string representation of a Point object.
	 * @return String representation of a Point object
	 */
	public String toString()
	{
		return "Point " + point.toString();
	}
	
	/**
	 * Converts a given list expression into a Point object.
	 * @param le List representation of a Point object
	 * @return Point object
	 */
	public static Point fromListExpr(ListExpr le)
	{
		Point2D staticPoint = Point2D.readPoint(le);
		return new Point(staticPoint);
	}
		
	/**
	 * Draws the Point object.
	 * @param g Graphic context
	 */
	public void draw(Graphics g)
	{
		point.draw(g, projectionParameters, renderParameters, nodeStatus);
	}
}
