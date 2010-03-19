package viewer.rtree.gui;

import java.awt.*;
import java.io.ObjectInputStream.GetField;

/**
 * RenderParameters contains detail information on 
 * colors, line and fill parameters used to render bounding boxes
 * and entities referenced by the rtree.
 * 
 * @author Oliver Feuer
 * @author Christian Oevermann
 * @since 08.03.2010
 * @version 1.2
 */
public class RenderParameters { 
	
	// current node bounding boxes
	Color nodeBBLineColor;
	Color nodeBBFillColor;
	
	// selected node bounding boxes
	Color selectedNodeBBLineColor;
	Color selectedNodeBBFillColor;
	
	// child node bounding boxes
	Color childNodeBBLineColor;
	Color childNodeBBFillColor;
	
	// entry tuple bounding boxes
	Color entryTupleBBLineColor;
	Color entryTupleBBFillColor;
	int minBBsize;
	
	// selected entry tuple bounding boxes
	Color selectedTupleBBLineColor;
	Color selectedTupleBBFillColor;
	
	// circle
	int circleWidth;
	Color circleLineColor;
	Color circleFillColor;
	Color selectedCircleLineColor;
	Color selectedCircleFillColor;
	Color childCircleLineColor;
	Color childCircleFillColor;

	// line
	Color lineColor;
	Color selectedLineColor;
	Color childLineColor;
		
	// constructors
	
	/**
	 * Creates a new RenderParameters object with default values.
	 */
	public RenderParameters() 
	{
		this.nodeBBLineColor = new Color(255, 0, 0, 255);		//red
		this.nodeBBFillColor = new Color(255, 0, 0, 50);		//red transparent

		this.selectedNodeBBLineColor = new Color(0, 0, 0, 255);		//black
		this.selectedNodeBBFillColor = new Color(255, 255, 255, 255);	//white

		this.childNodeBBLineColor = new Color(200, 0, 0, 255);		//light red
		this.childNodeBBFillColor = new Color(200, 0, 0, 50);		//light red transparent

		this.entryTupleBBLineColor = new Color(46, 139, 87, 255);	//green
		this.entryTupleBBFillColor = new Color(46, 139, 87, 50);	//green transparent

		this.minBBsize = 5; 						

		this.selectedTupleBBLineColor = new Color(0, 0, 255, 255);	//blue 
		this.selectedTupleBBFillColor = new Color(0, 0, 255, 50);	//blue transparent

		this.circleWidth = 6; 						// must be divideable by 2

		this.circleLineColor = new Color(255, 0, 0, 255);		//red
		this.circleFillColor = new Color(255, 0, 0, 50);		//red transparent

		this.selectedCircleLineColor = new Color(0, 0, 255, 255);	//blue 
		this.selectedCircleFillColor = new Color(0, 0, 255, 50);	//blue transparent

		this.childCircleLineColor = new Color(200, 0, 0, 255);		//light red
		this.childCircleFillColor = new Color(200, 0, 0, 50);		//light red transparent

		this.lineColor = new Color(255, 0, 0, 255);			//red
		this.selectedLineColor = new Color(0, 0, 255, 255);		//blue
		this.childLineColor = new Color(200, 0, 0, 255);		//light red
	}

	// public members
	
	/**
	 * Gets the current node's bounding box line color.
	 * @return Current node's bounding box line color
	 */
	public Color getNodeBBLineColor()
	{
		return this.nodeBBLineColor;
	}
	
	/**
	 * Gets the current node's bounding box fill color.
	 * @return Current node's bounding box fill color
	 */
	public Color getNodeBBFillColor() 
	{
		return this.nodeBBFillColor;
	}
	
	/**
	 * Gets the selected node's bounding box line color.
	 * @return Selected node's bounding box line color
	 */
	public Color getSelectedNodeBBLineColor() 
	{
		return this.selectedNodeBBLineColor;
	}
	
	/**
	 * Gets the selected node's bounding box fill color.
	 * @return Selected node's bounding box fill color
	 */
	public Color getSelectedNodeBBFillColor() 
	{
		return this.selectedNodeBBFillColor;
	}
	
	/**
	 * Gets the child node's bounding box line color.
	 * @return Child node's bounding box line color
	 */
	public Color getChildNodeBBLineColor() 
	{
		return this.childNodeBBLineColor;
	}
	
	/**
	 * Gets the child node's bounding box fill color.
	 * @return Child node's bounding box fill color
	 */
	public Color getChildNodeBBFillColor() 
	{
		return this.childNodeBBFillColor;
	}
	
	
	/**
	 * Gets the entry tuple's bounding box line color.
	 * @return entry tuple's bounding box line color
	 */
	public Color getEntryTupleBBLineColor() 
	{
		return this.entryTupleBBLineColor;
	}
	
	/**
	 * Gets the entry tuple's bounding box fill color.
	 * @return entry tuple's bounding box fill color
	 */
	public Color getEntryTupleBBFillColor() 
	{
		return this.entryTupleBBFillColor;
	}
	
	/**
	 * Gets the minimum entry-BB size.
	 * @return BB size
	 */
	public int getminBBsize() 
	{
		return this.minBBsize;
	}
	
	/**
	 * Gets the selected tuple's bounding box line color.
	 * @return selected tuple's bounding box line color
	 */
	public Color getSelectedTupleBBLineColor() 
	{
		return this.selectedTupleBBLineColor;
	}
	
	/**
	 * Gets the selected tuple's bounding box fill color.
	 * @return selected tuple's bounding box fill color
	 */
	public Color getSelectedTupleBBFillColor() 
	{
		return this.selectedTupleBBFillColor;
	}
	
	/**
	 * Gets the circle width.
	 * @return Circle width
	 */
	public int getCircleWidth() 
	{
		return this.circleWidth;
	}
	
	public Color chooseCircleLineColor(NodeStatus status)
	{
		switch (status)
		{
			case CURRENT:	return circleLineColor;
			case SELECTED:	return selectedCircleLineColor;
			case CHILD:	return childCircleLineColor;
			default: 	return null;
		}
	}
	
	public Color chooseCircleFillColor(NodeStatus status)
	{
		switch (status)
		{
			case CURRENT:	return circleFillColor;
			case SELECTED:	return selectedCircleFillColor;
			case CHILD:	return childCircleFillColor;
			default: 	return null;
		}
	}
	
	public Color chooseLineColor(NodeStatus status)
	{
		switch (status)
		{
			case CURRENT:	return lineColor;
			case SELECTED:	return selectedLineColor;
			case CHILD:	return childLineColor;
			default: 	return null;
		}
	}

}
