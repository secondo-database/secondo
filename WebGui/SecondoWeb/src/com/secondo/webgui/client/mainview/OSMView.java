package com.secondo.webgui.client.mainview;

import java.util.ArrayList;

import org.gwtopenmaps.openlayers.client.Bounds;
import org.gwtopenmaps.openlayers.client.LonLat;
import org.gwtopenmaps.openlayers.client.Map;
import org.gwtopenmaps.openlayers.client.MapOptions;
import org.gwtopenmaps.openlayers.client.MapWidget;
import org.gwtopenmaps.openlayers.client.Marker;
import org.gwtopenmaps.openlayers.client.Projection;
import org.gwtopenmaps.openlayers.client.Style;
import org.gwtopenmaps.openlayers.client.control.LayerSwitcher;
import org.gwtopenmaps.openlayers.client.control.MousePosition;
import org.gwtopenmaps.openlayers.client.control.OverviewMap;
import org.gwtopenmaps.openlayers.client.control.ScaleLine;
import org.gwtopenmaps.openlayers.client.feature.VectorFeature;
import org.gwtopenmaps.openlayers.client.geometry.LineString;
import org.gwtopenmaps.openlayers.client.geometry.LinearRing;
import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3MapType;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3Options;
import org.gwtopenmaps.openlayers.client.layer.Markers;
import org.gwtopenmaps.openlayers.client.layer.OSM;
import org.gwtopenmaps.openlayers.client.layer.Vector;
import org.gwtopenmaps.openlayers.client.layer.VectorOptions;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.secondo.webgui.client.datatypes.DataType;
import com.secondo.webgui.client.datatypes.Line;
import com.secondo.webgui.client.datatypes.Polyline;

public class OSMView extends Composite implements View{

	private FlowPanel contentPanel = new FlowPanel();
	private HorizontalPanel headerPanel = new HorizontalPanel();
	private HTML mapHeading = new HTML("<h2>Open Street View </h2>");
	private Double latitude = 51.3760448; //Fernuni Hagen
	private Double longitude = 7.4947253;
	private Button updateButton= new Button("Update Map");
	private Button resetButton= new Button("Reset Map");
	private Map map;
	private MapWidget mapWidget;
	private Bounds bounds = new Bounds();
	private Projection externalProjection = new Projection("EPSG:4326");
	private Projection internalProjection = new Projection("EPSG:900913");
	private Markers markers = new Markers("Markers");
	private ArrayList<LineString> lineArray = new ArrayList<LineString>();
    private ArrayList<Point> pointArray = new ArrayList<Point>();
    private ArrayList<LinearRing> polygonArray = new ArrayList<LinearRing>();
    private int width=Window.getClientWidth()-90;
    private int height=Window.getClientHeight()-330;
    
    private ArrayList<DataType> currentResultTypeList = new ArrayList<DataType>();
    
