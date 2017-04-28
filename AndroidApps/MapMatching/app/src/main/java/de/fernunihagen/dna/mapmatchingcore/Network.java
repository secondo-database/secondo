package de.fernunihagen.dna.mapmatchingcore;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import sj.lang.ListExpr;

public class Network {

    private List<NetworkEdge> networkEdges = new ArrayList<NetworkEdge>();
    private List<NetworkNode> networkNodes = new ArrayList<NetworkNode>();

    private List<NetworkEdge> edgesToAdd = new ArrayList<NetworkEdge>();

    private boolean noOneWayRoads;
    private int boundingBoxSize;
    private double boundingBoxUpdateRadius;
    private Point boundingBoxSouthWest;
    private Point boundingBoxNorthEast;
    private Point boundingBoxMiddle;

    public Network(){
    }

    public Network(boolean noOneWayRoads, int boundingBoxSize, double boundingBoxUpdateRadius){
        this.noOneWayRoads = noOneWayRoads;
        this.boundingBoxSize = boundingBoxSize;
        this.boundingBoxUpdateRadius = boundingBoxUpdateRadius;
    }

    public boolean readNetworkData(SecondoDB secondoDB, List<MapMatchingPath> paths) {

        if(!secondoDB.queryEdges(getBoundingBoxString())){
            return false;
        }
        ListExpr allEdges = secondoDB.getResultList().rest().first();

        networkEdges.clear();
        networkNodes.clear();

        while (!allEdges.isEmpty()) {
            int sourceID = allEdges.first().first().intValue();
            int targetID = allEdges.first().rest().first().intValue();
            if (!addEdgeFromPath(sourceID, targetID, paths, false)) {
                readEdge(allEdges.first());
            }
            allEdges = allEdges.rest();
        }
        createNodes();

        if (noOneWayRoads) {
            extendWaysWithOneDirection(paths);
            networkNodes.clear();
            createNodes();
        }
        return true;
    }

    private boolean addEdgeFromPath(int sourceID, int targetID, List<MapMatchingPath> paths, boolean extendWaysWithOneDirection) {
        for (MapMatchingPath path : paths) {
            int indexOfLastEdge = path.getEdges().size() - 1;
            NetworkEdge edge = path.getEdges().get(indexOfLastEdge);
            if (edge.getSourceID() == sourceID && edge.getTargetID() == targetID) {
                if(extendWaysWithOneDirection){
                    edgesToAdd.add(edge);
                }
                else{
                    networkEdges.add(edge);
                }
                return true;
            }
        }
        return false;
    }

    private void extendWaysWithOneDirection(List<MapMatchingPath> paths) {
        edgesToAdd.clear();
        for (NetworkEdge edge : networkEdges) {
            if (edge.getTargetNode() != null) {
                boolean fromTargetToSource = false;
                for (NetworkEdge edgeOutgoingFromTargetNode : edge.getTargetNode().getOutgoingEdges()) {
                    if (edgeOutgoingFromTargetNode.getTargetID() == edge.getSourceID()) {
                        fromTargetToSource = true;
                        break;
                    }
                }
                if (!fromTargetToSource) {
                    if (!addEdgeFromPath(edge.getTargetID(), edge.getSourceID(), paths, true)) {
                        NetworkEdge networkEdge = new NetworkEdge(edge, true);
                        edgesToAdd.add(networkEdge);
                    }
                }
            } else {
                if (!addEdgeFromPath(edge.getTargetID(), edge.getSourceID(), paths,true)) {
                    NetworkEdge networkEdge = new NetworkEdge(edge, true);
                    edgesToAdd.add(networkEdge);
                }
            }
        }
        networkEdges.addAll(edgesToAdd);
    }

    private void createNodes() {
        for (NetworkEdge edge : networkEdges) {
            Boolean nodeExists = false;
            for (NetworkNode node : networkNodes) {
                if (node.getNodeID() == edge.getSourceID()) {
                    nodeExists = true;
                    node.getOutgoingEdges().add(edge);
                    edge.setSourceNode(node);
                }
            }
            if (!nodeExists) {
                NetworkNode networkNode = new NetworkNode();
                networkNode.setNodeID(edge.getSourceID());
                networkNode.getOutgoingEdges().add(edge);
                networkNodes.add(networkNode);
                edge.setSourceNode(networkNode);
            }
        }
        //Link TargetNodes
        for (NetworkEdge edge : networkEdges) {
            for (NetworkNode node : networkNodes) {
                if (node.getNodeID() == edge.getTargetID()) {
                    edge.setTargetNode(node);
                }
            }
        }
    }

