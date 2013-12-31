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

package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import com.google.gwt.user.client.DOM;
import com.google.gwt.user.client.Element;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.secondo.webgui.client.controller.GVMPointController;
import com.secondo.webgui.client.controller.GVPolylineController;
import com.secondo.webgui.client.controller.GVPointController;
import com.secondo.webgui.client.controller.GVPolygonController;
import com.secondo.webgui.shared.model.DataType;
import com.secondo.webgui.shared.model.Line;
import com.secondo.webgui.shared.model.MPoint;
import com.secondo.webgui.shared.model.Point;
import com.secondo.webgui.shared.model.Polygon;
import com.secondo.webgui.shared.model.Polyline;
import com.secondo.webgui.shared.model.TimeInterval;

/**
*  This class represents all elements of the graphical view to display graphical datatypes with the D3-JavaScript library.
*  
*  @author Kristina Steiger
*  
**/
public class GraphicalView extends Composite implements View{
	
	/**Main panel of the graphical view*/
	private FlowPanel contentPanel = new FlowPanel();
	
	/**Width of the graphical view*/
	private int width=Window.getClientWidth()-293;
	
	/**Height of the graphical view*/
    private int height= Window.getClientHeight()-321;
    
    /**Controller for point objects*/
    private GVPointController pointController = new GVPointController();
    
    /**Controller for polyline objects*/
    private GVPolylineController polylineController = new GVPolylineController();
    
    /**Controller for polygon objects*/
    private GVPolygonController polygonController = new GVPolygonController();
    
    /**Controller for moving point objects*/
    private GVMPointController mpointController = new GVMPointController();

    /**Resultlist for current secondo query result*/
    private ArrayList<DataType> currentResultList = new ArrayList<DataType>();
    
    /**Value is true if data has finished loading*/
    private boolean dataLoaded= false;
    private boolean dataInitialized = false;
    
	public GraphicalView() {
	       	    
		//create DOM element for main svg and attach it to parent
		Element maindiv = DOM.createDiv();
		contentPanel.getElement().appendChild(maindiv);
		contentPanel.getElement().setClassName("graphicalcontentpanel");

		//reset all data
	    resetData();
	    
	    //initialize the main svg with the current window height
		createSVGJS(maindiv, width, height);	
	}
	
	/**On resizing of the browser window the elements of the graphical view are readjusted with the commandpanel displayed
	 * 
	 * @param width The new width of the graphical view
	 * @param height The new height of the graphical view
	 * */
	@Override
	public void resizeWithCP(int width, int height){
		
		 if(width > 1000){  	
		  //add 3 pixel for borders, 50px for sidebar, 20px for sidepadding, 220 for toolbox
		  contentPanel.setWidth(width-293 + "px");
		  this.width = width-293; 
		 }
		 else{
			 contentPanel.setWidth(1000-293 + "px");
			 this.width = 1000-293;
		 }
		  if(height> 650){
		  //10 bottom + 50 header + 30 statusbar + 1 border + 230 commandpanel and menu 
		  contentPanel.setHeight(height-321 + "px");
		  this.height= height-321; 
		 }
		  else{
			  contentPanel.setHeight(650-321 + "px");
			  this.height= 650-321; 
		  }
	}
	
	/**On resizing of the browser window the elements of the graphical view are readjusted with the textpanel displayed
	 * 
	 * @param width The new width of the graphical view
	 * @param height The new height of the graphical view
	 * */
	@Override
	public void resizeWithTextPanel(int width, int height){
		
		 if(width > 1000){ 
		  //add 3 pixel for borders, 50px for sidebar, 20px for sidepadding, 220 for toolbox + 300 textpanel
		  contentPanel.setWidth(width-593 + "px");
		  this.width = width-593; 
		 }
		 else{
			 contentPanel.setWidth(1000-593 + "px");
			 this.width = 1000-593;
		 }
		 if(height > 650){	
			//10 + 50 header + 30 statusbar + 1 border
		  contentPanel.setHeight(height-91 +"px");
		  this.height= height-91;
		 }
		 else{
			  contentPanel.setHeight(650-91 + "px");
			  this.height= 650-91; 
		  }
	}
	
