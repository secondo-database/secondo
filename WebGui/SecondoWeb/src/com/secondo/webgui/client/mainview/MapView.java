package com.secondo.webgui.client.mainview;

import java.util.ArrayList;

import com.google.gwt.core.client.JavaScriptObject;
import com.google.gwt.core.client.JsArrayNumber;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.Element;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.TextArea;
import com.google.gwt.user.client.ui.VerticalPanel;

public class MapView extends Composite implements View{
	
	private ScrollPanel textPanel = new ScrollPanel();
	private ScrollPanel scrollPanel = new ScrollPanel();
	private FlowPanel contentPanel = new FlowPanel();
	private VerticalPanel mapVp = new VerticalPanel();
	private HTML mapHeading = new HTML("<h2>Google Maps View</h2>");
	//private String latitude = "51.3760448"; //Fernuni Hagen
	//private String longitude = "7.4947253";
	private Button updateButton= new Button("Update Map");
	private ArrayList<Double> polylinePath = new ArrayList<Double>();
    private JsArrayNumber jsDataLat = JavaScriptObject.createArray().cast();
    private JsArrayNumber jsDataLng = JavaScriptObject.createArray().cast();
    private int width=Window.getClientWidth()-160;
    private int height=Window.getClientHeight()-420;
	
	public MapView(){

		//scrollPanel.setSize("600px", "460px");
		scrollPanel.add(contentPanel);
			
		mapVp.setSize(width + "px", height + "px");
		mapVp.getElement().setId("mapview");		
		contentPanel.add(mapHeading);
	    contentPanel.add(mapVp);
	    
	    contentPanel.add(updateButton);
	    updateButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  //initializeMap(latitude, longitude);
	        	  //drawPolygonTest();
	          }
		 });				
		
	   // load data from somewhere 
		   /* double[] data = new double[10];
			for (int i = 0; i < data.length; i++) {
				data[i] = Math.random() + .1;
			}*/

			// convert data into jsarray
			/*JsArrayNumber jsData = JavaScriptObject.createArray().cast();
			for (int i = 0; i < data.length; i++) {
				jsData.push(data[i]);
			}*/

	}
	
	@Override
	public void resizeWithCP(int width, int height){
		  scrollPanel.setWidth(width-120 + "px");
		  scrollPanel.setHeight(height-320 + "px");
		  mapVp.setWidth(width-140 + "px");
		  mapVp.setHeight(height-330 + "px");

	}
	

	@Override
	public void resizeWithTextPanel(int width, int height) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void resizeToFullScreen(int width, int height) {
		// TODO Auto-generated method stub
		
	}
	
	/** call a googlemaps function from an external js-file, to initialize a map with the given center location*/
	public native void initializeMap(double lat, double lng)/*-{
		
		var location = new $wnd.google.maps.LatLng(lat, lng);
		
				$wnd.initializeMap(location);
					}-*/;
	
	/** call a googlemaps function from an external js-file, adds a marker to the marker array*/
	public native void addMarker(double lat, double lng)/*-{
		
		var location = new $wnd.google.maps.LatLng(lat, lng);
		
				$wnd.addMarker(location);
					}-*/;
	
	/** Shows any overlays currently in the array*/
	public native void showMarkerOverlays()/*-{
		
				$wnd.showMarkerOverlays();
					}-*/;
	
	/** Removes the overlays from the map, but keeps them in the array*/
	public native void clearMarkerOverlays()/*-{
		
				$wnd.clearOverlays();
					}-*/;
	
	/** Deletes all markers in the array by removing references to them*/
	public native void deleteAllMarkers()/*-{
		
				$wnd.deleteAllMarkers();
					}-*/;
	
	/** call a googlemaps function from an external js-file, adds a location to the location array*/
	public native void addLocation(double lat, double lng)/*-{
		
		var location = new $wnd.google.maps.LatLng(lat, lng);
		
				$wnd.addLocation(location);
					}-*/;
	
	/** Deletes all locations in the array by removing references to them*/
	public native void deleteAllLocations()/*-{
		
				$wnd.deleteAllLocations();
					}-*/;
	
	/** Adds a polyline of 2 locations to the line array*/
	public native void addLine(double lat1, double lng1, double lat2, double lng2)/*-{
		
		var location1 = new $wnd.google.maps.LatLng(lat1, lng1);
		var location2 = new $wnd.google.maps.LatLng(lat2, lng2);
		
				$wnd.addLine(location1, location2);
					}-*/;
	
	/** Shows all overlays currently in the line array*/
	public native void showLineOverlays()/*-{
		
				$wnd.showLineOverlays();
					}-*/;
	
	/** Deletes all lines in the array by removing references to them*/
	public native void deleteAllLines()/*-{
		
				$wnd.deleteAllLines();
					}-*/;
	
	/**Adds a polyline to the polyline array with all locations that are currently in the locations array*/
	public native void addPolyline()/*-{
		
				$wnd.addPolyline();
					}-*/;
	
	/** Shows an overlays with a polyline of all locations that are currently in the location array*/
	public native void drawPolyline()/*-{
		
				$wnd.drawPolyline();
					}-*/;
	
	
	/** Shows all overlays that are currently in the polyline array */
	public native void showPolylineOverlays()/*-{
		
				$wnd.showPolylineOverlays();
					}-*/;

	
	/** Deletes all polylines in the polyline array */
	public native void deleteAllPolylines()/*-{
		
				$wnd.deleteAllPolylines();
					}-*/;
	
	/**Adds a polygon to the polygon array with all locations that are currently in the locations array*/
	public native void addPolygon()/*-{
		
				$wnd.addPolygon();
					}-*/;
	
	/** Shows an overlays with a polygon of all locations that are currently in the location array*/
	public native void drawPolygon()/*-{
		
				$wnd.drawPolygon();
					}-*/;
	
	/** Shows all overlays that are currently in the polygon array */
	public native void showPolygonOverlays()/*-{
		
				$wnd.showPolygonOverlays();
					}-*/;

	
	/** Deletes all polygons in the polygon array */
	public native void deleteAllPolygons()/*-{
		
				$wnd.deleteAllPolygons();
					}-*/;
	
	/** Resets the bounds to null*/
	public native void resetBounds()/*-{
		
				$wnd.resetBounds();
					}-*/;
	
	/** Resets the bounds to null*/
	public native void resetMap()/*-{
		
				$wnd.resetMap();
					}-*/;

	/** initialize a googlemap with the given langitude and latitude*/
	public native void initializeMapTest(double lat, double lng)/*-{
		
		// Enable the visual refresh
         $wnd.google.maps.visualRefresh = true;
         
         //var chicago = new $wnd.google.maps.LatLng(41.850033, -87.6500523);
       //  var hagen = new $wnd.google.maps.LatLng(51.3760448, 7.4947253);
         

	     var mapOptions = {
	        center: new $wnd.google.maps.LatLng(lat, lng),
	        zoom: 10, //0 = whole earth, 18 = streetview
	        mapTypeId: $wnd.google.maps.MapTypeId.ROADMAP
	    };

	    
	    var map = new $wnd.google.maps.Map($doc.getElementById("mapview"), mapOptions);  
	    
	    //add a marker with the given position
	    var marker = new $wnd.google.maps.Marker({
            position: new $wnd.google.maps.LatLng(lat, lng),
            map: map,
            title: 'Point from Secondo'
        });
        
        //add a info window with information about the marker position
        var contentString = '<div id="content">'+
      '<div id="siteNotice">'+
      '</div>'+
      '<h3 id="firstHeading" class="firstHeading">Point from Secondo</h3>'+
      '<div id="bodyContent">'+
      '<p> </p>'+
      '<p>Koordinaten: '+ lat + lng +
      '</p>'+
      '</div>'+
      '</div>';
        
        
        var infowindow = new $wnd.google.maps.InfoWindow({
            content: contentString
        });
            
         $wnd.google.maps.event.addListener(marker, 'click', function() {
             infowindow.open(map,marker);
        });
        
							}-*/;
	

	
	/** draw a googlemap with the given linearray*/
	public native void drawPolygonTest()/*-{
		
		// Enable the visual refresh
         $wnd.google.maps.visualRefresh = true;      

	     var mapOptions = {
	        center: new $wnd.google.maps.LatLng(51.3670777, 7.4632841),
	        zoom: 9, //0 = whole earth, 18 = streetview
	        mapTypeId: $wnd.google.maps.MapTypeId.TERRAIN
	    };
	    
	    var map = new $wnd.google.maps.Map($doc.getElementById("mapview"), mapOptions);  
         
         var triangle;

         var triangleCoords = [
              new $wnd.google.maps.LatLng(51.3670777, 7.4632841), //hagen
              new $wnd.google.maps.LatLng(51.5135872, 7.4652981), //dortmund
              new $wnd.google.maps.LatLng(51.4556432, 7.0115552), //essen
              new $wnd.google.maps.LatLng(51.3670777, 7.4632841)//hagen
             ];

          triangle = new $wnd.google.maps.Polygon({
                  paths: triangleCoords,
                  strokeColor: "#FF0000",
                  strokeOpacity: 0.8,
                  strokeWeight: 2,
                  fillColor: "#FF0000",
                  fillOpacity: 0.35
                });

          triangle.setMap(map); 
      
							}-*/;
	
	public native void drawPolylineTest()/*-{
	
	// Enable the visual refresh
     $wnd.google.maps.visualRefresh = true;      

     var mapOptions = {
        center: new $wnd.google.maps.LatLng(51.3670777, 7.4632841),
        zoom: 9, //0 = whole earth, 18 = streetview
        mapTypeId: $wnd.google.maps.MapTypeId.TERRAIN
    };

    
    var map = new $wnd.google.maps.Map($doc.getElementById("mapview"), mapOptions);  
    
    var pathCoordinates = [
            new $wnd.google.maps.LatLng(37.772323, -122.214897),
            new $wnd.google.maps.LatLng(21.291982, -157.821856),
            new $wnd.google.maps.LatLng(-18.142599, 178.431),
           new $wnd.google.maps.LatLng(-27.46758, 153.027892)
     ];
     
     //adding a new data with  path.push(latLng);   
    var flightPath = new $wnd.google.maps.Polyline({
          path: pathCoordinates,
          strokeColor: "#FF0000",
          strokeOpacity: 1.0,
          strokeWeight: 2
      });

   flightPath.setMap(map);
  
						}-*/;
	

	public ScrollPanel getTextPanel() {
		return textPanel;
	}

	public void setTextPanel(ScrollPanel textPanel) {
		this.textPanel = textPanel;
	}

	public ScrollPanel getScrollPanel() {
		return scrollPanel;
	}

	public void setScrollPanel(ScrollPanel scrollPanel) {
		this.scrollPanel = scrollPanel;
	}

	@Override
	public void resizeWithTextAndCP(int width, int height) {
		// TODO Auto-generated method stub
		
	}

	
}
