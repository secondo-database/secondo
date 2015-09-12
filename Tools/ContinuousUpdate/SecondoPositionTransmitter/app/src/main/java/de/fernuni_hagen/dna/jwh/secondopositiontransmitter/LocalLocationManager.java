package de.fernuni_hagen.dna.jwh.secondopositiontransmitter;

import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.util.Log;

/**
 * Created by Jerome on 15.02.2015.
 */
public class LocalLocationManager {

    private static final int minDeltaTime = 1000 * 60 * 2; // 2 minutes
    private static final int significantDistance = 5; //meters

    private Context ctx;
    private Integer updateRate;
    private Location bestLocation;
    private LocationInfo currentLocationInfo;
    private LocationManager locationManager;
    private LocationListener locationListener;


    public LocalLocationManager(Context ctx, Integer updateRate) {
        this.ctx = ctx;
        this.updateRate = updateRate;
        this.currentLocationInfo = new LocationInfo();
        getLocationManager();
    }

    /**
     * Registers the Instance for Location Updates
     */
    private void getLocationManager() {
        locationManager = (LocationManager) ctx.getSystemService(Context.LOCATION_SERVICE);

        /* Listener for location Updates
         */
        locationListener = new LocationListener() {
            public void onLocationChanged(Location location) {
                handleNewLocation(location);
            }

            @Override
            public void onStatusChanged(String provider, int status, Bundle extras) {

            }

            @Override
            public void onProviderEnabled(String provider) {

            }

            @Override
            public void onProviderDisabled(String provider) {

            }
        };

        /* Register the listener for network and GPS providers */
        locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, updateRate, 0, locationListener);
        locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, updateRate, 0, locationListener);
    }

    /**
     * Handler for new Locations, called by the LocationManager
     * @param location
     */
    private synchronized void handleNewLocation(Location location) {
        Log.i(this.getClass().getSimpleName(), "Location Update Received");
        if (isBetterLocation(location, bestLocation)) {
            Log.d(getClass().getSimpleName(), "Location is better");
            bestLocation = location;
        } else {
            Log.d(getClass().getSimpleName(), "Location is not better");
        }
        if (currentLocationInfo.start == null) {
            currentLocationInfo.start = bestLocation;
            currentLocationInfo.end = bestLocation;
        } else {
            currentLocationInfo.end = bestLocation;
        }
    }

    /**
     * Dertermine if the received location is better than the currently available
     * @param location
     * @param currentBestLocation
     * @return
     */
    protected boolean isBetterLocation(Location location, Location currentBestLocation) {
        if (currentBestLocation == null) {
            /* First location, is always better than none at all */
            return true;
        }

        /* Check if the new location is really newer than the old one */
        long delta = location.getTime() - currentBestLocation.getTime();

        if (delta > minDeltaTime) {
            return true;
        } else if (delta < 0) {
            /* An older location must be worse, at least worse for secondo */
            return false;
        }

        /* Check accuracy */
        int accuracyDelta = (int) (location.getAccuracy() - currentBestLocation.getAccuracy());
        boolean isLessAccurate = accuracyDelta > 0;
        boolean isMoreAccurate = accuracyDelta < 0;
        boolean isSignificantlyLessAccurate = accuracyDelta > 100;

        /* determine provider */
        boolean isFromSameProvider = isSameProvider(location.getProvider(),
                currentBestLocation.getProvider());

        if (isMoreAccurate) {
            return true;
        } else if (!isLessAccurate) {
            return true;
        } else if (!isSignificantlyLessAccurate && isFromSameProvider) {
            return true;
        }

        return false;
    }

    /**
     * Checks if the two providers are the same, used by isBetterLocation
     */
    private boolean isSameProvider(String provider1, String provider2) {
        if (provider1 == null || provider2 == null) {
            return false;
        }
        return provider1.equals(provider2);
    }


    /**
     *
     * @return current Location Info (Start/End)
     */
    public synchronized LocationInfo getCurrentLocationInfo() {
        LocationInfo info = new LocationInfo(currentLocationInfo);
        currentLocationInfo = new LocationInfo();
        currentLocationInfo.start = info.end;
        return info;
    }

    /**
     * Determine if the movement was enough
     * @return
     */
    public boolean significantMovement() {
        if (movementAvailable() && currentLocationInfo.start.distanceTo(currentLocationInfo.end) >= significantDistance) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Used to tell if a new Position is available
     * @return
     */
    public boolean movementAvailable() {
        return (currentLocationInfo != null && currentLocationInfo.start != null && currentLocationInfo.end != null && currentLocationInfo.start.getTime() < currentLocationInfo.end.getTime());
    }

    public void close(){
        locationManager.removeUpdates(locationListener);
    }


    /**
     * Local Class to represent the start/end Location of a movement
     */
    public class LocationInfo {
        public Location start;
        public Location end;

        LocationInfo(LocationInfo newInfo) {
            super();
            if (newInfo != null) {
                if (newInfo.start != null) {
                    this.start = new Location(newInfo.start);
                }
                if (newInfo.end != null) {
                    this.end = new Location(newInfo.end);
                }
            }
        }

        LocationInfo() {
            super();
        }
    }
}