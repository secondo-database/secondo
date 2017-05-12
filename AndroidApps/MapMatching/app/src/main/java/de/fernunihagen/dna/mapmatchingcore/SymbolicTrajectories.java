package de.fernunihagen.dna.mapmatchingcore;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class SymbolicTrajectories {

    private MapMatching mapMatching;

    private boolean directTransmission;
    private boolean generateStreetnames;
    private boolean generateCardinalDirections ;
    private boolean generateSpeedProfile;
    private boolean generateHeightProfile;

    private List<SymbolicTrajectory> resultsStreetnames = new ArrayList<SymbolicTrajectory>();
    private List<SymbolicTrajectory> resultsCardinalDirections = new ArrayList<SymbolicTrajectory>();
    private List<SymbolicTrajectory> resultsSpeedIntervals = new ArrayList<SymbolicTrajectory>();
    private List<SymbolicTrajectory> resultsHeightIntervals = new ArrayList<SymbolicTrajectory>();

    private List<SymbolicTrajectory> directTransmittedStreetnames = new ArrayList<SymbolicTrajectory>();
    private List<SymbolicTrajectory> directTransmittedCardinalDirections = new ArrayList<SymbolicTrajectory>();
    private List<SymbolicTrajectory> directTransmittedSpeedIntervals = new ArrayList<SymbolicTrajectory>();
    private List<SymbolicTrajectory> directTransmittedHeightIntervals = new ArrayList<SymbolicTrajectory>();

    private SymbolicTrajectory currentStreetname;
    private SymbolicTrajectory currentCardinalDirection;
    private SymbolicTrajectory currentSpeedInterval;
    private SymbolicTrajectory currentHeightInterval;

    private SymbolicTrajectory saveCurrentStreetname;
    private SymbolicTrajectory saveCurrentCardinalDirection;
    private SymbolicTrajectory saveCurrentSpeedInterval;
    private SymbolicTrajectory saveCurrentHeightInterval;

    private SymbolicTrajectory updatedStreetname;
    private SymbolicTrajectory updatedCardinalDirection;
    private SymbolicTrajectory updatedSpeedInterval;
    private SymbolicTrajectory updatedHeightInterval;

    private long timeOfLocationPoint;
    private long cancelTime;
    private boolean cancelTimeSet=false;

    private boolean localMode;
    private boolean transmit = true;
    private boolean update = false;

    private SymbolicTrajectoryTextFile symbolicTrajectoryTextFile = new SymbolicTrajectoryTextFile(mapMatching);

    List<SymbolicTrajectory> skippedStreetnames = new ArrayList<SymbolicTrajectory>();
    List<Double> skippedStreetnamesDistance = new ArrayList<Double>();
    List<Long> skippedStreetnamesTime = new ArrayList<Long>();

    public SymbolicTrajectories(MapMatching mapMatching){
        this.mapMatching = mapMatching;
    }

    public boolean update(Point locationPoint, MapMatchingPath currentResult){
        if(directTransmission&&!localMode){
            updatedStreetname =null;
            updatedCardinalDirection =null;
            updatedSpeedInterval =null;
            updatedHeightInterval =null;
            skippedStreetnames.clear();

            timeOfLocationPoint = locationPoint.getTime();

            update = true;
            if(generateStreetnames){
                if(!updateSreetnames(currentResult)){
                    update = false;
                    return false;
                }
            }
            if(generateCardinalDirections){
                if(!updateCardinalDirections(locationPoint)){
                    update = false;
                    return false;
                }
            }
            if(generateSpeedProfile){
                if(!updateSpeedIntervals(locationPoint)){
                    update = false;
                    return false;
                }
            }
            if(generateHeightProfile){
                if(!updateHeightProfile(locationPoint)){
                    update = false;
                    return false;
                }
            }
        }
        update = false;
        return true;
    }

    public boolean generateAllResults(boolean transmit, List<MapMatchingPath> results){
        if(results.size()!=0){
            this.transmit = transmit;
            if(!initializeAllResultsGeneration()){
                this.transmit = true;
                return false;
            }
            if(generateStreetnames||localMode||!transmit){
                if(!generateAllStreetnames(results)){
                    mapMatching.getMapMatchingListener().showLineErrorMessage(mapMatching.getDbErrorMessage());
                    this.transmit = true;
                    return false;
                }
            }
            if(generateCardinalDirections||localMode||!transmit){
                if(!generateAllCardinalDirections(results)){
                    mapMatching.getMapMatchingListener().showLineErrorMessage(mapMatching.getDbErrorMessage());
                    this.transmit = true;
                    return false;
                }
            }
            if(generateSpeedProfile||localMode||!transmit){
                if(!generateAllSpeedIntervals(results)){
                    mapMatching.getMapMatchingListener().showLineErrorMessage(mapMatching.getDbErrorMessage());
                    this.transmit = true;
                    return false;
                }
            }
            if(generateHeightProfile||localMode||!transmit){
                if(!generateAllHeightIntervals(results)){
                    mapMatching.getMapMatchingListener().showLineErrorMessage(mapMatching.getDbErrorMessage());
                    this.transmit = true;
                    return false;
                }
            }
            if(!finalizeAllResultsGeneration()){
                this.transmit = true;
                return false;
            }
        }
        this.transmit = true;
        return true;
    }

    private boolean initializeAllResultsGeneration(){
        if(directTransmission){
            saveCurrentStreetname = currentStreetname;
            saveCurrentCardinalDirection = currentCardinalDirection;
            saveCurrentSpeedInterval = currentSpeedInterval;
            saveCurrentHeightInterval = currentHeightInterval;
        }
        currentStreetname = null;
        currentCardinalDirection = null;
        currentSpeedInterval = null;
        currentHeightInterval = null;

        resultsStreetnames.clear();
        resultsCardinalDirections.clear();
        resultsSpeedIntervals.clear();
        resultsHeightIntervals.clear();

        cancelTimeSet = true;
        cancelTime = System.currentTimeMillis();

        if(transmit&&!localMode){
            if(!connectionCheck()){
                return false;
            }
        }
        else{
            if(transmit&&localMode){
                symbolicTrajectoryTextFile = new SymbolicTrajectoryTextFile(mapMatching);
            }
        }
        return true;
    }

    private boolean connectionCheck(){
        if(mapMatching.getSecondoDB().isUseResultServer()){
            if(!mapMatching.getSecondoResultDB().isConnected()){
                return false;
            }
        }
        else{
            if(!mapMatching.getSecondoDB().isConnected()){
                return false;
            }
        }
        return true;
    }

    private boolean finalizeAllResultsGeneration(){
        if(transmit&&localMode){
            if(symbolicTrajectoryTextFile.save()){
                for (MapMatchingPath path : mapMatching.getAllResults()) {
                    path.setResultsTransmitted(true);
                }
            }
        }
        else{
            if(transmit){
                if(!transmitAllResults()){
                    return false;
                }
                for (MapMatchingPath path : mapMatching.getAllResults()) {
                    path.setResultsTransmitted(true);
                }
            }
        }
        if(directTransmission){
            currentStreetname = saveCurrentStreetname;
            currentCardinalDirection = saveCurrentCardinalDirection;
            currentSpeedInterval = saveCurrentSpeedInterval;
            currentHeightInterval = saveCurrentHeightInterval;
        }
        return true;
    }

    public void reset(){
        if(!cancelTimeSet){
            cancelTime = System.currentTimeMillis();
        }
        else{
            cancelTimeSet = false;
        }

        if(directTransmission&&!localMode){
            transmitLastResults();
        }

        directTransmittedStreetnames.clear();
        directTransmittedCardinalDirections.clear();
        directTransmittedSpeedIntervals.clear();
        directTransmittedHeightIntervals.clear();

        currentStreetname = null;
        currentCardinalDirection = null;
        currentSpeedInterval = null;
        currentHeightInterval = null;
    }

    private void transmitLastResults(){
        boolean streetname=false,direction=false,speed=false,height=false;
        long timeStartTransmitResults = System.currentTimeMillis();

        if(currentStreetname !=null){
            currentStreetname.setEndDate(cancelTime);
            if(saveSymbolicTrajectory("SymbolicTrajectoriesStreetnames", currentStreetname)){
                streetname = true;
            }
        }
        if(currentCardinalDirection !=null){
            currentCardinalDirection.setEndDate(cancelTime);
            if(saveSymbolicTrajectory("SymbolicTrajectoriesCardinalDirections", currentCardinalDirection)){
                direction = true;
            }
        }
        if(currentSpeedInterval !=null){
            currentSpeedInterval.setEndDate(cancelTime);
            if(saveSymbolicTrajectory("SymbolicTrajectoriesSpeedProfile", currentSpeedInterval)){
                speed = true;
            }
        }
        if(currentHeightInterval !=null){
            currentHeightInterval.setEndDate(cancelTime);
            if(saveSymbolicTrajectory("SymbolicTrajectoriesHeightProfile", currentHeightInterval)){
                height = true;
            }
        }

        SecondoDB db;
        if(mapMatching.getSecondoDB().isUseResultServer()){
            db = mapMatching.getSecondoResultDB();
        }
        else {
            db = mapMatching.getSecondoDB();
        }
        if(generateStreetnames){
            db.sendSymbolicTrajectoriesMovingLabel("SymbolicTrajectoriesStreetnames", directTransmittedStreetnames);
        }
        if(generateCardinalDirections){
            db.sendSymbolicTrajectoriesMovingLabel("SymbolicTrajectoriesCardinalDirections", directTransmittedCardinalDirections);
        }
        if(generateSpeedProfile){
            db.sendSymbolicTrajectoriesMovingLabel("SymbolicTrajectoriesSpeedProfile", directTransmittedSpeedIntervals);
        }
        if(generateHeightProfile){
            db.sendSymbolicTrajectoriesMovingLabel("SymbolicTrajectoriesHeightProfile", directTransmittedHeightIntervals);
        }
        long timeEndTransmitResults = System.currentTimeMillis();
        long timeForTransmitResults = timeEndTransmitResults - timeStartTransmitResults;
        if(streetname||direction||speed||height){
            mapMatching.getMapMatchingListener().showResultMessage("Last Results and Moving-Label transmitted ("+(timeForTransmitResults)+" Millisec):<br>");
            if(streetname){
                showStreetnameTransmittedMessage(currentStreetname);
            }
            if(direction){
                showCardinalDirectionTransmittedMessage(currentCardinalDirection);
            }
            if(speed){
                showSpeedIntervalTransmittedMessage(currentSpeedInterval);
            }
            if(height){
                showHeightIntervalTransmittedMessage(currentHeightInterval);
            }
            mapMatching.getMapMatchingListener().showResultMessage("<br>");
        }
    }

    private boolean updateSreetnames(MapMatchingPath currentResult) {
        int indexOfLastEdge = currentResult.getEdges().size() - 1;
        NetworkEdge lastResultEdge = currentResult.getEdges().get(indexOfLastEdge);
        String newStreetname = SymbolicTrajectory.getStreetname(lastResultEdge);

        if(!generateStreetname(timeOfLocationPoint, newStreetname, currentResult, indexOfLastEdge)){return false;}
        return true;
    }

    private boolean updateCardinalDirections(Point locationPoint) {
        String newCardinalDirection = SymbolicTrajectory.getCardinalDirection(locationPoint.getBearing());
        if(!generateCardinalDirection(timeOfLocationPoint, newCardinalDirection)){return false;}
        return true;
    }

    private boolean updateSpeedIntervals(Point locationPoint) {
        String speedInterval = SymbolicTrajectory.getSpeedInterval(locationPoint.getSpeed());
        if(!generateSpeedInterval(timeOfLocationPoint,speedInterval)){return false;}
        return true;
    }

    private boolean updateHeightProfile(Point locationPoint) {
        if(locationPoint.getAltitude()!=Double.MAX_VALUE){
            String heightInterval = SymbolicTrajectory.getHeightInterval(locationPoint.getAltitude());
            if(!generateHeightInterval(timeOfLocationPoint,heightInterval)){return false;}
        }
        return true;
    }

    private boolean generateAllStreetnames(List<MapMatchingPath> results){
        for (int j=0;j<results.size();j++) {
            for(int i = 0; i<results.get(j).getEdges().size(); i++){

                NetworkEdge edge = results.get(j).getEdges().get(i);
                List<MapMatchingPathHistoryEntry> edgeHistory = results.get(j).getHistoryPerEdge().get(i);

                if(edgeHistory.size()!=0){
                    long time = edgeHistory.get(0).getLocationPoint().getTime();
                    String streetname = SymbolicTrajectory.getStreetname(edge);
                    if(!generateStreetname(time,streetname,results.get(j),i)){return false;}
                }
            }
            if(currentStreetname !=null){
                if(j == results.size()-1){
                    currentStreetname.setEndDate(cancelTime);
                }
                else{
                    //Time of last Point before reset
                    List<MapMatchingPathHistoryEntry> history = results.get(j).getHistoryPerEdge().get(results.get(j).getHistoryPerEdge().size()-1);
                    currentStreetname.setEndDate(history.get(history.size()-1).getLocationPoint().getTime());
                }
                if(!saveSymbolicTrajectory("SymbolicTrajectoriesStreetnames", currentStreetname)){
                    return false;
                }
                currentStreetname = null;
            }
        }
        return true;
    }

    private boolean generateAllCardinalDirections(List<MapMatchingPath> results){
        for (int j=0;j<results.size();j++) {
            for(int i = 0; i<results.get(j).getEdges().size(); i++){
                for (MapMatchingPathHistoryEntry historyEntry : results.get(j).getHistoryPerEdge().get(i)) {
                    long time = historyEntry.getLocationPoint().getTime();
                    String newCardinalDirection = SymbolicTrajectory.getCardinalDirection(historyEntry.getLocationPoint().getBearing());

                    if(!generateCardinalDirection(time,newCardinalDirection)){return false;}
                }
            }
            if(currentCardinalDirection !=null){
                if(j == results.size()-1){
                    currentCardinalDirection.setEndDate(cancelTime);
                }
                else{
                    //Time of last Point before reset
                    List<MapMatchingPathHistoryEntry> history = results.get(j).getHistoryPerEdge().get(results.get(j).getHistoryPerEdge().size()-1);
                    currentCardinalDirection.setEndDate(history.get(history.size()-1).getLocationPoint().getTime());
                }
                if(!saveSymbolicTrajectory("SymbolicTrajectoriesCardinalDirections", currentCardinalDirection)){
                    return false;
                }
                currentCardinalDirection = null;
            }
        }
        return true;
    }

    private boolean generateAllSpeedIntervals(List<MapMatchingPath> results){
        for (int j=0;j<results.size();j++) {
            for (List<MapMatchingPathHistoryEntry> history : results.get(j).getHistoryPerEdge()) {
                for (MapMatchingPathHistoryEntry historyEntry : history) {
                    long time = historyEntry.getLocationPoint().getTime();
                    String speedInterval = SymbolicTrajectory.getSpeedInterval(historyEntry.getLocationPoint().getSpeed());
                    if(!generateSpeedInterval(time,speedInterval)){return false;}
                }
            }
            if(currentSpeedInterval !=null){
                if(j == results.size()-1){
                    currentSpeedInterval.setEndDate(cancelTime);
                }
                else{
                    //Time of last Point before reset
                    List<MapMatchingPathHistoryEntry> history = results.get(j).getHistoryPerEdge().get(results.get(j).getHistoryPerEdge().size()-1);
                    currentSpeedInterval.setEndDate(history.get(history.size()-1).getLocationPoint().getTime());
                }
                if(!saveSymbolicTrajectory("SymbolicTrajectoriesSpeedProfile", currentSpeedInterval)){
                    return false;
                }
                currentSpeedInterval = null;
            }
        }
        return true;
    }

    private boolean generateAllHeightIntervals(List<MapMatchingPath> results){
        for (int j=0;j<results.size();j++) {
            for (List<MapMatchingPathHistoryEntry> history : results.get(j).getHistoryPerEdge()) {
                for (MapMatchingPathHistoryEntry historyEntry : history) {
                    if(historyEntry.getLocationPoint().getAltitude()!=Double.MAX_VALUE){
                        long time = historyEntry.getLocationPoint().getTime();
                        String heightInterval = SymbolicTrajectory.getHeightInterval(historyEntry.getLocationPoint().getAltitude());
                        if(!generateHeightInterval(time,heightInterval)){return false;}
                    }
                }
            }
            if(currentHeightInterval !=null){
                if(j == results.size()-1){
                    currentHeightInterval.setEndDate(cancelTime);
                }
                else{
                    //Time of last Point before reset
                    List<MapMatchingPathHistoryEntry> history = results.get(j).getHistoryPerEdge().get(results.get(j).getHistoryPerEdge().size()-1);
                    currentHeightInterval.setEndDate(history.get(history.size()-1).getLocationPoint().getTime());
                }
                if(!saveSymbolicTrajectory("SymbolicTrajectoriesHeightProfile", currentHeightInterval)){
                    return false;
                }
                currentHeightInterval = null;
            }
        }
        return true;
    }

    private boolean generateStreetname(long time, String streetname, MapMatchingPath path, int index){
        if(currentStreetname ==null){
            currentStreetname = new SymbolicTrajectory(time, streetname);
        }
        else{
            if(!currentStreetname.getLabel().equals(streetname)){
                if(SymbolicTrajectory.isCalcStreetnameTime()){
                    time = calcStreetnameEntryTime(path,index, time, mapMatching, currentStreetname.getStartDate());
                }
                findSkippedStreetnames(currentStreetname.getLabel(),streetname,path,index);
                if(skippedStreetnames.size()!=0){
                    calculateSkippedStreetnamesTimes(time);
                    long timeA = skippedStreetnamesTime.get(0);
                    long timeB = currentStreetname.getStartDate();
                    skippedStreetnames.get(0).setStartDate(timeA+timeB);
                    currentStreetname.setEndDate(timeA+timeB);
                }
                else{
                    currentStreetname.setEndDate(time);
                }
                if(!saveSymbolicTrajectory("SymbolicTrajectoriesStreetnames", currentStreetname)){
                    return false;
                }

                if(!generateSkippedStreetnames(time)){
                    return false;
                }
                updatedStreetname = currentStreetname;
                currentStreetname = new SymbolicTrajectory(time, streetname);
            }
        }
        return true;
    }

    private boolean generateCardinalDirection(long time, String cardinalDirection){
        if(currentCardinalDirection ==null){
            currentCardinalDirection = new SymbolicTrajectory(time, cardinalDirection);
        }
        else{
            if(!currentCardinalDirection.getLabel().equals(cardinalDirection)){
                currentCardinalDirection.setEndDate(time);
                if(!saveSymbolicTrajectory("SymbolicTrajectoriesCardinalDirections", currentCardinalDirection)){
                    return false;
                }
                updatedCardinalDirection = currentCardinalDirection;
                currentCardinalDirection = new SymbolicTrajectory(time, cardinalDirection);
            }
        }
        return true;
    }

    private boolean generateSpeedInterval(long time, String speedInterval){
        if(currentSpeedInterval ==null){
            currentSpeedInterval = new SymbolicTrajectory(time, speedInterval);
        }
        else{
            if(!currentSpeedInterval.getLabel().equals(speedInterval)){
                currentSpeedInterval.setEndDate(time);
                if(!saveSymbolicTrajectory("SymbolicTrajectoriesSpeedProfile", currentSpeedInterval)){
                    return false;
                }
                updatedSpeedInterval = currentSpeedInterval;
                currentSpeedInterval = new SymbolicTrajectory(time, speedInterval);
            }
        }
        return true;
    }

    private boolean generateHeightInterval(long time, String heightInterval){
        if(currentHeightInterval ==null){
            currentHeightInterval = new SymbolicTrajectory(time, heightInterval);
        }
        else{
            if(!currentHeightInterval.getLabel().equals(heightInterval)){
                currentHeightInterval.setEndDate(time);

                if(!saveSymbolicTrajectory("SymbolicTrajectoriesHeightProfile", currentHeightInterval)){
                    return false;
                }
                updatedHeightInterval = currentHeightInterval;
                currentHeightInterval = new SymbolicTrajectory(time, heightInterval);
            }
        }
        return true;
    }

    private long calcStreetnameEntryTime(MapMatchingPath path, int index, long timeOfLocationPoint, MapMatching mapMatching, long startTime){
        NetworkEdge edge = path.getEdges().get(index);
        MapMatchingPathHistoryEntry historyEntry = path.getHistoryPerEdge().get(index).get(0);
        long timeOfCurrentPoint=0, timeOfPreviousPoint=0;

        timeOfCurrentPoint = historyEntry.getLocationPoint().getTime();
        //Distance from Start of Edge to Projected Point
        double partDistance = mapMatching.getUtil().distanceOfEdgeStartToPoint(edge,historyEntry);
        double distance = partDistance;

        //Distance to previous Projected Point
        while(index>0){
            index = index - 1;
            if( path.getHistoryPerEdge().get(index).size()==0){
                distance = distance + mapMatching.getUtil().distanceOfEdge(path.getEdges().get(index));
            }
            else{
                edge = path.getEdges().get(index);
                historyEntry = path.getHistoryPerEdge().get(index).get(path.getHistoryPerEdge().get(index).size()-1);
                timeOfPreviousPoint = historyEntry.getLocationPoint().getTime();
                //Distance from previous Projected Point to End of Edge
                double distanceFromPreviousProjectionToEdgeStart = mapMatching.getUtil().distanceOfEdgeStartToPoint(edge,historyEntry);
                distance = distance + mapMatching.getUtil().distanceOfEdge(path.getEdges().get(index)) - distanceFromPreviousProjectionToEdgeStart;
                break;
            }
        }

        if(timeOfPreviousPoint==0){
            return timeOfLocationPoint;
        }
        long timeToDistribute = timeOfCurrentPoint - timeOfPreviousPoint;
        double percentalValue = (partDistance*100)/distance;
        double resultTime = (percentalValue*timeToDistribute)/100;

        long calcTime = Math.round(timeOfLocationPoint-resultTime);

        if(calcTime<startTime){
            return timeOfLocationPoint;
        }
        return calcTime;
    }

    private boolean generateSkippedStreetnames(long time){
        for (int i = 0; i<skippedStreetnames.size();i++) {
            if(i!=skippedStreetnames.size()-1){
                long timeA = skippedStreetnamesTime.get(i+1);
                long timeB = skippedStreetnames.get(i).getStartDate();
                skippedStreetnames.get(i).setEndDate(timeA+timeB);
                skippedStreetnames.get(i+1).setStartDate(timeA+timeB);
            }else{
                skippedStreetnames.get(i).setEndDate(time);
            }
            if(!saveSymbolicTrajectory("SymbolicTrajectoriesStreetnames", skippedStreetnames.get(i))){
                return false;
            }
        }
        return true;
    }

    private void calculateSkippedStreetnamesTimes(long time){
        skippedStreetnamesTime.clear();
        long timeToDistribute = time -currentStreetname.getStartDate();
        double completeDistance = 0.0;
        for (Double distance : skippedStreetnamesDistance) {
            completeDistance = completeDistance + distance;
        }
        for (Double distance : skippedStreetnamesDistance) {
            double percentalValue = (distance*100)/completeDistance;
            double result = (percentalValue*timeToDistribute)/100;
            skippedStreetnamesTime.add(Math.round(result));
        }
    }

    private void findSkippedStreetnames(String lastName,String currentName, MapMatchingPath path, int index){
        skippedStreetnames.clear();
        skippedStreetnamesDistance.clear();
        for (int i=index; i>=0 ; i--)
        {
            if(SymbolicTrajectory.getStreetname(path.getEdges().get(i)).equals(lastName)){
                double distance = 0.0;
                while(path.getHistoryPerEdge().get(i).size()==0){
                    distance = distance + mapMatching.getUtil().distanceOfEdge(path.getEdges().get(i));
                    i = i-1;
                }
                MapMatchingPathHistoryEntry historyEntry = path.getHistoryPerEdge().get(i).get(path.getHistoryPerEdge().get(i).size()-1);

                for (int j=path.getEdges().get(i).getStartSegmentPoint().size()-1 ; j>=0 ; j--)
                {
                    if(historyEntry.getSegmentIndex()==j){
                        distance = distance + mapMatching.getUtil().distance(historyEntry.getProjectedPoint(),path.getEdges().get(i).getEndSegmentPoint().get(j));
                        break;
                    }
                    else{
                        distance = distance + mapMatching.getUtil().distance(path.getEdges().get(i).getStartSegmentPoint().get(j),path.getEdges().get(i).getEndSegmentPoint().get(j));
                    }
                }
                skippedStreetnamesDistance.add(distance);
                break;
            }
            if(!SymbolicTrajectory.getStreetname(path.getEdges().get(i)).equals(currentName)){
                SymbolicTrajectory skippedStreet = new SymbolicTrajectory(currentStreetname.getEndDate(),SymbolicTrajectory.getStreetname(path.getEdges().get(i)));
                skippedStreetnames.add(skippedStreet);
                skippedStreetnamesDistance.add(mapMatching.getUtil().distanceOfEdge(path.getEdges().get(i)));
                currentName = SymbolicTrajectory.getStreetname(path.getEdges().get(i));
            }
            else{
                if(skippedStreetnamesDistance.size()!=0){
                    double oldValue = skippedStreetnamesDistance.get(skippedStreetnamesDistance.size()-1);
                    double distance = mapMatching.getUtil().distanceOfEdge(path.getEdges().get(i));
                    skippedStreetnamesDistance.set(skippedStreetnamesDistance.size()-1, oldValue + distance);
                }
            }
        }
        Collections.reverse(skippedStreetnames);
        Collections.reverse(skippedStreetnamesDistance);
    }

    private boolean saveSymbolicTrajectory(String type, SymbolicTrajectory symbolicTrajectory){
        if(!update){
            if(type.equals("SymbolicTrajectoriesStreetnames")){
                resultsStreetnames.add(symbolicTrajectory);
            }
            else if(type.equals("SymbolicTrajectoriesCardinalDirections")){
                resultsCardinalDirections.add(symbolicTrajectory);
            }
            else if(type.equals("SymbolicTrajectoriesSpeedProfile")){
                resultsSpeedIntervals.add(symbolicTrajectory);
            }
            else if(type.equals("SymbolicTrajectoriesHeightProfile")){
                resultsHeightIntervals.add(symbolicTrajectory);
            }
        }
        if (directTransmission){
            if(!localMode && transmit){
                if(!transmitSingleResult(type, symbolicTrajectory)){
                    return false;
                }
                if(type.equals("SymbolicTrajectoriesStreetnames")){
                    directTransmittedStreetnames.add(symbolicTrajectory);
                }
                else if(type.equals("SymbolicTrajectoriesCardinalDirections")){
                    directTransmittedCardinalDirections.add(symbolicTrajectory);
                }
                else if(type.equals("SymbolicTrajectoriesSpeedProfile")){
                    directTransmittedSpeedIntervals.add(symbolicTrajectory);
                }
                else if(type.equals("SymbolicTrajectoriesHeightProfile")){
                    directTransmittedHeightIntervals.add(symbolicTrajectory);
                }
            }
        }
        if(localMode&&transmit){
            String entry = "["+ SymbolicTrajectory.getTimeFromDateLongWithTimeZone(symbolicTrajectory.getStartDate())+" - " + SymbolicTrajectory.getTimeFromDateLongWithTimeZone(symbolicTrajectory.getEndDate()) + "] - " + symbolicTrajectory.getLabel();
            if(type.equals("SymbolicTrajectoriesStreetnames")){
                symbolicTrajectoryTextFile.getResultStreetnames().add(entry);
            }
            else if(type.equals("SymbolicTrajectoriesCardinalDirections")){
                symbolicTrajectoryTextFile.getResultCardinalDirections().add(entry);
            }
            else if(type.equals("SymbolicTrajectoriesSpeedProfile")){
                entry = entry.replace("&gt;", ">");
                symbolicTrajectoryTextFile.getResultSpeed().add(entry);
            }
            else if(type.equals("SymbolicTrajectoriesHeightProfile")){
                entry = entry.replace("&lt;", "<");
                symbolicTrajectoryTextFile.getResultHeight().add(entry);
            }
        }
        return true;
    }

    private boolean transmitSingleResult(String type, SymbolicTrajectory symbolicTrajectory){
        if(mapMatching.getSecondoDB().isUseResultServer()){
            if(!mapMatching.getSecondoResultDB().sendSymbolicTrajectory(type, symbolicTrajectory)){return false;}
        }
        else {
            if(!mapMatching.getSecondoDB().sendSymbolicTrajectory(type, symbolicTrajectory)){return false;}
        }
        return true;
    }

    private boolean transmitAllResults(){
        SecondoDB secondo;
        if(mapMatching.getSecondoDB().isUseResultServer()){
            secondo = mapMatching.getSecondoResultDB();
        }
        else {
            secondo = mapMatching.getSecondoDB();
        }
        if(!transmitAllSteetnames(secondo)){
            return false;
        }
        if(!transmitAllCardinalDirections(secondo)){
            return false;
        }
        if(!transmitAllSpeedIntervals(secondo)){
            return false;
        }
        if(!transmitAllHeightIntervals(secondo)){
            return false;
        }
        return true;
    }

    private boolean transmitAllSteetnames(SecondoDB secondo){
        if(resultsStreetnames.size()!=0){
            long timeStartTransmitResults, timeEndTransmitResults, timeForTransmitResults;
            mapMatching.getMapMatchingListener().showResultMessage("Transmitting all Streetnames ...<br>");
            for (SymbolicTrajectory symbolicTrajectory : resultsStreetnames) {
                showStreetnameTransmittedMessage(symbolicTrajectory);
            }
            timeStartTransmitResults = System.currentTimeMillis();
            if(!secondo.sendSymbolicTrajectories("SymbolicTrajectoriesStreetnames", resultsStreetnames)){
                return false;
            }
            timeEndTransmitResults = System.currentTimeMillis();
            timeForTransmitResults = timeEndTransmitResults - timeStartTransmitResults;
            mapMatching.getMapMatchingListener().showLineResultMessage("All Streetnames transmitted ("+(timeForTransmitResults)+" Millisec).");
        }
        return true;
    }
    private boolean transmitAllCardinalDirections(SecondoDB secondo){
        if(resultsCardinalDirections.size()!=0){
            long timeStartTransmitResults, timeEndTransmitResults, timeForTransmitResults;
            mapMatching.getMapMatchingListener().showResultMessage("Transmitting all Cardinal Directions ...<br>");
            for (SymbolicTrajectory symbolicTrajectory : resultsCardinalDirections) {
                showCardinalDirectionTransmittedMessage(symbolicTrajectory);
            }
            timeStartTransmitResults = System.currentTimeMillis();
            if(!secondo.sendSymbolicTrajectories("SymbolicTrajectoriesCardinalDirections", resultsCardinalDirections)){
                return false;
            }
            timeEndTransmitResults = System.currentTimeMillis();
            timeForTransmitResults = timeEndTransmitResults - timeStartTransmitResults;
            mapMatching.getMapMatchingListener().showLineResultMessage("All Cardinal Directions transmitted ("+(timeForTransmitResults)+" Millisec).");
        }
        return true;
    }
    private boolean transmitAllSpeedIntervals(SecondoDB secondo){
        if(resultsSpeedIntervals.size()!=0){
            long timeStartTransmitResults, timeEndTransmitResults, timeForTransmitResults;
            mapMatching.getMapMatchingListener().showResultMessage("Transmitting all Speed Intervals ...<br>");
            for (SymbolicTrajectory symbolicTrajectory : resultsSpeedIntervals) {
                showSpeedIntervalTransmittedMessage(symbolicTrajectory);
            }
            timeStartTransmitResults = System.currentTimeMillis();
            if(!secondo.sendSymbolicTrajectories("SymbolicTrajectoriesSpeedProfile", resultsSpeedIntervals)){
                return false;
            }
            timeEndTransmitResults = System.currentTimeMillis();
            timeForTransmitResults = timeEndTransmitResults - timeStartTransmitResults;
            mapMatching.getMapMatchingListener().showLineResultMessage("All Speed Intervals transmitted ("+(timeForTransmitResults)+" Millisec).");
        }
        return true;
    }
    private boolean transmitAllHeightIntervals(SecondoDB secondo){
        if(resultsHeightIntervals.size()!=0){
            long timeStartTransmitResults, timeEndTransmitResults, timeForTransmitResults;
            mapMatching.getMapMatchingListener().showResultMessage("Transmitting all Height Intervals ...<br>");
            for (SymbolicTrajectory symbolicTrajectory : resultsHeightIntervals) {
                showHeightIntervalTransmittedMessage(symbolicTrajectory);
            }
            timeStartTransmitResults = System.currentTimeMillis();
            if(!secondo.sendSymbolicTrajectories("SymbolicTrajectoriesHeightProfile", resultsHeightIntervals)){
                return false;
            }
            timeEndTransmitResults = System.currentTimeMillis();
            timeForTransmitResults = timeEndTransmitResults - timeStartTransmitResults;
            mapMatching.getMapMatchingListener().showLineResultMessage("All Height Intervals transmitted ("+(timeForTransmitResults)+" Millisec).");
        }
        return true;
    }

    public boolean resultsTransmitted(){
        if(updatedStreetname !=null|| updatedCardinalDirection !=null|| updatedSpeedInterval !=null|| updatedHeightInterval !=null){
            return true;
        }
        return false;
    }

    public void showStreetnameTransmittedMessage(SymbolicTrajectory streetname){
        mapMatching.getMapMatchingListener().showResultMessage("["+ SymbolicTrajectory.getTimeFromDateLong(streetname.getStartDate())+" - " + SymbolicTrajectory.getTimeFromDateLong(streetname.getEndDate()) + "] - " + streetname.getLabel()+"<br>");
    }
    public void showCardinalDirectionTransmittedMessage(SymbolicTrajectory cardinalDirection){
        mapMatching.getMapMatchingListener().showResultMessage("["+ SymbolicTrajectory.getTimeFromDateLong(cardinalDirection.getStartDate())+" - " + SymbolicTrajectory.getTimeFromDateLong(cardinalDirection.getEndDate()) + "] - " + cardinalDirection.getLabel()+"<br>");
    }
    public void showSpeedIntervalTransmittedMessage(SymbolicTrajectory speed){
        mapMatching.getMapMatchingListener().showResultMessage("["+ SymbolicTrajectory.getTimeFromDateLong(speed.getStartDate())+" - " + SymbolicTrajectory.getTimeFromDateLong(speed.getEndDate()) + "] - " + speed.getLabel()+"<br>");
    }
    public void showHeightIntervalTransmittedMessage(SymbolicTrajectory height){
        mapMatching.getMapMatchingListener().showResultMessage("["+ SymbolicTrajectory.getTimeFromDateLong(height.getStartDate())+" - " + SymbolicTrajectory.getTimeFromDateLong(height.getEndDate()) + "] - " + height.getLabel()+"<br>");
    }

    public void setDirectTransmission(boolean directTransmission) {
        this.directTransmission = directTransmission;
    }

    public void setGenerateStreetnames(boolean generateStreetnames) {
        this.generateStreetnames = generateStreetnames;
    }

    public void setGenerateCardinalDirections(boolean generateCardinalDirections) {
        this.generateCardinalDirections = generateCardinalDirections;
    }

    public void setGenerateSpeedProfile(boolean generateSpeedProfile) {
        this.generateSpeedProfile = generateSpeedProfile;
    }

    public void setGenerateHeightProfile(boolean generateHeightProfile) {
        this.generateHeightProfile = generateHeightProfile;
    }

    public boolean isDirectTransmission() {
        return directTransmission;
    }

    public boolean isGenerateStreetnames() {
        return generateStreetnames;
    }

    public boolean isGenerateCardinalDirections() {
        return generateCardinalDirections;
    }

    public boolean isGenerateSpeedProfile() {
        return generateSpeedProfile;
    }

    public boolean isGenerateHeightProfile() {
        return generateHeightProfile;
    }

    public SymbolicTrajectory getUpdatedStreetname() {
        return updatedStreetname;
    }

    public SymbolicTrajectory getUpdatedCardinalDirection() {
        return updatedCardinalDirection;
    }

    public SymbolicTrajectory getUpdatedSpeedInterval() {
        return updatedSpeedInterval;
    }

    public SymbolicTrajectory getUpdatedHeightInterval() {
        return updatedHeightInterval;
    }

    public long getCancelTime() {
        return cancelTime;
    }

    public List<SymbolicTrajectory> getSkippedStreetnames() {
        return skippedStreetnames;
    }

    public void setLocalMode(boolean localMode) {
        this.localMode = localMode;
    }

    public boolean isLocalMode() {
        return localMode;
    }

    public List<SymbolicTrajectory> getResultsStreetnames() {
        return resultsStreetnames;
    }

    public List<SymbolicTrajectory> getResultsCardinalDirections() {
        return resultsCardinalDirections;
    }

    public List<SymbolicTrajectory> getResultsSpeedIntervals() {
        return resultsSpeedIntervals;
    }

    public List<SymbolicTrajectory> getResultsHeightIntervals() {
        return resultsHeightIntervals;
    }
}
