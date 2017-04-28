package de.fernunihagen.dna.mapmatching;

import android.graphics.Color;
import android.graphics.Paint;
import android.location.Location;

import org.osmdroid.api.IMapController;
import org.osmdroid.util.GeoPoint;
import org.osmdroid.views.MapView;
import org.osmdroid.views.overlay.Marker;
import org.osmdroid.views.overlay.Polyline;

import java.util.ArrayList;
import java.util.List;

import de.fernunihagen.dna.mapmatchingcore.MapMatchingPath;
import de.fernunihagen.dna.mapmatchingcore.MapMatchingPathHistoryEntry;
import de.fernunihagen.dna.mapmatchingcore.NetworkEdge;
import de.fernunihagen.dna.mapmatchingcore.Point;
import de.fernunihagen.dna.mapmatchingcore.SymbolicTrajectory;

public class MapMatchingMap {

    private static MapView mapView;
    private static MapMatchingActivity mapMatchingActivity;
    private static boolean showMap;
    private static boolean showGpsPoints;
    private static List<MapMatchingPath> recentResults = new ArrayList<MapMatchingPath>();

    private static boolean mapReload = false;

    public static void updateMap(List<MapMatchingPath> cleanResults){
        if(showMap && cleanResults.size()!=0){

            MapMatchingPath recentPath = cleanResults.get(cleanResults.size()-1);
            int indexOfLastEdge = recentPath.getEdges().size()-1;
            List<MapMatchingPathHistoryEntry> history = recentPath.getHistoryPerEdge().get(indexOfLastEdge);
            int indexOfLastHistoryEntry = history.size()-1;
            String streetname = SymbolicTrajectory.getStreetname(recentPath.getEdges().get(indexOfLastEdge));
            String direction = SymbolicTrajectory.getCardinalDirection(history.get(indexOfLastHistoryEntry).getLocationPoint().getBearing());
            mapMatchingActivity.getLabelMapStreetname().setText(streetname);
            mapMatchingActivity.getLabelMapDirection().setText(direction);

            recentResults = cleanResults;
            mapView.getOverlays().clear();
            createPathPolylineOverlays(cleanResults);

            if(showGpsPoints){
                createGpsMarkerOverlays(cleanResults);
            }
            IMapController mapController = mapView.getController();
            if(mapView.getZoomLevel()==0){
                mapController.setZoom(18);
            }
            if(!mapReload){
                mapController.setCenter(cleanResults.get(cleanResults.size()-1).getRecentProjectedPoint().getGeoPoint());
            }
            mapView.invalidate();
        }
    }

    public static void initialize(Location locationPoint){
        IMapController mapController = mapView.getController();
        mapController.setZoom(18);
        mapController.setCenter(new GeoPoint(locationPoint.getLatitude(), locationPoint.getLongitude()));
    }

    public static void reload(){
        if(recentResults!=null){
            mapReload=true;
            updateMap(recentResults);
            mapReload=false;
        }
    }

    public static void refreshMap(double latitude, double longitude, int zoom){
        IMapController mapController = mapView.getController();
        mapController.setCenter(new GeoPoint(latitude,longitude));
        mapController.setZoom(zoom);
    }

    public static void clearData(){
        mapMatchingActivity.getLabelMapDirection().setText("-");
        mapMatchingActivity.getLabelMapStreetname().setText("-");
        recentResults.clear();
        mapView.getOverlays().clear();
        mapView.invalidate();
    }

    private static void createPathPolylineOverlays(List<MapMatchingPath> results){
        List<List<GeoPoint>> startWayPoints = MapMatchingCoreInterface.getStartWayPoints();
        List<List<GeoPoint>> endWayPoints = MapMatchingCoreInterface.getEndWayPoints();
        List<Polyline> pathPolylineOverlays = new ArrayList<Polyline>();

        for(int i=0;i<startWayPoints.size();i++){
            GeoPoint lastEndPoint = null;
            for(int j=0;j<startWayPoints.get(i).size();j++){
                //Secure connected Line
                if(lastEndPoint != null && !(startWayPoints.get(i).get(j).getLatitude()==lastEndPoint.getLatitude() && startWayPoints.get(i).get(j).getLongitude()==lastEndPoint.getLongitude())){
                    Polyline polyline = new Polyline();
                    List<GeoPoint> linePoints = new ArrayList<GeoPoint>();
                    linePoints.add(lastEndPoint);
                    linePoints.add(startWayPoints.get(i).get(j));
                    polyline.setPoints(linePoints);
                    styleWaypointOverlay(polyline.getPaint());
                    pathPolylineOverlays.add(polyline);
                }
                lastEndPoint = endWayPoints.get(i).get(j);

                Polyline polyline = new Polyline();
                List<GeoPoint> linePoints = new ArrayList<GeoPoint>();
                linePoints.add(startWayPoints.get(i).get(j));
                linePoints.add(endWayPoints.get(i).get(j));
                polyline.setPoints(linePoints);
                styleWaypointOverlay(polyline.getPaint());
                pathPolylineOverlays.add(polyline);
            }
        }
        mapView.getOverlays().addAll(pathPolylineOverlays);
    }

