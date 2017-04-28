package de.fernunihagen.dna.mapmatchingcore;

import org.osmdroid.util.GeoPoint;

import java.util.ArrayList;
import java.util.List;

public class MapMatchingUtilities {

    private Point lastProjectedPoint;
    private MapMatching mapMatching;

    private int distanceMultiplicator;

    private boolean extensionCompareAngle;
    private int extensionAngleTolerance;

    private int countOfDisplayedEdges;
    private boolean cleanMapData;
    private List<List<GeoPoint>> startWayPoints = new ArrayList<List<GeoPoint>>();
    private List<List<GeoPoint>> endWayPoints = new ArrayList<List<GeoPoint>>();

    public MapMatchingUtilities(MapMatching mapMatching){
        this.mapMatching = mapMatching;
    }

    public double distance(Point pointA, Point pointB) {
        double R = 6371.01;
        double dLat = Math.toRadians(pointB.getLatitude() - pointA.getLatitude());
        double dLon = Math.toRadians(pointB.getLongitude() - pointA.getLongitude());
        double latA = Math.toRadians(pointA.getLatitude());
        double latB = Math.toRadians(pointB.getLatitude());

        double a = Math.pow(Math.sin(dLat / 2), 2) + Math.pow(Math.sin(dLon / 2), 2) * Math.cos(latA) * Math.cos(latB);
        double c = 2 * Math.asin(Math.sqrt(a));
        return R * c * 1000;
    }

    public double bearing(Point pointA, Point pointB) {
        double longitude1 = pointA.getLongitude();
        double longitude2 = pointB.getLongitude();
        double latitude1 = Math.toRadians(pointA.getLatitude());
        double latitude2 = Math.toRadians(pointB.getLatitude());
        double longDiff = Math.toRadians(longitude2 - longitude1);
        double y = Math.sin(longDiff) * Math.cos(latitude2);
        double x = Math.cos(latitude1) * Math.sin(latitude2) - Math.sin(latitude1) * Math.cos(latitude2) * Math.cos(longDiff);

        return (Math.toDegrees(Math.atan2(y, x)) + 360) % 360;
    }

    public Point projectPoint(Point p, double angle, double distance) {
        double rLat1 = Math.toRadians(p.getLatitude());
        double rLon1 = Math.toRadians(p.getLongitude());
        double rAngle = Math.toRadians(angle);
        double rDistance = distance / (6371.01 * 1000);

        double lat = Math.asin(Math.sin(rLat1) * Math.cos(rDistance) + Math.cos(rLat1) * Math.sin(rDistance) * Math.cos(rAngle));
        double lon = rLon1 + Math.atan2(Math.sin(rAngle) * Math.sin(rDistance) * Math.cos(rLat1), Math.cos(rDistance) - Math.sin(rLat1) * Math.sin(lat));

        return new Point(Math.toDegrees(lat), Math.toDegrees(lon));
    }

    public double differenceBetweenBearings(double bearingA, double bearingB)
    {
        double difference = bearingB - bearingA;
        if (difference > 180){
            difference = difference - 360;
        }
        if (difference < -180){
            difference = difference + 360;
        }
        return difference;
    }

    public double calculateScore(Point locationPoint, MapMatchingPath path) {
        int indexOfLastEdge = path.getEdges().size() - 1;
        NetworkEdge edge = path.getEdges().get(indexOfLastEdge);

        double distance = distance(locationPoint, path.getRecentProjectedPoint());
        double distanceScore = distance * distanceMultiplicator;

        int segmentIndex = path.getRecentProjectedPointSegmentIndex();
        double bearingSegment = bearing(edge.getStartSegmentPoint().get(segmentIndex), edge.getEndSegmentPoint().get(segmentIndex));
        double bearingLocationPoint = locationPoint.getBearing();
        double bearingScore = Math.abs(differenceBetweenBearings(bearingSegment,bearingLocationPoint));

        double finalScore = distanceScore + bearingScore;

        if(mapMatching.isTemporaryPreferRoads()){
            if(!isRoad(edge.getRoadType())){
                finalScore = finalScore+20;
            }
        }

        return finalScore;
    }

