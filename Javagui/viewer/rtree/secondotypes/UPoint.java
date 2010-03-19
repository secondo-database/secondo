package viewer.rtree.secondotypes;

import java.awt.Graphics;

import sj.lang.ListExpr;
import viewer.rtree.datatypes.*;
import viewer.rtree.gui.*;

/**
 * The UPoint class represents the secondo datatype upoint from the TemporalAlgebra.
 * 
 * A upoint can be visualized by a segment consisting of two Point3D objects
 * 
 * @author Benedikt Buer
 * @version 1.1
 * @since 21.01.2010
 *
 */
public class UPoint extends TypeBase {

	Point3D start, end;
	 
	public UPoint(Point3D start, Point3D end)
	{
		this.start = start;
		this.end = end;
	}
	
	public String toString()
	{
		return "UPoint" + toStringWithoutDatatype();
	}
	
	public String toStringWithoutDatatype()
	{
		return "(" + start.toString() + " " + end.toString() + ")";
	}
		
	/**
	 * Extracts an upoint object from a list expression
	 * @param le The list expression
	 * @return The constructed upoint object
	 */
	public static UPoint fromListExpr(ListExpr le)
	{
		TimeInterval timeInterval = TimeInterval.fromListExpr(le.first());
		ListExpr segment = le.second();
		
		// We can`t create the segment directly from the list because we have
		// to add the time coordinate. First create two points and then create the segment
		double x1,y1,x2,y2;
		x1 = segment.first().realValue();
		y1 = segment.second().realValue();
		x2 = segment.third().realValue();
		y2 = segment.fourth().realValue();
		
		Point3D start = new Point3D(x1, y1, timeInterval.getStartCoordinate(1));
		Point3D end = new Point3D(x2, y2, timeInterval.getEndCoordinate(1));
		
		return new UPoint(start, end);
	}

	
	public void draw(Graphics g)
	{
		Segment temporalSegment = new Segment(start, end);
		temporalSegment.draw(g, projectionParameters, renderParameters, nodeStatus);
	}
	
	// MPoint needs to call this method because it hides the 
	// UPoint object from the NodeViewerPanel and so the parameters are not set
	public void draw(Graphics g, ProjectionParameters pp, RenderParameters rp, NodeStatus status)
	{
		Segment temporalSegment = new Segment(start, end);
		temporalSegment.draw(g, pp, rp, status);
	}
}
