//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.spacetimecube;


import gui.idmanager.ID;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.image.BufferedImage;
import java.util.*;
import javax.swing.*;
import javax.swing.border.LineBorder;
import viewer.SpaceTimeCubeViewer;


/**
 * Class representing the panel on the top right of the viewer showing the 2D map.
 * @author Franz Fahrmeier
 *
 */
public class View2DSTC extends JPanel implements MouseListener, MouseMotionListener {
	
	private BufferedImage img; // map of the 2D-view
	private Vector<Vector> ptArrays; // 2D points for line creation
	private int areaLength; // length of the viewing area
	private boolean initialized; // 2D-view already initialized
	private boolean pressed; // mouse button pressed
	private Point startPoint, endPoint; // dragging start and end point
	private int borderWeight; // weight/width of the 2D-view border
	private Hashtable<ID,float[]> colorSecObj; // stores the color of all SecondoObjects identified by ID
	private SpaceTimeCubeViewer STCViewer; // this variable provides the link to the STCV

	/**
	 * @param image
	 * 		map as BufferedImage
	 * @param pointArrays
	 * 		2D points for line creation
	 * @param length
	 * 		length of the viewing area
	 * @param stcv
	 * 		link to the STCV
	 */
	public void initialize(BufferedImage image, Vector<Vector> pointArrays, int length, SpaceTimeCubeViewer stcv) {
		removeAll(); // remove all from the java.awt.Container
		
		LineBorder border = (LineBorder)getBorder();
		borderWeight = border.getThickness();
		img = image;
		ptArrays = pointArrays;
		areaLength = length-(borderWeight*2); // for calculation purposes the border needs to be subtracted
		if (img != null) img = getScaledImage(img, areaLength, areaLength);
		
		addMouseListener(this);
		addMouseMotionListener(this);
		
		// set initial start and end point
		startPoint = new Point(borderWeight, borderWeight);
		endPoint = new Point(areaLength, areaLength);
		
		STCViewer = stcv;
		colorSecObj = STCViewer.getColorSO();
		
		initialized = true;
	}
	
	public void paint(Graphics g) {
		super.paint(g);
		
		if (initialized) {
			// paint map with border's weight as insets
			if (img != null) g.drawImage(img, borderWeight, borderWeight, null);
			
			double maxX=0, maxY=0, minX=0, minY=0;
			for (int i=0;i<ptArrays.size();i++) {
				Vector<Point2DSTC> pts = ptArrays.get(i); // corresponds to a single MPoint
				for (int a=0;a<pts.size();a++) {
					Point2DSTC pt = (pts.get(a));
					if (i==0 && a==0) {
						maxX = pt.getX();
						maxY = pt.getY();
						minX = pt.getX();
						minY = pt.getY();
					}
					if (pt.getX() > maxX) maxX = pt.getX();
					if (pt.getY() > maxY) maxY = pt.getY();
					if (pt.getX() < minX) minX = pt.getX();
					if (pt.getY() < minY) minY = pt.getY();
				}
			}
			
			/*
			 * Some additional space at X and Y axis needs to be added
			 * so that the map will not immediately start with a trajectory.
			 * Background: Otherwise it's hard to determine where the trajectory starts.
			 */
			double factorAdd = 0.05;
			double lengthX = maxX - minX;
			double lengthY = maxY - minY;
			minX -= (lengthX*factorAdd);
			maxX += (lengthX*factorAdd);
			minY -= (lengthY*factorAdd);
			maxY += (lengthY*factorAdd);
			
			double length; // total X-,Y-axis dimension based on min and max values
			if ((maxX-minX)>(maxY-minY)) length = maxX-minX;
			else length = maxY-minY;
			
			float[] colSO = {0,0,0}; // default color of all SecondoObjects/MPoints
			
			// lines are getting drawn for each MPoint
			for (int i=0;i<ptArrays.size();i++) {
				Vector<Point2DSTC> pts = ptArrays.get(i);
				for (int a=0;a<pts.size();a++) {
					Point2DSTC pt = (pts.get(a));
					Point2DSTC tempPt1 = pt;
					Point2DSTC tempPt2;
					
					colSO = colorSecObj.get(pt.getSecondoID());
					g.setColor(new Color(colSO[0],colSO[1],colSO[2])); // line/trajectory color
					
					if (a==pts.size()-1) { tempPt2 = pt; }
					else { tempPt2 = (pts.get(a+1)); }
					int x1 = ((int)Math.round((areaLength/length*tempPt1.getX())))+borderWeight;
					int y1 = ((int)Math.round((areaLength-(areaLength/length*tempPt1.getY()))))+borderWeight;
					int x2 = ((int)Math.round((areaLength/length*tempPt2.getX())))+borderWeight;
					int y2 = ((int)Math.round((areaLength-(areaLength/length*tempPt2.getY()))))+borderWeight;
					if (x1 <= borderWeight) { x1 = 1; }
					if (y1 <= borderWeight) { y1 = 1; }
					if (x2 <= borderWeight) { x2 = 1; }
					if (y2 <= borderWeight) { y2 = 1; }
					
					if (x1 >= (areaLength+borderWeight)) { x1 = areaLength; }
					if (y1 >= (areaLength+borderWeight)) { y1 = areaLength; }
					if (x2 >= (areaLength+borderWeight)) { x2 = areaLength; }
					if (y2 >= (areaLength+borderWeight)) { y2 = areaLength; }
					g.drawLine(x1,y1,x2,y2);
				}
			}
			
			/*
			 * If a mouse button is pressed the dragging area/selection will be painted.
			 * This only works because the method repaint() is called within mousePressed(MouseEvent e).
			 */
			if (pressed) {
				Color prevCol = new Color(g.getColor().getRGB()); // mpoint/trajectory color is stored
				g.setColor(Color.BLACK);
				g.drawLine(startPoint.x, startPoint.y, endPoint.x, startPoint.y);
				g.drawLine(startPoint.x, startPoint.y, startPoint.x, endPoint.y);
				g.drawLine(endPoint.x, endPoint.y, endPoint.x, startPoint.y);
				g.drawLine(endPoint.x, endPoint.y, startPoint.x, endPoint.y);
				g.setColor(prevCol);
			}
		}
	}
	