    public void averageSpeedOfLastThreePoints(Point locationPoint){
        mapMatching.setAverageSpeed(locationPoint.getSpeed());
        int counter = 1;
        MapMatchingPath path = mapMatching.getCurrentResult();

        if(path.getEdges().size()!=0){
            for (int i=path.getHistoryPerEdge().size()-1 ; i>=0 ; i--)
            {
                for (int j=path.getHistoryPerEdge().get(i).size()-1 ; j>=0 ; j--)
                {
                    counter = counter + 1;
                    mapMatching.setAverageSpeed(mapMatching.getAverageSpeed() + path.getHistoryPerEdge().get(i).get(j).getLocationPoint().getSpeed());
                    if(counter==3){break;}
                }
                if(counter==3){break;}
            }
        }

        mapMatching.setAverageSpeed(mapMatching.getAverageSpeed() / counter);
    }

    public boolean isRoad(String roadType){
        if(roadType.equals("motorway")||roadType.equals("trunk")||roadType.equals("primary")||roadType.equals("secondary")||roadType.equals("tertiary")||roadType.equals("unclassified")||roadType.equals("residential")||roadType.equals("service")||roadType.equals("motorway_link")||roadType.equals("trunk_link")||roadType.equals("primary_link")||roadType.equals("secondary_link")||roadType.equals("tertiary_link")||roadType.equals("living_street")||roadType.equals("pedestrian")||roadType.equals("track")||roadType.equals("bus_guideway")||roadType.equals("escape")||roadType.equals("raceway")||roadType.equals("road")){
            return true;
        }
        else{
            return false;
        }
    }

    public double projectPointToSegment(Point locationPoint, Point segmentPointA, Point segmentPointB) {
        double xLeft, yLeft, xRight, yRight, x, y;

        x = locationPoint.getLatitude();
        y = locationPoint.getLongitude();

        if (segmentPointA.getLatitude() <= segmentPointB.getLatitude()) {
            xLeft = segmentPointA.getLatitude();
            yLeft = segmentPointA.getLongitude();
            xRight = segmentPointB.getLatitude();
            yRight = segmentPointB.getLongitude();
        } else {
            xLeft = segmentPointB.getLatitude();
            yLeft = segmentPointB.getLongitude();
            xRight = segmentPointA.getLatitude();
            yRight = segmentPointA.getLongitude();
        }

        //vertikales , division durch 0 wenn nicht alternativ behandelt
        if (xLeft == xRight) {
            if ((yLeft <= y && y <= yRight) || (yRight <= y && y <= yLeft)) {
                Point p = new Point(xLeft, y);
                lastProjectedPoint = p;
                return distance(p, locationPoint);
            }
        } else {
            double k = (yRight - yLeft) / (xRight - xLeft);
            double a = yLeft - k * xLeft;
            double xProjected = (k * (y - a) + x) / (k * k + 1);
            double yProjected = k * xProjected + a;

            if (xLeft <= xProjected && xProjected <= xRight) {
                Point p = new Point(xProjected, yProjected);
                lastProjectedPoint = p;
                return distance(p, locationPoint);
            }
        }

        double distanceA = distance(segmentPointA, locationPoint);
        double distanceB = distance(segmentPointB, locationPoint);

        if (distanceA <= distanceB) {
            lastProjectedPoint = segmentPointA;
            return distanceA;
        } else {
            lastProjectedPoint = segmentPointB;
            return distanceB;
        }
    }

