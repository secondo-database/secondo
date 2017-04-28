package de.fernunihagen.dna.mapmatchingcore;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

public class MovementResults {

    MapMatching mapMatching;
    private List<List<MovementResult>> allMovementResults = new ArrayList<List<MovementResult>>();
    private List<List<MovingPointWaypoint>> allMovingPointWaypoints = new ArrayList <List<MovingPointWaypoint>>();

    private List<MovementResult> movementResults;
    private List<MovingPointWaypoint> movingPointWaypoints;

    private boolean transmitMovementData;
    private boolean preciseMovingPoint;

    long timeToDistribute;
    double relativeDistance;

    public MovementResults(MapMatching mapMatching){
        this.mapMatching = mapMatching;
    }

    public void generate(List<MapMatchingPath> paths){
        allMovementResults.clear();
        allMovingPointWaypoints.clear();

        for(MapMatchingPath path : paths){
            movementResults = new ArrayList<MovementResult>();
            if(path.getEdges().size()!=0){
                if(path.getEdges().size()!=1){
                    generateFirstMovementResult(path);
                    generateMovementResults(path);
                    generateLastMovementResult(path);

                    generateSegmentTimings(path);
                    generateMovingPointWaypoints();

                    allMovementResults.add(movementResults);
                }
                else{
                    mapMatching.getMapMatchingListener().showLineInfoMessage("A Movement Result was skipped. Generating a Movement Results requires a path with at least two edges.");
                }
            }
        }
    }

    private void generateFirstMovementResult(MapMatchingPath path){
        MovementResult firstMovementResult = new MovementResult(path.getEdges().get(0).getSourceID(), path.getEdges().get(0).getTargetID(),path.getEdges().get(0).getStartSegmentPoint(),path.getEdges().get(0).getEndSegmentPoint(),path.getHistoryPerEdge().get(0));
        long sourceTime = path.getHistoryPerEdge().get(0).get(0).getLocationPoint().getTime();

        int segmentIndex =  path.getHistoryPerEdge().get(0).get(0).getSegmentIndex();
        List<Point> startSegmentPointsFromFirstPoint = firstMovementResult.getStartSegmentPoints();
        List<Point> endSegmentPointsFromFirstPoint = firstMovementResult.getEndSegmentPoints();

        startSegmentPointsFromFirstPoint = startSegmentPointsFromFirstPoint.subList(segmentIndex,startSegmentPointsFromFirstPoint.size());
        endSegmentPointsFromFirstPoint = endSegmentPointsFromFirstPoint.subList(segmentIndex,endSegmentPointsFromFirstPoint.size());

        if(!path.getHistoryPerEdge().get(0).get(0).getProjectedPoint().equals(endSegmentPointsFromFirstPoint.get(0))){
            startSegmentPointsFromFirstPoint.set(0,path.getHistoryPerEdge().get(0).get(0).getProjectedPoint());
        }
        else{
            startSegmentPointsFromFirstPoint.remove(0);
            endSegmentPointsFromFirstPoint.remove(0);
        }

        firstMovementResult.setStartSegmentPoints(startSegmentPointsFromFirstPoint);
        firstMovementResult.setEndSegmentPoints(endSegmentPointsFromFirstPoint);

        firstMovementResult.setSourceIDTime(sourceTime);

        if(path.getEdges().size()>1){
            if(path.getEdges().get(0).getSourceID()==path.getEdges().get(1).getSourceID()){
                generateUTurnMovementResult(firstMovementResult,path.getEdges().get(0));
            }
        }
        movementResults.add(firstMovementResult);
    }

