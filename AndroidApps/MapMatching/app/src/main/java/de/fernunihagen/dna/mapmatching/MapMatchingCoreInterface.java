package de.fernunihagen.dna.mapmatching;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.provider.Settings;

import org.osmdroid.util.GeoPoint;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.List;

import de.fernunihagen.dna.mapmatchingcore.*;

public class MapMatchingCoreInterface implements MapMatchingListener {

    private static MapMatching mapMatching = new MapMatching(new MapMatchingCoreInterface());
    private static MapMatchingActivity mapMatchingActivity;
    private static Point locationPoint;
    private static boolean mapMatchingActive = false;
    private static boolean mapMatchingCoreActive = false;
    private static boolean reconnect = false;
    private static boolean exitApp = false;
    private static int reconnectCounter = 0;
    private static boolean connectionFailed = false;

    public static void processLocationPoint(Point locationPoint){
        if(!isMapMatchingCoreActive())
        {
            mapMatchingActivity.showLineInfoMessage("Location Point is being processed...");
            setMapMatchingCoreActive(true);
            MapMatchingCoreInterface.locationPoint = locationPoint;
            ProcessLocationPoint processLocationPoint = new ProcessLocationPoint();
            processLocationPoint.execute();
        }
        else{
            mapMatchingActivity.showLineInfoMessage("Location Point refused. Another Task is still being processed.");
        }
    }

    public static void startMapMatching(){
        setMapMatchingCoreActive(true);
        mapMatchingActivity.showInfoMessage("Connecting to Secondo Server ...<br>");
        mapMatchingActivity.disableStartStopButton();
        new Thread(new Runnable() {
            public void run() {
                try {
                    if(mapMatching.getSecondoDB().initializeConnection())
                    {
                        if(mapMatching.getSecondoDB().isUseResultServer()){
                            if(!mapMatching.getSecondoResultDB().initializeConnection()){
                                throw new Exception();
                            }
                        }
                        connectionEstablished();
                    }
                    else{
                        connectionFailed();
                    }
                } catch (Exception e) {
                    connectionFailed();
                }
            }
        }).start();
    }

