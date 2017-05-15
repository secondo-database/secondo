package de.fernunihagen.dna.mapmatchingcore;

import java.util.ArrayList;
import java.util.List;

import sj.lang.*;

public class SecondoDB {

    MapMatching mapMatching;

    private String serverIp;
    private int serverPort;
    private String serverUsername;
    private String serverPassword;

    private String databaseName;
    private String rTreeName;
    private String edgeIndexName;
    private long timeout;

    private String android_id;
    private boolean matchFootways;
    private boolean useEdgesRelation;

    private boolean useResultServer;
    private boolean resultDB;

    private String roadsOnlyFilter = "filter[(.RoadType = \"motorway\") or (.RoadType = \"trunk\") or (.RoadType = \"primary\") or (.RoadType = \"secondary\") or (.RoadType = \"tertiary\") or (.RoadType = \"unclassified\") or (.RoadType = \"residential\") or (.RoadType = \"service\") or (.RoadType = \"motorway_link\") or (.RoadType = \"trunk_link\") or (.RoadType = \"primary_link\") or (.RoadType = \"secondary_link\") or (.RoadType = \"tertiary_link\") or (.RoadType = \"living_street\") or (.RoadType = \"pedestrian\") or (.RoadType = \"track\") or (.RoadType = \"bus_guideway\") or (.RoadType = \"escape\") or (.RoadType = \"raceway\") or (.RoadType = \"road\")]";
    private String projection = "project[Source, Target, SourcePos, TargetPos, Curve, RoadName, RoadType]";
    private String testBoundingBox = "0.000000000000000 0.000000000000001 0.000000000000000 0.000000000000001";

    private ESInterface secondoInterface = new ESInterface();
    private StringBuffer errorMessage = new StringBuffer();
    private IntByReference errorCode = new IntByReference();
    private IntByReference errorPos = new IntByReference();
    private ListExpr resultList = new ListExpr();

    private Boolean commandSend;
    private Boolean showError = true;

    public SecondoDB(MapMatching mapMatching, boolean resultDB) {
        this.mapMatching = mapMatching;
        this.resultDB = resultDB;
    }

    public boolean initializeConnection() {
        if(!connect()){
            return false;
        }
        if(!isConnected()){
            return false;
        }
        if(resultDB){
            showError = false;
        }
        if(!sendCommand("open database " + databaseName)){
            if(resultDB){
                if(!sendCommand("create database " + databaseName)){
                    showError = true;
                    return false;
                }
                if(!sendCommand("open database " + databaseName)){
                    showError = true;
                    return false;
                }
            }
            else{
                return false;
            }
        }
        if(resultDB){
            showError = true;
        }
        if(!resultDB){
            showError = false;
            if(!queryEdges(testBoundingBox)){
                showError = true;
                mapMatching.getMapMatchingListener().showLineErrorMessage("Database does not contain valid way data.");
                return false;
            }
            showError = true;
        }
        if(useResultServer){
            if(resultDB){
                createResultTables();
            }
        }
        else{
            createResultTables();
        }
        return true;
    }

