package viewer.rtree.datatypes;
import java.awt.Graphics;
import java.util.*;
import viewer.rtree.gui.NodeStatus;
import viewer.rtree.gui.ProjectionParameters;
import viewer.rtree.gui.RenderParameters;

/**
 * The face class is the abstract base class for static and moving faces.
 * 
 * A face consists of n cycles (n > 1). The first cycle is considered the outer cycle,
 * the remaining cycles are considered inner cycles. See SpatialAlgebra for more informaiton
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public abstract class Face 
{
	// public members
	
	public abstract LinkedList<Cycle> getCycles();
	public abstract Face clone();
	
	/**
	 * Returns the string representation of a face.
	 * @return String representation of a face
	 */
	public String toString()
	{
		LinkedList<Cycle> cycles = getCycles();
		
		String result = "(";
		for (Cycle c : cycles)
		{
			result += c.toString() + "\n";
		}
		
		result = result.substring(0, result.length()-2); // Cut the last "\n"
		result += ")";
		return result;
	}
	
	/**
	 * Draws the Face object.
	 * @param g Graphic context
	 * @param pp Projection parameters
	 * @param rp Render parameters
	 * @param status Current object status
	 */
	public void draw(Graphics g, ProjectionParameters pp, RenderParameters rp, 
																	NodeStatus status)
	{
		LinkedList<Cycle> cycles = getCycles();
		
		for (Cycle c : cycles)
		{
			c.draw(g, pp, rp, status);
		}
	}
}
