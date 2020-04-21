package de.fernunihagen.dna.mapmatchingcore;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class NetworkEdge {

    private int sourceID, targetID;
    private Point sourcePos, targetPos;
    private List<Point> startSegmentPoints = new ArrayList<Point>();
    private List<Point> endSegmentPoints = new ArrayList<Point>();
    private String roadName, roadType;
    private NetworkNode sourceNode, targetNode;

    public NetworkEdge(int sourceID, int targetID, Point sourcePos, Point targetPos, List<Point> startSegmentPoint, List<Point> endSegmentPoint, String roadName, String roadType) {
        this.sourceID = sourceID;
        this.targetID = targetID;
        this.sourcePos = sourcePos;
        this.targetPos = targetPos;
        this.startSegmentPoints = startSegmentPoint;
        this.endSegmentPoints = endSegmentPoint;
        this.roadName = roadName;
        this.roadType = roadType;
    }

    public NetworkEdge(NetworkEdge edge, boolean reverseCopy) {
        if(reverseCopy){
            sourceID = edge.getTargetID();
            targetID = edge.getSourceID();
            sourcePos = edge.getTargetPos();
            targetPos = edge.getSourcePos();
            startSegmentPoints = new ArrayList<Point>(edge.getEndSegmentPoint());
            endSegmentPoints = new ArrayList<Point>(edge.getStartSegmentPoint());
            roadName = edge.getRoadName();
            roadType = edge.getRoadType();
            Collections.reverse(startSegmentPoints);
            Collections.reverse(endSegmentPoints);
        }
        else{
            sourceID = edge.getSourceID();
            targetID = edge.getTargetID();
            sourcePos = edge.getSourcePos();
            targetPos = edge.getTargetPos();
            startSegmentPoints = new ArrayList<Point>(edge.getStartSegmentPoint());
            endSegmentPoints = new ArrayList<Point>(edge.getEndSegmentPoint());
            roadName = edge.getRoadName();
            roadType = edge.getRoadType();
        }
    }

    public boolean equals(NetworkEdge edge) {
        if (this.sourceID == edge.getSourceID() && this.targetID == edge.getTargetID()) {
            return true;
        } else {
            return false;
        }
    }

    public List<Point> getStartSegmentPoint() {
        return startSegmentPoints;
    }

    public List<Point> getEndSegmentPoint() {
        return endSegmentPoints;
    }

    public Point getSourcePos() {
        return sourcePos;
    }

    public Point getTargetPos() {
        return targetPos;
    }

    public int getSourceID() {
        return sourceID;
    }

    public int getTargetID() {
        return targetID;
    }

    public String getRoadName() {
        return roadName;
    }

    public String getRoadType() {
        return roadType;
    }

    public NetworkNode getTargetNode() {
        return targetNode;
    }

    public NetworkNode getSourceNode() {
        return sourceNode;
    }

    public void setTargetNode(NetworkNode targetNode) {
        this.targetNode = targetNode;
    }

    public void setSourceNode(NetworkNode sourceNode) {
        this.sourceNode = sourceNode;
    }

}
