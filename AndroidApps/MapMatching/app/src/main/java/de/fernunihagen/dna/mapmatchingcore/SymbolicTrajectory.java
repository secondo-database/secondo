package de.fernunihagen.dna.mapmatchingcore;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

public class SymbolicTrajectory {

    private static boolean calcStreetnameTime;
    private static boolean useUtcTime;

    private static int heightIntervalRange;
    private static boolean useSpeedIntervalRange;
    private static int speedIntervalRange;
    private static int speedIntervalEndpoint1;
    private static int speedIntervalEndpoint2;
    private static int speedIntervalEndpoint3;
    private static int speedIntervalEndpoint4;
    private static int speedIntervalEndpoint5;
    private static int speedIntervalEndpoint6;
    private static int speedIntervalEndpoint7;
    private static int speedIntervalEndpoint8;
    private static int speedIntervalEndpoint9;
    private static int speedIntervalEndpoint10;

    private String label;
    private long startDate, endDate;

    public SymbolicTrajectory(long startDate, String label) {
        this.startDate = startDate;
        this.label = label;
    }

    public static String getTimeFromDateLong(long dateLong){
        SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss");
        if(useUtcTime){
            timeFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
        }
        Date date = new Date(dateLong);
        return timeFormat.format(date);
    }

    public static String getTimeFromDateLongWithTimeZone(long dateLong){
        SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss z", Locale.ENGLISH);
        if(useUtcTime){
            timeFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
        }
        Date date = new Date(dateLong);
        return timeFormat.format(date);
    }

    public static String getDateFromDateLong(long dateLong){
        SimpleDateFormat dateOnlyFormat = new SimpleDateFormat("dd/MM/yyyy");
        if(useUtcTime){
            dateOnlyFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
        }
        Date date = new Date(dateLong);
        return dateOnlyFormat.format(date);
    }

    public static String getStreetname(NetworkEdge edge){
        if(edge.getRoadName().equals("undefined")){
            return edge.getRoadType();
        }
        else{
            return edge.getRoadName();
        }
    }

    public static String getCardinalDirection(double bearing){
        if(bearing >= 0 && bearing <= 22.5 || bearing >= 337.5 && bearing <= 360 ){return "North";}
        else if(bearing >= 22.5 && bearing <= 67.5){return "Northeast";}
        else if(bearing >= 67.5 && bearing <= 112.5){return "East";}
        else if(bearing >= 112.5 && bearing <= 157.5){return "Southeast";}
        else if(bearing >= 157.5 && bearing <= 202.5){return "South";}
        else if(bearing >= 202.5 && bearing <= 247.5){return "Southwest";}
        else if(bearing >= 247.5 && bearing <= 292.5){return "West";}
        else {return "Northwest ";}
    }

    public static String getSpeedInterval(double speed){
        if(useSpeedIntervalRange){
            int intervalStart = (int) speed/ speedIntervalRange;
            intervalStart = intervalStart * speedIntervalRange;
            int intervelEnd = intervalStart + speedIntervalRange;
            String speedInterval = "Speed ["+intervalStart+"-"+intervelEnd+"]";
            return speedInterval;
        }
        else {
            if(speed <= speedIntervalEndpoint1){return "Speed [0-"+speedIntervalEndpoint1+"]";}
            else if(speed >= speedIntervalEndpoint1 && speed <= speedIntervalEndpoint2){return "Speed ["+speedIntervalEndpoint1+"-"+speedIntervalEndpoint2+"]";}
            else if(speed >= speedIntervalEndpoint2 && speed <= speedIntervalEndpoint3){return "Speed ["+speedIntervalEndpoint2+"-"+speedIntervalEndpoint3+"]";}
            else if(speed >= speedIntervalEndpoint3 && speed <= speedIntervalEndpoint4){return "Speed ["+speedIntervalEndpoint3+"-"+speedIntervalEndpoint4+"]";}
            else if(speed >= speedIntervalEndpoint4 && speed <= speedIntervalEndpoint5){return "Speed ["+speedIntervalEndpoint4+"-"+speedIntervalEndpoint5+"]";}
            else if(speed >= speedIntervalEndpoint5 && speed <= speedIntervalEndpoint6){return "Speed ["+speedIntervalEndpoint5+"-"+speedIntervalEndpoint6+"]";}
            else if(speed >= speedIntervalEndpoint6 && speed <= speedIntervalEndpoint7){return "Speed ["+speedIntervalEndpoint6+"-"+speedIntervalEndpoint7+"]";}
            else if(speed >= speedIntervalEndpoint7 && speed <= speedIntervalEndpoint8){return "Speed ["+speedIntervalEndpoint7+"-"+speedIntervalEndpoint8+"]";}
            else if(speed >= speedIntervalEndpoint8 && speed <= speedIntervalEndpoint9){return "Speed ["+speedIntervalEndpoint8+"-"+speedIntervalEndpoint9+"]";}
            else if(speed >= speedIntervalEndpoint9 && speed <= speedIntervalEndpoint10){return "Speed ["+speedIntervalEndpoint9+"-"+speedIntervalEndpoint10+"]";}
            else {return "Speed [&gt;"+speedIntervalEndpoint10+"]";}
        }
    }

