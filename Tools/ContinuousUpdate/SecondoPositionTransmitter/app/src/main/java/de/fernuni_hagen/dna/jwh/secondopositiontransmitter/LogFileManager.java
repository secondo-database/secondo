package de.fernuni_hagen.dna.jwh.secondopositiontransmitter;

import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * Created by Jerome on 11.09.2015.
 */
public class LogFileManager {
    private boolean writeable;
    private FileWriter fileWriter;

    LogFileManager(){
        String state = Environment.getExternalStorageState();
        if(state.equals(Environment.MEDIA_MOUNTED)){
            writeable = true;
        }

        if(writeable){
            File logfile = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).getAbsolutePath(), "Positiontransmitter.log");
            try {
                fileWriter = new FileWriter(logfile, true);
            } catch (IOException e) {
                Log.e(LogFileManager.class.getSimpleName(), "Could not open File");
                writeable = false;
            }
        }
    }

    public void close(){
        try {
            fileWriter.close();
        } catch (IOException e) {
            Log.e(LogFileManager.class.getSimpleName(), "Could not close File");
        }
    }

    public void write(String text){
        try {
            if(writeable) {
                fileWriter.write(text + "\n");
                fileWriter.flush();
            }else{
                Log.d(LogFileManager.class.getSimpleName(), "File no longer writable");
                            }

        } catch (IOException e) {
            Log.e(LogFileManager.class.getSimpleName(), "Could not write to File");
        }
    }

}