    private void generateMovementResults(MapMatchingPath path){
        int skippedEdges  = 0;
        for(int i=1; i<path.getEdges().size(); i++){

            if(path.getHistoryPerEdge().get(i).size()!=0){
                long startTime = movementResults.get(i-1-skippedEdges).getSourceIDTime();
                long endTime = calcTimeFromFirstProjectedPointToEdgeStart(path,i,startTime);
                movementResults.get(i-1-skippedEdges).setTargetIDTime(endTime);
                MovementResult movementResult = new MovementResult(path.getEdges().get(i).getSourceID(), path.getEdges().get(i).getTargetID(),path.getEdges().get(i).getStartSegmentPoint(), path.getEdges().get(i).getEndSegmentPoint(),path.getHistoryPerEdge().get(i));
                movementResult.setSourceIDTime(endTime);
                if(path.getEdges().size()-1!=i){
                    if(path.getEdges().get(i).getSourceID()==path.getEdges().get(i+1).getSourceID()){
                        generateUTurnMovementResult(movementResult,path.getEdges().get(i));
                    }
                }
                movementResults.add(movementResult);
            }
            else{
                long startTime = movementResults.get(i-1-skippedEdges).getSourceIDTime();
                long endTime = calcTimeOfEdgesWithoutPoint(path,i,startTime);
                movementResults.get(i-1-skippedEdges).setTargetIDTime(endTime);
                MovementResult movementResult = new MovementResult(path.getEdges().get(i).getSourceID(), path.getEdges().get(i).getTargetID(), path.getEdges().get(i).getStartSegmentPoint(), path.getEdges().get(i).getEndSegmentPoint(), path.getHistoryPerEdge().get(i));
                movementResult.setSourceIDTime(endTime);
                if(path.getEdges().size()-1!=i){
                    if(path.getEdges().get(i).getSourceID()==path.getEdges().get(i+1).getSourceID()){
                        skippedEdges = skippedEdges + 1;
                        continue;
                    }
                }
                movementResults.add(movementResult);
            }
        }
    }

    private void generateLastMovementResult(MapMatchingPath path){
        MovementResult lastMovementResult = movementResults.get(movementResults.size()-1);
        List<MapMatchingPathHistoryEntry> lastHistory = path.getHistoryPerEdge().get(path.getHistoryPerEdge().size()-1);
        if (lastMovementResult.getSourceIDTime()<lastHistory.get(lastHistory.size()-1).getLocationPoint().getTime()){
            lastMovementResult.setTargetIDTime(lastHistory.get(lastHistory.size()-1).getLocationPoint().getTime());
        }
        else {
            lastMovementResult.setTargetIDTime(lastMovementResult.getSourceIDTime());
        }
        int lastPointSegmentIndex = lastHistory.get(lastHistory.size()-1).getSegmentIndex();
        List<Point> startSegmentPointsToLastPoint = lastMovementResult.getStartSegmentPoints();
        List<Point> endSegmentPointsToLastPoint = lastMovementResult.getEndSegmentPoints();

        startSegmentPointsToLastPoint = startSegmentPointsToLastPoint.subList(0,lastPointSegmentIndex+1);
        endSegmentPointsToLastPoint = endSegmentPointsToLastPoint.subList(0,lastPointSegmentIndex+1);

        if(!lastHistory.get(lastHistory.size()-1).getProjectedPoint().equals(startSegmentPointsToLastPoint.get(startSegmentPointsToLastPoint.size()-1))){
            endSegmentPointsToLastPoint.set(endSegmentPointsToLastPoint.size()-1,lastHistory.get(lastHistory.size()-1).getProjectedPoint());
        }
        else{
            startSegmentPointsToLastPoint.remove(startSegmentPointsToLastPoint.size()-1);
            endSegmentPointsToLastPoint.remove(endSegmentPointsToLastPoint.size()-1);
        }

        lastMovementResult.setStartSegmentPoints(startSegmentPointsToLastPoint);
        lastMovementResult.setEndSegmentPoints(endSegmentPointsToLastPoint);
    }

    private void generateUTurnMovementResult(MovementResult movementResult, NetworkEdge edge){
        MapMatchingPathHistoryEntry turnPoint = mapMatching.getUtil().getLastProjectedPointOnEdge(edge, movementResult.getEdgeHistory());
        int turnSegmentIndex = turnPoint.getSegmentIndex();

        movementResult.setStartSegmentPoints(movementResult.getStartSegmentPoints().subList(0,turnSegmentIndex+1));
        movementResult.setEndSegmentPoints(movementResult.getEndSegmentPoints().subList(0,turnSegmentIndex+1));

        movementResult.getEndSegmentPoints().set(turnSegmentIndex, turnPoint.getProjectedPoint());

        List<Point> wayBackStartPoints = new ArrayList<Point>(movementResult.getEndSegmentPoints());
        List<Point> wayBackEndPoints = new ArrayList<Point>(movementResult.getStartSegmentPoints());

        Collections.reverse(wayBackStartPoints);
        Collections.reverse(wayBackEndPoints);

        movementResult.getStartSegmentPoints().addAll(wayBackStartPoints);
        movementResult.getEndSegmentPoints().addAll(wayBackEndPoints);

        movementResult.setuTurnResult(true);
    }