    private boolean connect(){
        long timeStart = System.currentTimeMillis();
        setCommandSend(false);
        Thread t = new Thread(new Runnable() {
            public void run() {
                secondoInterface = new ESInterface();

                secondoInterface.setHostname(serverIp);
                secondoInterface.setPort(serverPort);

                secondoInterface.setUserName(serverUsername);
                secondoInterface.setPassWd(serverPassword);

                secondoInterface.useBinaryLists(true);
                tools.Environment.MEASURE_TIME = false;

                secondoInterface.connect();
                setCommandSend(true);
            }
        });
        t.start();
        while(!getCommandSend()){
            long time = System.currentTimeMillis() - timeStart;
            if(time>=timeout){
                mapMatching.getMapMatchingListener().showErrorMessage("Secondo Server Timeout.<br>");
                try{
                    t.stop();
                }catch(Exception e){
                    return false;
                }
                return false;
            }
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        return true;
    }

    private void createResultTables(){
        showError = false;
        sendCommand("let SymbolicTrajectoriesStreetnames = [const rel(tuple([DeviceId: string, StartDate: string, StartTime: string, EndDate: string, EndTime: string, StreetName: string])) value ()]");
        sendCommand("let SymbolicTrajectoriesCardinalDirections = [const rel(tuple([DeviceId: string, StartDate: string, StartTime: string, EndDate: string, EndTime: string, CardinalDirection: string])) value ()]");
        sendCommand("let SymbolicTrajectoriesSpeedProfile = [const rel(tuple([DeviceId: string, StartDate: string, StartTime: string, EndDate: string, EndTime: string, Speed: string])) value ()]");
        sendCommand("let SymbolicTrajectoriesHeightProfile = [const rel(tuple([DeviceId: string, StartDate: string, StartTime: string, EndDate: string, EndTime: string, Height: string])) value ()]");

        sendCommand("let MovingLabelStreetnames = [const rel(tuple([DeviceId: string, Date: string, Time: string, StreetName: mlabel])) value ()]");
        sendCommand("let MovingLabelCardinalDirections = [const rel(tuple([DeviceId: string, Date: string, Time: string, CardinalDirection: mlabel])) value ()]");
        sendCommand("let MovingLabelSpeedProfile = [const rel(tuple([DeviceId: string, Date: string, Time: string, Speed: mlabel])) value ()]");
        sendCommand("let MovingLabelHeightProfile = [const rel(tuple([DeviceId: string, Date: string, Time: string, Height: mlabel])) value ()]");

        sendCommand("let MapMatchingMovementData = [const rel(tuple([DeviceId: string, Date: string, Time: string, Movement: mpoint, Trajectory: line])) value ()]");
        showError = true;
    }

    private boolean sendCommand(String command) {
        setCommandSend(false);
        long timeStart = System.currentTimeMillis();
        Thread t = new Thread(new Runnable() {
            public void run() {
                secondoInterface.secondo(command, resultList, errorCode, errorPos, errorMessage);
                setCommandSend(true);
            }
        });
        t.start();

        while(!getCommandSend()){
            long time = System.currentTimeMillis() - timeStart;
            if(time>=timeout){
                mapMatching.getMapMatchingListener().showErrorMessage("Secondo Server Timeout.<br>");
                try{
                    t.stop();
                }catch(Exception e){
                    return false;
                }
                return false;
            }
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (errorCode.value != 0) {
            //System.out.println(command);
            String error = String.valueOf(errorMessage);
            error.trim();
            System.out.println("Error:"+error);
            if(showError&&!error.equals("")){
                mapMatching.getMapMatchingListener().showErrorMessage(error+"<br>");
            }
            return false;
        }
        return true;
    }

    public void terminate() {
        if(secondoInterface!=null){
            secondoInterface.terminate();
        }
    }

    public boolean isConnected(){
        if(secondoInterface!=null){
            return secondoInterface.isConnected();
        }
        return false;
    }

    public boolean queryEdges(String boundingBox) {
        if(useEdgesRelation){
            if (!isMatchFootways()) {
                if(sendCommand("query "+rTreeName+" "+edgeIndexName+" windowintersects[[const rect value (" + boundingBox + ")]] remove[Box] "
                        + "loopsel[Edges orange[.Source, .Target; .Source, .Target]] " + roadsOnlyFilter + " " + projection + " consume")){
                    return true;
                }
            } else {
                if(sendCommand("query "+rTreeName+" "+edgeIndexName+" windowintersects[[const rect value (" + boundingBox + ")]] remove[Box] "
                        + "loopsel[Edges orange[.Source, .Target; .Source, .Target]] " + projection + " consume")){
                    return true;
                }
            }
        }
        else{
            if (!isMatchFootways()) {
                if(sendCommand("query "+rTreeName+" "+edgeIndexName+" windowintersects[[const rect value (" + boundingBox + ")]] remove[Box] "
                        + roadsOnlyFilter + " " + projection + " consume")){
                    return true;
                }
            } else {
                if(sendCommand("query "+rTreeName+" "+edgeIndexName+" windowintersects[[const rect value (" + boundingBox + ")]] remove[Box] "
                        + projection + " consume")){
                    return true;
                }
            }
        }
        return false;
    }

    public boolean sendSymbolicTrajectoriesMovingLabel(String type, List<SymbolicTrajectory> symbolicTrajectories){
        if(symbolicTrajectories.size()!=0){
            StringBuilder valueList = new StringBuilder();
            for (SymbolicTrajectory symbolicTrajectory : symbolicTrajectories) {
                String startTime, endTime, label;
                long startDate = symbolicTrajectory.getStartDate();
                long endDate = symbolicTrajectory.getEndDate();

                startTime = MovementResults.getDateStringUtc(startDate);
                endTime = MovementResults.getDateStringUtc(endDate);
                label = symbolicTrajectory.getLabel();
                label = label.replace("&lt;","<");
                label = label.replace("&gt;",">");

                if(!startTime.equals(endTime) && startDate<endDate){
                    valueList.append("((\""+startTime+"\" \""+endTime+"\" TRUE FALSE) \""+label+"\")");
                }
            }

            long time = symbolicTrajectories.get(0).getStartDate();
            String dateOnly = SymbolicTrajectory.getDateFromDateLong(time);
            String timeOnly = SymbolicTrajectory.getTimeFromDateLongWithTimeZone(time);

            String command = "";
            if(type.equals("SymbolicTrajectoriesStreetnames")){
                command = "query [const rel(tuple ([DeviceId: string, Date: string, Time: string, StreetName: mlabel]))value((\""+android_id+"\" \""+dateOnly+"\" \""+timeOnly+"\" ("+valueList.toString()+") ))] feed MovingLabelStreetnames insert count";
            }
            else if(type.equals("SymbolicTrajectoriesCardinalDirections")){
                command = "query [const rel(tuple ([DeviceId: string, Date: string, Time: string, CardinalDirection: mlabel]))value((\""+android_id+"\" \""+dateOnly+"\" \""+timeOnly+"\" ("+valueList.toString()+") ))] feed MovingLabelCardinalDirections insert count";
            }
            else if(type.equals("SymbolicTrajectoriesSpeedProfile")){
                command = "query [const rel(tuple ([DeviceId: string, Date: string, Time: string, Speed: mlabel]))value((\""+android_id+"\" \""+dateOnly+"\" \""+timeOnly+"\" ("+valueList.toString()+") ))] feed MovingLabelSpeedProfile insert count";
            }
            else if(type.equals("SymbolicTrajectoriesHeightProfile")){
                command = "query [const rel(tuple ([DeviceId: string, Date: string, Time: string, Height: mlabel]))value((\""+android_id+"\" \""+dateOnly+"\" \""+timeOnly+"\" ("+valueList.toString()+") ))] feed MovingLabelHeightProfile insert count";
            }

            if(!sendCommand("\n"+command)){
                return false;
            }
        }
        return true;
    }


    public boolean sendSymbolicTrajectory(String type, SymbolicTrajectory symbolicTrajectory){

        String startDateOnly, startTime, endDateOnly, endTime, label;
        long startDate = symbolicTrajectory.getStartDate();
        long endDate = symbolicTrajectory.getEndDate();
        startDateOnly = SymbolicTrajectory.getDateFromDateLong(startDate);
        startTime = SymbolicTrajectory.getTimeFromDateLongWithTimeZone(startDate);
        endDateOnly = SymbolicTrajectory.getDateFromDateLong(endDate);
        endTime = SymbolicTrajectory.getTimeFromDateLongWithTimeZone(endDate);
        label = symbolicTrajectory.getLabel();
        label = label.replace("&lt;","<");
        label = label.replace("&gt;",">");

        String command = "query " + type + " inserttuple[\"" + android_id + "\", \""+startDateOnly + "\", \"" + startTime + "\", \"" + endDateOnly + "\", \"" + endTime + "\", \"" + label + "\"] count";
        if(!sendCommand(command)){
            return false;
        }
        return true;
    }

    public boolean sendSymbolicTrajectories(String type, List<SymbolicTrajectory> symbolicTrajectories){
        StringBuilder valueList = new StringBuilder();
        for (SymbolicTrajectory symbolicTrajectory : symbolicTrajectories) {
            String startDateOnly, startTime, endDateOnly, endTime, label;
            long startDate = symbolicTrajectory.getStartDate();
            long endDate = symbolicTrajectory.getEndDate();
            startDateOnly = SymbolicTrajectory.getDateFromDateLong(startDate);
            startTime = SymbolicTrajectory.getTimeFromDateLongWithTimeZone(startDate);
            endDateOnly = SymbolicTrajectory.getDateFromDateLong(endDate);
            endTime = SymbolicTrajectory.getTimeFromDateLongWithTimeZone(endDate);
            label = symbolicTrajectory.getLabel();
            label = label.replace("&lt;","<");
            label = label.replace("&gt;",">");

            valueList.append("(\""+android_id+"\" \""+startDateOnly+"\" \""+startTime+"\" \""+endDateOnly+"\" \""+endTime+"\" \""+label+"\")");
        }
        String command = "";
        if(type.equals("SymbolicTrajectoriesStreetnames")){
            command = "query [const rel(tuple ([DeviceId: string, StartDate: string, StartTime: string, EndDate: string, EndTime: string, StreetName: string]))value("+valueList.toString()+")] feed SymbolicTrajectoriesStreetnames insert count";
        }
        else if(type.equals("SymbolicTrajectoriesCardinalDirections")){
            command = "query [const rel(tuple ([DeviceId: string, StartDate: string, StartTime: string, EndDate: string, EndTime: string, CardinalDirection: string]))value("+valueList.toString()+")] feed SymbolicTrajectoriesCardinalDirections insert count";
        }
        else if(type.equals("SymbolicTrajectoriesSpeedProfile")){
            command = "query [const rel(tuple ([DeviceId: string, StartDate: string, StartTime: string, EndDate: string, EndTime: string, Speed: string]))value("+valueList.toString()+")] feed SymbolicTrajectoriesSpeedProfile insert count";
        }
        else if(type.equals("SymbolicTrajectoriesHeightProfile")){
            command = "query [const rel(tuple ([DeviceId: string, StartDate: string, StartTime: string, EndDate: string, EndTime: string, Height: string]))value("+valueList.toString()+")] feed SymbolicTrajectoriesHeightProfile insert count";
        }
        if(!sendCommand(command)){
            return false;
        }

        if(!sendSymbolicTrajectoriesMovingLabel(type,symbolicTrajectories)){
            return false;
        }

        return true;
    }

    public boolean sendMovementData(List<MovementResult> movementResults,List<MovingPointWaypoint> movingPointWaypoints, long time){
        StringBuilder mPointValues = new StringBuilder();
        StringBuilder lineValues = new StringBuilder();

        Point lastEndPoint = null;
        for (MovementResult movementResult : movementResults) {
            for(int i=0;i<movementResult.getStartSegmentPoints().size();i++){
                Point startSegmentPoint = movementResult.getStartSegmentPoints().get(i);
                Point endSegmentPoint = movementResult.getEndSegmentPoints().get(i);
                if(lastEndPoint != null&&!startSegmentPoint.equals(lastEndPoint)){
                    lineValues.append( "("+lastEndPoint.getLongitude()+" "+lastEndPoint.getLatitude()+" "+startSegmentPoint.getLongitude()+" "+startSegmentPoint.getLatitude()+")" );
                }
                lastEndPoint = movementResult.getEndSegmentPoints().get(i);
                lineValues.append( "("+startSegmentPoint.getLongitude()+" "+startSegmentPoint.getLatitude()+" "+endSegmentPoint.getLongitude()+" "+endSegmentPoint.getLatitude()+")" );
            }
        }

        for (MovingPointWaypoint waypoint : movingPointWaypoints){
            String startDate = waypoint.getStartTime();
            String endDate = waypoint.getEndTime();
            Point startPoint = waypoint.getStartPoint();
            Point endPoint = waypoint.getEndPoint();
            mPointValues.append("((\""+startDate+"\" \""+endDate+"\" TRUE FALSE)("+startPoint.getLongitude()+" "+startPoint.getLatitude()+" "+endPoint.getLongitude()+" "+endPoint.getLatitude()+"))");
        }

        String dateOnly = SymbolicTrajectory.getDateFromDateLong(time);
        String timeOnly = SymbolicTrajectory.getTimeFromDateLongWithTimeZone(time);

        String command = "query [const rel(tuple ([DeviceId: string, Date: string, Time: string, Movement: mpoint, Trajectory: line]))value((\""+android_id+"\" \""+dateOnly+"\" \""+timeOnly+"\" ("+mPointValues.toString()+") ("+lineValues.toString()+") ))] feed MapMatchingMovementData insert count";
        if(!sendCommand(command)){
            return false;
        }
        return true;
    }

    public void setrTreeName(String rTreeName) {
        this.rTreeName = rTreeName;
    }

    public void setEdgeIndexName(String edgeIndexName) {
        this.edgeIndexName = edgeIndexName;
    }

    public ListExpr getResultList() {
        return resultList;
    }

    public void setServerIp(String serverIp) {
        this.serverIp = serverIp;
    }

    public void setServerPort(int serverPort) {
        this.serverPort = serverPort;
    }

    public void setDatabaseName(String databaseName) {
        this.databaseName = databaseName;
    }

    public void setServerUsername(String serverUsername) {
        this.serverUsername = serverUsername;
    }

    public void setServerPassword(String serverPassword) {
        this.serverPassword = serverPassword;
    }

    public synchronized void setMatchFootways(boolean matchFootways) {
        this.matchFootways = matchFootways;
    }

    public void setAndroid_id(String android_id) {
        this.android_id = android_id;
    }

    public synchronized boolean isMatchFootways() {
        return matchFootways;
    }

    public synchronized Boolean getCommandSend() {
        return commandSend;
    }

    public synchronized void setCommandSend(Boolean commandSend) {
        this.commandSend = commandSend;
    }

    public void setTimeout(long timeout) {
        this.timeout = timeout;
    }

    public void setUseEdgesRelation(boolean useEdgesRelation) {
        this.useEdgesRelation = useEdgesRelation;
    }

    public boolean isUseResultServer() {
        return useResultServer;
    }

    public void setUseResultServer(boolean useResultServer) {
        this.useResultServer = useResultServer;
    }

}