	/**On resizing of the browser window the elements of the graphical view are readjusted with the textpanel and commandpanel displayed
	 * 
	 * @param width The new width of the graphical view
	 * @param height The new height of the graphical view
	 * */
	@Override
	public void resizeWithTextAndCP(int width, int height) {
		  if(width > 1000){ 
			//add 3 pixel for borders, 50px for sidebar, 20px for sidepadding, 220 for toolbox + 300 textpanel
			  contentPanel.setWidth(width-593 + "px");
			  this.width = width-593; 
		  }
		  else{
				 contentPanel.setWidth(1000-593 + "px");
				 this.width = 1000-593;
			 }
		  if(height> 650){
			//10 bottom + 50 header + 30 statusbar + 1 border + 230 commandpanel and menu
			  contentPanel.setHeight(height-321 + "px");
			  this.height= height-321; 
		  }
		  else{
			  contentPanel.setHeight(650-321 + "px");
			  this.height= 650-321; 
		  }
	}
	
	/**On resizing of the browser window the elements of the graphical view are readjusted with to fullscreen
	 * 
	 * @param width The new width of the graphical view
	 * @param height The new height of the graphical view
	 * */
	@Override
    public void resizeToFullScreen(int width, int height){
		
		if(width > 1000){  	
			//add 3 pixel for borders, 50px for sidebar, 20px for sidepadding, 220 for toolbox
			  contentPanel.setWidth(width-293 + "px");
			  this.width = width-293; 
		}
		else{
			 contentPanel.setWidth(1000-293 + "px");
			 this.width = 1000-293;
		 }
    	
		if(height > 650){		  
			  contentPanel.setHeight(height-91 +"px");
			  this.height= height-91;
		}
		 else{
			  contentPanel.setHeight(650-91 + "px");
			  this.height= 650-91; 
		  }
	}
	
	/**Gets the current secondo result from the current resultlist and adds it to the datatype lists*/
	public void initDataTypes(){ 
		
		if(!currentResultList.isEmpty()){
			dataLoaded=false;
			dataInitialized= false;
			
			for(DataType data : currentResultList){
				if(data.getType().equals("Point")){
					
					addPointToOutputRange(((Point) data).getX(), ((Point) data).getY()); //for all points to calculate the axis and scale
					pointController.addPointToDataset(((Point) data).getX(), ((Point) data).getY(), ((Point)data).getName(), data.getId(), data.getColor());
					pointController.getPointArray().add((Point) data);
				}
				
               if(data.getType().equals("Polyline")){
            	   
       			   polylineController.deleteAllLines();
            	   
            	   for(Line line : ((Polyline) data).getPath()){
					
					   addPointToOutputRange(line.getPointA().getX(), line.getPointA().getY());//for all points to calculate the axis and scale
					   addPointToOutputRange(line.getPointB().getX(), line.getPointB().getY());
					   polylineController.addLineToDataset(line.getPointA().getX(), line.getPointA().getY(), line.getPointB().getX(), line.getPointB().getY());
            	   }
            	   
            	   polylineController.addPolyline();
            	   polylineController.getPolylineArray().add((Polyline) data);
				}
				
				if(data.getType().equals("Polygon")){
                	   
					polygonController.deleteAllPolygonPoints();
                	   
						for (Point point: ((Polygon) data).getPath()){	
							addPointToOutputRange(point.getX(), point.getY()); //for all polygons to calculate the axis and scale
							polygonController.addPointToPolygonPath(point.getX(), point.getY());
						}	
						polygonController.addPolygon(((Polygon) data).getName(), data.getId(), data.getColor());	
						polygonController.getPolygonArray().add((Polygon) data);
					}
                if(data.getType().equals("MPoint")){
                	
        			mpointController.deletePathOfMPoint();
				
                	for(Line line: ((MPoint)data).getPath()){
                		addPointToOutputRange(line.getPointA().getX(), line.getPointA().getY());//for all points to calculate and scale the axis
                		//add just pointA, not pointB, because thats the same as pointA from the next element
                		mpointController.addPointToMPointPath(line.getPointA().getX(), line.getPointA().getY());
                	}     

                	//add dates to time range
                	for(TimeInterval time: ((MPoint)data).getTime()){
             		   //do not add duplicates
             		   if(!mpointController.getTimeBounds().contains(time.getTimeA())){
             			    mpointController.getTimeBounds().add(time.getTimeA());//for all dates to calculate the time bounds
             		   }

             		   if(!mpointController.getTimeBounds().contains(time.getTimeB())){
             			    mpointController.getTimeBounds().add(time.getTimeB());
             		   }
                	   }
             	    //add mpoint to array
             	    mpointController.getMpointArray().add((MPoint)data);
                	mpointController.addMPoint();
				   }	
			}
			dataInitialized= true;
		}		
	}
	
