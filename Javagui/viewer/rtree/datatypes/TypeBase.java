package viewer.rtree.datatypes;

import java.awt.*;
import viewer.rtree.gui.*;
import sj.lang.*;
/**
 * TypeBase is the common base for all Secondo type classes.
 * It is used to provide the actual render parameter and
 * projection parameter sets used by the derived classes
 * draw() method.
 * 
 * @author Oliver Feuer
 * @author Christian Oevermann
 * @author Benedikt Buer
 * @since 08.03.2010
 * @version 1.3
 */
public abstract class TypeBase implements Drawable
{
	protected NodeStatus nodeStatus;
	protected RenderParameters renderParameters;
	protected ProjectionParameters projectionParameters;
	protected int tupleId = 0;
	
	// flags
	boolean projectionParametersSet = false;
	boolean renderParametersSet = false;
	
	// public methods
	
	/**
	 * Gets the actual node status.
	 * @return nodeStatus Node status
	 */
	public NodeStatus getNodeStatus()
	{
		return nodeStatus;
	}
	
	/**
	 * Gets the actual render parameter set.
	 * @return renderParams Render parameters
	 */
	public RenderParameters getRenderParameters()
	{
		return renderParameters;
	}
	
	/**
	 * Sets the actual render parameter set and node status.
	 * @param renderParams Render parameters
	 * @param nodeStatus Current node status
	 */
	public void setRenderParameters(RenderParameters renderParams, NodeStatus nodeStatus)
	{
		this.renderParameters = renderParams;
		this.nodeStatus = nodeStatus;
		renderParametersSet = true;
	}

	/**
	 * Gets the corresponding tuple-Id.
	 * @param TID tupleId
	 */
	public int getTupleId()
	{
		return this.tupleId;
	}

	/**
	 * Sets the corresponding tuple-Id.
	 * @param TID tupleId
	 */
	public void setTupleId(int TID)
	{
		this.tupleId = TID;
	}

	/**
	 * Gets the actual projection parameter set.
	 * @return projectionParams Projection parameters
	 */
	public ProjectionParameters getProjectionParameters()
	{
		return projectionParameters;
	}

	/**
	 * Sets the actual projection parameter set.
	 * @param projectionParams Projection parameters
	 */
	public void setProjectionParameters(ProjectionParameters projectionParams)
	{
		this.projectionParameters = projectionParams;
		projectionParametersSet = true;
	}
	
	/**
	 * Draws the object. Must be implemented in derived type classes.
	 * @param g Graphic context
	 */
	public abstract void draw(Graphics g);

	// protected methods
	
	protected static boolean isDatatype(ListExpr le, String datatype)
	{
		// Check number of items and their type
		if (le.listLength() != 2)
		{
			return false;
		}
		
		if (! le.first().isAtom()) 
		{
			return false;
		}
		
		
		// Check if the ListExpr has the correct typename
		if (!(( le.first().atomType() == ListExpr.SYMBOL_ATOM ) &&
			  ( le.first().stringValue().equals(datatype))))
		{
			return false;
		}
		
		return true;
	}
	
	/**
	 * Indicates if all parameters have been set.
	 * @return True if all parameters have been set, otherwise false.
	 */
	protected boolean parametersSet()
	{
		return (this.renderParametersSet && this.projectionParametersSet);		
	}
}
