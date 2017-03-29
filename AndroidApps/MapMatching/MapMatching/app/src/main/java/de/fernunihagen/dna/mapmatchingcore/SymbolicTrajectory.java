package de.fernunihagen.dna.mapmatchingcore;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

public class SymbolicTrajectory {

    private static boolean calcStreetnameTime;
    private static boolean useUtcTime;

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
        if(speed <= 6){return "Speed [0-6]";}
        else if(speed >= 6 && speed <= 15){return "Speed [6-15]";}
        else if(speed >= 15 && speed <= 30){return "Speed [15-30]";}
        else if(speed >= 30 && speed <= 50){return "Speed [30-50]";}
        else if(speed >= 50 && speed <= 100){return "Speed [50-100]";}
        else if(speed >= 100 && speed <= 200){return "Speed [100-200]";}
        else {return "Speed [&gt;200]";}
    }

    public static String getHeightInterval(double height){
        if(height < 0 ){
            return "Height [&lt;0]";
        }
        else {
            int intervalStart = (int) height/10;
            intervalStart = intervalStart * 10;
            int intervelEnd = intervalStart + 10;
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
}
