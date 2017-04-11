package de.fernunihagen.dna.mapmatching;

import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

import de.fernunihagen.dna.mapmatchingcore.SymbolicTrajectory;

public class MapMatchingOverview {

    private static MapMatchingActivity mapMatchingActivity;

    private static List<SymbolicTrajectory> resultsStreetnames = new ArrayList<SymbolicTrajectory>();
    private static List<SymbolicTrajectory> resultsCardinalDirections = new ArrayList<SymbolicTrajectory>();
    private static List<SymbolicTrajectory> resultsSpeedIntervals = new ArrayList<SymbolicTrajectory>();
    private static List<SymbolicTrajectory> resultsHeightIntervals = new ArrayList<SymbolicTrajectory>();

    public static void update(){

        resultsStreetnames = MapMatchingCoreInterface.getResultsStreetnames();
        resultsCardinalDirections = MapMatchingCoreInterface.getResultsCardinalDirections();
        resultsSpeedIntervals = MapMatchingCoreInterface.getResultsSpeedIntervals();
        resultsHeightIntervals = MapMatchingCoreInterface.getResultsHeightIntervals();

        clearData();
        updateStreetname();
        updateDirection();
        updateSpeed();
        updateHeight();
    }

    public static void clearData(){
        List<TextView>labelsStreetname = mapMatchingActivity.getLabelsOverviewStreetname();
        List<TextView> labelsDirection = mapMatchingActivity.getLabelsOverviewDirection();
        List<TextView> labelsSpeed = mapMatchingActivity.getLabelsOverviewSpeed();
        List<TextView> labelsHeight = mapMatchingActivity.getLabelsOverviewHeight();
        for(int i=0;i<labelsStreetname.size();i++){
            labelsStreetname.get(i).setText(i+1 +". -");
        }
        for(int i=0;i<labelsDirection.size();i++){
            labelsDirection.get(i).setText(i+1 +". -");
        }
        for(int i=0;i<labelsSpeed.size();i++){
            labelsSpeed.get(i).setText(i+1 +". -");
        }
        for(int i=0;i<labelsHeight.size();i++){
            labelsHeight.get(i).setText(i+1 +". -");
        }
    }

    public static void cancel(long cancelTime){
        if(resultsStreetnames.size()!=0){
            int index = resultsStreetnames.size()-1;
            mapMatchingActivity.getLabelsOverviewStreetname().get(0).setText("1. "+"["+SymbolicTrajectory.getTimeFromDateLong(resultsStreetnames.get(index).getStartDate())+" - "+SymbolicTrajectory.getTimeFromDateLong(cancelTime)+"] - "+resultsStreetnames.get(index).getLabel());
        }
        if(resultsCardinalDirections.size()!=0){
            int index = resultsCardinalDirections.size()-1;
            mapMatchingActivity.getLabelsOverviewDirection().get(0).setText("1. "+"["+SymbolicTrajectory.getTimeFromDateLong(resultsCardinalDirections.get(index).getStartDate())+" - "+SymbolicTrajectory.getTimeFromDateLong(cancelTime)+"] - "+resultsCardinalDirections.get(index).getLabel());
        }
        if(resultsSpeedIntervals.size()!=0){
            int index = resultsSpeedIntervals.size()-1;
            mapMatchingActivity.getLabelsOverviewSpeed().get(0).setText("1. "+"["+SymbolicTrajectory.getTimeFromDateLong(resultsSpeedIntervals.get(index).getStartDate())+" - "+SymbolicTrajectory.getTimeFromDateLong(cancelTime)+"] - "+resultsSpeedIntervals.get(index).getLabel());
        }
        if(resultsHeightIntervals.size()!=0){
            int index = resultsHeightIntervals.size()-1;
            mapMatchingActivity.getLabelsOverviewHeight().get(0).setText("1. "+"["+SymbolicTrajectory.getTimeFromDateLong(resultsHeightIntervals.get(index).getStartDate())+" - "+SymbolicTrajectory.getTimeFromDateLong(cancelTime)+"] - "+resultsHeightIntervals.get(index).getLabel());
        }
    }

