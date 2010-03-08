package viewer.rtree;
import java.awt.geom.Rectangle2D;
import java.text.*;

/**
 * BoundingBox represents an n-dimensional bounding box.
 * The bounding box is defined by one interval per dimension,
 * e.g. a rectangle is defined by two intervals.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @since 16.12.2009
 * @version 1.1
 * 
 */
public class BoundingBox { 
	
	// array of defining intervals
	private Interval[] intervals;
	
	/**
	 * Creates a new bounding box.
	 * @param intervals Array of intervals, one per dimension
	 */
	public BoundingBox(Interval[] intervals) 
	{
		this.intervals = intervals;
	}
	
	/**
	 * Projects the bounding box to a 2-dimensional rectangle
	 * @param dimX First projection dimension
	 * @param dimY Second projection dimension
	 * @return Projected rectangle
	 */
	public Rectangle2D.Double project(int dimX, int dimY) 
	{
		// Rechteck aus den beiden Intervallen bilden
		// C-D	(oben / unten i2)
		// | |
		// A-B	(links / rechts i1)
		// Ax = i1.left();	Ay = i2.left();
		// Bx = i1.right(); By = i2.left();
		// Cx = i1.left(); 	Cy = i2.right();
		// Dx = i1.right(); Dy = i2.right();
			
		Interval x = this.intervals[dimX];
		Interval y = this.intervals[dimY];
		
		return new Rectangle2D.Double(x.getLeft(), y.getLeft(), x.getLength(), y.getLength());
	}
	
	
	/**
	 * Gets the number of dimensions.
	 * @return Number of dimensions
	 */
	public int getNoOfDimension()
	{
		return this.intervals.length;
	}

	/**
	 * Returns string representation of the bounding box.
	 * @return string with coordinates
	 */
	public String toString()
	{
		String res ="";
		DecimalFormat dcFormat = new DecimalFormat("#.###");

		String leftRes  = "("+dcFormat.format(intervals[0].getLeft());
		String rightRes = "("+dcFormat.format(intervals[0].getRight());

		for (int i = 1; i < intervals.length; i++)
		{
			leftRes  += " , "+dcFormat.format(intervals[i].getLeft());
			rightRes += " , "+dcFormat.format(intervals[i].getRight());
		}
		res = leftRes + ") - " + rightRes+")";
		return res;
	}

	/**
	 * Returns string representation of the projected bounding box.
	 * @return string with coordinates dimx, dimy
	 */
	public String projecttoString(int dimX, int dimY)
	{
		String res ="";
		DecimalFormat dcFormat = new DecimalFormat("#.###");

		res += "("+dcFormat.format(intervals[dimX].getLeft());
		res += " , "+dcFormat.format(intervals[dimY].getLeft())+") - ";

		res += "("+dcFormat.format(intervals[dimX].getRight());
		res += " , "+dcFormat.format(intervals[dimY].getRight())+")";

		return res;
	}
}
