package viewer.rtree.datatypes;

import sj.lang.*;
import viewer.rtree.gui.ProjectionParameters;
import java.util.*;

/**
 * The Point2D class represents a two dimensional point.
 * 
 * It implements projection and offers functionality to retrieve a point
 * or a list of points from a list expression.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 22.01.2010
 */
public class Point2D extends Point {

	public double x, y;
	
	// constructors
	
	/**
	 * Creates a new Point2D object with the given coordinates.
	 * @param x X coordinate
	 * @param y Y coordinate
	 */
	public Point2D(double x, double y)
	{
		this.x = x;
		this.y = y;
	}
	
	// public methods
	
	/**
	 * Clones the Point2D object.
	 * @return Cloned Point2D object
	 */
	public Point2D clone()
	{
		return new Point2D(x, y);
	}
		
	/**
	 * Returns the string representation of a Point2D object.
	 * @return String representation of a Point2D object
	 */
	public String toString()
	{
		return "(" + x + " " + y + ")";
	}
	
	/**
	 * Checks if the given list expression represents a Point2D object.
	 * @return True if the given list expression represents a Point2D object, otherwise false.
	 */
	public static boolean isPoint(ListExpr le)
	{
		if ( le.listLength() != 2 )
		{
			return false;
		}
			
		ListExpr x = le.first();
		ListExpr y = le.second();
		
		if (! (x.isAtom() && x.atomType() == ListExpr.REAL_ATOM &&
				y.isAtom() && y.atomType() == ListExpr.REAL_ATOM))
		{
			return false;
		}
		
		return true;
	}
	
	
	/**
	 * Checks if the given list expression represents a list of Point2D objects.
	 * @return True if the given list expression represents a list of Point2D objects, otherwise false.
	 */
	public static boolean isPointList(ListExpr le)
	{
		ListExpr listOfPoints = le;	
		while (! listOfPoints.isEmpty())
		{
			if (! Point2D.isPoint( listOfPoints.first() ))
			{
				return false;
			}
			
			listOfPoints = listOfPoints.rest();
		}
		
		return true;
	}
	
	/**
	 * Converts a given list expression into a Point2D object.
	 * @param le List representation of a Point object
	 * @return Point2D object
	 */
	public static Point2D readPoint(ListExpr le)
	{
		double x = le.first().realValue();
		double y = le.second().realValue();
		
		return new Point2D(x, y);
	}
	
	/**
	 * Converts a given list expression into a list of Point2D objects.
	 * @param le List representation of a list of Point objects
	 * @return List of Point2D objects
	 */
	public static LinkedList<Point2D> readPointList(ListExpr le)
	{
		ListExpr listOfPoints = le;
		LinkedList<Point2D> points = new LinkedList<Point2D>();

		while (! listOfPoints.isEmpty())
		{
			points.add( Point2D.readPoint( listOfPoints.first() ));
			listOfPoints = listOfPoints.rest();
		}
		
		return points;
	}
	
	/**
	 * Projects the point and applies offset and scaling to it.
	 * @param pp Projection parameters to use
	 * @return SimplePoint object with projected coordinates
	 */
	public SimplePoint project(ProjectionParameters pp)
	{
		// retrieve projection information
		double scaleFactor, offsetX, offsetY;
		int padding, extraPaddingTop;
		
		scaleFactor 	= pp.getScaleFactor();
		offsetX 		= pp.getOffsetX();
		offsetY 		= pp.getOffsetY();
		padding 		= pp.getPadding();
		extraPaddingTop = pp.getExtraPaddingTop();
		
		// project and scale, save results to p
		SimplePoint p = new SimplePoint();
		switch (pp.getProjectionDimX())
		{
			case Drawable.AXIS_X: p.x = (int) ((x + offsetX) * scaleFactor); break;
			case Drawable.AXIS_Y: p.x = (int) ((y + offsetX) * scaleFactor); break;
			default: return null;
		}
		
		switch (pp.getProjectionDimY())
		{
			case Drawable.AXIS_X: p.y = (int) ((x + offsetY) * scaleFactor); break;
			case Drawable.AXIS_Y: p.y = (int) ((y + offsetY) * scaleFactor); break;
			default: return null;
		}		
		
		// apply padding
		p.x = p.x + padding;
		p.y = p.y + padding + extraPaddingTop;
		
		return p;
	}
}