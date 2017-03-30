package de.fernunihagen.dna.mapmatchingcore;


import android.os.Environment;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class SymbolicTrajectoryTextFile {

    private MapMatching mapMatching;

    private List<String> resultStreetnames = new ArrayList<String>();
    private List<String> resultCardinalDirections = new ArrayList<String>();
    private List<String> resultSpeed = new ArrayList<String>();
    private List<String> resultHeight = new ArrayList<String>();

    private String path;

    long time;

    public SymbolicTrajectoryTextFile(MapMatching mapMatching){
        this.mapMatching = mapMatching;
    }

    public boolean save(){
        time = System.currentTimeMillis();
        if(resultStreetnames.size()!=0){
            if(canWriteOnExternalStorage()){
                File sdcard = Environment.getExternalStorageDirectory();
                File dir = new File(sdcard.getAbsolutePath() + "/Map Matching Results/");
                dir.mkdir();
                String filename = generateFileName(time);
                File file = new File(dir, filename);
                path = sdcard.getAbsolutePath() + "/Map Matching Results/"+filename;
                try {
                    Writer out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), "UTF8"));
                    String data = generateFileData();
                    out.append(data);
                    out.flush();
                    out.close();
                } catch (FileNotFoundException e) {
                    mapMatching.getMapMatchingListener().showLineErrorMessage("Result file was not created.");
                    return false;
                } catch (IOException e) {
                    mapMatching.getMapMatchingListener().showLineErrorMessage("I/O Exception. Result file was not created.");
                    return false;
                }
            }
            else{
                mapMatching.getMapMatchingListener().showLineErrorMessage("Error accessing external storage. Result file was not created.");
                return false;
            }
            mapMatching.getMapMatchingListener().showResultMessage("Result file has been saved to:<br>");
            mapMatching.getMapMatchingListener().showLineResultMessage(path);
        }
        return true;
    }

    private String generateFileData(){
        StringBuilder stringBuilder = new StringBuilder();

        stringBuilder.append("Map Matching Results from "+generateDateString(time)+"\n\n");

        stringBuilder.append("Streetnames:\n\n");
        for (String streetname : resultStreetnames) {
            stringBuilder.append(streetname+"\n");
        }
        stringBuilder.append("\nCardinal Directions:\n\n");
        for (String cardinalDirection : resultCardinalDirections) {
            stringBuilder.append(cardinalDirection+"\n");
        }
        stringBuilder.append("\nSpeed Intervals:\n\n");
        for (String speed : resultSpeed) {
            stringBuilder.append(speed+"\n");
        }
        stringBuilder.append("\nHeight Intervals:\n\n");
        for (String height : resultHeight) {
            stringBuilder.append(height+"\n");
        }

        return stringBuilder.toString();
    }

    private String generateFileName(long time){
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH-mm-ss");
        Date date = new Date(time);
        return dateFormat.format(date)+".txt";
    }

    private String generateDateString(long time){
        SimpleDateFormat dateFormat = new SimpleDateFormat("EEEE dd/MM/yyyy HH:mm:ss", Locale.US);
        Date date = new Date(time);
        return dateFormat.format(date);
    }


    public static boolean canWriteOnExternalStorage() {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            return true;
        }
        return false;
    }


    public List<String> getResultStreetnames() {
        return resultStreetnames;
    }

    public List<String> getResultCardinalDirections() {
        return resultCardinalDirections;
    }

    public List<String> getResultSpeed() {
        return resultSpeed;
    }

    public List<String> getResultHeight() {
        return resultHeight;
    }
}