    private void generateMovingPointWaypoints(){
        movingPointWaypoints = new ArrayList<MovingPointWaypoint>();
        for(MovementResult movementResult : movementResults){
            String startDate = getDateStringUtc(movementResult.getSourceIDTime());
            String endDate = getDateStringUtc(movementResult.getTargetIDTime());
            if(!startDate.equals(endDate)){
                for(int i=0; i<movementResult.getStartSegmentPoints().size();i++){
                    String startSegmentDate = getDateStringUtc(movementResult.getStartSegmentTime().get(i));
                    String endSegmentDate = getDateStringUtc(movementResult.getEndSegmentTime().get(i));
                    if(!startSegmentDate.equals(endSegmentDate)){
                        if(movingPointWaypoints.size()!=0){
                            MovingPointWaypoint currentWaypoint = movingPointWaypoints.get(movingPointWaypoints.size()-1);
                            MovingPointWaypoint waypoint = new MovingPointWaypoint();
                            waypoint.setStartTime(currentWaypoint.getEndTime());
                            waypoint.setStartPoint(currentWaypoint.getEndPoint());
                            waypoint.setEndTime(getDateStringUtc(movementResult.getEndSegmentTime().get(i)));
                            waypoint.setEndPoint(movementResult.getEndSegmentPoints().get(i));
                            if(!waypoint.getStartPoint().equals(waypoint.getEndPoint()) && getLongFromDateStringUtc(waypoint.getStartTime()) < getLongFromDateStringUtc(waypoint.getEndTime())){
                                movingPointWaypoints.add(waypoint);
                            }
                        }
                        else{
                            MovingPointWaypoint firstWaypoint = new MovingPointWaypoint();
                            firstWaypoint.setStartPoint(movementResult.getStartSegmentPoints().get(0));
                            firstWaypoint.setStartTime(getDateStringUtc(movementResult.getStartSegmentTime().get(i)));
                            firstWaypoint.setEndPoint(movementResult.getEndSegmentPoints().get(i));
                            firstWaypoint.setEndTime(getDateStringUtc(movementResult.getEndSegmentTime().get(i)));
                            if(!firstWaypoint.getStartPoint().equals(firstWaypoint.getEndPoint()) && getLongFromDateStringUtc(firstWaypoint.getStartTime()) < getLongFromDateStringUtc(firstWaypoint.getEndTime())){
                                movingPointWaypoints.add(firstWaypoint);
                            }
                        }
                    }
                }
            }
        }
        allMovingPointWaypoints.add(movingPointWaypoints);
    }

    private void generateSegmentTimings(MapMatchingPath path){
        for(int i=0;i<movementResults.size();i++){
            MovementResult movementResult = movementResults.get(i);

            if(preciseMovingPoint){
                if(movementResult.getEdgeHistory().size()==0){
                    distributeSegmentTimingsToDistance(movementResult);
                }
                else{
                    if(movementResult.getStartSegmentPoints().size()>1 && !movementResult.isuTurnResult() && i!=0){
                        calculateSegmentTimings(movementResult);
                    }
                    else {
                        distributeSegmentTimingsToDistance(movementResult);
                    }
                }
                if(!movementResult.isuTurnResult()&&i!=0){
                    insertNewSegments(movementResult);
                }
            }
            else{
                distributeSegmentTimingsToDistance(movementResult);
            }
        }
    }

    private void distributeSegmentTimingsToDistance(MovementResult movementResult){
        long timeToDistribute = movementResult.getTargetIDTime() - movementResult.getSourceIDTime();
        double completeDistance = mapMatching.getUtil().distanceOfMovementResult(movementResult);
        for (int j=0; j<movementResult.getStartSegmentPoints().size(); j++) {
            double distance = mapMatching.getUtil().distance(movementResult.getStartSegmentPoints().get(j), movementResult.getEndSegmentPoints().get(j));
            double percentalValue = (distance*100)/completeDistance;
            double resultTime = (percentalValue*timeToDistribute)/100;
            if(j==0){
                movementResult.getStartSegmentTime().add(movementResult.getSourceIDTime());
            }
            else{
                movementResult.getStartSegmentTime().add(movementResult.getEndSegmentTime().get(j-1));
            }
            if(j==movementResult.getStartSegmentPoints().size()-1){
                movementResult.getEndSegmentTime().add(movementResult.getTargetIDTime());
            }
            else{
                movementResult.getEndSegmentTime().add(Math.round(movementResult.getStartSegmentTime().get(j)+resultTime));
            }
        }
    }

