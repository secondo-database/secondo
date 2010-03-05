package viewer.rtree.datatypes;
import viewer.rtree.gui.ProjectionParameters;

/**
 * The Point3D represents a two dimensional point with a time coordinate.
 * 
 * The Point3D implements projection.
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 22.01.2010
 */
public class Point3D extends Point {

	private double x, y, time;
	
	public Point3D( double x, double y, double time)
	{
		this.x = x;
		this.y = y;
		this.time = time;
	}
	
	public Point3D clone()
	{
		return new Point3D(x, y, time);
	}
	
	public String toStringWithoutDatatype()
	{
		return "(" + x + " " + y + " " + time + ")";
	}


	/**
	 * Projects the point and applies offset & scaling to it
	 * @param pp The projection parameters to use
	 */
	public SimplePoint project(ProjectionParameters pp)
	{
		// Retrieve the necessary projection information
		double scaleFactor, timeScaleFactor, offsetX, offsetY;
		int padding, extraPaddingTop;
		
		scaleFactor 	= 	pp.getScaleFactor();
		timeScaleFactor =	pp.getTimeScaleFactor();
		offsetX 		= 	pp.getOffsetX();
		offsetY 		= 	pp.getOffsetY();
		padding 		= 	pp.getPadding();
		extraPaddingTop = 	pp.getExtraPaddingTop();
		
		// Do the projection & scaling, save results in p
		SimplePoint p = new SimplePoint();
		switch ( pp.getProjectionDimX() )
		{
			case Drawable.AXIS_X: p.x = (int) ((x + offsetX) * scaleFactor); break;
			case Drawable.AXIS_Y: p.x = (int) ((y + offsetX) * scaleFactor); break;
			case Drawable.AXIS_TIME: p.x = (int) ((time + offsetX) * timeScaleFactor); break;
			default: return null;
		}
		
		switch ( pp.getProjectionDimY() )
		{
			case Drawable.AXIS_X: p.y = (int) ((x + offsetY) * scaleFactor); break;
			case Drawable.AXIS_Y: p.y = (int) ((y + offsetY) * scaleFactor); break;
			case Drawable.AXIS_TIME: p.y = (int) ((time + offsetY) * timeScaleFactor); break;
			default: return null;
		}		
		
		// Apply paddings
		p.x = p.x + padding;
		p.y = p.y + padding + extraPaddingTop;
		
		return p;
	}
}
