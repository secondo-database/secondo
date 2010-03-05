package viewer.rtree.secondotypes;

import java.awt.Graphics;
import sj.lang.*;
import viewer.rtree.datatypes.TypeBase;

import java.util.*;

/**
 * The MovingRegion class visualizes the secondo datatyp movingregion from the
 * MovingRegionAlgebra. A secondo movingregion consists of several uregions
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 24.01.2010
 */
public class MovingRegion extends TypeBase {

	private LinkedList<URegion> uregions; 
	
	public MovingRegion(LinkedList<URegion> uregions)
	{
		this.uregions = uregions;
	}
	
	public MovingRegion clone()
	{
		LinkedList<URegion> clonedRegions = new LinkedList<URegion>();
		for (URegion uregion : uregions)
		{
			clonedRegions.add( uregion.clone() );
		}
		
		return new MovingRegion(clonedRegions);
	}
	
	public String toString()
	{
		String result = "MRegion(\n";
		for (URegion uregion : uregions)
		{
			result += uregion.toString() + "\n";
		}
		
		return result;
	}
	

	public static MovingRegion fromListExpr(ListExpr le)
	{
		ListExpr listOfURegions = le;
		
		LinkedList<URegion> uregions = new LinkedList<URegion>();
		
		while (! listOfURegions.isEmpty())
		{
			uregions.add( URegion.fromListExpr( listOfURegions.first() ));
			listOfURegions = listOfURegions.rest();
		}
		
		return new MovingRegion(uregions);
	}
	
	public void draw(Graphics g)
	{
		for (URegion uregion : uregions)
		{
			uregion.draw(g, projectionParameters, renderParameters, nodeStatus);
		}
	}
}