    private static void createGpsMarkerOverlays(List<MapMatchingPath> results){
        List<Marker> markerOverlays = new ArrayList<Marker>();
        List<Polyline> markerConnectionOverlays = new ArrayList<Polyline>();
        for (MapMatchingPath path : results) {
            for (int i=0;i<path.getHistoryPerEdge().size();i++){
                NetworkEdge edge = path.getEdges().get(i);
                for(int j=0;j<path.getHistoryPerEdge().get(i).size();j++){
                    MapMatchingPathHistoryEntry historyEntry = path.getHistoryPerEdge().get(i).get(j);
                    Marker marker = new Marker(mapView);
                    styleMarkerOverlay(marker, historyEntry, edge);
                    marker.setPosition(historyEntry.getLocationPoint().getGeoPoint());
                    markerOverlays.add(marker);

                    Polyline markerPolyline = new Polyline();
                    styleMarkerConnectionOverlay(markerPolyline.getPaint());
                    List<GeoPoint> markerPolylinePoints = new ArrayList<GeoPoint>();
                    markerPolylinePoints.add(historyEntry.getLocationPoint().getGeoPoint());
                    markerPolylinePoints.add(historyEntry.getProjectedPoint().getGeoPoint());
                    markerPolyline.setPoints(markerPolylinePoints);
                    markerConnectionOverlays.add(markerPolyline);
                }
            }
        }
        mapView.getOverlays().addAll(markerOverlays);
        mapView.getOverlays().addAll(markerConnectionOverlays);
    }

    private static void styleMarkerOverlay(Marker marker, MapMatchingPathHistoryEntry historyEntry, NetworkEdge edge){
        marker.setAnchor(Marker.ANCHOR_CENTER, Marker.ANCHOR_BOTTOM);
        marker.setIcon(mapMatchingActivity.getResources().getDrawable(R.drawable.mapmarker));

        Point locationPoint = historyEntry.getLocationPoint();
        String streetname, time, cardinalDirection, speed ="-", height="-", accuracy, pdop, hdop, vdop, satellitesVisible, satellitesUsed, score, distance, angleDifference;
        streetname = SymbolicTrajectory.getStreetname(edge);
        double scoreRounded = Math.round(historyEntry.getScore()*10)/10.0;
        score = String.valueOf(scoreRounded);
        double accuracyRounded = Math.round(locationPoint.getAccuracy()*10)/10.0;
        accuracy = String.valueOf(accuracyRounded);
        time = SymbolicTrajectory.getTimeFromDateLongWithTimeZone(locationPoint.getTime());
        cardinalDirection = SymbolicTrajectory.getCardinalDirection(locationPoint.getBearing());

        double speedRounded = Math.round(locationPoint.getSpeed()*10)/10.0;
        speed = String.valueOf(speedRounded);

        if(locationPoint.getAltitude()!= Double.MAX_VALUE){
            double heightRounded = Math.round(locationPoint.getAltitude()*10)/10.0;
            height = String.valueOf(heightRounded);
        }
        pdop = String.valueOf(locationPoint.getPdop());
        hdop = String.valueOf(locationPoint.getHdop());
        vdop = String.valueOf(locationPoint.getVdop());
        satellitesVisible = String.valueOf(locationPoint.getSatellitesVisible());
        satellitesUsed = String.valueOf(locationPoint.getSatellitesUsed());

        double distanceRounded = Math.round(historyEntry.getDistanceBetweenLocationAndProjectedPoint()*10)/10.0;
        distance = String.valueOf(distanceRounded);
        double angleDifferenceRounded = Math.round(historyEntry.getAbsoluteAngleDifference()*10)/10.0;
        angleDifference = String.valueOf(angleDifferenceRounded);

        marker.setTitle("Time: "+time+"\nStreetname: "+streetname+"\nDirection: "+cardinalDirection+"\nSpeed: "+speed+" km/h"+"\nHeight: "+height+" meter"+"\nProbable Accuracy: "+accuracy+" meter" + "\nPDOP: "+pdop+", HDOP: "+hdop+", VDOP: "+vdop+"\nSatellites: Used "+satellitesUsed+", Visible "+satellitesVisible+"\nScore: "+score+"\nDistance: "+distance+" meter"+"\nAbsolute Angle Difference: "+angleDifference+(char) 0x00B0);
    }

    private static void styleMarkerConnectionOverlay(Paint paint){
        paint.setColor(Color.GREEN);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeJoin(Paint.Join.ROUND);
        paint.setStrokeCap(Paint.Cap.ROUND);
        //paint.setAlpha(90);
        paint.setStrokeWidth(2);
    }

    private static void styleWaypointOverlay(Paint paint){
        paint.setColor(Color.rgb(0,179,253));
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeJoin(Paint.Join.ROUND);
        paint.setStrokeCap(Paint.Cap.ROUND);
        //paint.setAlpha(90);
        paint.setStrokeWidth(10);
    }

    public static void setMapView(MapView mapView) {
        MapMatchingMap.mapView = mapView;
    }

    public static void setMapMatchingActivity(MapMatchingActivity mapMatchingActivity) {
        MapMatchingMap.mapMatchingActivity = mapMatchingActivity;
    }

    public static boolean isShowMap() {
        return showMap;
    }

    public static void setShowMap(boolean showMap) {
        MapMatchingMap.showMap = showMap;
    }

    public static void setShowGpsPoints(boolean showGpsPoints) {
        MapMatchingMap.showGpsPoints = showGpsPoints;
    }


}
