
//This file is part of SECONDO.

//Copyright (C) 2013, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


package  viewer.hoese;


import javax.sound.sampled.Clip;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Line;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import java.io.File;

public class ToneGenerator {

     private byte[] data;   // the sinus data 
     private Clip clip;     // the clip playing the audio
     private boolean loop;  // run audio in a loop
     private int length;    // length of playing if not loop
     private AudioFormat af; //
     private int currentFreq;
     private File soundFile;


     public ToneGenerator(){
         // create audio format
         af = new AudioFormat(44100, 16, 1, true, false); // sample rate, sample size, Channels, signed, bigendian
         // create sinus wave
         data = getSinusTone(440, af);
         currentFreq = 440;
         loop = true;
         length = 1000;
         soundFile = null;
         try{
            clip = (Clip) AudioSystem.getLine(new Line.Info(Clip.class));
            clip.open(af, data,0 , data.length);
         } catch(javax.sound.sampled.LineUnavailableException e){
            clip = null;
         }
     }

     public boolean getLoop(){ return loop; }
     public int getLength(){ return length; }
     public int getFrequency(){ return currentFreq; }
     public File getSoundFile(){ return soundFile; }

     public void setLoop(boolean on){
         loop = on;
     }

     public boolean setLength(int length){
        if(length>=0){
          this.length = length;
          return true;
        }
        return false;
     }

     public boolean setFrequency(int hz){
         if(currentFreq == hz){
            return true;
         }
         if(hz<20  || hz > 20000){
            return false;
         }
         if(clip==null){
            return false;
         }
         currentFreq = hz;
         data = getSinusTone(hz, af);
         if(clip.isRunning()){
            clip.stop();
         }
         clip.close();
         try{
            clip.open(af,data,0,data.length);
         } catch(javax.sound.sampled.LineUnavailableException e){
            return false;
         }
         return true;
     }
     
     boolean setSoundFile(File file){
         if(clip==null){
            return false;
         }
         AudioInputStream in;
         try{
           in = AudioSystem.getAudioInputStream(file);
         } catch(javax.sound.sampled.UnsupportedAudioFileException e){
             return false;
         } catch(java.io.IOException e2){
             return false;
         } 

         if(clip.isRunning()){
            clip.stop();
         } 
         clip.close();
         soundFile = file;
         try{
            clip.open(in);
         } catch(javax.sound.sampled.LineUnavailableException e){
            return false;
         } catch(java.io.IOException e2){
             return false;
         } 
         return true;
          
     }

    
     private byte[] getSinusTone(double frequency, AudioFormat af) {
         byte sample_size = (byte) (af.getSampleSizeInBits() / 8);
         byte[] data = new byte[(int) af.getSampleRate() * sample_size];
         double step_width = (2 * Math.PI) / af.getSampleRate();
         double x = 0;
 
         for (int i = 0; i < data.length; i += sample_size) {
             int sample_max_value = (int) Math.pow(2, af.getSampleSizeInBits()) / 2 - 1;
             int value = (int) (sample_max_value * Math.sin(frequency * x));
             for (int j = 0; j < sample_size; j++) {
                 byte sample_byte = (byte) ((value >> (8 * j)) & 0xff);
                 data[i + j] = sample_byte;
             }
             x += step_width;
         }
         return data;
     }


     public void play(){ // todo: play in a thread
        if(clip!=null){
           if(clip.isRunning()){
                return;
           }
           if(!loop){
              new Thread(new PlayThread(clip,length)).start();
           } else {
              clip.loop(Clip.LOOP_CONTINUOUSLY);
           }
        }  
     }    

     public void stop(){
         if(!clip.isRunning()){
            return;
         }
         clip.stop();
     }


     private class PlayThread implements Runnable{
         PlayThread(Clip clip, int length){
            this.clip=clip;
            this.length=length;
         } 
         public void run(){
             clip.loop(Clip.LOOP_CONTINUOUSLY);
             try{
                Thread.sleep(length);
             } catch(Exception e){

             }
             clip.stop();
         }
         private Clip clip;
         private int length;
     }

 
}

