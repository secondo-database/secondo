package viewer.rtree.secondotypes;

import java.awt.Graphics;
import java.util.*;
import sj.lang.ListExpr;
import viewer.rtree.datatypes.Point2D;
import viewer.rtree.datatypes.TypeBase;

/**
 * The Points class renders the Secondo datatype Points from the SpatialAlgebra.
 * A Secondo Points object consists of several two dimensional Point objects.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public class Points extends TypeBase {

	private LinkedList<Point2D> points;
	
	// constructors
	
	/**
	 * Creates a new Points object from a given list of Point2D objects.
	 * @param point A list of Point2D points
	 */
	public Points (LinkedList<Point2D> points)
	{
		this.points = points;
	}
	
	// public methods
	
	/**
	 * Returns the string representation of a Points object.
	 * @return String representation of a Points object
	 */
	public String toString()
	{
		String result = "Points (";
		for (Point2D p: points)
		{
			result +=  p.toString();
		}
		result += ")";

		return result;
	}
	
	/**
	 * Converts a given list expression into a Points object.
	 * @param le List representation of a Points object
	 * @return Points object
	 */
	public static Points fromListExpr(ListExpr le)
	{
		LinkedList<Point2D> pointList = Point2D.readPointList(le);
		return new Points(pointList);
	}
	
	
	/**
	 * Draws the Points object.
	 * @param g Graphic context
	 */
	public void draw(Graphics g)
	{
		for (Point2D p : points)
		{
			p.draw(g, projectionParameters, renderParameters, nodeStatus);
		}
	}
}