    private static void connectionEstablished(){
        setMapMatchingCoreActive(false);
        setReconnect(false);
        setReconnectCounter(0);
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.showLineInfoMessage("Connection established.");
                MapMatchingGps.startLocationUpdates();
                mapMatchingActivity.enableStartStopButton();
            }
        });
    }

    private static void connectionFailed(){
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.enableStartStopButton();
                setMapMatchingCoreActive(false);
                if(!isReconnect()){
                    Dialogs.showDialogConncetionFailed();
                    mapMatchingActivity.showLineErrorMessage("Connection failed.");
                    connectionFailed = true;
                    mapMatchingActivity.mapMatchingStop();
                }
                else{
                    mapMatchingActivity.showLineErrorMessage("Reconnect failed.");
                    reconnect();
                }
            }
        });
    }

    public static void stopMapMatching(){
        mapMatchingActivity.disableStartStopButton();
        if(!isReconnect()){
            MapMatchingGps.stopLocationUpdates();
        }
        setReconnect(false);
        setReconnectCounter(0);
        new Thread(new Runnable() {
            public void run() {
                //Wait until active task has finished
                int i = 0;
                while(isMapMatchingCoreActive()){
                    try {
                        i = i + 1;
                        if(isExitApp() && i > 5000){
                            mapMatchingActivity.finish();
                            System.exit(0);
                        }
                        Thread.sleep(4);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                setMapMatchingCoreActive(true);
                transmitResults();
                mapMatching.reset();
                connectionFailed = false;
                mapMatching.getSecondoDB().terminate();
                if(mapMatching.getSecondoDB().isUseResultServer()){
                    mapMatching.getSecondoResultDB().terminate();
                }
                setMapMatchingCoreActive(false);
                if(isExitApp()){
                    mapMatchingActivity.finish();
                    System.exit(0);
                }
                mapMatchingStoppedMessage();
            }
        }).start();
    }

    private static void mapMatchingStoppedMessage(){
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.enableMenu();
                mapMatchingActivity.enableStartStopButton();
                if(!Dialogs.isDialogConncetionFailedActive()){
                    mapMatchingActivity.showLineInfoMessage("Map Matching stopped.");
                }
            }
        });
    }

    private static void transmitResults(){
        try{
            boolean symbolicTrajectoriesNoError = true;
            if(!connectionFailed){
                if(!mapMatching.getSymbolicTrajectories().isDirectTransmission()||mapMatching.getSymbolicTrajectories().isLocalMode()){
                    if(!mapMatching.getSymbolicTrajectories().generateAllResults(true, mapMatching.getNotTransmittedResults())){
                        symbolicTrajectoriesNoError = false;
                        transmissionFailed();
                    }
                }

                if(mapMatching.getMovementResults().isTransmitMovementData() && !mapMatching.getSymbolicTrajectories().isLocalMode() &&symbolicTrajectoriesNoError){
                    mapMatching.getMovementResults().generate(mapMatching.getNotTransmittedMovementResults());
                    if(!mapMatching.getMovementResults().transmitResults()){
                        transmissionFailed();
                    }
                }
            }
        }
        catch(Exception e){
            ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
            {
                public void run()
                {
                    mapMatchingActivity.showLineErrorMessage("Error transmitting Results:<br>"+e.getMessage());
                    StringWriter errors = new StringWriter();
                    e.printStackTrace(new PrintWriter(errors));
                    mapMatchingActivity.showLineErrorMessage(errors.toString());
                }
            });
        }
    }

    private static void transmissionFailed(){
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.showErrorMessage("Transmission failed.<br>");
                mapMatchingActivity.showLineErrorMessage("To repeat transmission start and stop Map Matching again.");
            }
        });
    }

    public static void reconnect(){
        setReconnectCounter(getReconnectCounter()+1);
        setReconnect(true);
        mapMatchingActivity.showLineInfoMessage(getReconnectCounter()+". Connection-Retry: Waiting for 15 seconds ...");
        new Thread(new Runnable() {
            public void run() {
                try {
                    Thread.sleep(15000);
                    if(isReconnect()){
                        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
                        {
                            public void run()
                            {
                                startMapMatching();
                            }
                        });
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    public static void clearData(){
        mapMatching.clearData();
        MapMatchingMap.clearData();
        mapMatchingActivity.clearConsoleData();
        MapMatchingOverview.clearData();
    }

    public static void setSettings() {
        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(mapMatchingActivity);

        //Server
        String savedServerIp = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverIp_key), mapMatchingActivity.getString(R.string.preference_serverIp_default));
        int savedServerPort = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverPort_key), mapMatchingActivity.getString(R.string.preference_serverPort_default)));
        String savedServerUsername = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverUsername_key), mapMatchingActivity.getString(R.string.preference_serverUsername_default));
        String savedServerPassword = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverPassword_key), mapMatchingActivity.getString(R.string.preference_serverPassword_default));
        String savedServerDatabaseName = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverDatabaseName_key), mapMatchingActivity.getString(R.string.preference_serverDatabaseName_default));
        String savedServerDatabaseRTreeName = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverDatabaseRTreeName_key), mapMatchingActivity.getString(R.string.preference_serverDatabaseRTreeName_default));
        String savedServerDatabaseEdgeIndexName = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverDatabaseEdgeIndexName_key), mapMatchingActivity.getString(R.string.preference_serverDatabaseEdgeIndexName_default));
        long savedServerTimeout = Long.parseLong(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverTimeout_key), mapMatchingActivity.getString(R.string.preference_serverTimeout_default)));
        savedServerTimeout = savedServerTimeout*1000;
        boolean savedEdgesRelation = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_serverEdgesRelation_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_serverEdgesRelation_default)));
        boolean savedResultServer = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_serverResultServer_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_serverResultServer_default)));
        String savedResultServerIp = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverResultIp_key), mapMatchingActivity.getString(R.string.preference_serverResultIp_default));
        int savedResultServerPort = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverResultPort_key), mapMatchingActivity.getString(R.string.preference_serverPort_default)));
        String savedResultServerUsername = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverResultUsername_key), mapMatchingActivity.getString(R.string.preference_serverUsername_default));
        String savedResultServerPassword = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverResultPassword_key), mapMatchingActivity.getString(R.string.preference_serverPassword_default));
        String savedResultServerDatabaseName = sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_serverResultDatabaseName_key), mapMatchingActivity.getString(R.string.preference_serverDatabaseName_default));

        //General
        boolean savedMapMatchingShowMap = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_mapMatchingShowMap_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_mapMatchingShowMap_default)));
        boolean savedMapMatchingShowGpsPoints = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_mapMatchingShowGpsPoints_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_mapMatchingShowGpsPoints_default)));
        boolean savedMapMatchingMatchFootways = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_mapMatchingMatchFootways_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_mapMatchingMatchFootways_default)));
        boolean savedMapMatchingNoOneWay = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_mapMatchingNoOneWay_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_mapMatchingNoOneWay_default)));
        boolean savedResultDirectTransmission = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_resultDirectTransmission_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_resultDirectTransmission_default)));
        boolean savedResultStreetname = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_resultStreetname_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_resultStreetname_default)));
        boolean savedResultCardinalDirection = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_resultCardinalDirection_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_resultCardinalDirection_default)));
        boolean savedResultSpeed = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_resultSpeed_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_resultSpeed_default)));
        boolean savedResultHeight = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_resultHeight_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_resultHeight_default)));
        boolean savedResultLocalMode = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_resultLocalMode_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_resultLocalMode_default)));
        boolean savedResultMovementData = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_resultMovementData_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_resultMovementData_default)));
        boolean savedResultTimeUtc = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_resultTime_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_resultTime_default)));
        //GPS
        boolean savedAdvancedFirstLocation = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_gpsFirstLocation_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_gpsFirstLocation_default)));
        long savedAdvancedUpdateRate = Long.parseLong(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_gpsUpdateRate_key), mapMatchingActivity.getString(R.string.preference_gpsUpdateRate_default)));
        float savedAdvancedUpdateDistance = Float.parseFloat(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_gpsUpdateDistance_key), mapMatchingActivity.getString(R.string.preference_gpsUpdateDistance_default)));
        int savedAdvancedAccuracy = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_gpsAccuracy_key), mapMatchingActivity.getString(R.string.preference_gpsAccuracy_default)));
        int savedAdvancedSatUsed = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_gpsSatUsed_key), mapMatchingActivity.getString(R.string.preference_gpsSatUsed_default)));
        int savedAdvancedMaxPdop = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_gpsMaxPdop_key), mapMatchingActivity.getString(R.string.preference_gpsMaxPdop_default)));
        int savedAdvancedMaxHdop = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_gpsMaxHdop_key), mapMatchingActivity.getString(R.string.preference_gpsMaxHdop_default)));
        int savedAdvancedMaxVdop = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_gpsMaxVdop_key), mapMatchingActivity.getString(R.string.preference_gpsMaxVdop_default)));
        //Advanced
        boolean savedResultStreetnameCalc = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedStreetnameCalc_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedStreetnameCalc_default)));
        boolean savedResultPreciseMPoint = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedPreciseMPoint_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedPreciseMPoint_default)));
        boolean savedAdvancedUTurn = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedUTurn_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedUTurn_default)));
        int savedAdvancedUTurnMalus = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedUTurnMalus_key), mapMatchingActivity.getString(R.string.preference_advancedUTurnMalus_default)));
        boolean savedAdvancedMultipleUTurn = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedMultipleUTurn_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedMultipleUTurn_default)));
        boolean savedAdvancedCleanPaths = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedCleanPaths_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedCleanPaths_default)));
        int savedAdvancedCleanPathsCount = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedCleanPathsCount_key), mapMatchingActivity.getString(R.string.preference_advancedCleanPathsCount_default)));
        int savedAdvancedCleanPathsEdges = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedCleanPathsEdges_key), mapMatchingActivity.getString(R.string.preference_advancedCleanPathsEdges_default)));
        boolean savedAdvancedCleanMap = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedCleanMap_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedCleanMap_default)));
        int savedAdvancedCleanMapCount = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedCleanMapCount_key), mapMatchingActivity.getString(R.string.preference_advancedCleanMapCount_default)));
        int savedAdvancedBoundingBoxSize = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedBoundingBoxSize_key), mapMatchingActivity.getString(R.string.preference_advancedBoundingBoxSize_default)));
        double savedAdvancedBoundingBoxRadius = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedBoundingBoxRadius_key), mapMatchingActivity.getString(R.string.preference_advancedBoundingBoxRadius_default)));
        int savedAdvancedMaxPathsSize = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedMaxPathsSize_key), mapMatchingActivity.getString(R.string.preference_advancedMaxPathsSize_default)));
        int savedAdvancedExtensionDepth = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedExtensionDepth_key), mapMatchingActivity.getString(R.string.preference_advancedExtensionDepth_default)));
        int savedAdvancedResetDistance = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedResetDistance_key), mapMatchingActivity.getString(R.string.preference_advancedResetDistance_default)));
        boolean savedAdvancedPreferSmallerPaths = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedPreferSmallerPaths_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedPreferSmallerPaths_default)));
        int savedAdvancedPreferSmallerPathsMalus = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedPreferSmallerPathsMalus_key), mapMatchingActivity.getString(R.string.preference_advancedPreferSmallerPathsMalus_default)));
        boolean savedAdvancedFootwaysDifferent = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedFootwaysDifferent_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedFootwaysDifferent_default)));
        boolean savedAdvancedFootwaysPerformance = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedFootwaysPerformance_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedFootwaysPerformance_default)));
        int savedFootwaySpeed = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedFootwaySpeed_key), mapMatchingActivity.getString(R.string.preference_advancedFootwaySpeed_default)));
        int savedWayOrRoadMalus = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedWayOrRoadMalus_key), mapMatchingActivity.getString(R.string.preference_advancedWayOrRoadMalus_default)));
        int savedDistanceMulti = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedScoreDistanceMulti_key), mapMatchingActivity.getString(R.string.preference_advancedScoreDistanceMulti_default)));
        boolean savedAlwaysExtend = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedAlwaysExtend_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedAlwaysExtend_default)));
        boolean savedCompareAngle = sharedPrefs.getBoolean(mapMatchingActivity.getString(R.string.preference_advancedCompareAngle_key), Boolean.valueOf(mapMatchingActivity.getString(R.string.preference_advancedCompareAngle_default)));
        int savedCompareAngleTolerance = Integer.parseInt(sharedPrefs.getString(mapMatchingActivity.getString(R.string.preference_advancedCompareAngleTolerance_key), mapMatchingActivity.getString(R.string.preference_advancedCompareAngleTolerance_default)));

        String androidId = Settings.Secure.getString(mapMatchingActivity.getContentResolver(),Settings.Secure.ANDROID_ID);

        setServerSettings(savedServerIp,savedServerPort,savedServerUsername,savedServerPassword,savedServerDatabaseName,savedServerDatabaseRTreeName,savedServerDatabaseEdgeIndexName,savedEdgesRelation,savedServerTimeout,androidId,savedResultServer,savedResultServerIp,savedResultServerPort,savedResultServerUsername,savedResultServerPassword,savedResultServerDatabaseName);
        setGeneralSettings(savedMapMatchingMatchFootways,savedMapMatchingNoOneWay,savedResultDirectTransmission,savedResultStreetname,savedResultCardinalDirection,savedResultSpeed,savedResultHeight,savedResultLocalMode,savedResultMovementData,savedResultTimeUtc);
        setGpsSettings(savedAdvancedFirstLocation, savedAdvancedUpdateRate, savedAdvancedUpdateDistance, savedAdvancedAccuracy, savedAdvancedSatUsed, savedAdvancedMaxPdop, savedAdvancedMaxHdop, savedAdvancedMaxVdop);
        setAdvancedSettings(savedResultStreetnameCalc,savedResultPreciseMPoint,savedAdvancedUTurn, savedAdvancedMultipleUTurn, savedAdvancedBoundingBoxSize, savedAdvancedBoundingBoxRadius, savedAdvancedMaxPathsSize, savedAdvancedExtensionDepth, savedAdvancedResetDistance, savedAdvancedPreferSmallerPaths,savedAdvancedPreferSmallerPathsMalus, savedAdvancedUTurnMalus, savedAdvancedCleanPaths, savedAdvancedCleanPathsCount, savedAdvancedCleanPathsEdges, savedAdvancedFootwaysDifferent, savedFootwaySpeed, savedWayOrRoadMalus, savedDistanceMulti,savedAlwaysExtend, savedAdvancedFootwaysPerformance, savedCompareAngle, savedCompareAngleTolerance);
        setMapSettings(savedMapMatchingShowMap,savedMapMatchingShowGpsPoints,savedAdvancedCleanMap,savedAdvancedCleanMapCount);
    }

    private static void setMapSettings(boolean showMap, boolean showGpsPoints, boolean cleanMap, int cleanMapCount){
        MapMatchingMap.setShowMap(showMap);
        MapMatchingMap.setShowGpsPoints(showGpsPoints);
        mapMatching.getUtil().setCleanMapData(cleanMap);
        mapMatching.getUtil().setCountOfDisplayedEdges(cleanMapCount);
    }

    private static void setServerSettings(String ip, int port, String user, String password, String databaseName, String rTreeName, String edgeIndexName,boolean edgesRelation, long timeout, String androidId, boolean resultServer, String resultIp, int resultPort, String resultUser, String resultPassword, String resultDatabaseName){
        mapMatching.getSecondoDB().setServerIp(ip);
        mapMatching.getSecondoDB().setServerPort(port);
        mapMatching.getSecondoDB().setServerUsername(user);
        mapMatching.getSecondoDB().setServerPassword(password);
        mapMatching.getSecondoDB().setDatabaseName(databaseName);
        mapMatching.getSecondoDB().setrTreeName(rTreeName);
        mapMatching.getSecondoDB().setEdgeIndexName(edgeIndexName);
        mapMatching.getSecondoDB().setTimeout(timeout);
        mapMatching.getSecondoDB().setAndroid_id(androidId);
        mapMatching.getSecondoDB().setUseEdgesRelation(edgesRelation);
        mapMatching.getSecondoDB().setUseResultServer(resultServer);

        mapMatching.getSecondoResultDB().setServerIp(resultIp);
        mapMatching.getSecondoResultDB().setServerPort(resultPort);
        mapMatching.getSecondoResultDB().setServerUsername(resultUser);
        mapMatching.getSecondoResultDB().setServerPassword(resultPassword);
        mapMatching.getSecondoResultDB().setDatabaseName(resultDatabaseName);
        mapMatching.getSecondoResultDB().setrTreeName(rTreeName);
        mapMatching.getSecondoResultDB().setEdgeIndexName(edgeIndexName);
        mapMatching.getSecondoResultDB().setTimeout(timeout);
        mapMatching.getSecondoResultDB().setAndroid_id(androidId);
        mapMatching.getSecondoResultDB().setUseEdgesRelation(edgesRelation);
        mapMatching.getSecondoResultDB().setUseResultServer(resultServer);
    }

    private static void setGeneralSettings(boolean matchFootways, boolean noOneWayRoads, boolean directTransmission, boolean streetname, boolean cardinalDirection, boolean speed, boolean height, boolean localMode, boolean movementData, boolean resultTimeUtc){
        mapMatching.setPreferenceMatchFootwaysChange(matchFootways);
        mapMatching.getSecondoDB().setMatchFootways(matchFootways);
        mapMatching.getSecondoResultDB().setMatchFootways(matchFootways);
        mapMatching.getNetwork().setNoOneWayRoads(noOneWayRoads);
        mapMatching.getSymbolicTrajectories().setLocalMode(localMode);
        mapMatching.getSymbolicTrajectories().setDirectTransmission(directTransmission);
        mapMatching.getSymbolicTrajectories().setGenerateStreetnames(streetname);
        mapMatching.getSymbolicTrajectories().setGenerateCardinalDirections(cardinalDirection);
        mapMatching.getSymbolicTrajectories().setGenerateSpeedProfile(speed);
        mapMatching.getSymbolicTrajectories().setGenerateHeightProfile(height);
        mapMatching.getMovementResults().setTransmitMovementData(movementData);
        SymbolicTrajectory.setUseUtcTime(resultTimeUtc);
    }

    private static void setGpsSettings(boolean firstLocation, long updateRate, float updateDistance, int accuracy, int satUsed, int maxPdop, int maxHdop, int maxVdop){
        MapMatchingGps.setSkipFirstLocationPointProperty(firstLocation);
        MapMatchingGps.setMinimalTimeBetweenUpdates(updateRate);
        MapMatchingGps.setMinimalDistanceChangeBetweenUpdates(updateDistance);
        MapMatchingGps.setMaximalAllowedAccuracy(accuracy);
        MapMatchingGps.setMinimalAllowedUsedSat(satUsed);
        MapMatchingGps.setMaximalAllowedPdop(maxPdop);
        MapMatchingGps.setMaximalAllowedHdop(maxHdop);
        MapMatchingGps.setMaximalAllowedVdop(maxVdop);
    }

    private static void setAdvancedSettings(boolean streetnameCalc, boolean preciseMPoint, boolean allowUTurns, boolean multipleUTurns, int boundingBoxSize, double boundingBoxRadius, int maxPathsSize, int extensionDepth, int resetDistance, boolean preferSmallerPaths,int preferSmallerPathsMalus, int uTurnMalus, boolean cleanPaths, int cleanPathsCount, int cleanPathsEdges, boolean footwaysDifferent, int footwaySpeed, int wayOrRoadMalus, int distanceMulti, boolean alwaysExtend, boolean footwayPerformance, boolean compareAngle, int compareAngleTolerance){
        SymbolicTrajectory.setCalcStreetnameTime(streetnameCalc);
        mapMatching.setAllowUTurns(allowUTurns);
        mapMatching.setAllowMultipleUTurns(multipleUTurns);
        mapMatching.getNetwork().setBoundingBoxSize(boundingBoxSize);
        mapMatching.getNetwork().setBoundingBoxUpdateRadius(boundingBoxRadius/100);
        mapMatching.setMaxPathsSize(maxPathsSize);
        mapMatching.setMaxPathsSizePrefValue(maxPathsSize);
        mapMatching.setExtensionDepth(extensionDepth);
        mapMatching.setResetDistance(resetDistance);
        mapMatching.setPreferSmallerPaths(preferSmallerPaths);
        mapMatching.setPreferSmallerPathsMalus(preferSmallerPathsMalus);
        mapMatching.setuTurnMalus(uTurnMalus);
        mapMatching.setCleanPaths(cleanPaths);
        mapMatching.setMaxLocationCountUntilClean(cleanPathsCount);
        mapMatching.setEdgesLeftInCleanPaths(cleanPathsEdges);
        mapMatching.setTreatFootwaysDifferent(footwaysDifferent);
        mapMatching.setImproveFootwayPerformance(footwayPerformance);
        mapMatching.setFootwaySpeed(footwaySpeed);
        mapMatching.setRoadMalus(wayOrRoadMalus);
        mapMatching.getUtil().setDistanceMultiplicator(distanceMulti);
        mapMatching.setAlwaysExtent(alwaysExtend);
        mapMatching.getMovementResults().setPreciseMovingPoint(preciseMPoint);
        mapMatching.getUtil().setExtensionCompareAngle(compareAngle);
        mapMatching.getUtil().setExtensionAngleTolerance(compareAngleTolerance);
    }

    public static void changeMatchFootwaysPref(boolean matchFootways){
        mapMatching.setPreferenceMatchFootwaysChange(matchFootways);
    }

    public static class ProcessLocationPoint extends AsyncTask<Void, Void, Boolean>
    {
        @Override
        protected Boolean doInBackground(Void... params) {
                return mapMatching.processLocationPoint(locationPoint);
        }

        @Override
        protected void onPostExecute(Boolean result) {
            if(!result){
                MapMatchingGps.stopLocationUpdates();
                reconnect();
            }
            else{
                MapMatchingMap.updateMap(mapMatching.getUtil().cleanResults(mapMatching.getAllResultsConnected()));
                MapMatchingOverview.update();
            }
            setMapMatchingCoreActive(false);
        }
    }

    @Override
    public void showLineResultMessage(String message) {
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.showLineResultMessage(message);
            }
        });
    }

    @Override
    public void showResultMessage(String message) {
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.showResultMessage(message);
            }
        });
    }

    @Override
    public void showLineInfoMessage(String message) {
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.showLineInfoMessage(message);
            }
        });
    }

    @Override
    public void showInfoMessage(String message) {
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.showInfoMessage(message);
            }
        });
    }

    @Override
    public void showLineErrorMessage(String message) {
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.showLineErrorMessage(message);
            }
        });
    }

    @Override
    public void showErrorMessage(String message) {
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                mapMatchingActivity.showErrorMessage(message);
            }
        });
    }

    @Override
    public void reset() {
        ((Activity) mapMatchingActivity).runOnUiThread(new Runnable()
        {
            public void run()
            {
                MapMatchingOverview.cancel(mapMatching.getSymbolicTrajectories().getCancelTime());
            }
        });
    }

    public static List<List<GeoPoint>> getStartWayPoints(){
        return mapMatching.getUtil().getStartWayPoints();
    }

    public static List<List<GeoPoint>> getEndWayPoints(){
        return mapMatching.getUtil().getEndWayPoints();
    }

    public static List<SymbolicTrajectory> getResultsStreetnames(){
        return mapMatching.getSymbolicTrajectories().getResultsStreetnames();
    }
    public static List<SymbolicTrajectory> getResultsCardinalDirections(){
        return mapMatching.getSymbolicTrajectories().getResultsCardinalDirections();
    }
    public static List<SymbolicTrajectory> getResultsSpeedIntervals(){
        return mapMatching.getSymbolicTrajectories().getResultsSpeedIntervals();
    }
    public static List<SymbolicTrajectory> getResultsHeightIntervals(){
        return mapMatching.getSymbolicTrajectories().getResultsHeightIntervals();
    }

    public static void setMapMatchingActivity(MapMatchingActivity mapMatchingActivity) {
        MapMatchingCoreInterface.mapMatchingActivity = mapMatchingActivity;
    }

    public static boolean isMapMatchingActive() {
        return mapMatchingActive;
    }

    public static void setMapMatchingActive(boolean mapMatchingActive) {
        MapMatchingCoreInterface.mapMatchingActive = mapMatchingActive;
    }

    public static synchronized boolean isMapMatchingCoreActive() {
        return mapMatchingCoreActive;
    }

    public static synchronized void setMapMatchingCoreActive(boolean mapMatchingCoreActive) {
        MapMatchingCoreInterface.mapMatchingCoreActive = mapMatchingCoreActive;
    }

    public static synchronized boolean isReconnect() {
        return reconnect;
    }

    public static synchronized void setReconnect(boolean reconnect) {
        MapMatchingCoreInterface.reconnect = reconnect;
    }

    public static synchronized boolean isExitApp() {
        return exitApp;
    }

    public static synchronized void setExitApp(boolean exitApp) {
        MapMatchingCoreInterface.exitApp = exitApp;
    }

    public static synchronized int getReconnectCounter() {
        return reconnectCounter;
    }

    public static synchronized void setReconnectCounter(int reconnectCounter) {
        MapMatchingCoreInterface.reconnectCounter = reconnectCounter;
    }

}