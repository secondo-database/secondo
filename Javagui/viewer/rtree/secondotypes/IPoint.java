package viewer.rtree.secondotypes;
import java.awt.Graphics;
import sj.lang.*;
import viewer.rtree.datatypes.*;

/**
 * The IPoint class represents the secondo datatype ipoint from the TemporalAlgebra
 * 
 * A Point3D object is used to provide storage and the projection/drawing functionality
 * 
 * @author Benedikt Buer
 * @version 1.1
 * @since 21.1.2010
 *
 */
public class IPoint extends TypeBase {

	private Point3D tempPoint;

	
	/**
	 * Constructs the IPoint
	 * @param secondoDate Format YYYY-MM-DD-HH:MM:SS.xxx.. (x arbritrary precision)
	 * @param x X-coordinate of the point
	 * @param y Y-coordinate of the point
	 */
	public IPoint( Point3D tempPoint)
	{
		this.tempPoint = tempPoint;
	}
	
	public String toString()
	{
		return "IPoint( " + tempPoint.toString() + ")";
	}
	
	/**
	 * Checks if a list expression represents an ipoint object
	 * @param le The list expression
	 * @return True, if the list represents an ipoint, false else
	 */
	public static boolean isIPoint(ListExpr le)
	{
		if (! isDatatype(le, "ipoint"))
		{
			return false;
		}
		
		// IPoint consists of instant and point value
		ListExpr ipoint = le.second();
		
		if ( le.second().listLength() != 2)
		{
			return false;
		}

		return Instant.isInstant(ipoint.first()) && Point2D.isPoint(ipoint.second());
	}
	

	/**
	 * Constructs an ipoint object from a list expression
	 * @param le The list expression
	 * @return The constructed ipoint object
	 */
	public static IPoint fromListExpr(ListExpr le)
	{
		Instant i = Instant.readInstant(le.first());
		Point2D p = Point2D.readPoint(le.second());

		Point3D tempPoint = new Point3D(p.x, p.y, i.toPixel());
		return new IPoint(tempPoint);
	}
	
	public void draw(Graphics g)
	{
		tempPoint.draw(g, projectionParameters, renderParameters, nodeStatus);
	}
}