    private void calculateSegmentTimings(MovementResult movementResult){
        List<MapMatchingPathHistoryEntry> history = movementResult.getEdgeHistory();
        int historyIndex = 0;

        while(history.size()-1!=historyIndex && history.get(historyIndex).getSegmentIndex()>=history.get(historyIndex+1).getSegmentIndex()){
            historyIndex = historyIndex + 1;
        }
        timeToDistribute = history.get(historyIndex).getLocationPoint().getTime()-movementResult.getSourceIDTime();
        relativeDistance = distanceFromStartToPoint(movementResult, historyIndex);

        for(int i=0;i<movementResult.getStartSegmentPoints().size();i++){

            if(i==history.get(historyIndex).getSegmentIndex()) {

                int oldHistoryIndex = historyIndex;
                while(history.size()-1!=historyIndex && history.get(historyIndex).getSegmentIndex()>=history.get(historyIndex+1).getSegmentIndex()){
                    historyIndex = historyIndex + 1;
                }

                if(history.size()-1!=oldHistoryIndex){
                    segmentWithLocationPoint(movementResult,i,history,historyIndex,oldHistoryIndex);
                }
                else{
                    segmentWithLastLocationPoint(movementResult,i,history,historyIndex);
                }
            }
            else{
                segmentWithoutLocationPoint(movementResult,i);
            }
        }
    }

    private void segmentWithLocationPoint(MovementResult movementResult, int i, List<MapMatchingPathHistoryEntry> history, int historyIndex, int oldHistoryIndex){
        long timeFromStartToNewPoint = history.get(historyIndex).getLocationPoint().getTime()-movementResult.getSourceIDTime();
        long timeFromStartToRecentPoint = history.get(oldHistoryIndex).getLocationPoint().getTime()-movementResult.getSourceIDTime();
        timeToDistribute = timeFromStartToNewPoint - timeFromStartToRecentPoint;
        relativeDistance = distanceToNextProjectedPoint(movementResult, oldHistoryIndex, historyIndex);

        double distance = mapMatching.getUtil().distance(history.get(oldHistoryIndex).getProjectedPoint(), movementResult.getEndSegmentPoints().get(i));
        double percentalValue = (distance*100)/relativeDistance;
        double resultTime = (percentalValue*timeToDistribute)/100;

        if(i==0){
            movementResult.getStartSegmentTime().add(movementResult.getSourceIDTime());
        }
        else{
            movementResult.getStartSegmentTime().add(movementResult.getEndSegmentTime().get(i-1));
        }
        if(i==movementResult.getStartSegmentPoints().size()-1){
            movementResult.getEndSegmentTime().add(movementResult.getTargetIDTime());
        }
        else{
            long result = Math.round(history.get(oldHistoryIndex).getLocationPoint().getTime()+resultTime);
            if(result<movementResult.getSourceIDTime()){
                movementResult.getEndSegmentTime().add(movementResult.getSourceIDTime());
            }
            else{
                if(result>movementResult.getTargetIDTime()){
                    movementResult.getEndSegmentTime().add(movementResult.getTargetIDTime());
                }
                else {
                    movementResult.getEndSegmentTime().add(result);
                }
            }
        }
    }

    private void segmentWithoutLocationPoint(MovementResult movementResult, int i){
        double distance = mapMatching.getUtil().distance(movementResult.getStartSegmentPoints().get(i), movementResult.getEndSegmentPoints().get(i));
        double percentalValue = (distance*100)/relativeDistance;
        double resultTime = (percentalValue*timeToDistribute)/100;

        if(i==0){
            movementResult.getStartSegmentTime().add(movementResult.getSourceIDTime());
        }
        else{
            movementResult.getStartSegmentTime().add(movementResult.getEndSegmentTime().get(i-1));
        }
        if(i==movementResult.getStartSegmentPoints().size()-1){
            movementResult.getEndSegmentTime().add(movementResult.getTargetIDTime());
        }
        else{
            long result = Math.round(movementResult.getStartSegmentTime().get(i)+resultTime);
            if(result<movementResult.getSourceIDTime()){
                movementResult.getEndSegmentTime().add(movementResult.getSourceIDTime());
            }
            else{
                if(result>movementResult.getTargetIDTime()){
                    movementResult.getEndSegmentTime().add(movementResult.getTargetIDTime());
                }
                else {
                    movementResult.getEndSegmentTime().add(result);
                }
            }
        }
    }

