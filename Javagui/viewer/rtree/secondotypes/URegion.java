package viewer.rtree.secondotypes;

import java.awt.Graphics;
import java.util.LinkedList;
import viewer.rtree.datatypes.*;
import viewer.rtree.gui.NodeStatus;
import viewer.rtree.gui.ProjectionParameters;
import viewer.rtree.gui.RenderParameters;
import sj.lang.*;



/**
 * The URegion class visualizes the secondo datatyp uregion from the
 * MovingRegionAlgebra. A secondo uregion consists of a (moving) region and a time intervall
 * 
 * The URegoin class constructs a GenericRegion out of moving faces
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 23.01.2010
 */
public class URegion extends TypeBase {

	private GenericRegion genericRegion;
	
	public URegion(LinkedList<MovingFace> faces)
	{
		LinkedList<Face> genericFaces = new LinkedList<Face>(faces);
		genericRegion = new GenericRegion(genericFaces);
	}

	public URegion(GenericRegion genericRegion)
	{
		this.genericRegion = genericRegion;
	}
	
	public URegion clone()
	{		
		return new URegion(genericRegion.clone());
	}
		
	public String toString()
	{
		return "URegion\n" +  genericRegion.toString();
	}
		
	public static URegion fromListExpr(ListExpr le)
	{
		ListExpr uRegionData = le;
		ListExpr intervall = uRegionData.first();
		ListExpr listOfFaces = uRegionData.second();

		LinkedList<MovingFace> faces = new LinkedList<MovingFace>();
		
		while (! listOfFaces.isEmpty())
		{
			faces.add( MovingFace.readMovingFace(intervall, listOfFaces.first()));
			
			listOfFaces = listOfFaces.rest();
		}
		
		
		return new URegion(faces);
	}
	
	public void draw(Graphics g)
	{
		genericRegion.draw(g, projectionParameters, renderParameters, nodeStatus);
	}

	// MRegion needs to call this method because it hides the 
	// URegion object from the NodeViewerPanel and so the parameters are not set
	public void draw(Graphics g, ProjectionParameters pp, RenderParameters rp, 
																NodeStatus status)
	{
		genericRegion.draw(g, pp, rp, status);
	}
}
