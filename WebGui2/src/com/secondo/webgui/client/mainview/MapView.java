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
import org.gwtopenmaps.openlayers.client.Bounds;
import org.gwtopenmaps.openlayers.client.LonLat;
import org.gwtopenmaps.openlayers.client.Map;
import org.gwtopenmaps.openlayers.client.MapOptions;
import org.gwtopenmaps.openlayers.client.MapWidget;
import org.gwtopenmaps.openlayers.client.control.LayerSwitcher;
import org.gwtopenmaps.openlayers.client.control.MousePosition;
import org.gwtopenmaps.openlayers.client.control.OverviewMap;
import org.gwtopenmaps.openlayers.client.control.ScaleLine;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3MapType;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3Options;
import org.gwtopenmaps.openlayers.client.layer.OSM;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.secondo.webgui.client.controller.MVMPointController;
import com.secondo.webgui.client.controller.MVPointController;
import com.secondo.webgui.client.controller.MVPolygonController;
import com.secondo.webgui.client.controller.MVPolylineController;
import com.secondo.webgui.shared.model.DataType;
import com.secondo.webgui.shared.model.Line;
import com.secondo.webgui.shared.model.MPoint;
import com.secondo.webgui.shared.model.Polygon;
import com.secondo.webgui.shared.model.Polyline;
import com.secondo.webgui.shared.model.TimeInterval;

/**
*  This class represents all elements of the map view to display graphical datatypes on a map with the gwt-openlayers library.
*  
*  @author Kristina Steiger
*  
**/
public class MapView extends Composite implements View{

	/**The Main panel of the map view*/
	private FlowPanel contentPanel = new FlowPanel();
	
	/**The Map element*/
	private Map map;
	
	/**The MapWidget containing the map*/
	private MapWidget mapWidget;
	
	/**The Bounds object containing all geographic bound points of all queries*/
	private Bounds boundsAll = new Bounds();
	
	/**The Bounds object containing all geographic bound points of the last query*/
	private Bounds boundsLast = new Bounds();
	
	 /**Controller for point objects*/
	private MVPointController pointController = new MVPointController();
	
	 /**Controller for polyline objects*/
	private MVPolylineController polylineController = new MVPolylineController();
	
	 /**Controller for polygon objects*/
	private MVPolygonController polygonController = new MVPolygonController();
	
	 /**Controller for moving point objects*/
	private MVMPointController mpointController = new MVMPointController();
    
    /**Width of the map view*/
    private int width=Window.getClientWidth()-293;
    
    /**Height of the map view*/
    private int height=Window.getClientHeight()-321;
    
    /**Resultlist for current secondo query result*/
    private ArrayList<DataType> currentResultTypeList = new ArrayList<DataType>();
    
    /**Value is true if data has finished loading*/
    private boolean dataLoaded= false;
    private boolean dataInitialized = false;
    
    private boolean zoomToAll = false;
    
    public MapView(){	
 					
 		contentPanel.getElement().setClassName("mapcontentpanel");
   
        //create a mapwidget and add it to the contentpanel
        MapOptions defaultMapOptions = new MapOptions();
        mapWidget = new MapWidget(width + "px",  height + "px", defaultMapOptions);
        mapWidget.getElement().setClassName("mapwidget");
        contentPanel.add(mapWidget);
        
        map = mapWidget.getMap();
 	   
       //initialize the map with Fernuni Hagen as the default center of the map
 	    initOsmMap(51.3760448, 7.4947253); 
 	    initGoogleLayers();
     }
     
     /**Initializes the map with Open Street Map layers
      * 
      * @param lat The latitude of the default center point
      * @param lng The longitude of the default center point
      * */ 
     public void initOsmMap(double lat, double lng){
     	  
     	LonLat lonLat = new LonLat(lng, lat); 
     	lonLat.transform("EPSG:4326", "EPSG:900913");
     	
     	map = mapWidget.getMap();

     	OSM osm_1 = OSM.Mapnik("Open Street Map Mapnik "); // Label for menu 'LayerSwitcher'
     	    osm_1.setIsBaseLayer(true);

     	OSM osm_2 = OSM.CycleMap("Open Street Map CycleMap "); 
     	
     	map.addLayer(osm_1);
     	map.addLayer(osm_2);
     	
         //Add some default controls to the map
     	map.addControl(new MousePosition()); //shows the coordinates of the mouseposition in the lowerright corner
        map.addControl(new LayerSwitcher()); //+ sign in the upperright corner to display the layer switcher
        map.addControl(new OverviewMap()); //+ sign in the lowerright to display the overviewmap
        map.addControl(new ScaleLine()); //Display the scaleline
   
     	map.setCenter(lonLat, 10);  
     	 
     	//force the map to fall behind popups
        mapWidget.getElement().getFirstChildElement().getStyle().setZIndex(0);
     }
         