    private void segmentWithLastLocationPoint(MovementResult movementResult, int i, List<MapMatchingPathHistoryEntry> history, int historyIndex){
        timeToDistribute = movementResult.getTargetIDTime() - history.get(historyIndex).getLocationPoint().getTime();
        relativeDistance = distanceFromPointToEnd(movementResult,historyIndex);

        double distance = mapMatching.getUtil().distance(history.get(historyIndex).getProjectedPoint(), movementResult.getEndSegmentPoints().get(i));
        double percentalValue = (distance*100)/relativeDistance;
        double resultTime = (percentalValue*timeToDistribute)/100;

        if(i==0){
            movementResult.getStartSegmentTime().add(movementResult.getSourceIDTime());
        }
        else{
            movementResult.getStartSegmentTime().add(movementResult.getEndSegmentTime().get(i-1));
        }
        if(i==movementResult.getStartSegmentPoints().size()-1){
            movementResult.getEndSegmentTime().add(movementResult.getTargetIDTime());
        }
        else{
            long result = Math.round(history.get(historyIndex).getLocationPoint().getTime()+resultTime);
            if(result<movementResult.getSourceIDTime()){
                movementResult.getEndSegmentTime().add(movementResult.getSourceIDTime());
            }
            else{
                if(result>movementResult.getTargetIDTime()){
                    movementResult.getEndSegmentTime().add(movementResult.getTargetIDTime());
                }
                else {
                    movementResult.getEndSegmentTime().add(result);
                }
            }
        }
    }

    private void insertNewSegments(MovementResult movementResult){
        List<MapMatchingPathHistoryEntry> history = movementResult.getEdgeHistory();
        List<MapMatchingPathHistoryEntry> pointsToAdd = new ArrayList<MapMatchingPathHistoryEntry>();

        int currentSegment = 0;
        for(int i = 0; i<history.size(); i++){
            if(history.get(i).getSegmentIndex()>=currentSegment){
                pointsToAdd.add(history.get(i));
                currentSegment = history.get(i).getSegmentIndex();
            }
        }

        int addSegmentCounter = 0;
        for(int i=0;i<pointsToAdd.size();i++){
            int index = pointsToAdd.get(i).getSegmentIndex() + addSegmentCounter;
            Point pointToAdd =pointsToAdd.get(i).getProjectedPoint();
            long newTime = pointsToAdd.get(i).getLocationPoint().getTime();

            if(!pointToAdd.equals(movementResult.getStartSegmentPoints().get(index)) && !pointToAdd.equals(movementResult.getEndSegmentPoints().get(index))){
                Point oldEndPoint = movementResult.getEndSegmentPoints().get(index);
                long oldEndTime = movementResult.getEndSegmentTime().get(index);
                long oldStartTime = movementResult.getStartSegmentTime().get(index);

                if(newTime>oldStartTime && newTime<oldEndTime){
                    movementResult.getEndSegmentPoints().set(index, pointToAdd);
                    movementResult.getEndSegmentTime().set(index, newTime);

                    movementResult.getStartSegmentPoints().add(index+1,pointToAdd);
                    movementResult.getStartSegmentTime().add(index+1,newTime);
                    movementResult.getEndSegmentPoints().add(index+1,oldEndPoint);
                    movementResult.getEndSegmentTime().add(index+1,oldEndTime);

                    addSegmentCounter = addSegmentCounter + 1;
                }
            }
        }
    }

