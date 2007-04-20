

package gui;

import javax.swing.*;
import sj.lang.*;
import java.awt.Color;
import java.awt.Font;

public class ProgressTimer extends JLabel implements MessageListener{


public ProgressTimer(){
   super(empty);
   setFont(new Font("Monospaced",Font.PLAIN,12));
   setForeground(Color.BLUE);
}


public void processMessage(ListExpr message){
   if(message.listLength()!=2 ||
      message.first().atomType()!=ListExpr.SYMBOL_ATOM ||
      !message.first().symbolValue().equals("progress")){
      // no progress message
      return;
   }

   message = message.second();
   if(message.listLength()!=2 ||
      message.first().atomType()!=ListExpr.INT_ATOM ||
      message.second().atomType()!=ListExpr.INT_ATOM){
      // wrong format
      return;
   }

   int current = message.first().intValue();
   int max = message.second().intValue();

   if(current<0 || max<0 ){
       setText(empty);
       startTime = System.currentTimeMillis();
   } else {
       int p = (int)((100.0*current)/max+0.5);
       //  setText(""+p+" %"); // display in percent
       long time  =  (System.currentTimeMillis()-startTime);
       long value = Math.max(0,(time*max) / current -time);
       long millies = value % 1000;
       value = value/1000;
       long seconds = value % 60;
       value = value / 60;
       long minutes = value % 60;
       value = value / 60;
       long hours = value % 24;
       long day = value / 24;
       text=empty;
       if(day>0){
         text = day + "." + ((10*hours)/24) + "days";
       } else if (hours>0){
         text = hours +"."+ ((10*minutes)/60) + "hours";
       } else if(minutes>0){
         String sec = seconds<10 ? "0"+seconds : ""+seconds;
         text = minutes+":"+sec+ "min";
       } else if(seconds>0 || millies>0){
          text = seconds+"."+(millies/100)+"sec";
       }
       setText("  "+text);

   }
   paintImmediately(0,0,getWidth(),getHeight());
 
}


public String toString(){
    return text;
}


private long startTime;
private static final String empty="            ";
private String text;


}
