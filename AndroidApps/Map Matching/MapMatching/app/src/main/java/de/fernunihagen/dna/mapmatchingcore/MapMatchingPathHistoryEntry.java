package de.fernunihagen.dna.mapmatchingcore;

public class MapMatchingPathHistoryEntry {

    private Point locationPoint;
    private Point projectedPoint;
    private int segmentIndex;
    private double score;
    private double distanceBetweenLocationAndProjectedPoint;
    private double absoluteAngleDifference;

    public MapMatchingPathHistoryEntry(Point locationPoint, Point projectedPoint, int segmentIndex, double score, double distanceBetweenLocationAndProjectedPoint, double absoluteAngleDifference) {
        this.projectedPoint = projectedPoint;
        this.segmentIndex = segmentIndex;
        this.locationPoint = locationPoint;
        this.score = score;
        this.distanceBetweenLocationAndProjectedPoint = distanceBetweenLocationAndProjectedPoint;
        this.absoluteAngleDifference = absoluteAngleDifference;
    }

    public Point getLocationPoint() {
        return locationPoint;
    }

    public int getSegmentIndex() {
        return segmentIndex;
    }

    public Point getProjectedPoint() {
        return projectedPoint;
    }

    public double getScore() {
        return score;
    }

    public double getDistanceBetweenLocationAndProjectedPoint() {
        return distanceBetweenLocationAndProjectedPoint;
    }

    public double getAbsoluteAngleDifference() {
        return absoluteAngleDifference;
    }
}