    private double distanceFromStartToPoint(MovementResult movementResult, int historyIndex){
        List<MapMatchingPathHistoryEntry> history = movementResult.getEdgeHistory();
        double distance = 0;
        int segmentIndex = history.get(historyIndex).getSegmentIndex();

        for(int i=0;i<movementResult.getStartSegmentPoints().size();i++){
            if(i==segmentIndex){
                distance = distance + mapMatching.getUtil().distance(movementResult.getStartSegmentPoints().get(i), history.get(historyIndex).getProjectedPoint());
                break;
            }
            else {
                distance = distance + mapMatching.getUtil().distance(movementResult.getStartSegmentPoints().get(i),movementResult.getEndSegmentPoints().get(i));
            }
        }
        return distance;
    }

    private double distanceFromPointToEnd(MovementResult movementResult, int historyIndex){
        List<MapMatchingPathHistoryEntry> history = movementResult.getEdgeHistory();
        double distance = 0;
        int segmentIndex = history.get(historyIndex).getSegmentIndex();

        for(int i=0;i<movementResult.getStartSegmentPoints().size();i++){
            if(i==segmentIndex){
                distance = distance + mapMatching.getUtil().distance(history.get(historyIndex).getProjectedPoint(), movementResult.getEndSegmentPoints().get(i));
            }
            else if(i>segmentIndex) {
                distance = distance + mapMatching.getUtil().distance(movementResult.getStartSegmentPoints().get(i),movementResult.getEndSegmentPoints().get(i));
            }
        }
        return distance;
    }

    private double distanceToNextProjectedPoint(MovementResult movementResult, int oldHistoryIndex, int newHistoryIndex){
        List<MapMatchingPathHistoryEntry> history = movementResult.getEdgeHistory();
        double distance = 0;
        int segmentIndexInitialPoint = history.get(oldHistoryIndex).getSegmentIndex();
        int segmentIndexNextPoint = history.get(newHistoryIndex).getSegmentIndex();

        for(int i=0;i<movementResult.getStartSegmentPoints().size();i++){
            if(i==segmentIndexInitialPoint){
                distance = distance + mapMatching.getUtil().distance(history.get(oldHistoryIndex).getProjectedPoint(), movementResult.getEndSegmentPoints().get(i));
            }
            if(i==segmentIndexNextPoint){
                distance = distance + mapMatching.getUtil().distance(movementResult.getStartSegmentPoints().get(i), history.get(newHistoryIndex).getProjectedPoint());
            }
            if(i>segmentIndexInitialPoint&&i<segmentIndexNextPoint){
                distance = distance + mapMatching.getUtil().distance(movementResult.getStartSegmentPoints().get(i),movementResult.getEndSegmentPoints().get(i));
            }
        }
        return distance;
    }

    private long calcTimeOfEdgesWithoutPoint(MapMatchingPath path, int index, long startTime){
        //find next edge with  point
        int counter = 1;
        while(!(path.getHistoryPerEdge().get(index+counter).size()!=0)){
            counter = counter + 1;
        }
        long time = calcTimeFromFirstProjectedPointToEdgeStart(path,index+counter,startTime);
        counter = counter - 1;

        double partDistance = 0;
        while(counter>=0){
            partDistance = partDistance + mapMatching.getUtil().distanceOfEdge(path.getEdges().get(index+counter));
            counter = counter - 1;
        }
        double distance = partDistance + mapMatching.getUtil().distanceOfEdge(path.getEdges().get(index-1));

        long timeToDistribute = time - startTime;
        double percentalValue = (partDistance*100)/distance;
        double resultTime = (percentalValue*timeToDistribute)/100;

        long calcTime = Math.round(time-resultTime);

        if(calcTime<startTime){
            return startTime;
        }
        return calcTime;
    }

    private long calcTimeFromFirstProjectedPointToEdgeStart(MapMatchingPath path, int index, long startTime){
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
            return timeOfCurrentPoint;
        }
        long timeToDistribute = timeOfCurrentPoint - timeOfPreviousPoint;
        double percentalValue = (partDistance*100)/distance;
        double resultTime = (percentalValue*timeToDistribute)/100;

        long calcTime = Math.round(timeOfCurrentPoint-resultTime);