     public OSMView(){	
 					
 		contentPanel.getElement().setClassName("mapcontentpanel");
 		//contentPanel.add(mapHeading);
   
        //create a mapwidget and add it to the contentpanel
        MapOptions defaultMapOptions = new MapOptions();
        mapWidget = new MapWidget(width + "px",  height + "px", defaultMapOptions);
        mapWidget.getElement().setClassName("mapwidget");
        contentPanel.add(mapWidget);
        
        map = mapWidget.getMap();
 	    
 	    //contentPanel.add(updateButton);
 	    updateButton.addClickHandler(new ClickHandler() {
 	          public void onClick(ClickEvent event) {
 	        	// initializeOSMMap(latitude, longitude);
 	        	  //initOsmMap(latitude, longitude);
 	        	  setMarker(52.519171, 13.4060912); //Berlin
 	          }
 		 });
 	    
 	   //contentPanel.add(resetButton);
 	   resetButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  resetMap();
	          }
		 });

 	   
 	   initOsmMap(latitude, longitude);
 	   initGoogleLayers();
     }
     
     @Override
     public void resizeWithCP(int width, int height){

    	 this.width = width-230;
    	 this.height = height-320;
    	 contentPanel.setWidth(width-230 + "px");
  		 contentPanel.setHeight(height-320 + "px");
 		 mapWidget.setWidth(width-230 + "px");
 		 mapWidget.setHeight(height-320 + "px");

 	}
     
     @Override
     public void resizeWithTextPanel(int width, int height){
    	 
		  this.width = width-570; 
		  this.height = height-90;
		  
		  contentPanel.setWidth(width-570 + "px"); 
		  contentPanel.setHeight(height-90 + "px");
		  mapWidget.setWidth(width-570 + "px");
		  mapWidget.setHeight(height-90 + "px");

	}
     

 	@Override
 	public void resizeWithTextAndCP(int width, int height) {
 		
 		  this.width = width-570; 
		  this.height= height-320;
		  
		  contentPanel.setWidth(width-570 + "px"); 
		  contentPanel.setHeight(height-320 + "px");
		  mapWidget.setWidth(width-570 + "px");
	 	  mapWidget.setHeight(height-320 + "px");
 		
 	}
     
     @Override
     public void resizeToFullScreen(int width, int height){
    	 
    	 this.width = width-230;
    	 this.height = height-90;
    	 contentPanel.setWidth(width-230 + "px");
    	 contentPanel.setHeight(height-90 + "px");
    	 mapWidget.setWidth(width-230 + "px");
    	 mapWidget.setHeight(height-90 + "px");
     }
     
     /**Get the current secondo result from the list and set it to the map*/
     public void updateCurrentResult(){
    	 if(!currentResultTypeList.isEmpty()){

				resetData();
								
				for(DataType data : currentResultTypeList){

					if(data.getName().equals("Point")){
						setMarker(((com.secondo.webgui.client.datatypes.Point) data).getY(), ((com.secondo.webgui.client.datatypes.Point) data).getX());
						addMarker(((com.secondo.webgui.client.datatypes.Point) data).getY(), ((com.secondo.webgui.client.datatypes.Point) data).getX());					
					}
					if(data.getName().equals("Line")){
						
						addLine(((Line) data).getA().getY(), ((Line) data).getA().getX(), ((Line) data).getB().getY(), ((Line) data).getB().getX());
					}
                if(data.getName().equals("Polyline")){
             	   
             	   deleteAllPoints();
             	   
						for (com.secondo.webgui.client.datatypes.Point point: ((Polyline) data).getPath()){
							addPoint(point.getY(), point.getX());	
						}	
						addPolygon();		
					}
				}
				//get the first element of the list and check its type
				if(currentResultTypeList.get(0).getName().equals("Point")){
				    showMarkerOverlay();
				    }
				if(currentResultTypeList.get(0).getName().equals("Line")){
				    showLineOverlay();
				    }
				if(currentResultTypeList.get(0).getName().equals("Polyline")){
				    showPolygonOverlays();
				    }
			}
			
			else{
			
			System.out.println("##############else of setmapview");
			
			resetData();				
			}
     }
     
     /**Remove all Data from the map*/
     public void resetData(){
    	    this.deleteAllMarkers();
			this.deleteAllLines();
			this.deleteAllPoints();
			this.deleteAllPolygons();
			this.resetBounds();
			this.resetMap();
     }
     
     /**Check if the given number is a geographic latitude*/
     public boolean isLatitude(double lat){
    	 
    	 //range lat = -90 +90
    	 if(lat < 90 && lat > -90){
    		 return true; 		 
    	 }
	 
    	 return false;
     }
     
     /**Check if the given number is a geographic longitude*/
     public boolean isLongitude(double lng){ 	 
    	 
    	 //range lng = -180 +180
    	 if(lng < 180 && lng > -180){
    		 return  true;
    	 }
    	 
    	 return false;
     }     
     
    /**Initialize the Open Street map view*/ 
    public void initOsmMap(double lat, double lng){
    	
    	//LonLat lonLat = new LonLat(6.95, 50.94);   
    	LonLat lonLat = new LonLat(lng, lat); 
    	lonLat.transform("EPSG:4326", "EPSG:900913");
    	
    	map = mapWidget.getMap();

    	OSM osm_1 = OSM.Mapnik("Open Street Map Mapnik ");   // Label for menu 'LayerSwitcher'
    	    osm_1.setIsBaseLayer(true);

    	OSM osm_2 = OSM.CycleMap("Open Street Map CycleMap "); 
    	    osm_2.setIsBaseLayer(true);
    	
    	map.addLayer(osm_1);
    	map.addLayer(osm_2);
    	
        //Add some default controls to the map
    	map.addControl(new MousePosition());
        map.addControl(new LayerSwitcher()); //+ sign in the upperright corner to display the layer switcher
        map.addControl(new OverviewMap()); //+ sign in the lowerright to display the overviewmap
        map.addControl(new ScaleLine()); //Display the scaleline
  
    	map.setCenter(lonLat, 10);  
    	 
        mapWidget.getElement().getFirstChildElement().getStyle().setZIndex(0); //force the map to fall behind popups
    }
    
    /**Sets one marker to the map in a markerslayer*/
    public void setMarker(double lat, double lng){
    	
    	LonLat lonLat = new LonLat(lng, lat); 
    	lonLat.transform("EPSG:4326", "EPSG:900913");

        Marker marker = new Marker(lonLat);
        marker.setImageUrl("resources/images/marker.png"); //add an image for the marker

        markers.addMarker(marker);
        map.addLayer(markers);
        map.setCenter(lonLat, 8);    	
    }
    
    /**Add a marker to the markerArray*/
    public void addMarker(double lat, double lng){
    	
    	if(isLatitude(lat) && isLongitude(lng)){
        	
        	LonLat lonLat = new LonLat(lng, lat); 
        	lonLat.transform("EPSG:4326", "EPSG:900913");
        	
        	Marker marker = new Marker(lonLat);
            marker.setImageUrl("resources/images/marker.png"); //add an image for the marker
            
            bounds.extend(lonLat);
            
            markers.addMarker(marker);
    	}
	
    }
    
    /**Show all markers in the array on the map*/
    public void showMarkerOverlay(){
    	
    	if(markers != null){
        	
    	    map.addLayer(markers);   

				map.zoomToExtent(bounds); 
				if(map.getZoom()> 10){ //not closer than zoom 10
					map.zoomTo(10);
				}
    	}
    }
    
    /**Delete all markers from the markerarray*/
    public void deleteAllMarkers(){
    	markers.clearMarkers();
    	//map.removeLayer(markers);
    }
    
    /**Add a marker to the markerArray*/
    public void addLine(double lat1, double lng1, double lat2, double lng2){
    	
    	if(isLatitude(lat1) && isLongitude(lng1)){
              
       // create one point 
       Point point1 = new Point(lng1, lat1);
       point1.transform(externalProjection, internalProjection);
       
       
       if(isLatitude(lat2) && isLongitude(lng2)){
       // create another point
       Point point2 = new Point(lng2, lat2);
       point2.transform(externalProjection, internalProjection);
       
       Point [] points = {point1, point2};

       LineString lineString = new LineString(points);

       lineArray.add(lineString);
        
       bounds.extend(point1);
       bounds.extend(point2);
       }
       }
	
    }
    
    public void showLineOverlay(){
    	
    	if(!lineArray.isEmpty()){
        	
            // Create the vectorOptions
            VectorOptions vectorOptions = new VectorOptions();
            
            //Add a style to the vectorlayer
            Style style = new Style();
            style.setStrokeColor("#0033ff");
            style.setStrokeWidth(2);
            vectorOptions.setStyle(style);  
            
        	// Create the layer
            Vector vectorLayer = new Vector("Line Layer", vectorOptions); //shows a layer for every linecombination / not wanted, just one layer? can be many if lines are not connected
            
            //add all lines to the vectorlayer
            for(LineString line : lineArray){
                
                VectorFeature feature = new VectorFeature(line);
                vectorLayer.addFeature(feature);
            }

            //add the layer to the map
           map.addLayer(vectorLayer);  	
        
           map.zoomToExtent(bounds); 
           if(map.getZoom()> 10){ //zoom not closer than 10
    			map.zoomTo(10);
    		}
    	}
    }
    
    public void deleteAllLines(){
    	lineArray.clear();
    	//map.removeLayer(vectorLayer);
    }
    
    /**Creates a point from the given values and adds it to the pointarray*/
    public void addPoint(double lat, double lng){

    	if(isLatitude(lat) && isLongitude(lng)){
        Point point = new Point(lng, lat);
        point.transform(externalProjection, internalProjection);
        
        pointArray.add(point);
        
        bounds.extend(point);
    	}
    	
    }
    
    /**Deletes all points from the pointarray*/
    public void deleteAllPoints(){
    	pointArray.clear();
    }
    
    
    /** Add a linear ring representing a polygon to the polygonarray*/
    public void addPolygon(){
    	
        LinearRing polygon = new LinearRing(pointArray.toArray(new Point[pointArray.size()]));
        
        polygonArray.add(polygon);   	
    }
    
    /**Draw all polygons of the polygonarray to the map*/
    public void showPolygonOverlays(){
    	
    	if(!polygonArray.isEmpty()){
        	
        	// Create the vectorOptions
            VectorOptions vectorOptions = new VectorOptions();
            
            //Add a style to the vectorlayer
            Style style = new Style();
            style.setStrokeColor("#0033ff");
            style.setStrokeWidth(2);
            vectorOptions.setStyle(style);  
            
        	// Create the layer
            Vector polygonLayer = new Vector("Polygon Layer", vectorOptions);
            
            //add all lines to the vectorlayer
            for(LinearRing polygon : polygonArray){
                
                VectorFeature feature = new VectorFeature(polygon);
                polygonLayer.addFeature(feature);
            }

            //add the layer to the map
           map.addLayer(polygonLayer);  	
        
           map.zoomToExtent(bounds); 
           if(map.getZoom()> 10){ //soll nicht näher als 10 zoomen
    			map.zoomTo(10);
    		}
    	}
    }
    
    public void deleteAllPolygons(){
    	polygonArray.clear();
    	//map.removeLayer(polygonLayer);
    	
    }
    
    public void resetBounds(){
    	bounds = new Bounds();
    }
    
    public void resetMap(){
    	map.removeOverlayLayers();
    }
    
    /**Add various layers of googlemaps*/
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
 
        //And add them to the map
        Map map = mapWidget.getMap();
        map.addLayer(gHybrid);
        map.addLayer(gNormal);
        map.addLayer(gSatellite);
        map.addLayer(gTerrain);
    }


	public FlowPanel getContentPanel() {
		return contentPanel;
	}

	public void setContentPanel(FlowPanel contentPanel) {
		this.contentPanel = contentPanel;
	}

	public ArrayList<DataType> getCurrentResultTypeList() {
		return currentResultTypeList;
	}

	public void setCurrentResultTypeList(ArrayList<DataType> currentResultTypeList) {
		this.currentResultTypeList = currentResultTypeList;
	}
    
}
