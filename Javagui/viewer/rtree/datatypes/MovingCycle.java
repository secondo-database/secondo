package viewer.rtree.datatypes;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Polygon;
import java.util.*;

import sj.lang.*;
import viewer.rtree.gui.NodeStatus;
import viewer.rtree.gui.ProjectionParameters;
import viewer.rtree.gui.RenderParameters;


/**
 * A moving cycle is a set of two polygosn consisting of n (n>3) 2D or 3D points.
 * The first polygon marks the position of the cycle at the beginning of the movement
 * the second polygon marks the position at the end of the movement.
 * 
 * Internally the moving cycle is saved as a set of segment. 
 * 
 * The moving cycle class is used in the moving face class.
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 23.01.2010
 */
public class MovingCycle extends Cycle{

	protected LinkedList<Segment> segments;
	
	public MovingCycle(LinkedList<Segment> segments)
	{
		this.segments = segments;
	}
	
	
	public String toString()
	{			
		String result = "(";
		for (Segment s: segments)
		{
			result +=  s.toString();
		}
		
		result += " )";
		return result;
	}
	
	
	public MovingCycle clone()
	{
		LinkedList<Segment> clonedSegments = new LinkedList<Segment>();
		
		for ( Segment s : segments)
		{
			clonedSegments.add( s.clone() );
		}
		
		return new MovingCycle(clonedSegments);
	}

	
	public static MovingCycle fromListExpr(ListExpr timeInterval, ListExpr segmentList)
	{
		// Get the time interval from the list
		TimeInterval time = TimeInterval.fromListExpr(timeInterval);
		
		// Now read the segments and create the moving cycle
		LinkedList<Segment> temporalSegments = new LinkedList<Segment>();
		double startTime = time.getStartCoordinate(1);
		double endTime = time.getEndCoordinate(1);
			
		while (! segmentList.isEmpty())
		{
			ListExpr segment = segmentList.first();
			
			double x1, y1, x2, y2;
			x1 = segment.first().realValue();
			y1 = segment.second().realValue();
			x2 = segment.third().realValue();
			y2 = segment.fourth().realValue();
			
			Point3D startPoint, endPoint;
			startPoint = new Point3D(x1, y1, startTime);
			endPoint = new Point3D(x2, y2, endTime);

			temporalSegments.add( new Segment(startPoint, endPoint));
			
			segmentList = segmentList.rest();
		}
		
		return new MovingCycle(temporalSegments);
	}
	
	
	

	public void draw(Graphics g, ProjectionParameters pp, RenderParameters rp, 
																	NodeStatus status)
	{
		// Cycles doesn`t use segments to draw the cycles because it is easier
		// to fill the cycle with the polygon datatype
		
		// Build the polygons
		Polygon startPolygon = new Polygon();
		Polygon endPolygon = new Polygon();
		
		Point startPoint, endPoint;
		SimplePoint sp, ep;
		
		for (Segment s : segments )
		{	
			startPoint = s.getStartPoint();
			endPoint = s.getEndPoint();
			
			sp = startPoint.project(pp);
			ep = endPoint.project(pp);
			
			startPolygon.addPoint(sp.x, sp.y);
			endPolygon.addPoint(ep.x, ep.y);
		}
		
		g.drawPolygon(startPolygon);
		g.drawPolygon(endPolygon);		
		
		
		// Draw the polygons
		Color lineColor = rp.chooseCircleLineColor(status);
		g.setColor(lineColor);
		
		// Draw the connections between the polygons
		for (Segment s : segments)
		{
			s.draw(g, pp, rp, status);
		}
	}
}
