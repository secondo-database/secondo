package de.fernunihagen.dna.mapmatching;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;

import java.util.Iterator;

import de.fernunihagen.dna.mapmatchingcore.Point;

public class MapMatchingGps implements LocationListener, GpsStatus.Listener, GpsStatus.NmeaListener {

    private static MapMatchingActivity mapMatchingActivity;
    private static MapMatchingGps mapMatchingGps = new MapMatchingGps();
    private static LocationManager locationManager;

    private static boolean locationUpdatesActive = false;
    private static boolean initialize = true;
    private static boolean skipLocationPoint = true;

    private static boolean skipFirstLocationPointProperty;

    private static long minimalTimeBetweenUpdates;
    private static float minimalDistanceChangeBetweenUpdates;
    private static int maximalAllowedAccuracy, minimalAllowedUsedSat, maximalAllowedPdop, maximalAllowedHdop, maximalAllowedVdop;

    private String latestPdop="",latestHdop="",latestVdop="";
    private int satellitesVisible, satellitesUsed;

    @Override
    public void onLocationChanged(Location location) {
        if(initialize){
            MapMatchingMap.initialize(location);
            initialize = false;
        }
        mapMatchingActivity.showInfoMessage("Location Point received:<br>");
        if(skipFirstLocationPointProperty){
            if(skipLocationPoint){
                skipLocationPoint = false;
                mapMatchingActivity.showInfoMessage("GPS (re)initialized. First Location Point is skipped due to accuracy.<br>");
                mapMatchingActivity.showLineInfoMessage("Move to receive next Location Point.");
                return;
            }
        }
        if(isLocationPointUseful(location)){
            Point locationPoint = new Point(location.getLatitude(), location.getLongitude());
            locationPoint.setPdop(Double.parseDouble(latestPdop));
            locationPoint.setHdop(Double.parseDouble(latestHdop));
            locationPoint.setVdop(Double.parseDouble(latestVdop));
            locationPoint.setSatellitesUsed(satellitesUsed);
            locationPoint.setSatellitesVisible(satellitesVisible);
            locationPoint.setBearing(location.getBearing());
            locationPoint.setTime(location.getTime());
            locationPoint.setAccuracy(location.getAccuracy());

            if(location.hasAltitude()){
                locationPoint.setAltitude(location.getAltitude());
            }
            if(location.hasSpeed()){
                locationPoint.setSpeed(location.getSpeed()*3.6);
            }

            MapMatchingCoreInterface.processLocationPoint(locationPoint);
        }
        else{
            mapMatchingActivity.showLineInfoMessage("Location Point refused due to GPS Filter Settings.");
        }
    }

    private boolean isLocationPointUseful(Location location) {
        if(satellitesUsed < minimalAllowedUsedSat){
            mapMatchingActivity.showInfoMessage("Only "+satellitesUsed+" satellites used.<br>");
            return false;
        }
        if(latestPdop.equals("")){
            mapMatchingActivity.showInfoMessage("No PDOP measured.<br>");
            return false;
        }
        if(Double.parseDouble(latestPdop) > maximalAllowedPdop){
            mapMatchingActivity.showInfoMessage("PDOP: "+latestPdop+" too bad.<br>");
            return false;
        }
        if(latestHdop.equals("")){
            mapMatchingActivity.showInfoMessage("No HDOP measured.<br>");
            return false;
        }
        if(Double.parseDouble(latestHdop) > maximalAllowedHdop){
            mapMatchingActivity.showInfoMessage("HDOP: "+latestHdop+" too bad.<br>");
            return false;
        }
        if(latestVdop.equals("")){
            mapMatchingActivity.showInfoMessage("No VDOP measured.<br>");
            return false;
        }
        if(Double.parseDouble(latestVdop) > maximalAllowedVdop){
            mapMatchingActivity.showInfoMessage("VDOP: "+latestVdop+" too bad.<br>");
            return false;
        }
        if (!location.hasBearing()) {
            mapMatchingActivity.showInfoMessage("Received Location Point has no Direction value.<br>");
            return false;
        }
        if(!location.hasSpeed()){
            mapMatchingActivity.showInfoMessage("Received Location Point has no Speed value.<br>");
            return false;
        }
        if (!location.hasAccuracy()) {
            mapMatchingActivity.showInfoMessage("Received Location Point has no Accuracy value.<br>");
            return false;
        } else {
            if (location.getAccuracy() > maximalAllowedAccuracy) {
                double accuracyRounded = Math.round(location.getAccuracy()*10)/10.0;
                mapMatchingActivity.showInfoMessage("Accuracy: "+String.valueOf(accuracyRounded)+" too bad.<br>");
                return false;
            }
        }
        return true;
    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
//        if (status == LocationProvider.OUT_OF_SERVICE) {}
//        if (status == LocationProvider.AVAILABLE) {}
//        if (status == LocationProvider.TEMPORARILY_UNAVAILABLE) {}
    }

    @Override
    public void onProviderEnabled(String provider) {
    }

    @Override
    public void onProviderDisabled(String provider) {
        mapMatchingActivity.mapMatchingStop();
        mapMatchingActivity.showLineErrorMessage("GPS was disabled.");
    }

