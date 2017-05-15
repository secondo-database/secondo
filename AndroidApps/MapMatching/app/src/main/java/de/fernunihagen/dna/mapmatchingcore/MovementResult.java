package de.fernunihagen.dna.mapmatchingcore;

import java.util.ArrayList;
import java.util.List;

public class MovementResult {

    private int sourceID, targetID;
    private long sourceIDTime, targetIDTime;
    private List<Point> startSegmentPoints = new ArrayList<Point>();
    private List<Point> endSegmentPoints = new ArrayList<Point>();
    private List<Long> startSegmentTime = new ArrayList<Long>();
    private List<Long> endSegmentTime = new ArrayList<Long>();
    private List<MapMatchingPathHistoryEntry> edgeHistory;

    private boolean uTurnResult = false;

    public MovementResult(int sourceID, int targetID, List<Point> startSegmentPoints, List<Point> endSegmentPoints, List<MapMatchingPathHistoryEntry> edgeHistory){
        this.sourceID = sourceID;
        this.targetID = targetID;
        this.startSegmentPoints.addAll(startSegmentPoints);
        this.endSegmentPoints.addAll(endSegmentPoints);
        this.edgeHistory = edgeHistory;
    }

    public int getSourceID() {
        return sourceID;
    }

    public int getTargetID() {
        return targetID;
    }

    public long getSourceIDTime() {
        return sourceIDTime;
    }

    public void setSourceIDTime(long sourceIDTime) {
        this.sourceIDTime = sourceIDTime;
    }

    public long getTargetIDTime() {
        return targetIDTime;
    }

    public void setTargetIDTime(long targetIDTime) {
        this.targetIDTime = targetIDTime;
    }

    public List<Point> getStartSegmentPoints() {
        return startSegmentPoints;
    }

    public List<Point> getEndSegmentPoints() {
        return endSegmentPoints;
    }

    public List<MapMatchingPathHistoryEntry> getEdgeHistory() {
        return edgeHistory;
    }

    public List<Long> getStartSegmentTime() {
        return startSegmentTime;
    }

    public List<Long> getEndSegmentTime() {
        return endSegmentTime;
    }

    public void setEndSegmentPoints(List<Point> endSegmentPoints) {
        this.endSegmentPoints = endSegmentPoints;
    }

    public void setStartSegmentPoints(List<Point> startSegmentPoints) {
        this.startSegmentPoints = startSegmentPoints;
    }

    public boolean isuTurnResult() {
        return uTurnResult;
    }

    public void setuTurnResult(boolean uTurnResult) {
        this.uTurnResult = uTurnResult;
    }
}
