package viewer.rtree.datatypes;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Polygon;
import java.util.LinkedList;
import sj.lang.ListExpr;
import viewer.rtree.gui.NodeStatus;
import viewer.rtree.gui.ProjectionParameters;
import viewer.rtree.gui.RenderParameters;

/**
 * A StaticCycle is a polygon consisting of n (n > 3) 2D or 3D points.
 * 
 * The static cycle class is used in the static face class.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public class StaticCycle extends Cycle 
{
	protected LinkedList<Point> points; // a StaticCycle can consist of temporalPoints too
	
	// constructors
	
	/**
	 * Creates a new StaticCycle object from a list of points.
	 * @param segments List of faces
	 */
	public StaticCycle(LinkedList<Point> points)
	{
		this.points = points;
	}
	
	// public methods
	
	/**
	 * Clones the StaticCycle object.
	 * @return Cloned StaticCycle object
	 */
	public StaticCycle clone()
	{
		LinkedList<Point> clonedPoints = new LinkedList<Point>();
		
		for (Point p : points)
		{
			clonedPoints.add(p.clone());
		}
		
		return new StaticCycle(clonedPoints);
	}
	
	/**
	 * Checks whether a list expression represents a static cycle.
	 * @param le List expression
	 * @return True if the list contains a static cycle, otherwise false
	 */
	public static boolean isStaticCycle(ListExpr le)
	{
		return Point2D.isPointList(le);
	}
	
	/**
	 * Reads a static cycle consisting of 2D Points from a list expression.
	 * @param le List expressoin
	 * @return Static cycle
	 */
	public static StaticCycle readNonTemporalCycle(ListExpr le)
	{
		LinkedList<Point2D> staticPoints = Point2D.readPointList( le );
		return new StaticCycle( new LinkedList<Point>(staticPoints) );
	}
	
	/**
	 * Reads a static cycle consisting of 3D Points from a list expression.
	 * @param leInstant List expression containing the instant for the points
	 * @param lePoints List expression containing the points
	 * @return Static cycle
	 */
	public static StaticCycle readTemporalCycle(ListExpr leInstant, ListExpr lePoints)
	{
		Instant instant = Instant.readInstant(leInstant);
		LinkedList<Point2D> staticPoints = Point2D.readPointList(lePoints);
		
		LinkedList<Point3D> tempPoints = new LinkedList<Point3D>();
		double timeCoordinate = instant.toPixel();
		
		for (Point2D p : staticPoints )
		{
			tempPoints.add( new Point3D(p.x, p.y, timeCoordinate));
		}
		
		StaticCycle result = new StaticCycle(new LinkedList<Point>(tempPoints));
		return result;
	}
	
	/**
	 * Returns the string representation of a static cycle.
	 * @return String representation of a static cycle
	 */
	public String toString()
	{			
		String result = "(";
		for (Point p: points)
		{
			result +=  p.toString();
		}
		
		result += ")";
		
		return result;
	}
	
	/**
	 * Draws the StaticCycle object.
	 * @param g Graphic context
	 * @param pp Projection parameters
	 * @param rp Render parameters
	 * @param status Current object status
	 */
	public void draw(Graphics g, ProjectionParameters pp, RenderParameters rp, 
																		NodeStatus status)
	{		
		// create the polygon
		Polygon poly = new Polygon();
		for (Point p : points )
		{
			// project and build polygon
			SimplePoint sp = p.project(pp);
			poly.addPoint(sp.x, sp.y);
		}
		
		// apply color
		Color lineColor = rp.chooseCircleLineColor(status);
		g.setColor(lineColor);
		
		// draw polygon
		g.drawPolygon(poly);
	}
}