    public boolean areEdgesEqual(MapMatchingPath pathA, MapMatchingPath pathB) {
        List<NetworkEdge> edgesA = pathA.getEdges();
        List<NetworkEdge> edgesB = pathB.getEdges();

        if (edgesA.size() == edgesB.size()) {
            for (int i = 0; i < edgesA.size(); i++) {
                if (!edgesA.get(i).equals(edgesB.get(i))) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    public boolean areProjectedPointsEqual(MapMatchingPath pathA, MapMatchingPath pathB) {
        List<Point> pathAPoints = new ArrayList<Point>();
        List<Point> pathBPoints = new ArrayList<Point>();

        for (List<MapMatchingPathHistoryEntry> historyPerEdge : pathA.getHistoryPerEdge()) {
            for (MapMatchingPathHistoryEntry history : historyPerEdge) {
                pathAPoints.add(history.getProjectedPoint());
            }
        }
        pathAPoints.add(pathA.getRecentProjectedPoint());


        for (List<MapMatchingPathHistoryEntry> historyPerEdge : pathB.getHistoryPerEdge()) {
            for (MapMatchingPathHistoryEntry history : historyPerEdge) {
                pathBPoints.add(history.getProjectedPoint());
            }
        }
        pathBPoints.add(pathB.getRecentProjectedPoint());


        if (pathAPoints.size() == pathBPoints.size()) {
            for (int i = 0; i < pathAPoints.size(); i++) {
                if (!pathAPoints.get(i).equals(pathBPoints.get(i))) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    public double distanceOfPath(MapMatchingPath path){
        double distance = 0;

        for (NetworkEdge edge : path.getEdges()) {
            distance = distance + distanceOfEdge(edge);
        }

        return distance;
    }

    public double distanceOfEdge(NetworkEdge edge) {
        double distance = 0;
        for (int i = 0; i < edge.getStartSegmentPoint().size(); i++) {
            distance = distance + distance(edge.getStartSegmentPoint().get(i), edge.getEndSegmentPoint().get(i));
        }
        return distance;
    }

    public double distanceOfMovementResult(MovementResult movementResult) {
        double distance = 0;
        for (int i = 0; i < movementResult.getStartSegmentPoints().size(); i++) {
            distance = distance + distance(movementResult.getStartSegmentPoints().get(i), movementResult.getEndSegmentPoints().get(i));
        }
        return distance;
    }

    public double distanceOfEdgeStartToPoint(NetworkEdge edge, MapMatchingPathHistoryEntry historyEntry){
        double distance = 0;

        for (int i = 0; i < edge.getStartSegmentPoint().size(); i++) {
            if (i==historyEntry.getSegmentIndex()){
                distance = distance + mapMatching.getUtil().distance(edge.getStartSegmentPoint().get(i), historyEntry.getProjectedPoint());
                break;
            }
            distance = distance + mapMatching.getUtil().distance(edge.getStartSegmentPoint().get(i), edge.getEndSegmentPoint().get(i));
        }

        return distance;
    }

    public double distanceOfPointsMappedToLastEdge(Point currentLocationPoint, MapMatchingPath path) {
        int indexOfLastEdge = path.getEdges().size() - 1;
        List<MapMatchingPathHistoryEntry> historyList = path.getHistoryPerEdge().get(indexOfLastEdge);
        double distance = 0;

        for (int i = 0; i < historyList.size(); i++) {
            if (i + 1 != historyList.size()) {
                distance = distance + distance(historyList.get(i).getLocationPoint(), historyList.get(i + 1).getLocationPoint());
            } else {
                distance = distance + distance(historyList.get(i).getLocationPoint(), currentLocationPoint);
            }
        }
        return distance;
    }

    public double percentalSpanOfProjectedPointsMappedToLastEdge(MapMatchingPath path, double distanceOfEdge) {
        int indexOfLastEdge = path.getEdges().size() - 1;
        NetworkEdge edge = path.getEdges().get(indexOfLastEdge);

        double distance = 0;

        Point endPoint = path.getRecentProjectedPoint();
        int endPointSegmentIndex = path.getRecentProjectedPointSegmentIndex();

        distance = distance + distance(endPoint, edge.getStartSegmentPoint().get(endPointSegmentIndex));

        for (int i = 0; i < endPointSegmentIndex; i++) {
            distance = distance + distance(edge.getStartSegmentPoint().get(i), edge.getEndSegmentPoint().get(i));
        }
        double percentalSpan = (distance * 100) / distanceOfEdge;

        return percentalSpan;
    }

    public boolean angleOfPointsComparedToAngleOfEdge(Point currentLocationPoint, MapMatchingPath path){
        if(extensionCompareAngle){
            int indexOfLastEdge = path.getEdges().size() - 1;
            List<MapMatchingPathHistoryEntry> historyList = path.getHistoryPerEdge().get(indexOfLastEdge);
            int indexOfLastHistoryEntry = historyList.size() - 1;
            int segmentIndex = path.getRecentProjectedPointSegmentIndex();
            NetworkEdge edge = path.getEdges().get(indexOfLastEdge);

            if(historyList.size()!=0 && historyList.get(indexOfLastHistoryEntry).getSegmentIndex() == segmentIndex){
                double averageBearingOfLastTwoPoints;
                double currentBearing = currentLocationPoint.getBearing();
                double lastBearing = historyList.get(indexOfLastHistoryEntry).getLocationPoint().getBearing();
                averageBearingOfLastTwoPoints = (currentBearing+lastBearing)/2;

                double bearingSegment = bearing(edge.getStartSegmentPoint().get(segmentIndex), edge.getEndSegmentPoint().get(segmentIndex));
                double bearingDifference = Math.abs(differenceBetweenBearings(bearingSegment,averageBearingOfLastTwoPoints));

                if(bearingDifference> extensionAngleTolerance){
                    return true;
                }
            }
            else{
                double currentBearing = currentLocationPoint.getBearing();
                double bearingSegment = bearing(edge.getStartSegmentPoint().get(segmentIndex), edge.getEndSegmentPoint().get(segmentIndex));
                double bearingDifference = Math.abs(differenceBetweenBearings(bearingSegment,currentBearing));
                if(bearingDifference> extensionAngleTolerance){
                    return true;
                }
            }
        }
        return false;
    }


    public void generateWaypoints(List<MapMatchingPath> paths){
        startWayPoints.clear();
        endWayPoints.clear();
        for(MapMatchingPath path : paths){
            List<GeoPoint> startWaypointsForMap = new ArrayList<GeoPoint>();
            List<GeoPoint> endWaypointsForMap = new ArrayList<GeoPoint>();
            NetworkEdge edge;
            List<MapMatchingPathHistoryEntry> edgeHistory;

            edge = path.getEdges().get(0);
            edgeHistory = path.getHistoryPerEdge().get(0);

            //first point or first edge only with more than one point
            if(path.getEdges().size()==1){
                if(edgeHistory.size()==1){
                    return;
                }
                else{
                    generateWayPointsFromFirstProjectedPointToLastProjectedPointOnEdge(startWaypointsForMap,endWaypointsForMap, edge, edgeHistory);
                    startWayPoints.add(startWaypointsForMap);
                    endWayPoints.add(endWaypointsForMap);
                    return;
                }
            }
            //first edge
            if(edgeHistory.size()!=0){
                generateWaypointsFromFirstProjectedPointToEndPoint(startWaypointsForMap,endWaypointsForMap, edge, edgeHistory);
            }
            else {
                generateWaypointsFromStartPointToEndPoint(startWaypointsForMap,endWaypointsForMap, edge);
            }
            //edges between
            for(int i = 1; i < path.getEdges().size()-1; i++){
                edge = path.getEdges().get(i);
                edgeHistory = path.getHistoryPerEdge().get(i);
                //Uturn check
                if(path.getEdges().get(i).getSourceID()==path.getEdges().get(i+1).getSourceID()){
                    if(edgeHistory.size()!=0){
                        generateWaypointsFromStartPointToLastProjectedPoint(startWaypointsForMap,endWaypointsForMap, edge, edgeHistory);
                    }
                }
                else{
                    generateWaypointsFromStartPointToEndPoint(startWaypointsForMap,endWaypointsForMap, edge);
                }
            }
            //last edge
            if(path.getEdges().size()>1){
                int indexOfLastEdge = path.getEdges().size()-1;
                edge = path.getEdges().get(indexOfLastEdge);
                edgeHistory = path.getHistoryPerEdge().get(indexOfLastEdge);
                generateWaypointsFromStartPointToLastProjectedPoint(startWaypointsForMap,endWaypointsForMap, edge, edgeHistory);
            }
            startWayPoints.add(startWaypointsForMap);
            endWayPoints.add(endWaypointsForMap);
        }
    }

    public List<MapMatchingPath> cleanResults(List<MapMatchingPath> results){
        if(cleanMapData){
            int countOfEdges = 0;
            for(MapMatchingPath path : results){
                countOfEdges = countOfEdges + path.getEdges().size();
            }
            if(countOfEdges>countOfDisplayedEdges){
                int countToDelete = countOfEdges - countOfDisplayedEdges;
                for(int i=0; i<results.size();i++){
                    if(results.get(i).getEdges().size()>countToDelete){
                        results.get(i).setEdges(results.get(i).getEdges().subList(0+countToDelete,results.get(i).getEdges().size()));
                        results.get(i).setHistoryPerEdge(results.get(i).getHistoryPerEdge().subList(0+countToDelete, results.get(i).getHistoryPerEdge().size()));
                        break;
                    }
                    else{
                        countToDelete = countToDelete - results.get(i).getEdges().size();
                        results.remove(i);
                    }
                }
            }
        }
        return results;
    }

    //first edge - more than one point
    private void generateWayPointsFromFirstProjectedPointToLastProjectedPointOnEdge(List<GeoPoint> startWaypoints,List<GeoPoint> endWaypoints, NetworkEdge edge, List<MapMatchingPathHistoryEntry> edgeHistory){
        MapMatchingPathHistoryEntry firstPointOnEdge = getFirstProjectedPointOnEdge(edge, edgeHistory);
        int firstPointSegmentIndex = firstPointOnEdge.getSegmentIndex();
        MapMatchingPathHistoryEntry lastPointOnEdge = getLastProjectedPointOnEdge(edge, edgeHistory);
        int lastPointSegmentIndex = lastPointOnEdge.getSegmentIndex();

        if(firstPointSegmentIndex==lastPointSegmentIndex){
            startWaypoints.add(firstPointOnEdge.getProjectedPoint().getGeoPoint());
            endWaypoints.add(lastPointOnEdge.getProjectedPoint().getGeoPoint());
            return;
        }

        startWaypoints.add(firstPointOnEdge.getProjectedPoint().getGeoPoint());
        endWaypoints.add(edge.getEndSegmentPoint().get(firstPointSegmentIndex).getGeoPoint());

        for(int i = firstPointSegmentIndex+1; i < edge.getStartSegmentPoint().size(); i++){
            if(i>=lastPointSegmentIndex){
                break;
            }
            startWaypoints.add(edge.getStartSegmentPoint().get(i).getGeoPoint());
            endWaypoints.add(edge.getEndSegmentPoint().get(i).getGeoPoint());
        }

        startWaypoints.add(edge.getStartSegmentPoint().get(lastPointSegmentIndex).getGeoPoint());
        endWaypoints.add(lastPointOnEdge.getProjectedPoint().getGeoPoint());
    }

    //first edge
    private void generateWaypointsFromFirstProjectedPointToEndPoint(List<GeoPoint> startWaypoints, List<GeoPoint> endWaypoints, NetworkEdge edge, List<MapMatchingPathHistoryEntry> edgeHistory){
        MapMatchingPathHistoryEntry firstPointOnEdge = getFirstProjectedPointOnEdge(edge, edgeHistory);
        int firstPointSegmentIndex = firstPointOnEdge.getSegmentIndex();

        startWaypoints.add(firstPointOnEdge.getProjectedPoint().getGeoPoint());
        endWaypoints.add(edge.getEndSegmentPoint().get(firstPointSegmentIndex).getGeoPoint());

        for(int i = firstPointSegmentIndex+1; i < edge.getStartSegmentPoint().size(); i++){
            startWaypoints.add(edge.getStartSegmentPoint().get(i).getGeoPoint());
            endWaypoints.add(edge.getEndSegmentPoint().get(i).getGeoPoint());
        }
    }

    //edges between
    private void generateWaypointsFromStartPointToEndPoint(List<GeoPoint> startWaypoints, List<GeoPoint> endWaypoints, NetworkEdge edge){
        for(int i = 0; i < edge.getStartSegmentPoint().size(); i++){
            startWaypoints.add(edge.getStartSegmentPoint().get(i).getGeoPoint());
            endWaypoints.add(edge.getEndSegmentPoint().get(i).getGeoPoint());
        }
    }

    //last edge
    private void generateWaypointsFromStartPointToLastProjectedPoint(List<GeoPoint> startWaypoints, List<GeoPoint> endWaypoints, NetworkEdge edge, List<MapMatchingPathHistoryEntry> edgeHistory){
        MapMatchingPathHistoryEntry lastPointOnEdge = getLastProjectedPointOnEdge(edge, edgeHistory);
        int lastPointSegmentIndex = lastPointOnEdge.getSegmentIndex();

        for(int i = 0; i < edge.getStartSegmentPoint().size(); i++){
            if(i==lastPointSegmentIndex){
                break;
            }
            startWaypoints.add(edge.getStartSegmentPoint().get(i).getGeoPoint());
            endWaypoints.add(edge.getEndSegmentPoint().get(i).getGeoPoint());
        }

        startWaypoints.add(edge.getStartSegmentPoint().get(lastPointSegmentIndex).getGeoPoint());
        endWaypoints.add(lastPointOnEdge.getProjectedPoint().getGeoPoint());
    }

    private MapMatchingPathHistoryEntry getFirstProjectedPointOnEdge(NetworkEdge edge, List<MapMatchingPathHistoryEntry> edgeHistory){

        int firstPointSegmentIndex = Integer.MAX_VALUE;
        MapMatchingPathHistoryEntry firstEntry = null;

        for (MapMatchingPathHistoryEntry historyEntry : edgeHistory) {
            if (historyEntry.getSegmentIndex() <= firstPointSegmentIndex) {
                if(firstPointSegmentIndex == historyEntry.getSegmentIndex())
                {
                    double distanceFromFirstEntryToStartPoint = distance(firstEntry.getProjectedPoint(), edge.getSourcePos());
                    double distanceFromEntryToCompareToStartPoint = distance(historyEntry.getProjectedPoint(), edge.getSourcePos());
                    //übernehme wenn die distanz vom neuen zum startknoten kleiner ist
                    if(distanceFromEntryToCompareToStartPoint < distanceFromFirstEntryToStartPoint)
                    {
                        firstPointSegmentIndex = historyEntry.getSegmentIndex();
                        firstEntry = historyEntry;
                    }
                }
                else
                {
                    firstPointSegmentIndex = historyEntry.getSegmentIndex();
                    firstEntry = historyEntry;
                }
            }
        }
        return firstEntry;
    }

    public MapMatchingPathHistoryEntry getLastProjectedPointOnEdge(NetworkEdge edge, List<MapMatchingPathHistoryEntry> edgeHistory){

        int lastPointSegmentIndex = -1;
        MapMatchingPathHistoryEntry lastEntry = null;

        for (MapMatchingPathHistoryEntry historyEntry : edgeHistory) {
            if (historyEntry.getSegmentIndex() >= lastPointSegmentIndex)
            {
                if(lastPointSegmentIndex == historyEntry.getSegmentIndex())
                {
                    double distanceFromLastEntryToEndPoint = distance(lastEntry.getProjectedPoint(), edge.getTargetPos());
                    double distanceFromEntryToCompareToEndPoint = distance(historyEntry.getProjectedPoint(), edge.getTargetPos());
                    //übernehme wenn die distanz vom neuen zum endknoten kleiner ist
                    if(distanceFromEntryToCompareToEndPoint < distanceFromLastEntryToEndPoint)
                    {
                        lastPointSegmentIndex = historyEntry.getSegmentIndex();
                        lastEntry = historyEntry;
                    }
                }
                else
                {
                    lastPointSegmentIndex = historyEntry.getSegmentIndex();
                    lastEntry = historyEntry;
                }
            }
        }
        return lastEntry;
    }

    public void printPaths(List<MapMatchingPath> paths) {
        System.out.println("\nAll Paths:");
        System.out.println("-----------------------------------------------------------------------------");
        for (int i = 0; i < paths.size(); i++) {
            System.out.println(i + 1 + ". Path:");
            printPath(paths.get(i));
        }
    }

    public void printHistories(List<MapMatchingPath> paths) {
        System.out.println("\nAll Histories:");
        System.out.println("-----------------------------------------------------------------------------");
        for (int i = 0; i < paths.size(); i++) {
            System.out.println(i + 1 + ". History:");
            printHistory(paths.get(i));
        }
    }

    public void printPath(MapMatchingPath path) {
        System.out.println("-----------------------------------------------------------------------------");
        System.out.println("Score: " + path.getScore());
        int index = 0;
        for (NetworkEdge edge : path.getEdges()) {
            index = index + 1;
            System.out.println(index + ". Edge SourceID " + edge.getSourceID() + " TargetID " + edge.getTargetID());
            System.out.println(edge.getRoadName());
        }
        System.out.println("-----------------------------------------------------------------------------");
    }

    public void printHistory(MapMatchingPath path) {
        System.out.println("-----------------------------------------------------------------------------");
        System.out.println("Score: " + path.getScore());
        int index = 0;
        for (int i = 0; i < path.getHistoryPerEdge().size(); i++) {
            for (MapMatchingPathHistoryEntry history : path.getHistoryPerEdge().get(i)) {
                index = index + 1;
                System.out.println(index + ". Edge SourceID: " + path.getEdges().get(i).getSourceID() + " TargetID: " + path.getEdges().get(i).getTargetID());
                System.out.println("ProjectedPoint: Lat: " + history.getProjectedPoint().getLatitude() + " Lon: " + history.getProjectedPoint().getLongitude());
                System.out.println("Score: "+history.getScore());
                System.out.println("Segment Index: "+history.getSegmentIndex());
            }
        }
        System.out.println("-----------------------------------------------------------------------------");
    }

    public Point getLastProjectedPoint() {
        return lastProjectedPoint;
    }

    public void setDistanceMultiplicator(int distanceMultiplicator) {
        this.distanceMultiplicator = distanceMultiplicator;
    }

    public void setCountOfDisplayedEdges(int countOfDisplayedEdges) {
        this.countOfDisplayedEdges = countOfDisplayedEdges;
    }

    public void setCleanMapData(boolean cleanMapData) {
        this.cleanMapData = cleanMapData;
    }

    public List<List<GeoPoint>> getEndWayPoints() {
        return endWayPoints;
    }

    public List<List<GeoPoint>> getStartWayPoints() {
        return startWayPoints;
    }

    public void setExtensionCompareAngle(boolean extensionCompareAngle) {
        this.extensionCompareAngle = extensionCompareAngle;
    }

    public void setExtensionAngleTolerance(int extensionAngleTolerance) {
        this.extensionAngleTolerance = extensionAngleTolerance;
    }
}
