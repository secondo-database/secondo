package viewer.rtree.datatypes;

import java.awt.Color;
import java.awt.Graphics;
import sj.lang.*;
import viewer.rtree.gui.NodeStatus;
import viewer.rtree.gui.ProjectionParameters;
import viewer.rtree.gui.RenderParameters;

/**
 * The Segment class represents a segment. It is used by classes 
 * such as LineBase, UPoint or MovingCycle.
 * 
 * Segment offers functionality to draw a segment and extract a segment from a
 * list expression.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public class Segment
{
	private Point start, end;
	
	// constructors
	
	/**
	 * Creates a new Segment object from two given points.
	 * @param start Start point
	 * @param end End point
	 */
	public Segment(Point start, Point end)
	{
		this.start = start;
		this.end = end;
	}
	
	// public methods
	
	/**
	 * Clones the Segment object.
	 * @return Cloned Segment object
	 */
	public Segment clone()
	{
		return new Segment(start, end);
	}
	
	/**
	 * Returns the string representation of a segment.
	 * @return String representation of a segment
	 */
	public String toString()
	{
		return start.toString() + " " + end.toString();
	}
	
	/**
	 * Checks whether a list expression represents a segment.
	 * @param le List expression
	 * @return True if the list contains a segment, otherwise false
	 */
	public static boolean isSegment(ListExpr le)
	{
		if ( le.listLength() != 4 )
		{
			return false;
		}
		
		ListExpr x1, y1, x2, y2;
		
		x1 = le.first();
		y1 = le.second();
		x2 = le.third();
		y2 = le.fourth();
		
		if (!(x1.isAtom() && x1.atomType() == ListExpr.REAL_ATOM &&
			  y1.isAtom() && y1.atomType() == ListExpr.REAL_ATOM &&
			  x2.isAtom() && x2.atomType() == ListExpr.REAL_ATOM &&
			  y2.isAtom() && y2.atomType() == ListExpr.REAL_ATOM))
		{
			return false;
		}
		
		return true;
	}
	
	
	/**
	 * Converts a given list expression of the form (<x1> <y1> <x2> <y2>)
	 * into a Segment object.
	 * @param le List representation of a Segment object
	 * @return Segment object
	 */
	public static Segment fromListExpr(ListExpr le)
	{
		double x1,y1,x2,y2;
		
		x1 = le.first().realValue();
		y1 = le.second().realValue();
		x2 = le.third().realValue();
		y2 = le.fourth().realValue();
	
		Point2D start = new Point2D(x1, y1);
		Point2D end = new Point2D(x2, y2);

		return new Segment(start, end);
	}
	
	/**
	 * Gets the start point.
	 * @return Start point
	 */
	public Point getStartPoint()
	{
		return start;
	}
	
	/**
	 * Gets the end point.
	 * @return End point
	 */
	public Point getEndPoint()
	{
		return end;
	}

	/**
	 * Draws the Segment object.
	 * @param g Graphic context
	 * @param pp Projection parameters
	 * @param rp Render parameters
	 * @param status Current object status
	 */
	public void draw(Graphics g, ProjectionParameters pp, RenderParameters rp, NodeStatus status)
	{
		// project
		SimplePoint p1, p2;
		p1 = start.project(pp);
		p2 = end.project(pp);	
		
		// apply color
		Color lineColor = rp.chooseLineColor(status);
		g.setColor(lineColor);
		
		// draw segment
		g.drawLine( p1.x, p1.y, p2.x, p2.y); 
	}
}
