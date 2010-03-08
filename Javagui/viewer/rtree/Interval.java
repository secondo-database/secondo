package viewer.rtree;

/**
 * Interval represented by its left and right borders.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.0
 * @since 16.12.2009
 */
public class Interval {

	// borders
	private double left, right;
	
	/**
	 * Creates a new interval.
	 * @param left Left border
	 * @param right Right border
	 */
	public Interval(double left, double right)
	{
		this.left = left;
		this.right = right;
	}
	
	/**
	 * Gets the left border.
	 * @return Left border
	 */
	public double getLeft()
	{
		return this.left;
	}
	
	/**
	 * Gets the right border.
	 * @return Right border
	 */
	public double getRight()
	{
		return this.right;
	}
	
	
	/**
	 * Gets the interval's length.
	 * @return Interval length
	 */
	public double getLength()
	{
		return right - left;
	}
}