     /**Adds various layers of googlemaps to the map*/
     public void initGoogleLayers(){
     	
     	//Create some Google Layers
         GoogleV3Options gHybridOptions = new GoogleV3Options();
         gHybridOptions.setIsBaseLayer(true);
         gHybridOptions.setType(GoogleV3MapType.G_HYBRID_MAP);
         GoogleV3 gHybrid = new GoogleV3("Google Hybrid", gHybridOptions);
  
         GoogleV3Options gNormalOptions = new GoogleV3Options();
         gNormalOptions.setIsBaseLayer(true);
         gNormalOptions.setType(GoogleV3MapType.G_NORMAL_MAP);
         GoogleV3 gNormal = new GoogleV3("Google Normal", gNormalOptions);
  
         GoogleV3Options gSatelliteOptions = new GoogleV3Options();
         gSatelliteOptions.setIsBaseLayer(true);
         gSatelliteOptions.setType(GoogleV3MapType.G_SATELLITE_MAP);
         GoogleV3 gSatellite = new GoogleV3("Google Satellite", gSatelliteOptions);
  
         GoogleV3Options gTerrainOptions = new GoogleV3Options();
         gTerrainOptions.setIsBaseLayer(true);
         gTerrainOptions.setType(GoogleV3MapType.G_TERRAIN_MAP);
         GoogleV3 gTerrain = new GoogleV3("Google Terrain", gTerrainOptions);
  
         //Add google layers to the map
         Map map = mapWidget.getMap();
         map.addLayer(gHybrid);
         map.addLayer(gNormal);
         map.addLayer(gSatellite);
         map.addLayer(gTerrain);
     }
     
     /**On resizing of the browser window the elements of the map view are readjusted with the commandpanel displayed
 	 * 
 	 * @param width The new width of the map view
 	 * @param height The new height of the map view
 	 * */
     @Override
     public void resizeWithCP(int width, int height){

    	//add 3 pixel for borders, 50px for sidebar, 20px for sidepadding, 220 for toolbox
    	 if(width > 1000){  	
    	    this.width = width-293; 
    	    contentPanel.setWidth(width-293 + "px"); 		 
 		    mapWidget.setWidth(width-293 + "px");
    	 }
    	 else{
    		 this.width = 1000-293;
    		 contentPanel.setWidth(1000-293 + "px"); 		 
  		     mapWidget.setWidth(1000-293 + "px");
    	 }
    	 if(height > 650){
  		    contentPanel.setHeight(height-321 + "px");
    	    this.height = height-321;  	
 		    mapWidget.setHeight(height-321 + "px"); 	
    	 }
    	 else{
    		contentPanel.setHeight(650-321 + "px");
     	    this.height = 650-321;  	
  		    mapWidget.setHeight(650-321 + "px");
    	 }
  		 map.updateSize();
 	}
     
     /**On resizing of the browser window the elements of the map view are readjusted with the textpanel displayed
 	 * 
 	 * @param width The new width of the map view
 	 * @param height The new height of the map view
 	 * */
     @Override
     public void resizeWithTextPanel(int width, int height){
    	 
    	 if(width > 1000){ 
    		 //add 3 pixel for borders, 50px for sidebar, 20px for sidepadding, 220 for toolbox + 300 textpanel
		     this.width = width-593;   
		     contentPanel.setWidth(width-593 + "px"); 	  
		     mapWidget.setWidth(width-593 + "px");
    	 }
    	 else{
    		 this.width = 1000-593;
    		 contentPanel.setWidth(1000-593 + "px"); 		 
  		     mapWidget.setWidth(1000-593 + "px");
    	 }
    	 if(height > 650){		  
		     this.height = height-91;	
		     contentPanel.setHeight(height-91 + "px");	
		     mapWidget.setHeight(height-91 + "px");	
    	 }
    	 else{
     		contentPanel.setHeight(650-91 + "px");
      	    this.height = 650-91;  	
   		    mapWidget.setHeight(650-91 + "px");
     	 }
		  map.updateSize();
	}
     
