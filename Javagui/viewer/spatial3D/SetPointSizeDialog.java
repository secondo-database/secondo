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

package  viewer.spatial3D; 

import javax.swing.*;
import java.awt.BorderLayout;
import java.awt.event.*; 
import viewer.Spatial3DViewer;

/**
 * class to set Point Size
 */
public class SetPointSizeDialog extends JDialog{
  
  private JTextField setPointSizeField;
  private JButton cancelBtn;
  private JButton acceptBtn;
  private JButton resetBtn;
  private JButton defaultBtn;
  private Spatial3DViewer viewer;
  private boolean isSinglePoint;

  public SetPointSizeDialog(Spatial3DViewer sv){
    super(sv.getMainFrame(), true);
    this.viewer=sv;
    setSize(450,150);
    getContentPane().setLayout(new BorderLayout());
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    panel.add(new JLabel("Please enter new Point Size"));
    
    setPointSizeField=new  JTextField(20);
    setPointSizeField.setText(""+viewer.getPointSize());
    panel.add(setPointSizeField);
    JPanel outerPanel = new JPanel();
    outerPanel.add(panel);

    cancelBtn = new JButton("Cancel");
    acceptBtn = new JButton("Accept");
    resetBtn  = new JButton("Reset");
    defaultBtn = new JButton("Default");

    ActionListener al = new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         Object source = evt.getSource();
         if(source.equals(cancelBtn)){
            cancel();
         } else if(source.equals(acceptBtn)){
            accept();
         } else if(source.equals(resetBtn)){
           setPointSizeField.setText(""+viewer.getPointSize());
         } else if(source.equals(defaultBtn)){
           setPointSizeField.setText("10.0");               // standard Value
         }
      }
    };
    cancelBtn.addActionListener(al); 
    acceptBtn.addActionListener(al); 
    resetBtn.addActionListener(al);  
    defaultBtn.addActionListener(al);  

    JPanel buttonPanel = new JPanel();
    buttonPanel.add(cancelBtn); 
    buttonPanel.add(acceptBtn); 
    buttonPanel.add(resetBtn);  
    buttonPanel.add(defaultBtn);
     
    getContentPane().setLayout(new BorderLayout());
    getContentPane().add(outerPanel, BorderLayout.CENTER);
    getContentPane().add(buttonPanel, BorderLayout.SOUTH);

    // allow finish the dialog using ESC or return
    setPointSizeField.addKeyListener(new KeyAdapter(){
      public void keyPressed(KeyEvent evt){
        int c = evt.getKeyCode();
        if(c == KeyEvent.VK_ENTER){
           accept();
        } else if(c==KeyEvent.VK_ESCAPE){
           cancel();
        }
      }
    });


  }


  private void cancel(){
    setPointSizeField.setText(""+viewer.getPointSize());
    setVisible(false);
  }

   private void accept(){
     try{
       float value = Float.parseFloat(setPointSizeField.getText().trim());
       if(value >= 1.0f && value <= 100.0f){
         if (isSinglePoint)
          viewer.setSinglePointSize(value);
         else
          viewer.changeAllPointSize(value);
         setVisible(false);
       } else {
         JOptionPane.showMessageDialog(this, "Point Size must be between 1 and 100");
       }
     } catch(Exception e){
       JOptionPane.showMessageDialog(this, "Point Size must be a number");
     }
 }

 public void setVisible(boolean visible, boolean isSinglePnt) {
   if (isSinglePnt)
    setPointSizeField.setText(""+viewer.getCurrentShape().getAppearance().getPointAttributes().getPointSize());
   else
    setPointSizeField.setText(""+viewer.getPointSize());
   
   this.isSinglePoint=isSinglePnt;
   this.setVisible(visible);
   
 }


}

