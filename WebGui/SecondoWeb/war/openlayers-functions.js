var map;
//var bounds = new google.maps.LatLngBounds();;
var markersArray = [];
var lineArray = [];
var locationsArray = [];
var polylineArray = [];
var polygonArray = [];
//var polygon = new google.maps.Polygon(); //make sure the google.maps.Polygon instance is global .. so u can then reference the same instance to remove it .. 
//var polyline = new google.maps.Polyline();
//var hagen = new OpenLayers.Geometry.Point(51.3760448, 7.4947253).transform('EPSG:4326', 'EPSG:3857');

//gets the coordinates for the center of the map
function initializeOSMMap() {
	
	//alert("osm-js called"); //funktioniert, function is called
	
	map = new OpenLayers.Map(document.getElementById("osmmap"));
    var mapnik         = new OpenLayers.Layer.OSM();
    var fromProjection = new OpenLayers.Projection("EPSG:4326");   // Transform from WGS 1984
    var toProjection   = new OpenLayers.Projection("EPSG:900913"); // to Spherical Mercator Projection
    var position       = new OpenLayers.LonLat(13.41,52.52).transform( fromProjection, toProjection);
    var zoom           = 15; 

    map.addLayer(mapnik);
    map.setCenter(position, zoom );
	
	/*var proj4326 = new OpenLayers.Projection("EPSG:4326");
    var projmerc = new OpenLayers.Projection("EPSG:900913");

    var lonlat = new OpenLayers.LonLat(13.38, 52.52);
    var zoom = 13;
    var mlonlat = new OpenLayers.LonLat(13.3776, 52.5162);
    
    var map = new OpenLayers.Map(document.getElementById("osmview"), {
		controls: [
			new OpenLayers.Control.KeyboardDefaults(),
			new OpenLayers.Control.Navigation(),
			new OpenLayers.Control.LayerSwitcher(),
			new OpenLayers.Control.PanZoomBar(),
            new OpenLayers.Control.MousePosition()
        ],
		maxExtent:
            new OpenLayers.Bounds(-20037508.34,-20037508.34,
                                   20037508.34, 20037508.34),
		numZoomLevels: 18,
        maxResolution: 156543,
        units: 'm',
        projection: projmerc,
        displayProjection: proj4326
     } );

    var mapnik_layer = new OpenLayers.Layer.OSM.Mapnik("Mapnik");
    map.addLayers([mapnik_layer]);

    lonlat.transform(proj4326, projmerc);
    map.setCenter(lonlat, zoom);

    var size = new OpenLayers.Size(32, 32);
    var offset = new OpenLayers.Pixel(-22, -30);
    var icon = new OpenLayers.Icon('/resources/images/marker.png', size, offset);
    var marker = new OpenLayers.Marker(mlonlat.transform(proj4326, projmerc), icon);

    var markers = new OpenLayers.Layer.Markers("Markers");
    markers.addMarker(marker);
    map.addLayer(markers);*/

    //map = new OpenLayers.Map(document.getElementById("osmview"));
	
    
 // The location of our marker and popup. We usually think in geographic
    // coordinates ('EPSG:4326'), but the map is projected ('EPSG:3857').
   /* var myLocation = new OpenLayers.Geometry.Point(10.2, 48.9)
        .transform('EPSG:4326', 'EPSG:3857');
    
 // The overlay layer for a marker, with a simple diamond as symbol
    var overlay = new OpenLayers.Layer.Vector('Overlay', {
        styleMap: new OpenLayers.StyleMap({
            externalGraphic: 'resources/images/marker.png',
            graphicWidth: 20, graphicHeight: 24, graphicYOffset: -24,
            title: '${tooltip}'
        })
    });
     // add the marker with a tooltip text to the overlay
    overlay.addFeatures([
        new OpenLayers.Feature.Vector(myLocation, {tooltip: 'OpenLayers'})
    ]);*/
    
 // A popup with some information about the location
  /*  var popup = new OpenLayers.Popup.FramedCloud("Popup", 
        myLocation.getBounds().getCenterLonLat(), null,
        '<a target="_blank" href="http://openlayers.org/">We</a> ' +
        'could be here.<br>Or elsewhere.', null,
        true // <-- true if we want a close (X) button, false otherwise
    );*/
    
 // create the map
    /*map = new OpenLayers.Map({
        div: document.getElementById("osmview"), projection: "EPSG:3857",
        layers: [new OpenLayers.Layer.OSM(), overlay],
        center: myLocation.getBounds().getCenterLonLat(), zoom: 15
    });*/


    // and add the popup to it.
    //map.addPopup(popup);
	
    //Layers are the ‘datasources’ in OpenLayers
	/*var wms = new OpenLayers.Layer.WMS(
			  "OpenLayers WMS",
			  "http://vmap0.tiles.osgeo.org/wms/vmap0",
			  {'layers':'basic'} );
	
	map.addLayer(wms);*/
    
	/*var dm_wms = new OpenLayers.Layer.WMS(
            "Canadian Data",
            "http://www2.dmsolutions.ca/cgi-bin/mswms_gmap",
            {
                layers: "bathymetry,land_fn,park,drain_fn,drainage," +
                        "prov_bound,fedlimit,rail,road,popplace",
                transparent: "true",
                format: "image/png"
            },
            {isBaseLayer: false}
        );
	
	//map.addLayers([wms, dm_wms]);*/
	
	//a marker overlay
	/*var vectorLayer = new OpenLayers.Layer.Vector("Overlay");
	var feature = new OpenLayers.Feature.Vector(
	              new OpenLayers.Geometry.Point(-71, 42),
	              {some:'data'},
	              {externalGraphic: 'resources/images/marker.png', graphicHeight: 21, graphicWidth: 16});
	vectorLayer.addFeatures(feature);
	//map.addLayer(vectorLayer);*/
	
	//map.zoomToMaxExtent();
  
}