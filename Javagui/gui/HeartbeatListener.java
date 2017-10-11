
package gui;

import javax.swing.*;
import sj.lang.*;
import java.awt.Color;
import java.awt.Font;

public class HeartbeatListener extends JLabel implements MessageListener{

  public HeartbeatListener() {
    super(" ");
    setFont(new Font("Monospaced",Font.PLAIN,16));
    setForeground(Color.RED);
  }
  static String values = "\u2665\u2661"; // heart full, heart border
  int pos = 0;


  public void processMessage(ListExpr message){
     if(message.listLength()!=2 ||
        message.first().atomType()!=ListExpr.SYMBOL_ATOM ||
        !message.first().symbolValue().equals("heartbeat") ||
        message.second().atomType()!=ListExpr.INT_ATOM){
        // no heartbeat  message
        return;
     }
     int v = message.second().intValue();
     if(v==0){ // end of query processor heartbeats
        setText(" ");
        return;
     }
     if(v!=1){
        // process only query processor heartbeats
        return;
     }
 
     String t = "" + values.charAt(pos);
     setText(t);
     pos = (pos + 1) % values.length();
     paintImmediately(0,0,getWidth(),getHeight());
  }

}
