package viewer.rtree.secondotypes;
import sj.lang.ListExpr;
import viewer.rtree.Interval;

/**
 * The Rect class renders the Secondo datatype rect from the SpatialAlgebra.
 * A Secondo rect consists of two intervals.
 * This class uses the RectBase class for most of its functionality.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.2
 * @since 17.12.2009
 *
 */
public class Rect extends RectBase 
{
	// constructors
	
	/**
	 * Creates a new Rect object from the given set of two intervals.
	 * @param Array of intervals
	 */
	public Rect(Interval[] intervals)
	{
		super(intervals);
	}
	
	// public methods

	/**
	 * Creates a Rect object from a list expression.
	 * @param le List expression
	 * @return Rect object
	 */
	public static Rect fromListExpr(ListExpr le)
	{		
		Interval[] intervals = readIntervalsFromListExpr(le, 2);
		return new Rect(intervals);
	}	
}
