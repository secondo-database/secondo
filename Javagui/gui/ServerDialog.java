//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

package gui;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;


public class ServerDialog extends JDialog{

private JLabel HostLabel;
private JLabel PortLabel;
private JTextField HostName;
private JTextField PortAddress;
private JButton OkBtn;
private JButton CancelBtn;

private int result;

public static final int OK = 0;
public static final int CANCEL = 1;


public int getResultValue(){
  return result;
}

public ServerDialog(Frame Owner){
   super(Owner,"Server-Settings",true);
   HostLabel = new JLabel("Host");
   PortLabel = new JLabel("Port");
   HostName = new JTextField(20);
   PortAddress = new JTextField(20);
   setSize(200,100);
   PortAddress.addKeyListener( new KeyAdapter(){
       public void keyReleased(KeyEvent EVT){
          char c = EVT.getKeyChar();
          if ( c<'0' || c>'9' ){
            // remove all not number chars 
               String Text = PortAddress.getText();
               String Text2="";
               char current;
               for(int i=0;i<Text.length();i++){
                 current=Text.charAt(i);
                 if(current>='0' && current<='9')
                    Text2+=current;
               }
               PortAddress.setText(Text2);
          }
        }});
    
   KeyListener KL = new KeyAdapter(){
     public void keyPressed(KeyEvent evt){
          int KC = evt.getKeyCode();
          if (KC==KeyEvent.VK_ENTER)
              closeDialog(true);
          if (KC==KeyEvent.VK_CANCEL || KC==KeyEvent.VK_ESCAPE)
              closeDialog(false);  
    } 
   };
   PortAddress.addKeyListener(KL);
   HostName.addKeyListener(KL);

   OkBtn = new JButton("ok");
   CancelBtn = new JButton("cancel");
   JPanel innerPanel1 = new JPanel(new GridLayout(2,2));
   innerPanel1.add(HostLabel);
   innerPanel1.add(HostName);
   innerPanel1.add(PortLabel);
   innerPanel1.add(PortAddress);
   JPanel innerPanel2 = new JPanel(new FlowLayout());
   innerPanel2.add(OkBtn);
   innerPanel2.add(CancelBtn);
   Container ContentPane = getContentPane();
   ContentPane.setLayout(new BorderLayout());
   ContentPane.add(innerPanel1,BorderLayout.CENTER);
   ContentPane.add(innerPanel2,BorderLayout.SOUTH);
   addListener();
}


private void closeDialog(boolean accept){
  if(accept){
     // check inputs
     String HN = HostName.getText().trim();
     int Port = getPortAddress();
     if(HN.equals("") || Port<0)
        JOptionPane.showMessageDialog(null,"wrong inputs","error",JOptionPane.ERROR_MESSAGE);
     else{
       result = OK;
       setVisible(false);      
     }
  }
  else{ // not accepted
      result = CANCEL;
      setVisible(false);
  } 
}

/** set the HostName and the PortAdress) **/
public void set(String HostName,int PortAddress){
  this.HostName.setText(HostName);
  this.PortAddress.setText(""+PortAddress);
}

/** get HostName **/
public String getHostName(){
   return HostName.getText().trim();
}


/** returns the portaddress, if this field not contains a
  * valid integer-value is -1 returned
  **/
public int getPortAddress(){
  int PA = -1;
  try{
    Integer I = new Integer(PortAddress.getText());
    PA = I.intValue();
  }
  catch(Exception e){}
  return PA;
}

private void  addListener(){
  OkBtn.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent evt){
                   closeDialog(true);
                }
            });

  CancelBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
           closeDialog(false);
        }});

}

}


