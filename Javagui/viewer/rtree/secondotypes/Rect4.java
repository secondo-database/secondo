package viewer.rtree.secondotypes;

import sj.lang.ListExpr;
import viewer.rtree.Interval;

/**
 * The Rect class renders the Secondo datatype rect4 from the SpatialAlgebra.
 * A Secondo rect4 consists of four intervals.
 * This class uses the RectBase class for most of its functionality.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.2
 * @since 17.12.2009
 *
 */
public class Rect4 extends RectBase 
{
	// constructors
	
	/**
	 * Creates a new Rect object from the given set of four intervals.
	 * @param Array of intervals
	 */
	public Rect4(Interval[] intervals)
	{
		super(intervals);
	}
	
	// public methods

	/**
	 * Creates a Rect4 object from a list expression.
	 * @param le List expression
	 * @return Rect object
	 */
	public static Rect4 fromListExpr(ListExpr le)
	{		
		Interval[] intervals = readIntervalsFromListExpr(le, 4);
		return new Rect4(intervals);
	}	
}
