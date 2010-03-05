package viewer.rtree.secondotypes;

import java.awt.Graphics;
import sj.lang.*;
import viewer.rtree.datatypes.*;
import java.util.*;

/**
 * The InTimeRegion class visualizes the secondo datatyp intimeregion from the
 * MovingRegionAlgebra. A secondo intimeregion consists  a region with an instant
 * 
 * The InTimeRegion class constructs a GenericRegion out of static faces which in 
 * turn consist of cycles of Point3D
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 23.01.2010
 */
public class InTimeRegion extends TypeBase {
	
	private GenericRegion genericRegion; // This object holds all the geometrical data
	
	public InTimeRegion(LinkedList<StaticFace> faces)
	{
		LinkedList<Face> genericFaces = new LinkedList<Face>(faces);
		genericRegion = new GenericRegion(genericFaces);
	}

	public InTimeRegion(GenericRegion genericRegion)
	{
		this.genericRegion = genericRegion;
	}
	
	public Region clone()
	{		
		return new Region(genericRegion.clone());
	}
		
	public String toString()
	{
		return "IRegion\n" +  genericRegion.toString();
	}
	
	public static InTimeRegion fromListExpr(ListExpr le)
	{
		ListExpr iRegionData = le;
		ListExpr instant = iRegionData.first();
		ListExpr listOfFaces = iRegionData.second();

		LinkedList<StaticFace> faces = new LinkedList<StaticFace>();
		
		while (! listOfFaces.isEmpty())
		{
			faces.add( StaticFace.readTemporalFace(instant, listOfFaces.first()));
			
			listOfFaces = listOfFaces.rest();
		}
		
		
		return new InTimeRegion(faces);
	}

	public void draw(Graphics g)
	{
		genericRegion.draw(g, projectionParameters, renderParameters, nodeStatus);
	}


}
