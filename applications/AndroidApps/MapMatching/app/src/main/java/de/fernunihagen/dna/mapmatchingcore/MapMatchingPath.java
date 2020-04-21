package de.fernunihagen.dna.mapmatchingcore;

import java.util.ArrayList;
import java.util.List;

public class MapMatchingPath implements Comparable<MapMatchingPath> {

    private List<NetworkEdge> edges;
    private List<List<MapMatchingPathHistoryEntry>> historyPerEdge;

    private double score = 0;
    private Point recentProjectedPoint;
    private int recentProjectedPointSegmentIndex;

    private int extensionCounter = 0;

    private boolean uTurnExtension = false;
    private boolean cleanedResultPath = false;
    private boolean resultsTransmitted = false;
    private boolean movementResultsTransmitted = false;

    public MapMatchingPath() {
        edges = new ArrayList<NetworkEdge>();
        historyPerEdge = new ArrayList<List<MapMatchingPathHistoryEntry>>();
    }

    public MapMatchingPath(NetworkEdge edge) {
        edges = new ArrayList<NetworkEdge>();
        historyPerEdge = new ArrayList<List<MapMatchingPathHistoryEntry>>();
        historyPerEdge.add(new ArrayList<MapMatchingPathHistoryEntry>());
        edges.add(edge);
    }

    public MapMatchingPath(MapMatchingPath clone) {
        edges = new ArrayList<NetworkEdge>(clone.getEdges());
        historyPerEdge = new ArrayList<List<MapMatchingPathHistoryEntry>>(clone.getHistoryPerEdge());

        for (int i = 0; i < historyPerEdge.size(); i++) {
            List<MapMatchingPathHistoryEntry> historyList = new ArrayList<MapMatchingPathHistoryEntry>(clone.getHistoryPerEdge().get(i));
            historyPerEdge.set(i, historyList);
        }

        score = clone.getScore();
        extensionCounter = clone.getExtensionCounter();
    }

    @Override
    public int compareTo(MapMatchingPath path) {
        double thisScore = this.score;
        double compareScore = path.getScore();

        if (thisScore < compareScore) {
            return -1;
        } else if (thisScore > compareScore) {
            return 1;
        }
        return 0;
    }

    public int getRecentProjectedPointSegmentIndex() {
        return recentProjectedPointSegmentIndex;
    }

    public void setRecentProjectedPointSegmentIndex(int recentProjectedPointSegmentIndex) {
        this.recentProjectedPointSegmentIndex = recentProjectedPointSegmentIndex;
    }

    public int getExtensionCounter() {
        return extensionCounter;
    }

    public void setExtensionCounter(int extensionCounter) {
        this.extensionCounter = extensionCounter;
    }

    public List<List<MapMatchingPathHistoryEntry>> getHistoryPerEdge() {
        return historyPerEdge;
    }

    public void setHistoryPerEdge(List<List<MapMatchingPathHistoryEntry>> historyPerEdge) {
        this.historyPerEdge = historyPerEdge;
    }

    public double getScore() {
        return score;
    }

    public void setScore(double score) {
        this.score = score;
    }

    public Point getRecentProjectedPoint() {
        return recentProjectedPoint;
    }

    public void setRecentProjectedPoint(Point recentProjectedPoint) {
        this.recentProjectedPoint = recentProjectedPoint;
    }

    public List<NetworkEdge> getEdges() {
        return edges;
    }

    public void setEdges(List<NetworkEdge> edges) {
        this.edges = edges;
    }

    public boolean isCleanedResultPath() {
        return cleanedResultPath;
    }

    public void setCleanedResultPath(boolean cleanedResultPath) {
        this.cleanedResultPath = cleanedResultPath;
    }

    public boolean isResultsTransmitted() {
        return resultsTransmitted;
    }

    public void setResultsTransmitted(boolean resultsTransmitted) {
        this.resultsTransmitted = resultsTransmitted;
    }

    public boolean isUTurnExtension() {
        return uTurnExtension;
    }

    public void setUTurnExtension(boolean uTurnExtension) {
        this.uTurnExtension = uTurnExtension;
    }

    public boolean isMovementResultsTransmitted() {
        return movementResultsTransmitted;
    }

    public void setMovementResultsTransmitted(boolean movementResultsTransmitted) {
        this.movementResultsTransmitted = movementResultsTransmitted;
    }
}
