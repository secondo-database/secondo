package de.fernunihagen.dna.mapmatchingcore;

import org.osmdroid.util.GeoPoint;

public class Point {
    private double latitude, longitude, bearing, speed, accuracy, pdop, hdop, vdop;
    private int satellitesVisible, satellitesUsed;
    private double altitude = Double.MAX_VALUE;
    private long time;

    public Point(double latitude, double longitude) {
        this.latitude = latitude;
        this.longitude = longitude;
    }

    //only for tests
    public Point(double latitude, double longitude, double bearing) {
        this.latitude = latitude;
        this.longitude = longitude;
        this.bearing = bearing;
    }

    //only for tests
    public Point(double latitude, double longitude, double bearing, double altitude, double speed, long time) {
        this.latitude = latitude;
        this.longitude = longitude;
        this.bearing = bearing;
        this.altitude = altitude;
        this.speed = speed;
        this.time = time;
    }

    public boolean equals(Point p) {
        if (this.latitude == p.getLatitude() && this.longitude == p.getLongitude()) {
            return true;
        } else {
            return false;
        }
    }

    public double getLongitude() {
        return longitude;
    }

    public double getLatitude() {
        return latitude;
    }

    public GeoPoint getGeoPoint(){
        return new GeoPoint(latitude,longitude);
    }

    public double getAltitude() {
        return altitude;
    }

    public void setAltitude(double altitude) {
        this.altitude = altitude;
    }

    public double getBearing() {
        return bearing;
    }

    public void setBearing(double bearing) {
        this.bearing = bearing;
    }

    public double getSpeed() {
        return speed;
    }

    public void setSpeed(double speed) {
        this.speed = speed;
    }

    public long getTime() {
        return time;
    }

    public void setTime(long time) {
        this.time = time;
    }

    public double getAccuracy() {
        return accuracy;
    }

    public void setAccuracy(double accuracy) {
        this.accuracy = accuracy;
    }

    public double getPdop() {
        return pdop;
    }

    public void setPdop(double pdop) {
        this.pdop = pdop;
    }

    public double getHdop() {
        return hdop;
    }

    public void setHdop(double hdop) {
        this.hdop = hdop;
    }

    public double getVdop() {
        return vdop;
    }

    public void setVdop(double vdop) {
        this.vdop = vdop;
    }

    public int getSatellitesVisible() {
        return satellitesVisible;
    }

    public void setSatellitesVisible(int satellitesVisible) {
        this.satellitesVisible = satellitesVisible;
    }

    public int getSatellitesUsed() {
        return satellitesUsed;
    }

    public void setSatellitesUsed(int satellitesUsed) {
        this.satellitesUsed = satellitesUsed;
    }
}
