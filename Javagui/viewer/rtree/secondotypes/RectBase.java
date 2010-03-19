package viewer.rtree.secondotypes;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.geom.Rectangle2D;

import viewer.rtree.Interval;
import viewer.rtree.datatypes.TypeBase;
import sj.lang.ListExpr;

/**
 * This is an abstract base class for the Rect and RectX Secondo datatypes.
 * 
 * An n-dimensional rectangle is represented by a list of n intervals.
 * This class provides functionality to extract those intervals from a 
 * list expression, project them to two dimensions and render the
 * resulting 2-dimensional rectangle.
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.2
 * @since 08.03.2010
 */
public abstract class RectBase extends TypeBase 
{
	private Interval[] intervals;
	
	// constructors
	
	/**
	 * Creates a new RectBase object from a given array of intervals.
	 * @param intervals Array of intervals
	 */
	public RectBase(Interval[] intervals)
	{
		this.intervals = intervals;
	}
	
	// public methods
	
	/**
	 * Returns the string representation of a rectangle object.
	 * @return String representation of a rectangle object
	 */
	public String toString()
	{
		String result = "Rect" + intervals.length + " (";
		
		for (Interval i: intervals)
		{
			result += " " + i.getLeft() + " " + i.getRight();
		}
		result += ")";

		return result;
	}
	
	/**
	 * Extracts a list of intervals from the given list expression.
	 * @param le List expression
	 * @param noDimensions The number of intervals
	 * @return Array of intervals
	 */
	public static Interval[] readIntervalsFromListExpr(ListExpr le, int noDimensions)
	{
		// Precondition: le must be a rectx object
		ListExpr intervalList = le;
		Interval[] intervals = new Interval[noDimensions];
		int dimensionCounter = 0;
		double left, right;
		
		// read all dimensions consecutively
		while (!intervalList.isEmpty())
		{
			left = intervalList.first().realValue();
			right = intervalList.second().realValue();
			intervals[dimensionCounter] = new Interval(left, right);
		
			intervalList = intervalList.rest();
			intervalList = intervalList.rest();
			
			dimensionCounter++;
		}
		
		return intervals;
	}
	
	
	/**
	 * Draws the rectangle.
	 * @param g Graphic context
	 */
	public void draw(Graphics g)
	{
		// retrieve projection information
		double scaleFactor, offsetX, offsetY;
		int projectionXDim, projectionYDim, padding, extraPaddingTop;
		
		scaleFactor 	= projectionParameters.getScaleFactor();
		offsetX 		= projectionParameters.getOffsetX();
		offsetY 		= projectionParameters.getOffsetY();
		projectionXDim	= projectionParameters.getProjectionDimX();
		projectionYDim	= projectionParameters.getProjectionDimY();
		padding 		= projectionParameters.getPadding();
		extraPaddingTop = projectionParameters.getExtraPaddingTop();
		
		// project
		if (projectionXDim > intervals.length-1 ||
			projectionYDim > intervals.length-1)
		{
			// if rectangle is not visible in this projection do nothing
			return;
		}
		
		Interval i1 = intervals[projectionXDim];
		Interval i2 = intervals[projectionYDim];
		
		// scale and apply offsets
		int x,y, width, height;
		x = (int)  ((i1.getLeft() + offsetX) * scaleFactor) + padding;
		y = (int)  ((i2.getLeft() + offsetY) * scaleFactor) + padding + extraPaddingTop;
		width = (int) (i1.getLength() * scaleFactor);
		height = (int) (i2.getLength() * scaleFactor);
		
		// apply fill color
		Color fillColor = renderParameters.chooseCircleFillColor(nodeStatus);
		g.setColor(fillColor);
		// draw rectangle area
		g.fillRect(x, y, width, height);

		// apply border color
		Color lineColor = renderParameters.chooseCircleLineColor(nodeStatus);
		g.setColor(lineColor);
		// draw rectangle
		g.drawRect(x, y, width, height);
	}
}