    private String getBoundingBoxString() {
        String minLon, maxLon, minLat, maxLat, boundingBoxString;
        minLon = Double.toString(boundingBoxSouthWest.getLongitude());
        minLat = Double.toString(boundingBoxSouthWest.getLatitude());
        maxLon = Double.toString(boundingBoxNorthEast.getLongitude());
        maxLat = Double.toString(boundingBoxNorthEast.getLatitude());
        boundingBoxString = minLon + " " + maxLon + " " + minLat + " " + maxLat;
        //System.out.println(boundingBoxString);
        return boundingBoxString;
    }

    private void readEdge(ListExpr element) {
        double longitude;
        double latitude;
        Point p;
        ListExpr curveValues;

        int sourceID = element.first().intValue();
        element = element.rest();

        int targetID = element.first().intValue();
        element = element.rest();

        longitude = element.first().first().realValue();
        latitude = element.first().rest().first().realValue();
        Point sourcePos = new Point(latitude, longitude);
        element = element.rest();

        longitude = element.first().first().realValue();
        latitude = element.first().rest().first().realValue();
        Point targetPos = new Point(latitude, longitude);
        element = element.rest();

        List<Point> startSegmentPoints = new ArrayList<Point>();
        List<Point> endSegmentPoints = new ArrayList<Point>();
        curveValues = element.first().first();
        while (!curveValues.isEmpty()) {
            longitude = curveValues.first().first().realValue();
            latitude = curveValues.first().rest().first().realValue();
            p = new Point(latitude, longitude);
            startSegmentPoints.add(p);

            longitude = curveValues.first().rest().rest().first().realValue();
            latitude = curveValues.first().rest().rest().rest().first().realValue();
            p = new Point(latitude, longitude);
            endSegmentPoints.add(p);

            curveValues = curveValues.rest();
        }
        boolean directionForward = element.first().rest().first().boolValue();
        element = element.rest();

        String roadName = correctString(element.first().stringValue());
        element = element.rest();

        String roadType = element.first().stringValue();

        if(!directionForward){
            Collections.reverse(startSegmentPoints);
            Collections.reverse(endSegmentPoints);
            List<Point> temp;
            temp = startSegmentPoints;
            startSegmentPoints = endSegmentPoints;
            endSegmentPoints = temp;
        }

        correctSimpleLineOrder(startSegmentPoints,endSegmentPoints);


        NetworkEdge networkEdge = new NetworkEdge(sourceID, targetID, sourcePos, targetPos, startSegmentPoints, endSegmentPoints, roadName, roadType);
        networkEdges.add(networkEdge);
    }

    private void correctSimpleLineOrder(List<Point> startSegmentPoints, List<Point> endSegmentPoints){
        if(startSegmentPoints.size()>1){
            //Endpoint A = Endpoint B
            if(endSegmentPoints.get(0).equals(endSegmentPoints.get(1))){
                //Change B
                Point oldStartPoint, oldEndPoint;
                oldStartPoint = startSegmentPoints.get(1);
                oldEndPoint = endSegmentPoints.get(1);
                startSegmentPoints.set(1, oldEndPoint);
                endSegmentPoints.set(1, oldStartPoint);
            }
            //Startpoint A = Startpoint B
            else if(startSegmentPoints.get(0).equals(startSegmentPoints.get(1))){
                //Change A
                Point oldStartPoint, oldEndPoint;
                oldStartPoint = startSegmentPoints.get(0);
                oldEndPoint = endSegmentPoints.get(0);
                startSegmentPoints.set(0, oldEndPoint);
                endSegmentPoints.set(0, oldStartPoint);
            }
            //Startpoint A = Endpoint B
            else if(startSegmentPoints.get(0).equals(endSegmentPoints.get(1))){
                //Change A and B
                Point oldStartPoint, oldEndPoint;
                oldStartPoint = startSegmentPoints.get(0);
                oldEndPoint = endSegmentPoints.get(0);
                startSegmentPoints.set(0, oldEndPoint);
                endSegmentPoints.set(0, oldStartPoint);

                oldStartPoint = startSegmentPoints.get(1);
                oldEndPoint = endSegmentPoints.get(1);
                startSegmentPoints.set(1, oldEndPoint);
                endSegmentPoints.set(1, oldStartPoint);
            }

            for(int i=1; i<startSegmentPoints.size();i++){
                if(i+1==startSegmentPoints.size()){
                    break;
                }
                //Endpoint A = Endpoint B
                if(endSegmentPoints.get(i).equals(endSegmentPoints.get(i+1))){
                    //Change B
                    Point oldStartPoint, oldEndPoint;
                    oldStartPoint = startSegmentPoints.get(i+1);
                    oldEndPoint = endSegmentPoints.get(i+1);
                    startSegmentPoints.set(i+1, oldEndPoint);
                    endSegmentPoints.set(i+1, oldStartPoint);
                }
            }
        }
    }

