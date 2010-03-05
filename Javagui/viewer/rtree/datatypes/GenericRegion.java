package viewer.rtree.datatypes;

import java.awt.Graphics;
import java.util.LinkedList;
import viewer.rtree.gui.*;

/**
 * The GenericRegion consists of a list of multiple faces (static or moving).
 * 
 * It is used to visualize the Secondo types region, intimeregion, uregion and movingregion
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public class GenericRegion
{
	private LinkedList<Face> faces;
	
	// constructors
	
	/**
	 * Creates a new GenericRegion object from a list of faces.
	 * @param segments List of faces
	 */
	public GenericRegion(LinkedList<Face> faces)
	{
		this.faces = faces;
	}
	
	/**
	 * Clones the GenericRegion object.
	 * @return Cloned GenericRegion object
	 */
	public GenericRegion clone()
	{
		LinkedList<Face> clonedFaces = new LinkedList<Face>();
		for (Face f : faces)
		{
			clonedFaces.add(f.clone());
		}
		
		return new GenericRegion(clonedFaces);
	}

	/**
	 * Returns the string representation of a generic region.
	 * @return String representation of a generic region
	 */
	public String toString()
	{
		String result = "(";
		for (Face f : faces)
		{
			result += f.toString() + "\n";
		}
		result = result.substring(0, result.length() - 2); // cut last "\n"
		result += ")";
		
		return result;	
	}

	/**
	 * Draws the GenericRegion object.
	 * @param g Graphic context
	 * @param pp Projection parameters
	 * @param rp Render parameters
	 * @param status Current object status
	 */
	public void draw(Graphics g, ProjectionParameters pp, RenderParameters rp, 
			NodeStatus status)
	{
		for (Face f : faces )
		{
			f.draw(g, pp, rp, status);
		}
	}
}