    public static String getHeightInterval(double height){
        if(height < 0 ){
            return "Height [&lt;0]";
        }
        else {
            int intervalStart = (int) height/ heightIntervalRange;
            intervalStart = intervalStart * heightIntervalRange;
            int intervelEnd = intervalStart + heightIntervalRange;
            String heightInterval = "Height ["+intervalStart+"-"+intervelEnd+"]";
            return heightInterval;
        }
    }

    public String getLabel() {
        return label;
    }

    public static boolean isCalcStreetnameTime() {
        return calcStreetnameTime;
    }

    public static void setCalcStreetnameTime(boolean calcStreetnameTime) {
        SymbolicTrajectory.calcStreetnameTime = calcStreetnameTime;
    }

    public long getStartDate() {
        return startDate;
    }

    public void setStartDate(long startDate) {
        this.startDate = startDate;
    }

    public long getEndDate() {
        return endDate;
    }

    public void setEndDate(long endDate) {
        this.endDate = endDate;
    }

    public static boolean isUseUtcTime() {
        return useUtcTime;
    }

    public static void setUseUtcTime(boolean useUtcTime) {
        SymbolicTrajectory.useUtcTime = useUtcTime;
    }

    public static void setHeightIntervalRange(int heightIntervalRange) {
        SymbolicTrajectory.heightIntervalRange = heightIntervalRange;
    }

    public static void setUseSpeedIntervalRange(boolean useSpeedIntervalRange) {
        SymbolicTrajectory.useSpeedIntervalRange = useSpeedIntervalRange;
    }

    public static void setSpeedIntervalRange(int speedIntervalRange) {
        SymbolicTrajectory.speedIntervalRange = speedIntervalRange;
    }

    public static void setSpeedIntervalEndpoint1(int speedIntervalEndpoint1) {
        SymbolicTrajectory.speedIntervalEndpoint1 = speedIntervalEndpoint1;
    }

    public static void setSpeedIntervalEndpoint2(int speedIntervalEndpoint2) {
        SymbolicTrajectory.speedIntervalEndpoint2 = speedIntervalEndpoint2;
    }

    public static void setSpeedIntervalEndpoint3(int speedIntervalEndpoint3) {
        SymbolicTrajectory.speedIntervalEndpoint3 = speedIntervalEndpoint3;
    }

    public static void setSpeedIntervalEndpoint4(int speedIntervalEndpoint4) {
        SymbolicTrajectory.speedIntervalEndpoint4 = speedIntervalEndpoint4;
    }

    public static void setSpeedIntervalEndpoint5(int speedIntervalEndpoint5) {
        SymbolicTrajectory.speedIntervalEndpoint5 = speedIntervalEndpoint5;
    }

    public static void setSpeedIntervalEndpoint6(int speedIntervalEndpoint6) {
        SymbolicTrajectory.speedIntervalEndpoint6 = speedIntervalEndpoint6;
    }

    public static void setSpeedIntervalEndpoint7(int speedIntervalEndpoint7) {
        SymbolicTrajectory.speedIntervalEndpoint7 = speedIntervalEndpoint7;
    }

    public static void setSpeedIntervalEndpoint8(int speedIntervalEndpoint8) {
        SymbolicTrajectory.speedIntervalEndpoint8 = speedIntervalEndpoint8;
    }

    public static void setSpeedIntervalEndpoint9(int speedIntervalEndpoint9) {
        SymbolicTrajectory.speedIntervalEndpoint9 = speedIntervalEndpoint9;
    }

    public static void setSpeedIntervalEndpoint10(int speedIntervalEndpoint10) {
        SymbolicTrajectory.speedIntervalEndpoint10 = speedIntervalEndpoint10;
    }
}