	/**
	 * @return
	 * 		the selected/filtered rectangle as pixel value.
	 */
	public Rectangle2D.Double getFilterArea() {
		double x, y;
		if (startPoint.x > endPoint.x) x = endPoint.x;
		else x = startPoint.x;
		if (startPoint.y > endPoint.y) y = endPoint.y;
		else y = startPoint.y;
		Rectangle2D.Double result = new Rectangle2D.Double(x, y, 
					Math.abs(endPoint.x-startPoint.x), Math.abs(endPoint.y-startPoint.y));
		return result;
	}
	
	 public void mouseClicked(MouseEvent e) {
		 repaint();
	 }
	 
	 public void mouseReleased(MouseEvent e) { pressed = false; }
	 
	 public void mouseEntered(MouseEvent e) { }
	 
	 // sets the start point
	 public void mousePressed(MouseEvent e) {
		 pressed = true;
		 startPoint =  e.getPoint();
	 }
	 
	 public void mouseExited(MouseEvent e) { }
	 
	 // sets the end point and REPAINTS the component
	 public void mouseDragged(MouseEvent e) {
		 endPoint = e.getPoint();
		 if (endPoint.getX() > getWidth()) endPoint.x = getWidth()-1;
		 else if (endPoint.getX() < 0) endPoint.x = 0;
		 if (endPoint.getY() > getHeight()) endPoint.y = getHeight()-1;
		 else if (endPoint.getY() < 0) endPoint.y = 0;
		 repaint();
	 }
	 
	 public void mouseMoved(MouseEvent e) { }
	 
	// returns a scaled image based on a source image and a target dimension
	private static BufferedImage getScaledImage(BufferedImage sourceImg, double targetWidth, double targetHeight) {
	    Graphics2D g;
	    int targetWidthInt = (int)Math.round(targetWidth);
	    int targetHeightInt = (int)Math.round(targetHeight); 
	    BufferedImage scaled = new BufferedImage(targetWidthInt, targetHeightInt,
	            BufferedImage.TYPE_INT_RGB);
	    g = scaled.createGraphics();
	    g.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
	            RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
	    g.drawImage(sourceImg, 0, 0, targetWidthInt, targetHeightInt, null);
	    return scaled;
	}
}