	/**Draws all elements to the main svg of the graphical view */
	@Override
	public void updateView(){
		
		dataLoaded = false;
		removeSVG();
		contentPanel.clear();			
		createSVG();
		
		pointController.removeCircles();
		polylineController.removeLines();
		polygonController.removePolygons();
		
		if(!currentResultList.isEmpty()){

		//Scale axes to output range and draw them
		drawAxes(); 
		
		if(pointController.hasPoints()){			
			pointController.drawAllPoints();
		}
		if(polylineController.hasPolylines()){
			polylineController.drawAllPolylines();
		}
		if(polygonController.hasPolygons()){
            polygonController.showPolygonArray();
		}	
		if(mpointController.hasMPoints()){
			mpointController.stopAllAnimations();
            mpointController.drawFirstMovingPoint();
		}	
		}
		dataLoaded = true;
	}	
	
	/**Removes all data from the graphical view*/
	@Override
	public void resetData(){
		pointController.deleteAllPoints(); 
		pointController.getPointArray().clear();
		polylineController.deleteAllLines();
		polylineController.deleteAllPolylines();
		polylineController.getPolylineArray().clear();
		polygonController.deleteAllPolygonPoints();
		polygonController.deleteAllPolygons();
		polygonController.getPolygonArray().clear();
		mpointController.deletePathOfMPoint(); 
		mpointController.deleteAllMPointPaths();
		mpointController.getMpointArray().clear();
		mpointController.getTimeBounds().clear();
		mpointController.getMpointTimerList().clear();
		resetOutputRange();
		removeSVG();
		contentPanel.clear();
		currentResultList.clear();
	}
	
	/**Creates the main svg */
	public void createSVG(){
		Element div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		createSVGJS(div, width, height);
	}
	
	/**Returns the content panel of the graphical view
	 * 
	 * @return The content panel of the graphical view
	 * */
	public FlowPanel getContentPanel() {
		return contentPanel;
	}

	/**Returns the current result list of all datatypes
	 * 
	 * @return The current result list of all datatypes
	 * */
	public ArrayList<DataType> getCurrentResultList() {
		return currentResultList;
	}

	/**Returns true if the data has finished loading
	 * 
	 * @return True if data is loaded
	 * */
	public boolean isDataLoaded() {
		return dataLoaded;
	}

	/**Sets the dataloaded attribute to the given value
	 * 
	 * @param dataLoaded True if data is loaded, else false
	 * */
	public void setDataLoaded(boolean dataLoaded) {
		this.dataLoaded = dataLoaded;
	}
	
	

	public boolean isDataInitialized() {
		return dataInitialized;
	}

	public void setDataInitialized(boolean dataInitialized) {
		this.dataInitialized = dataInitialized;
	}

	/**Returns the point controller of the graphical view
	 * 
	 * @return The point controller of the graphical view
	 * */
	public GVPointController getPointController() {
		return pointController;
	}

	/**Returns the polyline controller of the graphical view
	 * 
	 * @return The polyline controller of the graphical view
	 * */
	public GVPolylineController getPolylineController() {
		return polylineController;
	}

	/**Returns the polygon controller of the graphical view
	 * 
	 * @return The polygon controller of the graphical view
	 * */
	public GVPolygonController getPolygonController() {
		return polygonController;
	}

	/**Returns the moving point controller of the graphical view
	 * 
	 * @return The moving point controller of the graphical view
	 * */
	public GVMPointController getMpointController() {
		return mpointController;
	}

	
/*########### JSNI JavaScript Functions for main svg and axes #########
***********************************************************************/	
	
	/**Creates the svg with the given div, width and height
	 * 
	 * @param div The div element to which the main svg will be attached
	 * @param width The width of the main svg
	 * @param height The height of the main svg
	 * */
	public native void createSVGJS(Element div, int width, int height)/*-{		
		$wnd.createSVG(div, width, height);
			}-*/;
	
	/**Scales the axes to the points in the outputRangearray and draws the axes*/
	public native void drawAxes()/*-{	
		$wnd.drawAxes();
			}-*/;
	
	/** Adds a point to the dataset to calculate the range for the data
	 * 
	 * @param x The x value of the point for the output range
	 * @param y The y value of the point for the output range
	 * */
	public native void addPointToOutputRange(double x, double y)/*-{
		$wnd.addPointToOutputRange(x, y);
			}-*/;
	
	/** Deletes all data from the dataset of output range points*/
	public native void resetOutputRange()/*-{
		$wnd.resetOutputRange();
			}-*/;
	
	/** Removes all overlays from the svg*/
	public native void removeAllOverlays()/*-{
		$wnd.removeAllOverlays();
			}-*/;
	
	/** Removes the svg from the view*/
	public native void removeSVG()/*-{
		$wnd.removeSVG();
			}-*/;	
}