     /**On resizing of the browser window the elements of the map view are readjusted with the textpanel and commandpanel displayed
 	 * 
 	 * @param width The new width of the map view
 	 * @param height The new height of the map view
 	 * */
 	@Override
 	public void resizeWithTextAndCP(int width, int height) {
 		
 		if(width > 1000){ 
		     this.width = width-593; 	  
		     contentPanel.setWidth(width-593 + "px"); 	  
		     mapWidget.setWidth(width-593 + "px");
   	   }
 		else{
   		 this.width = 1000-603;
   		 contentPanel.setWidth(1000-593 + "px"); 		 
 		     mapWidget.setWidth(1000-593 + "px");
   	   }
   	   if(height > 650){	 
		  this.height= height-321;
		  contentPanel.setHeight(height-321 + "px");		  
	 	  mapWidget.setHeight(height-321 + "px");	 	  
   	   }
   	  else{
		contentPanel.setHeight(650-321 + "px");
 	    this.height = 650-321;  	
		    mapWidget.setHeight(650-321 + "px");
	 }
	 	  map.updateSize();		
 	}
     
 	/**On resizing of the browser window the elements of the map view are readjusted with to fullscreen
	 * 
	 * @param width The new width of the map view
	 * @param height The new height of the map view
	 * */
     @Override
     public void resizeToFullScreen(int width, int height){ 
    	 
    	 if(width > 1000){  	
     	    this.width = width-293; 
     	    contentPanel.setWidth(width-293 + "px"); 		 
  		    mapWidget.setWidth(width-293 + "px");
     	 }	 
    	 else{
    		 this.width = 1000-293;
    		 contentPanel.setWidth(1000-293 + "px"); 		 
  		     mapWidget.setWidth(1000-293 + "px");
    	 }
    	 if(height > 650){		  
		     this.height = height-91;	
		     contentPanel.setHeight(height-91 + "px");	
		     mapWidget.setHeight(height-91 + "px");	
    	 }
    	 else{
    			contentPanel.setHeight(650-91 + "px");
    	 	    this.height = 650-91;  	
    		    mapWidget.setHeight(650-91 + "px");
    		 }
    	 map.updateSize();
     }
     
     /**Gets the current secondo result from the list and put it into the datatype arrays*/
     public void initializeOverlays(){
    	 
    	 if(!currentResultTypeList.isEmpty()){

    		 dataLoaded=false;
    		 dataInitialized = false;
    		 boundsLast = new Bounds();
								
				for(DataType data : currentResultTypeList){

					if(data.getType().equals("Point")){
						
						pointController.addPoint(((com.secondo.webgui.shared.model.Point) data).getY(), 
								((com.secondo.webgui.shared.model.Point) data).getX(), data.getId(), boundsAll, boundsLast);					
					}
					
					if(data.getType().equals("Polyline")){
						
						polylineController.deleteAllLines();
						
						for(Line line : ((Polyline) data).getPath()){			
						    polylineController.addLine(line.getPointA().getY(), line.getPointA().getX(), 
						    		line.getPointB().getY(), line.getPointB().getX(), boundsAll, boundsLast);
						}
						polylineController.addPolyline(data.getId());
					}
                   if(data.getType().equals("Polygon")){
             	   
             	        polygonController.deleteAllPolygonPoints();
             	   
						for (com.secondo.webgui.shared.model.Point point: ((Polygon) data).getPath()){
							polygonController.addPolygonPoint(point.getY(), point.getX(), boundsAll, boundsLast);	
						}	
						polygonController.addPolygon(data.getId());		
					}
                   if(data.getType().equals("MPoint")){	

                	   mpointController.deleteAllLonLats();
                	   
              		 //add just pointA of all lines, not pointB, because thats the same as pointA from the next element
                	   for(Line line: ((MPoint)data).getPath()){
                   		  mpointController.addLonLat(line.getPointA().getY(), line.getPointA().getX(), boundsAll, boundsLast);
                   	   } 
                	   
                	   //add all dates to calculate the time bounds
                	   for(TimeInterval time: ((MPoint)data).getTime()){
                		   //do not add duplicates
                		   if(!mpointController.getTimeBounds().contains(time.getTimeA())){
                          		mpointController.getTimeBounds().add(time.getTimeA());
                		   }
               		       if(!mpointController.getTimeBounds().contains(time.getTimeB())){
                          		mpointController.getTimeBounds().add(time.getTimeB());
                		   }
                   	   }
                	   //add mpoint to array
                	   mpointController.getMpointArray().add((MPoint)data);
                	   mpointController.addMPoint(data.getId());
   				   }
				}				
				dataInitialized = true;
			}			
     }    
     
