package viewer.rtree.secondotypes;

import sj.lang.ListExpr;
import viewer.rtree.Interval;

/**
 * The Rect class renders the Secondo datatype rect3 from the SpatialAlgebra.
 * A Secondo rect3 consists of three intervals.
 * This class uses the RectBase class for most of its functionality.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.2
 * @since 17.12.2009
 *
 */
public class Rect3 extends RectBase 
{
	// constructors
	
	/**
	 * Creates a new Rect object from the given set of three intervals.
	 * @param Array of intervals
	 */
	public Rect3(Interval[] intervals)
	{
		super(intervals);
	}
	
	// public methods

	/**
	 * Creates a Rect3 object from a list expression.
	 * @param le List expression
	 * @return Rect object
	 */
	public static Rect3 fromListExpr(ListExpr le)
	{		
		Interval[] intervals = readIntervalsFromListExpr(le, 3);
		return new Rect3(intervals);
	}	
}