    private String correctString(String str) {
        str = str.replace("ÃŸ", "ß");
        str = str.replace("Ã¤", "ä");
        str = str.replace("Ã¶", "ö");
        str = str.replace("Ã¼", "ü");
        str = str.replace("Ã„", "Ä");
        str = str.replace("Ã–", "Ö");
        str = str.replace("Ãœ", "Ü");
        return str;
    }

    public void printEdges() {
        System.out.println("\nAll Network Edges:");
        System.out.println("-----------------------------------------------------------------------------");
        for (NetworkEdge elem : networkEdges) {
            System.out.println("SourceID " + elem.getSourceID());
            System.out.println("TargetID " + elem.getTargetID());
            System.out.println("SourcePos " + elem.getSourcePos().getLatitude() + " " + elem.getSourcePos().getLongitude());
            System.out.println("TargetPos " + elem.getTargetPos().getLatitude() + " " + elem.getTargetPos().getLongitude());
            System.out.println("Curve Segments:");

            for (int i = 0; i < elem.getStartSegmentPoint().size(); i++) {
                System.out.println("From: " + "Lat: " + elem.getStartSegmentPoint().get(i).getLatitude()
                        + " Lon: " + elem.getStartSegmentPoint().get(i).getLongitude());
                System.out.println("To:   " + "Lat: " + elem.getEndSegmentPoint().get(i).getLatitude()
                        + " Lon: " + elem.getEndSegmentPoint().get(i).getLongitude());
            }

            System.out.println("RoadName " + elem.getRoadName());
            System.out.println("RoadType " + elem.getRoadType());

            System.out.println("-----------------------------------------------------------------------------");
        }
    }

    public void printNodes() {
        System.out.println("\nAll Network Nodes:");
        System.out.println("-----------------------------------------------------------------------------");
        for (NetworkNode node : networkNodes) {
            System.out.println("NodeID " + node.getNodeID());
            for (int i = 0; i < node.getOutgoingEdges().size(); i++) {
                System.out.println("TargetNode: " + node.getOutgoingEdges().get(i).getTargetID());
            }
            System.out.println("-----------------------------------------------------------------------------");
        }
    }

    public List<NetworkEdge> getNetworkEdges() {
        return networkEdges;
    }

    public void setBoundingBoxSouthWest(Point boundingBoxSouthWest) {
        this.boundingBoxSouthWest = boundingBoxSouthWest;
    }

    public void setBoundingBoxNorthEast(Point boundingBoxNorthEast) {
        this.boundingBoxNorthEast = boundingBoxNorthEast;
    }

    public Point getBoundingBoxMiddle() {
        return boundingBoxMiddle;
    }

    public void setBoundingBoxMiddle(Point boundingBoxMiddle) {
        this.boundingBoxMiddle = boundingBoxMiddle;
    }

    public int getBoundingBoxSize() {
        return boundingBoxSize;
    }

    public void setBoundingBoxSize(int boundingBoxSize) {
        this.boundingBoxSize = boundingBoxSize;
    }

    public void setNoOneWayRoads(boolean noOneWayRoads) {
        this.noOneWayRoads = noOneWayRoads;
    }

    public boolean isNoOneWayRoads() {
        return noOneWayRoads;
    }

    public double getBoundingBoxUpdateRadius() {
        return boundingBoxUpdateRadius;
    }

    public void setBoundingBoxUpdateRadius(double boundingBoxUpdateRadius) {
        this.boundingBoxUpdateRadius = boundingBoxUpdateRadius;
    }
}