    @Override
    public void onGpsStatusChanged(int event) {
        switch (event) {
            case GpsStatus.GPS_EVENT_FIRST_FIX:
                //mapMatchingActivity.showLineInfoMessage("First Fix");
                break;
            case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
                //System.out.println("GPS_EVENT_SATELLITE_STATUS");
                if (ActivityCompat.checkSelfPermission(mapMatchingActivity, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                    return;
                }
                GpsStatus status = locationManager.getGpsStatus(null);
                int maxSatellites = status.getMaxSatellites();

                Iterator<GpsSatellite> it = status.getSatellites().iterator();
                satellitesVisible = 0;
                satellitesUsed=0;

                while (it.hasNext() && satellitesVisible <= maxSatellites) {
                    GpsSatellite sat = it.next();
                    if(sat.usedInFix()){
                        satellitesUsed++;
                    }
                    satellitesVisible++;
                }
                break;
            case GpsStatus.GPS_EVENT_STARTED:
                //mapMatchingActivity.showLineInfoMessage("GPS Event Started");
                break;
            case GpsStatus.GPS_EVENT_STOPPED:
                //mapMatchingActivity.showLineInfoMessage("GPS Event Stopped");
                break;
        }
    }

    @Override
    public void onNmeaReceived(long timestamp, String nmeaSentence) {
        if(isStringNullOrEmpty(nmeaSentence)){
            return;
        }
        String[] nmeaParts = nmeaSentence.split(",");

        if (nmeaParts[0].equalsIgnoreCase("$GPGSA")) {
            //System.out.println("onNmeaReceived");
            if (nmeaParts.length > 15 && !isStringNullOrEmpty(nmeaParts[15])) {
                latestPdop = nmeaParts[15];
            }
            else{
                latestPdop="";
            }
            if (nmeaParts.length > 16 &&!isStringNullOrEmpty(nmeaParts[16])) {
                latestHdop = nmeaParts[16];
            }
            else{
                latestHdop = "";
            }
            if (nmeaParts.length > 17 &&!isStringNullOrEmpty(nmeaParts[17]) && !nmeaParts[17].startsWith("*")) {
                latestVdop = nmeaParts[17].split("\\*")[0];
            }
            else{
                latestVdop = "";
            }
        }
    }

    private boolean isStringNullOrEmpty(String string){
        if(string == null){
            return true;
        }
        if(string.equals("")){
            return true;
        }
        return false;
    }

    public static boolean checkGpsEnabledAndPermissions() {
        if (ActivityCompat.checkSelfPermission(mapMatchingActivity, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED) {
            if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                return true;
            } else {
                Dialogs.showDialogGpsDisabled();
            }
        } else {
            ActivityCompat.requestPermissions(mapMatchingActivity, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 1);
        }
        return false;
    }

    public static void startLocationUpdates() {
        if (ActivityCompat.checkSelfPermission(mapMatchingActivity, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED) {
            locationUpdatesActive = true;
            if (initialize) {
                Location lastKnownLocation = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                if (lastKnownLocation != null) {
                    initialize = false;
                    MapMatchingMap.initialize(lastKnownLocation);
                }
            }
            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, minimalTimeBetweenUpdates, minimalDistanceChangeBetweenUpdates, mapMatchingGps);
            locationManager.addNmeaListener(mapMatchingGps);
            locationManager.addGpsStatusListener(mapMatchingGps);
            skipLocationPoint = true;
        } else {
            ActivityCompat.requestPermissions(mapMatchingActivity, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 1);
        }
    }

    public static void stopLocationUpdates() {
        if (ActivityCompat.checkSelfPermission(mapMatchingActivity, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED) {
            locationManager.removeUpdates(mapMatchingGps);
            locationManager.removeNmeaListener(mapMatchingGps);
            locationManager.removeGpsStatusListener(mapMatchingGps);
            locationUpdatesActive = false;
        } else {
            ActivityCompat.requestPermissions(mapMatchingActivity, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 1);
        }
    }

    public static void updateLocationManager(MapMatchingActivity mapMatchingActivity) {
        MapMatchingGps.mapMatchingActivity = mapMatchingActivity;
        if (locationUpdatesActive) {
            stopLocationUpdates();
            locationManager = (LocationManager) mapMatchingActivity.getSystemService(Context.LOCATION_SERVICE);
            startLocationUpdates();
        } else {
            locationManager = (LocationManager) mapMatchingActivity.getSystemService(Context.LOCATION_SERVICE);
        }
    }

    public static void setMinimalDistanceChangeBetweenUpdates(float minimalDistanceChangeBetweenUpdates) {
        MapMatchingGps.minimalDistanceChangeBetweenUpdates = minimalDistanceChangeBetweenUpdates;
    }

    public static void setMinimalTimeBetweenUpdates(long minimalTimeBetweenUpdates) {
        MapMatchingGps.minimalTimeBetweenUpdates = minimalTimeBetweenUpdates;
    }

    public static void setMaximalAllowedAccuracy(int maximalAllowedAccuracy) {
        MapMatchingGps.maximalAllowedAccuracy = maximalAllowedAccuracy;
    }

    public static void setMinimalAllowedUsedSat(int minimalAllowedUsedSat) {
        MapMatchingGps.minimalAllowedUsedSat = minimalAllowedUsedSat;
    }

    public static void setMaximalAllowedPdop(int maximalAllowedPdop) {
        MapMatchingGps.maximalAllowedPdop = maximalAllowedPdop;
    }

    public static void setMaximalAllowedHdop(int maximalAllowedHdop) {
        MapMatchingGps.maximalAllowedHdop = maximalAllowedHdop;
    }

    public static void setMaximalAllowedVdop(int maximalAllowedVdop) {
        MapMatchingGps.maximalAllowedVdop = maximalAllowedVdop;
    }

    public static void setSkipFirstLocationPointProperty(boolean skipFirstLocationPointProperty) {
        MapMatchingGps.skipFirstLocationPointProperty = skipFirstLocationPointProperty;
    }
}
