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

public class TestTextFile {

    private MapMatching mapMatching;

    private List<String> lineValues = new ArrayList<String>();
    private List<String> movingPointValues = new ArrayList<String>();
    private List<String> movingLabelValues = new ArrayList<String>();

    private String path;

    long time;

    public TestTextFile(MapMatching mapMatching){
        this.mapMatching = mapMatching;
    }


    public boolean saveLabel(){
        time = System.currentTimeMillis();
        if(movingLabelValues.size()!=0){
            if(canWriteOnExternalStorage()){
                File sdcard = Environment.getExternalStorageDirectory();
                File dir = new File(sdcard.getAbsolutePath() + "/Map Matching MovingLabel Log Files/");
                dir.mkdir();
                String filename = generateFileName(time);
                File file = new File(dir, filename);
                path = sdcard.getAbsolutePath() + "/Map Matching MovingLabel Log Files/"+filename;
                try {
                    Writer out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), "UTF8"));
                    String data = generateLabelFileData();
                    out.append(data);
                    out.flush();
                    out.close();
                } catch (FileNotFoundException e) {
                    mapMatching.getMapMatchingListener().showLineErrorMessage("Log file was not created.");
                    return false;
                } catch (IOException e) {
                    mapMatching.getMapMatchingListener().showLineErrorMessage("I/O Exception. Log file was not created.");
                    return false;
                }
            }
            else{
                mapMatching.getMapMatchingListener().showLineErrorMessage("Error accessing external storage. Log file was not created.");
                return false;
            }
//            mapMatching.getMapMatchingListener().showResultMessage("Result file has been saved to:<br>");
//            mapMatching.getMapMatchingListener().showResultMessage(path+"<br>");
        }
        return true;
    }

    private String generateLabelFileData(){
        StringBuilder stringBuilder = new StringBuilder();

        stringBuilder.append("Map Matching MovingLabel Log from "+generateDateString(time)+"\n\n");

        stringBuilder.append("MovingLabel Values:\n\n");
        for (String label : movingLabelValues) {
            stringBuilder.append(label+"\n");
        }

        return stringBuilder.toString();
    }

    public boolean save(){
        time = System.currentTimeMillis();
        if(lineValues.size()!=0){
            if(canWriteOnExternalStorage()){
                File sdcard = Environment.getExternalStorageDirectory();
                File dir = new File(sdcard.getAbsolutePath() + "/Map Matching Movement Log Files/");
                dir.mkdir();
                String filename = generateFileName(time);
                File file = new File(dir, filename);
                path = sdcard.getAbsolutePath() + "/Map Matching Movement Log Files/"+filename;
                try {
                    Writer out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), "UTF8"));
                    String data = generateFileData();
                    out.append(data);
                    out.flush();
                    out.close();
                } catch (FileNotFoundException e) {
                    mapMatching.getMapMatchingListener().showLineErrorMessage("Log file was not created.");
                    return false;
                } catch (IOException e) {
                    mapMatching.getMapMatchingListener().showLineErrorMessage("I/O Exception. Log file was not created.");
                    return false;
                }
            }
            else{
                mapMatching.getMapMatchingListener().showLineErrorMessage("Error accessing external storage. Log file was not created.");
                return false;
            }
//            mapMatching.getMapMatchingListener().showResultMessage("Result file has been saved to:<br>");
//            mapMatching.getMapMatchingListener().showResultMessage(path+"<br>");
        }
        return true;
    }

    private String generateFileData(){
        StringBuilder stringBuilder = new StringBuilder();

        stringBuilder.append("Map Matching Movement Log from "+generateDateString(time)+"\n\n");

        stringBuilder.append("Line Values:\n\n");
        for (String streetname : lineValues) {
            stringBuilder.append(streetname+"\n");
        }
        stringBuilder.append("\nMoving Point Values:\n\n");
        for (String cardinalDirection : movingPointValues) {
            stringBuilder.append(cardinalDirection+"\n");
        }

        return stringBuilder.toString();
    }

    private String generateFileName(long time){
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH-mm-ss.SSS");
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

    public List<String> getLineValues() {
        return lineValues;
    }

    public List<String> getMovingPointValues() {
        return movingPointValues;
    }

    public List<String> getMovingLabelValues() {
        return movingLabelValues;
    }
}
