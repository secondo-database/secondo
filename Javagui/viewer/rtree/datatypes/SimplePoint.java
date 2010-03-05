package viewer.rtree.datatypes;

/**
 * The SimplePoint class represents a point with two integer coordinates. 
 * 
 * It is used as a result of the projection() method from Point2D and Point3D. 
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public class SimplePoint
{
	public int x,y;

	// constructors
	
	/**
	 * Creates a new SimplePoint object with default coordinates.
	 */
	public SimplePoint()
	{
		this.x = 0;
		this.y = 0;
	}
	
	// public methods
	
	/**
	 * Creates a new SimplePoint object with the given coordinates.
	 * @param x X coordinate
	 * @param y Y coordinate
	 */
	public SimplePoint(int x, int y)
	{
		this.x = x;
		this.y = y;
	}
}
