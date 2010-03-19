package viewer.rtree.datatypes;
import java.awt.Color;
import java.awt.Graphics;

import viewer.rtree.gui.NodeStatus;
import viewer.rtree.gui.ProjectionParameters;
import viewer.rtree.gui.RenderParameters;

/**
 * The point class is the abstract base class for Point2D and Point3D.
 * 
 * Using this base class, high level objects can consist of either 2 or 3 dimensional points
 * and can easily implement projection.
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.2
 * @since 08.03.2010
 */
public abstract class Point 
{
	/**
	 * Projects the point and applies offset and scaling to it.
	 * @param pp Projection parameters to use
	 * @return SimplePoint object with projected coordinates
	 */
	public abstract SimplePoint project(ProjectionParameters pp);
	
	
	/**
	 * Clones the Point object.
	 * @return Cloned Point object
	 */
	public abstract Point clone();
	

	/**
	 * Draws the Point object.
	 * @param g Graphic context
	 * @param pp Projection parameters
	 * @param rp Render parameters
	 * @param status Current object status
	 */
	public void draw(Graphics g, ProjectionParameters pp, RenderParameters rp, NodeStatus status)
	{
		// project point
		SimplePoint p = project(pp);
		
		// if point is not visible in this projection do nothing
		if (p == null) 
		{ 
			return;
		}
		
		// apply colors and circle width
		Color lineColor = rp.chooseCircleLineColor(status);
		Color fillColor = rp.chooseCircleFillColor(status);
		int circleWidth = rp.getCircleWidth();
		
		// draw point
		g.setColor(fillColor);
		g.fillOval(p.x - circleWidth/2, p.y - circleWidth/2, circleWidth+1, circleWidth+1);
		
		g.setColor(lineColor);
		g.drawOval(p.x - circleWidth/2, p.y - circleWidth/2, circleWidth+1, circleWidth+1);
	}
}