     /**Draws all elements of the map with the new size of the mapwidget*/
     @Override
     public void updateView(){
    	 
    	    //Delete all overlays
    	    this.resetMap();
    	    dataLoaded = false;
    	    
    	    if(!currentResultTypeList.isEmpty()){

    	    	   if(!pointController.getPointMap().isEmpty()){
    	    		   if(zoomToAll == true){
    	    			   pointController.showPointOverlays(map, boundsAll);
    	    		   }
    	    		   else{
    	    			   pointController.showPointOverlays(map, boundsLast);
    	    		   }				      
				   }

				    if(!polylineController.getPolylineMap().isEmpty()){
				    	if(zoomToAll == true){
				    		polylineController.showPolylineOverlays(map, boundsAll);
				    	}
				    	else{
				    		polylineController.showPolylineOverlays(map, boundsLast);
				    	}				      
				    }
				    
				    if(!polygonController.getPolygonMap().isEmpty()){
				    	if(zoomToAll == true){
				    		polygonController.showPolygonOverlays(map, boundsAll);
				    	}
				    	else{
				    		polygonController.showPolygonOverlays(map, boundsLast);
				    	}				      
				    }	
				    
				    if(!mpointController.getMpointArray().isEmpty()){
					    mpointController.stopAllAnimations();
					    if(zoomToAll == true){
					    	mpointController.drawFirstMovingPoint(map, boundsAll);
					    }
					    else{
					    	mpointController.drawFirstMovingPoint(map, boundsLast);
					    }			            
				    }
    	    } 
    	    dataLoaded = true;
     }
     
     /**Removes all data from the map*/
     @Override
     public void resetData(){
    	    pointController.deleteAllPoints();
			polylineController.deleteAllLines();
			polylineController.deleteAllPolylines();
			polygonController.deleteAllPolygonPoints();
			polygonController.deleteAllPolygons();
			mpointController.deleteAllMPoints();
			mpointController.resetTimeBounds();
			mpointController.getMpointTimerList().clear();
			this.resetBounds();
			this.resetMap();
     }  
    
    /**Resets bounds by creating a new bounds object*/
    public void resetBounds(){
    	boundsAll = new Bounds();
    	boundsLast = new Bounds();
    }
    
    /**Resets the map by deleting all overlays from the map*/
    public void resetMap(){
    	map.removeOverlayLayers();
    }

    /**Returns the content panel of the map view
	 * 
	 * @return The content panel of the map view
	 * */
	public FlowPanel getContentPanel() {
		return contentPanel;
	}

	/**Returns the current result list of all datatypes
	 * 
	 * @return The current result list of all datatypes
	 * */
	public ArrayList<DataType> getCurrentResultTypeList() {
		return currentResultTypeList;
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

	/**Returns true if the data is initialized
	 * 
	 * @return True if data is initialized
	 * */
	public boolean isDataInitialized() {
		return dataInitialized;
	}

	/**Sets the datainitialized attribute to the given value
	 * 
	 * @param dataInitialized True if data is initialized, else false
	 * */
	public void setDataInitialized(boolean dataInitialized) {
		this.dataInitialized = dataInitialized;
	}

	/**Returns true if the map should zoom to all queries
	 * 
	 * @return True if the map should zoom to all queries
	 * */
	public boolean isZoomToAll() {
		return zoomToAll;
	}

	/**Sets the zoomToAll attribute to the given value
	 * 
	 * @param zoomToAll True if the map should zoom to all queries, else false
	 * */
	public void setZoomToAll(boolean zoomToAll) {
		this.zoomToAll = zoomToAll;
	}

	/**Returns the map element
	 * 
	 * @return The map element
	 * */
	public Map getMap() {
		return map;
	}

	/**Returns the point controller of the map view
	 * 
	 * @return The point controller of the map view
	 * */
	public MVPointController getPointController() {
		return pointController;
	}

	/**Returns the polyline controller of the map view
	 * 
	 * @return The polyline controller of the map view
	 * */
	public MVPolylineController getPolylineController() {
		return polylineController;
	}

	/**Returns the polygon controller of the map view
	 * 
	 * @return The polygon controller of the map view
	 * */
	public MVPolygonController getPolygonController() {
		return polygonController;
	}

	/**Returns the moving point controller of the map view
	 * 
	 * @return The moving point controller of the map view
	 * */
	public MVMPointController getMpointController() {
		return mpointController;
	}
}