        if(calcTime<startTime){
            return timeOfCurrentPoint;
        }
        return calcTime;
    }

    public boolean transmitResults(){
        if(allMovementResults.size()!=0){
            SecondoDB secondo;
            if(mapMatching.getSecondoDB().isUseResultServer()){
                secondo = mapMatching.getSecondoResultDB();
            }
            else {
                secondo = mapMatching.getSecondoDB();
            }
            long startTime,endTime;
            mapMatching.getMapMatchingListener().showResultMessage("Transmitting Movement Data ...<br>");
            startTime = System.currentTimeMillis();

            for(int i=0;i<allMovementResults.size();i++){
//                int indexOfLastMovementResult = allMovementResults.get(i).size()-1;
//                List<MapMatchingPathHistoryEntry> history = allMovementResults.get(i).get(indexOfLastMovementResult).getEdgeHistory();
//                long timeOfLastPoint = history.get(history.size()-1).getLocationPoint().getTime();
                long time = allMovementResults.get(i).get(0).getEdgeHistory().get(0).getLocationPoint().getTime();
                if(!secondo.sendMovementData(allMovementResults.get(i),allMovingPointWaypoints.get(i),time)){
                    return false;
                }
            }
            endTime = System.currentTimeMillis();
            long time = endTime - startTime;
            mapMatching.getMapMatchingListener().showLineResultMessage("Movement Data transmitted ("+(time)+" Millisec).");
        }

        for (MapMatchingPath path : mapMatching.getAllResults()) {
            path.setMovementResultsTransmitted(true);
        }
        return true;
    }

    public static String getDateStringUtc(long milliSeconds){
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss.SSS");
        if(SymbolicTrajectory.isUseUtcTime()){
            dateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
        }
        Date date = new Date(milliSeconds);
        return dateFormat.format(date);
    }

    public static long getLongFromDateStringUtc(String dateString){
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss.SSS");
        Date date = new Date();
        try {
            date = dateFormat.parse(dateString);
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return date.getTime();
    }

    public void print(){
        for (MovementResult movementResult : movementResults) {
            System.out.println("-----------------------------------------------------------------");
            System.out.println("Source ID: "+ movementResult.getSourceID());
            System.out.println("Target ID: "+ movementResult.getTargetID());
            System.out.println("Start Time: "+getDateStringUtc(movementResult.getSourceIDTime()));
            System.out.println("End Time: "+getDateStringUtc(movementResult.getTargetIDTime()));
            System.out.println("Curve:");

            for(int i=0;i<movementResult.getStartSegmentPoints().size();i++){

                System.out.println(getDateStringUtc(movementResult.getStartSegmentTime().get(i))+" - "+getDateStringUtc(movementResult.getEndSegmentTime().get(i)));
                System.out.println(movementResult.getStartSegmentPoints().get(i).getLatitude()+" "+movementResult.getStartSegmentPoints().get(i).getLongitude()+" - "+movementResult.getEndSegmentPoints().get(i).getLatitude()+" "+movementResult.getEndSegmentPoints().get(i).getLongitude());
            }

            System.out.println("-----------------------------------------------------------------");
        }
    }

    public void printMovingPoint(){
        for (MovingPointWaypoint waypoint : movingPointWaypoints) {
            System.out.println("-----------------------------------------------------------------");
            System.out.println(waypoint.getStartTime()+" - "+waypoint.getEndTime());
            System.out.println(waypoint.getStartPoint().getLatitude()+" "+waypoint.getStartPoint().getLongitude()+" - "+waypoint.getEndPoint().getLatitude()+" "+waypoint.getEndPoint().getLongitude());
            System.out.println("-----------------------------------------------------------------");
        }

//        System.out.println("Sline:");
//        for (MovementResult movementResult : movementResults) {
//            System.out.println("-----------------------------------------------------------------");
//            for(int i=0;i<movementResult.getStartSegmentPoints().size();i++){
//                Point startSegmentPoint = movementResult.getStartSegmentPoints().get(i);
//                Point endSegmentPoint = movementResult.getEndSegmentPoints().get(i);
//                System.out.println(startSegmentPoint.getLatitude()+" "+startSegmentPoint.getLongitude()+" - "+endSegmentPoint.getLatitude()+" "+endSegmentPoint.getLongitude() );
//            }
//            System.out.println("-----------------------------------------------------------------");
//        }
    }

    public void setTransmitMovementData(boolean transmitMovementData) {
        this.transmitMovementData = transmitMovementData;
    }

    public boolean isTransmitMovementData() {
        return transmitMovementData;
    }

    public void setPreciseMovingPoint(boolean preciseMovingPoint) {
        this.preciseMovingPoint = preciseMovingPoint;
    }
}
