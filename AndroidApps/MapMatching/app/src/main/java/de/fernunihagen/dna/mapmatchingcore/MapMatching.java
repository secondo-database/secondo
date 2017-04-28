package de.fernunihagen.dna.mapmatchingcore;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class MapMatching {

    private Network network = new Network();
    private SecondoDB secondoDB = new SecondoDB(this, false);
    private SecondoDB secondoResultDB = new SecondoDB(this, true);
    private SymbolicTrajectories symbolicTrajectories = new SymbolicTrajectories(this);
    private MapMatchingUtilities util = new MapMatchingUtilities(this);
    private MovementResults movementResults = new MovementResults(this);
    private MapMatchingListener mapMatchingListener;

    private List<MapMatchingPath> paths = new ArrayList<MapMatchingPath>();
    private List<MapMatchingPath> oldResults = new ArrayList<MapMatchingPath>();
    private List<MapMatchingPath> pathsToExtend = new ArrayList<MapMatchingPath>();
    private List<MapMatchingPath> uselessPathsToRemove = new ArrayList<MapMatchingPath>();
    private boolean initialize = true;

    private boolean preferenceMatchFootwaysChange;
    private boolean forceNetworkUpdate = false;

    private boolean improveFootwayPerformance;

    private int footwaySpeed;
    private int roadMalus;
    private boolean treatFootwaysDifferent;
    private boolean temporaryPreferRoads = false;

    private double averageSpeed;

    private boolean preferSmallerPaths;
    private int preferSmallerPathsMalus;
    private boolean allowUTurns;
    private boolean allowMultipleUTurns;
    private boolean alwaysExtent;
    private int maxPathsSize;
    private int maxPathsSizePrefValue;
    private int extensionDepth;
    private int uTurnMalus;
    private boolean cleanPaths;
    private int maxLocationCountUntilClean;
    private int resetDistance;
    private int edgesLeftInCleanPaths;

    private int locationCounterSinceLastClean = 0;
    private int extensionPercentalSpan = 80;

    private long timeStartProcessLocationPoint;
    private long timeEndProcessLocationPoint;
    private long timeStartNetworkUpdate;
    private long timeEndNetworkUpdate;
    private long timeStartTransmitResults;
    private long timeEndTransmitResults;
    private long timeForProcessingLocationPoint;

    //private long completeTimeProcessingLocationPoints = 0;

    private MapMatchingPath recentBestPath;

    private String dbErrorMessage = "Connection to Secondo Server has been lost.";
    private String resetMessageNoWay = "No Way Found. Map Matching reinitialized.";
    private String resetMessageDistance = "Nearest Projected Point too far away. Map Matching reinitialized.";
    private String cleanMessage = "Hypotheses were cleaned.";


    public MapMatching(MapMatchingListener mapMatchingListener){
        this.mapMatchingListener = mapMatchingListener;
    }

    public boolean processLocationPoint(Point locationPoint) {
        try{
            timeStartProcessLocationPoint = System.currentTimeMillis();

            checkPreferenceMatchFootways();
            calculateAverageSpeed(locationPoint);

            if (initialize) {
                if(!initialize(locationPoint)){
                    return false;
                }
                findProjections(locationPoint);
            } else {
                if(!updateNetwork(locationPoint)){
                    return false;
                }
                findProjections(locationPoint);
                pathsToExtend.clear();
                uselessPathsToRemove.clear();
                for (MapMatchingPath path : paths) {
                    findExtensions(locationPoint, path);
                }
                paths.addAll(pathsToExtend);
                paths.removeAll(uselessPathsToRemove);
            }

            setPathsScore(locationPoint);
            sortAndTrimPaths();
            updatePathHistory(locationPoint);

            if(!checkReset(locationPoint)){
                if(!updateTrajectories(locationPoint)){
                    return false;
                }
            }
            cleanPathsData();
            if(paths.size()!=0){
                util.generateWaypoints(util.cleanResults(getAllResultsConnected()));
                showMessages(locationPoint);
                recentBestPath = paths.get(0);
            }
        }catch (Exception e){
            mapMatchingListener.showLineErrorMessage("Error processing Location Point:<br>"+e.getMessage());
            StringWriter errors = new StringWriter();
            e.printStackTrace(new PrintWriter(errors));
            mapMatchingListener.showLineErrorMessage(errors.toString());
            reset();
        }
        return true;
    }

    private void checkPreferenceMatchFootways(){
        temporaryPreferRoads = false;
        if(secondoDB.isMatchFootways()!= isPreferenceMatchFootwaysChange()){
            if(isPreferenceMatchFootwaysChange()==false && paths.size()!=0){
                int indexOfLastEdge = paths.get(0).getEdges().size()-1;
                if(util.isRoad(paths.get(0).getEdges().get(indexOfLastEdge).getRoadType())){
                    forceNetworkUpdate = true;
                    secondoDB.setMatchFootways(isPreferenceMatchFootwaysChange());
                    secondoResultDB.setMatchFootways(isPreferenceMatchFootwaysChange());
                    mapMatchingListener.showLineInfoMessage("Matching Footways was disabled.");
                    //Delete Footway Hypothesis
                    List<MapMatchingPath> pathsToRemove = new ArrayList<MapMatchingPath>();
                    for (MapMatchingPath path : paths) {
                        indexOfLastEdge = path.getEdges().size()-1;
                        if(!util.isRoad(path.getEdges().get(indexOfLastEdge).getRoadType())){
                            pathsToRemove.add(path);
                        }
                    }
                    paths.removeAll(pathsToRemove);
                }
                else{
                    mapMatchingListener.showInfoMessage("Matching Footways will be disabled when a road is entered.<br>");
                    mapMatchingListener.showLineInfoMessage("Roads will be temporary preferred.");
                    temporaryPreferRoads = true;
                }
            }
            else {
                forceNetworkUpdate = true;
                secondoDB.setMatchFootways(isPreferenceMatchFootwaysChange());
                secondoResultDB.setMatchFootways(isPreferenceMatchFootwaysChange());
                if(isPreferenceMatchFootwaysChange()){
                    mapMatchingListener.showLineInfoMessage("Matching Footways was enabled.");
                    if(improveFootwayPerformance && paths.size()>5){
                        paths = paths.subList(0, 5);
                    }
                }
                else{
                    mapMatchingListener.showLineInfoMessage("Matching Footways was disabled.");
                }
            }
        }
        if(improveFootwayPerformance){
            if(secondoDB.isMatchFootways()){
                if(maxPathsSizePrefValue>5){
                    if(maxPathsSize!=5){
                        mapMatchingListener.showLineInfoMessage("Hypotheses Count was set to 5.");
                    }
                    maxPathsSize = 5;
                }
            }
            else{
                if(maxPathsSize!=maxPathsSizePrefValue){
                    mapMatchingListener.showLineInfoMessage("Hypotheses Count was set to "+maxPathsSizePrefValue+".");
                }
                maxPathsSize = maxPathsSizePrefValue;
            }
        }
    }

    private void calculateAverageSpeed(Point locationPoint){
        if(treatFootwaysDifferent&&secondoDB.isMatchFootways()){
            util.averageSpeedOfLastThreePoints(locationPoint);
        }
    }

    private boolean updateTrajectories(Point locationPoint){
        List<MapMatchingPath> results = new ArrayList<MapMatchingPath>();
        MapMatchingPath resultPath = getCurrentResult();
        int resultPathSize = resultPath.getEdges().size();
        if(resultPathSize>=500){
            resultPath.setEdges(resultPath.getEdges().subList(resultPathSize-500,resultPathSize));
            resultPath.setHistoryPerEdge(resultPath.getHistoryPerEdge().subList(resultPathSize-500,resultPathSize));
        }
        results.add(resultPath);
        symbolicTrajectories.generateAllResults(false, results);
        timeStartTransmitResults = System.currentTimeMillis();
        if(paths.size()!=0){
            if(!symbolicTrajectories.update(locationPoint, getCurrentResult())){
                reset();
                mapMatchingListener.showLineErrorMessage(dbErrorMessage);
                return false;
            }
        }
        timeEndTransmitResults = System.currentTimeMillis();
        return true;
    }

    public void reset(){
        initialize = true;
        locationCounterSinceLastClean = 0;
        recentBestPath = null;
        if(paths.size()!=0){
            resetNodesInPathsEdges();
            if(symbolicTrajectories.isDirectTransmission()){
                paths.get(0).setResultsTransmitted(true);
            }
            if(!movementResults.isTransmitMovementData()){
                paths.get(0).setMovementResultsTransmitted(true);
            }
            oldResults.add(paths.get(0));
        }
        symbolicTrajectories.reset();
        mapMatchingListener.reset();
        network = new Network(network.isNoOneWayRoads(), network.getBoundingBoxSize(), network.getBoundingBoxUpdateRadius());
        paths = new ArrayList<MapMatchingPath>();
    }

    private void sortAndTrimPaths(){
        Collections.sort(paths);
        if (paths.size() > maxPathsSize) {
            paths = paths.subList(0, maxPathsSize);
        }
    }

    private void setPathsScore(Point locationPoint){
        for (MapMatchingPath path : paths) {
            path.setScore(path.getScore()+util.calculateScore(locationPoint, path));
        }
    }

    private boolean updateNetwork(Point locationPoint) {
        if (initialize || util.distance(locationPoint, network.getBoundingBoxMiddle()) > (network.getBoundingBoxSize() / 2) * network.getBoundingBoxUpdateRadius() || forceNetworkUpdate) {
            forceNetworkUpdate = false;
            resetNodesInPathsEdges();
            network.setBoundingBoxSouthWest(util.projectPoint(locationPoint, 225, network.getBoundingBoxSize() / 2));
            network.setBoundingBoxNorthEast(util.projectPoint(locationPoint, 45, network.getBoundingBoxSize() / 2));
            network.setBoundingBoxMiddle(locationPoint);
            timeStartNetworkUpdate = System.currentTimeMillis();
            if(!network.readNetworkData(secondoDB, paths)){
                mapMatchingListener.showLineErrorMessage(dbErrorMessage);
                reset();
                return false;
            }
            timeEndNetworkUpdate = System.currentTimeMillis();
        }
        return true;
    }

    private boolean initialize(Point firstLocationPoint) {
        if(!updateNetwork(firstLocationPoint)){
            return false;
        }
        for (NetworkEdge edge : network.getNetworkEdges()) {
            paths.add(new MapMatchingPath(edge));
        }
        initialize = false;
        return true;
    }

    private void updatePathHistory(Point locationPoint) {
        for (MapMatchingPath path : paths) {
            int indexOfLastEdge = path.getEdges().size() - 1;
            NetworkEdge edge = path.getEdges().get(indexOfLastEdge);
            double distance = util.distance(locationPoint,  path.getRecentProjectedPoint());
            int segmentIndex = path.getRecentProjectedPointSegmentIndex();
            double bearingSegment = util.bearing(edge.getStartSegmentPoint().get(segmentIndex), edge.getEndSegmentPoint().get(segmentIndex));
            double bearingLocationPoint = locationPoint.getBearing();
            double bearingScore = Math.abs(util.differenceBetweenBearings(bearingSegment,bearingLocationPoint));
            MapMatchingPathHistoryEntry newHistoryEntry = new MapMatchingPathHistoryEntry(locationPoint, path.getRecentProjectedPoint(), path.getRecentProjectedPointSegmentIndex(), path.getScore(), distance, bearingScore);
            path.getHistoryPerEdge().get(indexOfLastEdge).add(newHistoryEntry);
        }
    }

    private void findExtensions(Point locationPoint, MapMatchingPath path) {
        int indexOfLastEdge = path.getEdges().size() - 1;
        NetworkEdge edge = path.getEdges().get(indexOfLastEdge);
        if (checkExtension(locationPoint, path, edge)) {
            List<NetworkEdge> edgesToExtent = getEdgesToExtent(path, edge, locationPoint);
            for (NetworkEdge edgeToExtent : edgesToExtent) {
                if (edgeToExtent.getTargetID() != edge.getSourceID() && edgeToExtent.getTargetID() != edge.getTargetID()) {
                    MapMatchingPath pathExtension = new MapMatchingPath(path);
                    pathExtension.getEdges().add(edgeToExtent);
                    pathExtension.getHistoryPerEdge().add(new ArrayList<MapMatchingPathHistoryEntry>());
                    pathExtension.setExtensionCounter(pathExtension.getExtensionCounter() + 1);
                    adjustScore(path, edge, edgeToExtent, pathExtension, locationPoint);
                    findProjection(locationPoint, pathExtension);
                    if(isNewPathUseful(locationPoint, pathExtension)){
                        if(!edgeToExtent.getSourcePos().equals(pathExtension.getRecentProjectedPoint())){
                            pathsToExtend.add(pathExtension);
                        }
                        findExtensions(locationPoint, pathExtension);
                        pathExtension.setExtensionCounter(0);
                    }
                }
            }
            path.setUTurnExtension(false);
        }
    }

    private void adjustScore(MapMatchingPath path, NetworkEdge edge, NetworkEdge edgeToExtent, MapMatchingPath pathExtension, Point locationPoint) {
        if(preferSmallerPaths){
            pathExtension.setScore(pathExtension.getScore()+preferSmallerPathsMalus);
        }
        if(path.isUTurnExtension()){
            if(edge.getSourceNode() != null){
                if(edge.getSourceNode().getOutgoingEdges().contains(edgeToExtent)){
                    pathExtension.setScore(pathExtension.getScore()+uTurnMalus);
                }
            }
        }
        //Service Road Malus
        if(edge.getRoadType().equals("service") && locationPoint.getSpeed() > 20){
            pathExtension.setScore(pathExtension.getScore()+25);
        }

        if(secondoDB.isMatchFootways()&&treatFootwaysDifferent&&!temporaryPreferRoads){
            if(averageSpeed<=footwaySpeed){
                if(util.isRoad(edge.getRoadType())){
                    pathExtension.setScore(pathExtension.getScore()+ roadMalus);
                }
            }
            else{
                if(!util.isRoad(edge.getRoadType())){
                    pathExtension.setScore(pathExtension.getScore()+200);
                }
            }
        }
    }

    private List<NetworkEdge> getEdgesToExtent(MapMatchingPath path, NetworkEdge edge, Point locationPoint) {
        List<NetworkEdge> edgesToExtent = new ArrayList<NetworkEdge>();
        if (allowUTurns) {
            if (path.getRecentProjectedPoint().equals(edge.getSourcePos()) || util.distance(edge.getSourcePos(),locationPoint)<util.distance(edge.getTargetPos(),locationPoint)) {
                path.setUTurnExtension(true);
                if(edge.getSourceNode() != null){
                    edgesToExtent.addAll(edge.getSourceNode().getOutgoingEdges());
                }
                if(edge.getTargetNode() != null){
                    edgesToExtent.addAll(edge.getTargetNode().getOutgoingEdges());
                }
            } else {
                if(edge.getTargetNode() != null){
                    edgesToExtent.addAll(edge.getTargetNode().getOutgoingEdges());
                }
            }
        } else {
            if(edge.getTargetNode() != null){
                edgesToExtent.addAll(edge.getTargetNode().getOutgoingEdges());
            }
        }
        return edgesToExtent;
    }

    private boolean checkExtension(Point locationPoint, MapMatchingPath path, NetworkEdge edge) {
        if (path.getExtensionCounter() < extensionDepth) {
            if (alwaysExtent || path.getRecentProjectedPoint().equals(edge.getSourcePos()) || path.getRecentProjectedPoint().equals(edge.getTargetPos())) {
                return true;
            }
            if(checkExtensionBasedOnPastLocations(locationPoint, path, edge)){
                return true;
            }
        }
        return false;
    }

    private boolean checkExtensionBasedOnPastLocations(Point locationPoint, MapMatchingPath path, NetworkEdge edge) {
        if (util.angleOfPointsComparedToAngleOfEdge(locationPoint,path)) {
            return true;
        }
        double distanceOfEdge = util.distanceOfEdge(edge);
        if(distanceOfEdge < util.distanceOfPointsMappedToLastEdge(locationPoint, path)){
            return true;
        }
        if(util.percentalSpanOfProjectedPointsMappedToLastEdge(path,distanceOfEdge) > extensionPercentalSpan) {
            return true;
        }
        return false;
    }

    private void findProjections(Point locationPoint){
        for (MapMatchingPath path : paths) {
            findProjection(locationPoint, path);
        }
    }

    private void findProjection(Point locationPoint, MapMatchingPath path) {
        int indexOfLastEdge = path.getEdges().size() - 1;
        NetworkEdge edge = path.getEdges().get(indexOfLastEdge);

        double bestDistance = Double.MAX_VALUE;
        Point bestProjection = null;
        int bestProjectionSegmentIndex = Integer.MAX_VALUE;

        for (int i = 0; i < edge.getStartSegmentPoint().size(); i++) {
            double distance = util.projectPointToSegment(locationPoint, edge.getStartSegmentPoint().get(i), edge.getEndSegmentPoint().get(i));

            if (distance < bestDistance) {
                bestProjection = util.getLastProjectedPoint();
                bestDistance = distance;
                bestProjectionSegmentIndex = i;
            }
        }
        path.setRecentProjectedPoint(bestProjection);
        path.setRecentProjectedPointSegmentIndex(bestProjectionSegmentIndex);
    }

    private boolean isNewPathUseful(Point locationPoint, MapMatchingPath newPath) {
        int indexOfLastEdgeNewPath = newPath.getEdges().size() - 1;

        if(allowUTurns && !allowMultipleUTurns){
            if(newPath.getEdges().size()>=3){
                NetworkEdge lastEdge = newPath.getEdges().get(indexOfLastEdgeNewPath);
                NetworkEdge secondLastEdge = newPath.getEdges().get(indexOfLastEdgeNewPath-1);
                NetworkEdge thirdLastEdge = newPath.getEdges().get(indexOfLastEdgeNewPath-2);
                if(lastEdge.getSourceID()==secondLastEdge.getSourceID() && secondLastEdge.getSourceID()==thirdLastEdge.getSourceID() ){
                    return false;
                }
            }
        }
        if(isAnEqualPathWithBetterScoreInList(locationPoint, newPath, paths, indexOfLastEdgeNewPath)){
            return false;
        }
        if(isAnEqualPathWithBetterScoreInList(locationPoint, newPath, pathsToExtend, indexOfLastEdgeNewPath)){
            return false;
        }
        return true;
    }

    private boolean isAnEqualPathWithBetterScoreInList(Point locationPoint, MapMatchingPath newPath, List<MapMatchingPath> list, int indexOfLastEdgeNewPath){
        for (MapMatchingPath path : list) {
            int indexOfLastEdge = path.getEdges().size() - 1;
            boolean lastEdgeEqual = path.getEdges().get(indexOfLastEdge).equals(newPath.getEdges().get(indexOfLastEdgeNewPath));
            boolean areEdgesEqual = false;
            if(lastEdgeEqual){
                areEdgesEqual=util.areEdgesEqual(path, newPath);
            }
            if (lastEdgeEqual && (areEdgesEqual || util.areProjectedPointsEqual(path, newPath))) {
                double tempScoreNewPath = util.calculateScore(locationPoint, newPath);
                double tempScoreOldPath = util.calculateScore(locationPoint, path);
                if (path.getScore()+tempScoreOldPath < newPath.getScore()+tempScoreNewPath) {
                    return true;
                }
                else if(path.getScore()+tempScoreOldPath == newPath.getScore()+tempScoreNewPath){
                    if(!areEdgesEqual){
                        if(path.getEdges().size()==newPath.getEdges().size()){
                            double distancePath = util.distanceOfPath(path);
                            double distanceNewPath = util.distanceOfPath(newPath);
                            if(distancePath<=distanceNewPath){
                                return true;
                            }
                            else{
                                uselessPathsToRemove.add(path);
                            }
                        }
                        else if(path.getEdges().size()<newPath.getEdges().size()){
                            return true;
                        }
                        else{
                            //path.setScore(path.getScore()+1);
                            uselessPathsToRemove.add(path);
                        }
                    }
                    else{
                        return true;
                    }
                }
                else{
                    uselessPathsToRemove.add(path);
                }
            }
        }
        return false;
    }

    private boolean checkReset(Point locationPoint) {
        if (paths.size() == 0) {
            mapMatchingListener.showLineInfoMessage(resetMessageNoWay);
            reset();
            return true;
        } else {
            double smallestDistance = Double.MAX_VALUE;
            for (MapMatchingPath path : paths) {
                double distance = util.distance(locationPoint, path.getRecentProjectedPoint());
                if (distance < smallestDistance) {
                    smallestDistance = distance;
                }
            }
            if (smallestDistance > resetDistance) {
                mapMatchingListener.showLineInfoMessage(resetMessageDistance);
                resetNodesInPathsEdges();
                if(recentBestPath!=null){
                    for (NetworkEdge edge : recentBestPath.getEdges()) {
                        edge.setSourceNode(null);
                        edge.setTargetNode(null);
                    }
                    if(recentBestPath.getEdges().size()!=0){
                        int indexOfLastEdge = recentBestPath.getEdges().size()-1;
                        List<MapMatchingPathHistoryEntry> history = recentBestPath.getHistoryPerEdge().get(indexOfLastEdge);
                        if(history.size()!=0){
                            int indexOfLastHistoryEntry = history.size()-1;
                            history.remove(indexOfLastHistoryEntry);
                            if(symbolicTrajectories.isDirectTransmission()){
                                recentBestPath.setResultsTransmitted(true);
                            }
                            if(!movementResults.isTransmitMovementData()){
                                recentBestPath.setMovementResultsTransmitted(true);
                            }
                            oldResults.add(recentBestPath);
                        }
                    }
                }
                paths.clear();
                reset();
                return true;
            }
        }
        return false;
    }

    private void cleanPathsData(){
        if(cleanPaths){
            if(paths.size()!=0){
                locationCounterSinceLastClean = locationCounterSinceLastClean + 1;
                if(locationCounterSinceLastClean >= maxLocationCountUntilClean){
                    if(checkCleanRequirements()){
                        locationCounterSinceLastClean = 0;
                        saveResultCopy();
                        List<MapMatchingPath> cleanedPaths = new ArrayList<MapMatchingPath>();
                        for (MapMatchingPath path : paths) {
                            MapMatchingPath cleanPath = new MapMatchingPath();
                            List<NetworkEdge> lastXEdgesOfPath = new ArrayList<NetworkEdge>();
                            List<List<MapMatchingPathHistoryEntry>> lastXHistoriesOfPath = new ArrayList<List<MapMatchingPathHistoryEntry>>();
                            int edgesCount = path.getEdges().size();
                            lastXEdgesOfPath = path.getEdges().subList(edgesCount- edgesLeftInCleanPaths, edgesCount);
                            lastXHistoriesOfPath = path.getHistoryPerEdge().subList(edgesCount- edgesLeftInCleanPaths, edgesCount);
                            for (int i = 0; i<lastXEdgesOfPath.size(); i++)
                            {
                                cleanPath.getEdges().add(lastXEdgesOfPath.get(i));
                                cleanPath.getHistoryPerEdge().add(new ArrayList<MapMatchingPathHistoryEntry>(lastXHistoriesOfPath.get(i)));
                            }
                            cleanPath.setScore(path.getScore());
                            cleanPath.setRecentProjectedPoint(path.getRecentProjectedPoint());
                            cleanedPaths.add(cleanPath);
                        }
                        paths.clear();
                        paths.addAll(cleanedPaths);
                        mapMatchingListener.showLineInfoMessage(cleanMessage);
                    }
                }
            }
        }
    }

    private boolean checkCleanRequirements(){
        for (MapMatchingPath path : paths) {
            if(path.getEdges().size()<= edgesLeftInCleanPaths){
                return false;
            }
        }
        return true;
    }

    private void saveResultCopy(){
        MapMatchingPath result = new MapMatchingPath(paths.get(0));
        result.setCleanedResultPath(true);

        List<NetworkEdge> edgesForResultPath = new ArrayList<NetworkEdge>();
        List<List<MapMatchingPathHistoryEntry>> historiesForResultPath = result.getHistoryPerEdge();

        for (NetworkEdge edge : result.getEdges()) {
            NetworkEdge newEdge = new NetworkEdge(edge, false);
            edgesForResultPath.add(newEdge);
        }

        int edgesCount = edgesForResultPath.size();
        edgesForResultPath = edgesForResultPath.subList(0,edgesCount- edgesLeftInCleanPaths);
        historiesForResultPath = historiesForResultPath.subList(0,edgesCount- edgesLeftInCleanPaths);

        result.setEdges(edgesForResultPath);
        result.setHistoryPerEdge(historiesForResultPath);

        if(symbolicTrajectories.isDirectTransmission()){
            result.setResultsTransmitted(true);
        }
        if(!movementResults.isTransmitMovementData()){
            result.setMovementResultsTransmitted(true);
        }
        oldResults.add(result);
    }

    private void resetNodesInPathsEdges() {
        for (MapMatchingPath path : paths) {
            for (NetworkEdge edge : path.getEdges()) {
                edge.setSourceNode(null);
                edge.setTargetNode(null);
            }
        }
    }

    public List<MapMatchingPath> getAllResults() {
        List<MapMatchingPath> results = new ArrayList<MapMatchingPath>(oldResults);
        if(paths.size()!=0){
            results.add(paths.get(0));
        }
        return results;
    }

    public List<MapMatchingPath> getAllResultsConnected(){
        List<MapMatchingPath> allResults = getAllResults();
        List<MapMatchingPath> relevantResults = new ArrayList<MapMatchingPath>();

        for(int i = 0; i<allResults.size(); i++){
            MapMatchingPath pathCopy = new MapMatchingPath(allResults.get(i));
            pathCopy.setRecentProjectedPoint(allResults.get(i).getRecentProjectedPoint());
            pathCopy.setRecentProjectedPointSegmentIndex(allResults.get(i).getRecentProjectedPointSegmentIndex());
            if(allResults.get(i).isCleanedResultPath()){
                int j = i;
                while(allResults.get(j+1).isCleanedResultPath()){
                    j = j+1;
                    pathCopy.getEdges().addAll(allResults.get(j).getEdges());
                    pathCopy.getHistoryPerEdge().addAll(allResults.get(j).getHistoryPerEdge());
                }
                j = j+1;
                pathCopy.getEdges().addAll(allResults.get(j).getEdges());
                pathCopy.getHistoryPerEdge().addAll(allResults.get(j).getHistoryPerEdge());
                pathCopy.setRecentProjectedPoint(allResults.get(j).getRecentProjectedPoint());
                pathCopy.setRecentProjectedPointSegmentIndex(allResults.get(j).getRecentProjectedPointSegmentIndex());
                i = j;
            }
            relevantResults.add(pathCopy);
        }
        return relevantResults;
    }

    public MapMatchingPath getCurrentResult(){
        MapMatchingPath currentResults = new MapMatchingPath();
        List<MapMatchingPath> allResults = getAllResults();
        int relevantIndex = -1;
        if(allResults.size()!=0){
            for (int i=allResults.size()-2 ; i>=0 ; i--)
            {
                if(allResults.get(i).isCleanedResultPath()){
                    relevantIndex = i;
                }
                else {
                    break;
                }
            }
            if(relevantIndex!=-1){
                for (int j = relevantIndex;j<allResults.size()-1;j++){
                    currentResults.getEdges().addAll(allResults.get(j).getEdges());
                    currentResults.getHistoryPerEdge().addAll(allResults.get(j).getHistoryPerEdge());
                }
            }
            currentResults.getEdges().addAll(allResults.get(allResults.size()-1).getEdges());
            currentResults.getHistoryPerEdge().addAll(allResults.get(allResults.size()-1).getHistoryPerEdge());
            if(paths.size()!=0){
                currentResults.setRecentProjectedPoint(paths.get(0).getRecentProjectedPoint());
                currentResults.setRecentProjectedPointSegmentIndex(paths.get(0).getRecentProjectedPointSegmentIndex());
            }
        }
        return currentResults;
    }

    public List<MapMatchingPath> getNotTransmittedResults(){
        List<MapMatchingPath> allResults = getAllResults();
        List<MapMatchingPath> relevantResults = new ArrayList<MapMatchingPath>();

        for(int i = 0; i<allResults.size(); i++){
            if(!allResults.get(i).isResultsTransmitted()){
                MapMatchingPath pathCopy = new MapMatchingPath(allResults.get(i));
                pathCopy.setRecentProjectedPoint(allResults.get(i).getRecentProjectedPoint());
                pathCopy.setRecentProjectedPointSegmentIndex(allResults.get(i).getRecentProjectedPointSegmentIndex());
                if(allResults.get(i).isCleanedResultPath()){
                    int j = i;
                    while(allResults.get(j+1).isCleanedResultPath()){
                        j = j+1;
                        pathCopy.getEdges().addAll(allResults.get(j).getEdges());
                        pathCopy.getHistoryPerEdge().addAll(allResults.get(j).getHistoryPerEdge());
                    }
                    j = j+1;
                    pathCopy.getEdges().addAll(allResults.get(j).getEdges());
                    pathCopy.getHistoryPerEdge().addAll(allResults.get(j).getHistoryPerEdge());
                    pathCopy.setRecentProjectedPoint(allResults.get(j).getRecentProjectedPoint());
                    pathCopy.setRecentProjectedPointSegmentIndex(allResults.get(j).getRecentProjectedPointSegmentIndex());
                    i = j;
                }
                relevantResults.add(pathCopy);
            }
        }
        return relevantResults;
    }

    public List<MapMatchingPath> getNotTransmittedMovementResults(){
        List<MapMatchingPath> allResults = getAllResults();
        List<MapMatchingPath> relevantResults = new ArrayList<MapMatchingPath>();

        for(int i = 0; i<allResults.size(); i++){
            if(!allResults.get(i).isMovementResultsTransmitted()){
                MapMatchingPath pathCopy = new MapMatchingPath(allResults.get(i));
                pathCopy.setRecentProjectedPoint(allResults.get(i).getRecentProjectedPoint());
                pathCopy.setRecentProjectedPointSegmentIndex(allResults.get(i).getRecentProjectedPointSegmentIndex());
                if(allResults.get(i).isCleanedResultPath()){
                    int j = i;
                    while(allResults.get(j+1).isCleanedResultPath()){
                        j = j+1;
                        pathCopy.getEdges().addAll(allResults.get(j).getEdges());
                        pathCopy.getHistoryPerEdge().addAll(allResults.get(j).getHistoryPerEdge());
                    }
                    j = j+1;
                    pathCopy.getEdges().addAll(allResults.get(j).getEdges());
                    pathCopy.getHistoryPerEdge().addAll(allResults.get(j).getHistoryPerEdge());
                    pathCopy.setRecentProjectedPoint(allResults.get(j).getRecentProjectedPoint());
                    pathCopy.setRecentProjectedPointSegmentIndex(allResults.get(j).getRecentProjectedPointSegmentIndex());
                    i = j;
                }
                relevantResults.add(pathCopy);
            }
        }
        return relevantResults;
    }

    public void clearData(){
        oldResults.clear();
    }

    private void showMessages(Point locationPoint){
        timeEndProcessLocationPoint = System.currentTimeMillis();
        timeForProcessingLocationPoint = timeEndProcessLocationPoint - timeStartProcessLocationPoint;

        showNetworkUpdateMessage();
        showResultDirectTransmissionMessage();
        showLocationPointProcessedMessage(locationPoint);
    }

    private void showNetworkUpdateMessage(){
        if(timeStartNetworkUpdate!=-1){
            long timeForProcessingNetworkUpdate = timeEndNetworkUpdate - timeStartNetworkUpdate;
            timeStartNetworkUpdate = -1;
            mapMatchingListener.showLineInfoMessage("Data-Update from Secondo Server ("+timeForProcessingNetworkUpdate+" Millisec).");
            timeForProcessingLocationPoint = timeForProcessingLocationPoint - timeForProcessingNetworkUpdate;
        }
    }

    private void showResultDirectTransmissionMessage(){
        long timeForTransmitResults = 0;
        boolean directTransmissionActive = (symbolicTrajectories.isDirectTransmission() && (symbolicTrajectories.isGenerateStreetnames()||symbolicTrajectories.isGenerateCardinalDirections()||symbolicTrajectories.isGenerateSpeedProfile()|| symbolicTrajectories.isGenerateHeightProfile()));
        if(directTransmissionActive&&!symbolicTrajectories.isLocalMode()){
            timeForTransmitResults = timeEndTransmitResults - timeStartTransmitResults;
            timeForProcessingLocationPoint = timeForProcessingLocationPoint - timeForTransmitResults;
            if(symbolicTrajectories.resultsTransmitted()){
                mapMatchingListener.showResultMessage("Results transmitted ("+(timeForTransmitResults)+" Millisec):<br>");
            }
            if(symbolicTrajectories.getUpdatedStreetname()!=null){
                SymbolicTrajectory streetnameT = symbolicTrajectories.getUpdatedStreetname();
                symbolicTrajectories.showStreetnameTransmittedMessage(streetnameT);
                if(symbolicTrajectories.getSkippedStreetnames().size()!=0){
                    for (SymbolicTrajectory skippedStreetname : symbolicTrajectories.getSkippedStreetnames()) {
                        symbolicTrajectories.showStreetnameTransmittedMessage(skippedStreetname);
                    }
                }
            }
            if(symbolicTrajectories.getUpdatedCardinalDirection()!=null){
                SymbolicTrajectory directionT = symbolicTrajectories.getUpdatedCardinalDirection();
                symbolicTrajectories.showCardinalDirectionTransmittedMessage(directionT);
            }
            if(symbolicTrajectories.getUpdatedSpeedInterval()!=null){
                SymbolicTrajectory speedT = symbolicTrajectories.getUpdatedSpeedInterval();
                symbolicTrajectories.showSpeedIntervalTransmittedMessage(speedT);
            }
            if(symbolicTrajectories.getUpdatedHeightInterval()!=null){
                SymbolicTrajectory heightT = symbolicTrajectories.getUpdatedHeightInterval();
                symbolicTrajectories.showHeightIntervalTransmittedMessage(heightT);
            }
            if(symbolicTrajectories.resultsTransmitted()){
                mapMatchingListener.showResultMessage("<br>");
            }
        }
    }

    private void showLocationPointProcessedMessage(Point locationPoint){
        int indexOfLastEdge = paths.get(0).getEdges().size() - 1;
        int indexOfLastHistoryEntry = paths.get(0).getHistoryPerEdge().get(indexOfLastEdge).size() - 1;
        MapMatchingPathHistoryEntry historyEntry = paths.get(0).getHistoryPerEdge().get(indexOfLastEdge).get(indexOfLastHistoryEntry);

        String streetname, time, cardinalDirection, speed ="-", height="-", accuracy, pdop, hdop, vdop, satellitesVisible, satellitesUsed, score, distance, angleDifference;

        streetname = SymbolicTrajectory.getStreetname(paths.get(0).getEdges().get(indexOfLastEdge));
        double scoreRounded = Math.round(paths.get(0).getScore()*10)/10.0;
        score = String.valueOf(scoreRounded);
        double accuracyRounded = Math.round(locationPoint.getAccuracy()*10)/10.0;
        accuracy = String.valueOf(accuracyRounded);
        time = SymbolicTrajectory.getTimeFromDateLongWithTimeZone(locationPoint.getTime());
        cardinalDirection = SymbolicTrajectory.getCardinalDirection(locationPoint.getBearing());

        double speedRounded = Math.round(locationPoint.getSpeed()*10)/10.0;
        speed = String.valueOf(speedRounded);

        if(locationPoint.getAltitude()!= Double.MAX_VALUE){
            double heightRounded = Math.round(locationPoint.getAltitude()*10)/10.0;
            height = String.valueOf(heightRounded);
        }
        pdop = String.valueOf(locationPoint.getPdop());
        hdop = String.valueOf(locationPoint.getHdop());
        vdop = String.valueOf(locationPoint.getVdop());
        satellitesVisible = String.valueOf(locationPoint.getSatellitesVisible());
        satellitesUsed = String.valueOf(locationPoint.getSatellitesUsed());

        double distanceRounded = Math.round(historyEntry.getDistanceBetweenLocationAndProjectedPoint()*10)/10.0;
        distance = String.valueOf(distanceRounded);
        double angleDifferenceRounded = Math.round(historyEntry.getAbsoluteAngleDifference()*10)/10.0;
        angleDifference = String.valueOf(angleDifferenceRounded);

        mapMatchingListener.showLineInfoMessage("Location Point processed ("+(timeForProcessingLocationPoint)+" Millisec):"+"<br>Time: "+time+"<br>Streetname: "+streetname+"<br>Direction: "+cardinalDirection+"<br>Speed: "+speed+" km/h"+"<br>Height: "+height+" meter"+"<br>Probable Accuracy: "+accuracy+" meter" + "<br>PDOP: "+pdop+", HDOP: "+hdop+", VDOP: "+vdop+"<br>Satellites: Used "+satellitesUsed+", Visible "+satellitesVisible+"<br>Score: "+score+"<br>Distance: "+distance+" meter"+"<br>Absolute Angle Difference: "+angleDifference+(char) 0x00B0);

//        completeTimeProcessingLocationPoints = completeTimeProcessingLocationPoints+timeForProcessingLocationPoint;
//        System.out.println("CompleteTimeForAllLocationsPoints: "+completeTimeProcessingLocationPoints);
//        mapMatchingListener.showLineInfoMessage("CompleteTimeForAllLocationsPoints: "+completeTimeProcessingLocationPoints);

        if(timeForProcessingLocationPoint>10000){
            boolean improveMessageShown = false;
            if(secondoDB.isMatchFootways()){
                mapMatchingListener.showLineInfoMessage("To improve performance consider disabling \"Match Footways\".");
            }
            if(maxPathsSize>5){
                mapMatchingListener.showLineInfoMessage("To improve performance consider reducing \"Hypotheses Count\" in Advanced Settings when Map Matching stopped. (e.g. to 5)");
                improveMessageShown = true;
            }
            if(extensionDepth>4){
                mapMatchingListener.showLineInfoMessage("To improve performance consider reducing \"Extension Depth\" in Advanced Settings when Map Matching stopped. (e.g. to 4)");
                improveMessageShown = true;
            }
            if(allowUTurns){
                mapMatchingListener.showLineInfoMessage("To improve performance consider disabling \"Allow (U)-Turns\" in Advanced Settings when Map Matching stopped.");
                improveMessageShown = true;
            }
            if(improveMessageShown){
                mapMatchingListener.showLineInfoMessage("Settings should be changed carefully. Changes can effect result quality and may make Map Matching problems more probable.");
            }
        }
    }

    public SecondoDB getSecondoDB() {
        return secondoDB;
    }

    public SymbolicTrajectories getSymbolicTrajectories() {
        return symbolicTrajectories;
    }

    public void setAllowUTurns(boolean allowUTurns) {
        this.allowUTurns = allowUTurns;
    }

    public void setMaxPathsSize(int maxPathsSize) {
        this.maxPathsSize = maxPathsSize;
    }

    public void setExtensionDepth(int extensionDepth) {
        this.extensionDepth = extensionDepth;
    }

    public void setuTurnMalus(int uTurnMalus) {
        this.uTurnMalus = uTurnMalus;
    }

    public void setCleanPaths(boolean cleanPaths) {
        this.cleanPaths = cleanPaths;
    }

    public void setMaxLocationCountUntilClean(int maxLocationCountUntilClean) {
        this.maxLocationCountUntilClean = maxLocationCountUntilClean;
    }

    public Network getNetwork() {
        return network;
    }

    public MapMatchingListener getMapMatchingListener() {
        return mapMatchingListener;
    }

    public MapMatchingUtilities getUtil() {
        return util;
    }

    public String getDbErrorMessage() {
        return dbErrorMessage;
    }

    public void setResetDistance(int resetDistance) {
        this.resetDistance = resetDistance;
    }

    public void setEdgesLeftInCleanPaths(int edgesLeftInCleanPaths) {
        this.edgesLeftInCleanPaths = edgesLeftInCleanPaths;
    }

    public void setAllowMultipleUTurns(boolean allowMultipleUTurns) {
        this.allowMultipleUTurns = allowMultipleUTurns;
    }

    public void setAlwaysExtent(boolean alwaysExtent) {
        this.alwaysExtent = alwaysExtent;
    }

    public SecondoDB getSecondoResultDB() {
        return secondoResultDB;
    }

    public MovementResults getMovementResults() {
        return movementResults;
    }

    public synchronized boolean isPreferenceMatchFootwaysChange() {
        return preferenceMatchFootwaysChange;
    }

    public synchronized void setPreferenceMatchFootwaysChange(boolean preferenceMatchFootwaysChange) {
        this.preferenceMatchFootwaysChange = preferenceMatchFootwaysChange;
    }
    public void setPreferSmallerPaths(boolean preferSmallerPaths) {
        this.preferSmallerPaths = preferSmallerPaths;
    }
    public void setMaxPathsSizePrefValue(int maxPathsSizePrefValue) {
        this.maxPathsSizePrefValue = maxPathsSizePrefValue;
    }
    public void setImproveFootwayPerformance(boolean improveFootwayPerformance) {
        this.improveFootwayPerformance = improveFootwayPerformance;
    }

    public void setPreferSmallerPathsMalus(int preferSmallerPathsMalus) {
        this.preferSmallerPathsMalus = preferSmallerPathsMalus;
    }

    public void setFootwaySpeed(int footwaySpeed) {
        this.footwaySpeed = footwaySpeed;
    }

    public void setRoadMalus(int roadMalus) {
        this.roadMalus = roadMalus;
    }

    public void setTemporaryPreferRoads(boolean temporaryPreferRoads) {
        this.temporaryPreferRoads = temporaryPreferRoads;
    }
    public void setTreatFootwaysDifferent(boolean treatFootwaysDifferent) {
        this.treatFootwaysDifferent = treatFootwaysDifferent;
    }

    public void setAverageSpeed(double averageSpeed) {
        this.averageSpeed = averageSpeed;
    }

    public double getAverageSpeed() {
        return averageSpeed;
    }

    public boolean isTemporaryPreferRoads() {
        return temporaryPreferRoads;
    }
}
