package viewer.rtree.datatypes;

import java.awt.Graphics;

import viewer.rtree.gui.NodeStatus;
import viewer.rtree.gui.ProjectionParameters;
import viewer.rtree.gui.RenderParameters;

/**
 * The Cycle class is the abstract base class for static and moving cycles.
 * 
 * By this means, a Face can consist of either static or moving cycles. See StaticCycle
 * and MovingCycle for additional information.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public abstract class Cycle
{
	// public methods
	
	/**
	 * Draws the Cycle object.
	 * @param g Graphic context
	 * @param pp Projection parameters
	 * @param rp Render parameters
	 * @param status Current object status
	 */
	public abstract void draw(Graphics g, ProjectionParameters pp, 
									RenderParameters rp, NodeStatus status);
}
