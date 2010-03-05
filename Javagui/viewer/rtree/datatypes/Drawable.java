package viewer.rtree.datatypes;

import java.awt.*;
import viewer.rtree.gui.*;

/**
 * This interface must be implemented by all 
 * referenced type classes to be displayed.
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.4
 * @since 20.02.2010
 */
public interface Drawable {

	public final static int AXIS_TIME = 2;
	public final static int AXIS_X = 0;
	public final static int AXIS_Y = 1;
	
	/**
	 * Draws the object
	 * @param g Graphic context
	 */
	public void draw(Graphics g);

	/**
	 * Sets the actual render parameter set based on the given node status.
	 * @param renderParams Render parameters
	 * @param nodeStatus Current node status
	 */
	public void setRenderParameters(RenderParameters renderParams, NodeStatus nodeStatus);

	/**
	 * Sets the actual projection parameter set.
	 * @param projectionParams Projection parameters
	 */
	public void setProjectionParameters(ProjectionParameters projectionParams);

	/**
	 * Sets the corresponding tuple-Id.
	 * @param TID tupleId
	 */
	public void setTupleId(int TID);

	/**
	 * Gets the corresponding tuple-Id.
	 */
	public int getTupleId();
}
