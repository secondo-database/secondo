var map;
var bounds = new google.maps.LatLngBounds();;
var markersArray = [];
var lineArray = [];
var locationsArray = [];
var polylineArray = [];
var polygonArray = [];
var polygon = new google.maps.Polygon(); //make sure the google.maps.Polygon instance is global .. so u can then reference the same instance to remove it .. 
var polyline = new google.maps.Polyline();
var hagen = new google.maps.LatLng(51.3760448, 7.4947253);

//gets the coordinates for the center of the map
function initializeMap(location) {
	
  google.maps.visualRefresh = true;

  var mapOptions = {
	        center: location,
	        zoom: 8, //0 = whole earth, 18 = streetview
	        maxzoom: 16,
	        mapTypeId: google.maps.MapTypeId.ROADMAP
	    };
  
  map =  new google.maps.Map(document.getElementById("mapview"), mapOptions);
  
  //instantiate the bounds object
  //bounds = new google.maps.LatLngBounds();
  
  //add a marker with the given position
  /*var marker = new google.maps.Marker({
      position: location,
      map: map,
      title: 'Point from Secondo'
  });
  
  marker.setMap(map);*/

  //event that adds a marker when clicking on the map
 /* google.maps.event.addListener(map, 'click', function(event) {
    addMarker(event.latLng);
  });*/
}

//Creates a new marker and adds it to the marker array
function addMarker(location) {
  marker = new google.maps.Marker({
    position: location,
    map: map
  });
  
  markersArray.push(marker);
  bounds.extend(location);
}


// Removes the overlays from the map, but keeps them in the array
function clearMarkerOverlays() {
  if (markersArray) {
    for (i in markersArray) {
      markersArray[i].setMap(null);
    }
  }
}

// Shows any overlays currently in the array
function showMarkerOverlays() {
	
  if (markersArray) {
  
    if (markersArray.length === 1) { //just one marker in the array
    	map.setOptions({ //klappt!!!!!!!
    	    maxZoom: 10
    	});
  	}
    
    else {
    	//zoom should not be bigger than 15 - klappt auch!
  	  /*var boundslistener = google.maps.event.addListener(map, 'bounds_changed', function(event) {
		    if (this.getZoom() > 15){
		        this.setZoom(15);
		    }
		    google.maps.event.removeListener(boundslistener);
		});*/
    	  map.fitBounds(bounds);
    	}
    
    for (i in markersArray) {
        markersArray[i].setMap(map);
      }
  } 
}

// Deletes all markers in the array by removing references to them
function deleteAllMarkers() {
  if (markersArray) {
    for (i in markersArray) {
      markersArray[i].setMap(null);
    }
    markersArray.length = 0;
  }
}

//adds a line to the linearray
function addLine(location1, location2){
	
	var lineCoordinates = [
	                        location1,
	                        location2,
	                           ];

	
	var line = new google.maps.Polyline({
        path: lineCoordinates,
        strokeColor: "#FF0000",
        strokeOpacity: 1.0,
        strokeWeight: 2
    });
	lineArray.push(line);
	bounds.extend(location1);
}

//Show the lines saved in the linearray 
function showLineOverlays() {
	 
	if (lineArray) {
	    for (i in lineArray) {
	      lineArray[i].setMap(map);
	    }
		  map.fitBounds(bounds);
	  }
}

//delete all lines from the linearray and from the map
function deleteAllLines(){

	if (lineArray) {
	    for (i in lineArray) {
	      lineArray[i].setMap(null);
	    }
	    while (lineArray.length > 0) {
	        lineArray.pop();
	      }
	  }
}

//Adds a location to the location array
function addLocation(location) {
	  locationsArray.push(location);
	  
	  bounds.extend(location);
}



//Deletes all locations in the array by removing references to them
function deleteAllLocations() {
  if (locationsArray) {
	
     while (locationsArray.length > 0) {
      locationsArray.pop();
    }
  }
}


//Adds a polyline to the polyline array
function addPolyline() {
			
	if(locationsArray){
		
	    var linePath = new google.maps.Polyline({
	          path: locationsArray,
	          strokeColor: "#FF0000",
	          strokeOpacity: 1.0,
	          strokeWeight: 2
	      });
	

	  polylineArray.push(linePath);
	  
	  //deleteAllLocations();
	  }
}

//Draws a polyline with the data from the locations array
function drawPolyline(){
	
	if (locationsArray) {
	
    polyline = new google.maps.Polyline({
          path: locationsArray, //just 1 path possible!
          strokeColor: "#FF0000",
          strokeOpacity: 1.0,
          strokeWeight: 2,
          map: map
      });
    
    polyline.setMap(map);	
    
    map.fitBounds(bounds);
	}
}

//Show the polylines saved in the polylinearray = zeigt nur das letzte polyline
function showPolylineOverlays() {
	 
	if (polylineArray) {
	    for (i in polylineArray) {
	      polylineArray[i].setMap(map);
	    }
		  map.fitBounds(bounds);
	  }
}

//deletes all polylines from the polylinearray
function deleteAllPolylines(){
	if (polylineArray) {
		
	     while (polylineArray.length > 0) {
	      polylineArray.pop();
	    }
	     //delete all overlays from the map
	 	map.overlayMapTypes.setAt( 0, null);
	  }
}


//a polyline cannot have multiple paths, just a polygon can have 2, polygon var has to be global!
//Draws a polygon with the data from all location arrays
function drawPolygon(){
	
	if (locationsArray) {
		
				
		polygon = new google.maps.Polygon({
            paths: locationsArray, //a polygon can have max 2 paths = an inner and outer line!
            strokeColor: "#FF0000",
            strokeOpacity: 0.8,
            strokeWeight: 2,
            fillColor: "#FFFFFF",
            fillOpacity: 0.35,
            map: map
          });
		
       polygon.setMap(map);	
    
       map.fitBounds(bounds);
	}
}

//Adds a polygon to the polygon array - the same polygon is used, just with different paths and options
function addPolygon() {
	
	if (polygonArray) {
		
		if(locationsArray){
		
	    var polygon = new google.maps.Polygon({
            paths: locationsArray,
            strokeColor: "#FF0000",
            strokeOpacity: 0.8,
            strokeWeight: 2,
            fillColor: "#FFFFFF",
            fillOpacity: 0.15
          });
	

	  polygonArray.push(polygon);

	  }
	}
}

//Show the polygons saved in the polygonarray - maybe add a layer for many overlays
function showPolygonOverlays() {
	 
	if (polygonArray) {
	    for (i in polygonArray) {
	      polygonArray[i].setMap(map);
	    }
		  map.fitBounds(bounds);
	  }
}

//deletes all polylines from the polylinearray
function deleteAllPolygons(){
	if (polygonArray) {
		
	     while (polygonArray.length > 0) {
	      polygonArray.pop();
	    }

	     //delete all overlays from the map
	 	map.overlayMapTypes.setAt( 0, null);
	  }
}


//resets the bounds of the map to null
function resetBounds(){
	  bounds = new google.maps.LatLngBounds(null); //setting LatLngBounds to null resets the current bounds and allows the new call for zoom in/out to be made directly against the latest markers to be plotted on the map
}

//resets the map to standard values
function resetMap() {
    map.setCenter(hagen);
    map.setZoom(8);

  }  