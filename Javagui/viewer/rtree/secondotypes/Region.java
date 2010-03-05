package viewer.rtree.secondotypes;

import java.awt.Graphics;

import sj.lang.ListExpr;
import viewer.rtree.datatypes.*;
import java.util.*;

/**
 * The Region class represents the Secondo datatype region from the SpatialAlgebra.
 * A Secondo region consists of a set of faces.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public class Region extends TypeBase 
{
	private GenericRegion genericRegion;
	
	// constructors
	
	/**
	 * Creates a new Region object from a list of faces.
	 * @param segments List of faces
	 */
	public Region(LinkedList<StaticFace> faces)
	{
		LinkedList<Face> genericFaces = new LinkedList<Face>(faces);
		this.genericRegion = new GenericRegion(genericFaces);
	}
	
	/**
	 * Creates a new Region object from a GenericRegion object.
	 * @param genericRegion Generic region object
	 */
	public Region(GenericRegion genericRegion)
	{
		this.genericRegion = genericRegion;
	}
	
	// public methods
	
	/**
	 * Clones the Region object.
	 * @return Cloned Region object
	 */
	public Region clone()
	{		
		return new Region(this.genericRegion.clone());
	}
		
	/**
	 * Returns the string representation of a region.
	 * @return String representation of a region
	 */
	public String toString()
	{
		return "Region\n" + genericRegion.toString();
	}
	
	/**
	 * Converts a given list expression into a Region object.
	 * @param le List representation of a Region object
	 * @return Region object
	 */
	public static Region fromListExpr(ListExpr le)
	{
		ListExpr listOfFaces = le;
		
		LinkedList<StaticFace> faces = new LinkedList<StaticFace>();
		
		while (!listOfFaces.isEmpty())
		{
			faces.add(StaticFace.readStaticFace(listOfFaces.first()));
			listOfFaces = listOfFaces.rest();
		}
		
		return new Region(faces);
	}

	/**
	 * Draws the Region object.
	 * @param g Graphic context
	 */
	public void draw(Graphics g)
	{
		genericRegion.draw(g, projectionParameters, renderParameters, nodeStatus);
	}
}