    private static void updateStreetname(){
        int counter = 0;
        List<TextView> labels = mapMatchingActivity.getLabelsOverviewStreetname();

        for (int i=resultsStreetnames.size()-1 ; i>=0 ; i--)
        {
            if(counter==0){
                String labelText = counter+1+". ["+SymbolicTrajectory.getTimeFromDateLong(resultsStreetnames.get(i).getStartDate())+" -  -- : -- : --  ] - "+resultsStreetnames.get(i).getLabel();
                labels.get(counter).setText(labelText);
                counter = counter + 1;
            }
            else{
                String labelText = counter+1+". ["+SymbolicTrajectory.getTimeFromDateLong(resultsStreetnames.get(i).getStartDate())+" - "+SymbolicTrajectory.getTimeFromDateLong(resultsStreetnames.get(i).getEndDate())+"] - "+resultsStreetnames.get(i).getLabel();
                labels.get(counter).setText(labelText);
                counter = counter + 1;
            }
            if(counter == 5){
                break;
            }
        }
    }

    private static void updateDirection(){
        int counter = 0;
        List<TextView> labels = mapMatchingActivity.getLabelsOverviewDirection();
        for (int i=resultsCardinalDirections.size()-1 ; i>=0 ; i--)
        {
            if(counter==0){
                String labelText = counter+1+". ["+SymbolicTrajectory.getTimeFromDateLong(resultsCardinalDirections.get(i).getStartDate())+" -  -- : -- : --  ] - "+resultsCardinalDirections.get(i).getLabel();
                labels.get(counter).setText(labelText);
                counter = counter + 1;
            }
            else{
                String labelText = counter+1+". ["+SymbolicTrajectory.getTimeFromDateLong(resultsCardinalDirections.get(i).getStartDate())+" - "+SymbolicTrajectory.getTimeFromDateLong(resultsCardinalDirections.get(i).getEndDate())+"] - "+resultsCardinalDirections.get(i).getLabel();
                labels.get(counter).setText(labelText);
                counter = counter + 1;
            }

            if(counter == 5){
                break;
            }
        }
    }

    private static void updateSpeed(){
        int counter = 0;
        List<TextView> labels = mapMatchingActivity.getLabelsOverviewSpeed();
        for (int i=resultsSpeedIntervals.size()-1 ; i>=0 ; i--)
        {
            if(counter==0){
                String labelText = counter+1+". ["+SymbolicTrajectory.getTimeFromDateLong(resultsSpeedIntervals.get(i).getStartDate())+" -  -- : -- : --  ] - "+resultsSpeedIntervals.get(i).getLabel();
                labelText = labelText.replace("&gt;", ">");
                labels.get(counter).setText(labelText);
                counter = counter + 1;
            }
            else{
                String labelText = counter+1+". ["+SymbolicTrajectory.getTimeFromDateLong(resultsSpeedIntervals.get(i).getStartDate())+" - "+SymbolicTrajectory.getTimeFromDateLong(resultsSpeedIntervals.get(i).getEndDate())+"] - "+resultsSpeedIntervals.get(i).getLabel();
                labelText = labelText.replace("&gt;", ">");
                labels.get(counter).setText(labelText);
                counter = counter + 1;
            }
            if(counter == 5){
                break;
            }
        }
    }

    private static void updateHeight(){
        int counter = 0;
        List<TextView> labels = mapMatchingActivity.getLabelsOverviewHeight();
        for (int i=resultsHeightIntervals.size()-1 ; i>=0 ; i--)
        {
            if(counter==0){
                String labelText = counter+1+". ["+SymbolicTrajectory.getTimeFromDateLong(resultsHeightIntervals.get(i).getStartDate())+" -  -- : -- : --  ] - "+resultsHeightIntervals.get(i).getLabel();
                labelText = labelText.replace("&lt;", "<");
                labels.get(counter).setText(labelText);
                counter = counter + 1;
            }
            else{
                String labelText = counter+1+". ["+SymbolicTrajectory.getTimeFromDateLong(resultsHeightIntervals.get(i).getStartDate())+" - "+SymbolicTrajectory.getTimeFromDateLong(resultsHeightIntervals.get(i).getEndDate())+"] - "+resultsHeightIntervals.get(i).getLabel();
                labelText = labelText.replace("&lt;", "<");
                labels.get(counter).setText(labelText);
                counter = counter + 1;
            }

            if(counter == 5){
                break;
            }
        }
    }

    public static void setMapMatchingActivity(MapMatchingActivity mapMatchingActivity) {
        MapMatchingOverview.mapMatchingActivity = mapMatchingActivity;
    }
}
