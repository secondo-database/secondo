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
import tools.Reporter;


public class UserDialog extends JDialog{

private JLabel userLabel;
private JLabel passLabel;
private JTextField userField;
private JPasswordField passwordField;
private JButton OkBtn;
private JButton CancelBtn;

private int result;

public static final int OK = 0;
public static final int CANCEL = 1;


public int getResultValue(){
  return result;
}

public UserDialog(Frame Owner){
   super(Owner,"User-Settings",true);
   userLabel = new JLabel("user name");
   passLabel = new JLabel("password");
   userField = new JTextField(20);
   passwordField = new JPasswordField(20);
   setSize(200,100);
    
   KeyListener KL = new KeyAdapter(){
     public void keyPressed(KeyEvent evt){
          int KC = evt.getKeyCode();
          if (KC==KeyEvent.VK_ENTER)
              closeDialog(true);
          if (KC==KeyEvent.VK_CANCEL || KC==KeyEvent.VK_ESCAPE)
              closeDialog(false);  
    } 
   };
   passwordField.addKeyListener(KL);
   userField.addKeyListener(KL);

   OkBtn = new JButton("ok");
   CancelBtn = new JButton("cancel");
   JPanel innerPanel1 = new JPanel(new GridLayout(2,2));
   innerPanel1.add(userLabel);
   innerPanel1.add(userField);
   innerPanel1.add(passLabel);
   innerPanel1.add(passwordField);
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
     result = OK;
     setVisible(false);      
  }
  else{ // not accepted
      result = CANCEL;
      setVisible(false);
  } 
}

/** set the infos **/
public void set(String user,String password){
  this.userField.setText(user);
  this.passwordField.setText(password);
}

/** get UserName **/
public String getUserName(){
   return userField.getText().trim();
}

/** get Password  **/
public String getPassword(){
   return passwordField.getText().trim();
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


