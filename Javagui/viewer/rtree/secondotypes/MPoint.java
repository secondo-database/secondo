package viewer.rtree.secondotypes;

import java.awt.Graphics;
import java.util.*;
import sj.lang.*;
import viewer.rtree.datatypes.TypeBase;

/**
 * The MPoint class represents the secondo datatype mpoint from the TemporalAlgebra.
 * 
 * A mpoint consists of several UPoint objects. Look there for additional information
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 23.1.2010
 */
public class MPoint extends TypeBase {

	LinkedList<UPoint> upoints;
	
	public MPoint(LinkedList<UPoint> upoints )
	{
		this.upoints = upoints;
	}
	
	public String toString()
	{
		String result ="MPoint(\n";
		for (UPoint up : upoints)
		{
			result += up.toStringWithoutDatatype() + "\n";
		}
		
		result = result.substring(0, result.length()-2); // Cut the last "\n"
		result += ")";
		return result;
	}
		
	public static MPoint fromListExpr(ListExpr le)
	{
		ListExpr listOfUpoints = le;
		LinkedList<UPoint> upoints = new LinkedList<UPoint>();
		
		while (! listOfUpoints.isEmpty())
		{
			upoints.add( UPoint.fromListExpr( listOfUpoints.first()) );
			
			listOfUpoints = listOfUpoints.rest();
		}
		
		return new MPoint(upoints);
	}
	
	public void draw(Graphics g)
	{
		for (UPoint up : upoints)
		{
			up.draw(g, projectionParameters, renderParameters, nodeStatus);
		}
	}
}
