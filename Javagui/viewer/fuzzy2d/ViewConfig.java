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

package viewer.fuzzy2d;

import javax.swing.*;
import viewer.*;
import java.awt.*;
import java.awt.event.*;
import java.util.Vector;
import javax.swing.event.*;

/** this class provided a ViewConfiguration for Fuzzy-Viewer*/
public class ViewConfig extends JDialog{


public ViewConfig(Frame F){
  super(F,true);
  // construct this Dialog
  setTitle("view configuration");
  JPanel BBPanel = new JPanel(new GridLayout(6,3));
  BBPanel.add(new JPanel()); // a dummy
  BBPanel.add(new JLabel("x"));
  BBPanel.add(new JLabel("y"));
  BBPanel.add(new JLabel("left-bottom"));
  BBPanel.add(ZeroX);
  BBPanel.add(ZeroY);
  BBPanel.add(new JPanel()); // a dummy
  BBPanel.add(new JLabel("width"));
  BBPanel.add(new JLabel("height"));
  BBPanel.add(new JLabel("dimension"));
  BBPanel.add(DimX);
  BBPanel.add(DimY);
  BBPanel.add(new JLabel("border size"));
  BBPanel.add(BorderSize);
  BBPanel.add(new JPanel()); // dummy
  BBPanel.add(AutoBoundingBox);
  BBPanel.add(KeepProportions);
  JPanel BBContainer = new JPanel();
  BBContainer.add(BBPanel);
  
  JPanel PSPanel = new JPanel();
  PSPanel.add(new JLabel("PointSize"));
  PSPanel.add(PointSizeText);
  JPanel ControlPanel = new JPanel();
  ControlPanel.add(OkBtn);
  ControlPanel.add(CancelBtn);
  ControlPanel.add(ResetBtn);
  ControlPanel.add(ApplyBtn);
  setSize(550,300);
  getContentPane().setLayout(new FlowLayout());
  getContentPane().add(BBContainer);
  getContentPane().add(BorderPaint);
  getContentPane().add(PSPanel);
  getContentPane().add(ControlPanel);
  BorderSize.setEnabled(false);
  addListeners();
}


/** AL is informed if the apply-button is pressed */
public void addApplyListener(ApplyListener AL){
  if(MyApplyListeners.indexOf(AL)<0)
     MyApplyListeners.add(AL);
}


/** add the Listeners */
private void addListeners(){
   ActionListener AL = new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        Object Source = evt.getSource();
        if(Source.equals(OkBtn)){
           if(processInputs()){
              for(int i=0;i<MyApplyListeners.size();i++)
                ((ApplyListener) MyApplyListeners.get(i)).apply(ViewConfig.this);
              setVisible(false);   
           }     
        }
        
        if(Source.equals(CancelBtn)){
           reset();
           setVisible(false);
        }
        
        if(Source.equals(ResetBtn)){
           reset();
        }
      
        if(Source.equals(ApplyBtn)){
          if(processInputs()){
             for(int i=0;i<MyApplyListeners.size();i++)
                ((ApplyListener) MyApplyListeners.get(i)).apply(ViewConfig.this);
          } 
        }
              
      }
   };
   
   OkBtn.addActionListener(AL);
   CancelBtn.addActionListener(AL);
   ResetBtn.addActionListener(AL);
   ApplyBtn.addActionListener(AL);
   
   
   ChangeListener CL = new ChangeListener(){
      public void stateChanged(ChangeEvent evt){
        Object Source = evt.getSource();

        if(Source.equals(AutoBoundingBox)){
           if(AutoBoundingBox.isSelected()){
              ZeroX.setEnabled(false);
              ZeroY.setEnabled(false);
              DimX.setEnabled(false);
              DimY.setEnabled(false);
              BorderSize.setEnabled(true);
           } else{
              ZeroX.setEnabled(true);
              ZeroY.setEnabled(true);
              DimX.setEnabled(true);
              DimY.setEnabled(true);
              BorderSize.setEnabled(false);
           }
        }
      
      }
   };
   
   AutoBoundingBox.addChangeListener(CL);

}

 
/** check the vadility of inputs;
  * if all inputs are valid then variables
  * BoundingBox and Bordersize are set
  */
private boolean processInputs(){
 boolean ok = true;
 int TMPPointSize=1;
 try{
    TMPPointSize = Integer.parseInt(PointSizeText.getText());
    if (TMPPointSize<1 || TMPPointSize>250){
       MessageBox.showMessage("the PointSize must be in range [1..250]");
       ok = false;
    }   
 }
 catch(Exception e){
   MessageBox.showMessage("the PointSize Field contains not an integer");
   ok = false;
 }

 if(ok)
   if(AutoBoundingBox.isSelected()){
       try{
          double BSTMP = Double.parseDouble(BorderSize.getText());
          if(BSTMP<0){
             MessageBox.showMessage("the border must be greater then zero");
             ok = false;
          }   
          else
             BorderSizeValue = BSTMP;
       }
       catch(Exception e){
           MessageBox.showMessage("the border size field contains not a double");
           ok = false;
       }
   }else{ // no AutoBoundingBox
      try{
        double TMPX = Double.parseDouble(ZeroX.getText());
        double TMPY = Double.parseDouble(ZeroY.getText());
        double TMPW = Double.parseDouble(DimX.getText());
        double TMPH = Double.parseDouble(DimY.getText());
        
        if(TMPW<MINWIDTH || TMPH<MINHEIGHT){
           MessageBox.showMessage("the dimension must be greater then ("+MINWIDTH+","+MINHEIGHT+")");
           ok =false;
        } else{
          BB.setTo(TMPX,TMPY,TMPX+TMPW,TMPY+TMPH);
        }
      
      }
      catch(Exception e){
         MessageBox.showMessage("a field of the bounding box contains not a double");
         ok = false;
      }
   }
   
   
 if(ok){
   OrigAuto = AutoBoundingBox.isSelected();
   OrigProportion = KeepProportions.isSelected();
   PointSize = TMPPointSize;
 }

return ok;
}


/** set the current Bounding Box and the TextFields*/
public void setBoundingBox(BoundingBox2D BB2){
    BB.equalize(BB2);
    ZeroX.setText(""+BB.getMinX());
    ZeroY.setText(""+BB.getMinY());
    DimX.setText(""+BB.getWidth());
    DimY.setText(""+BB.getHeight());
}

/** returns the current Bounding Box (not the content in the textfields !!!*/
public BoundingBox2D getBoundingBox(){
   return BB;
}


public void setAutoBoundingBox(boolean on){
   AutoBoundingBox.setSelected(true);
   OrigAuto = on;
}


public boolean getAutoBoundingBox(){
  return AutoBoundingBox.isSelected();
}


public void setProportional(boolean p){
   KeepProportions.setSelected(p);
   OrigProportion=p;
}

public boolean getProportional(){
  return KeepProportions.isSelected();
}


public void setBorderSize(double BS){
  BorderSizeValue = Math.abs(BS);
  BorderSize.setText(""+BorderSizeValue);
}


public double getBorderSize(){
   return BorderSizeValue;
}

private void reset(){
   ZeroX.setText(""+BB.getMinX());
   ZeroY.setText(""+BB.getMinY());
   DimX.setText(""+BB.getWidth());
   DimY.setText(""+BB.getHeight());
   BorderSize.setText(""+BorderSizeValue);
   AutoBoundingBox.setSelected(OrigAuto);
   KeepProportions.setSelected(OrigProportion);
}


public void setVisible(boolean on){
  reset();
  super.setVisible(on);
}


/** returns true if BorderPainting is enabled */
public boolean getBorderPaint(){
  return BorderPaint.isSelected();
}

/** set the value of BorderPainting */
public void setBorderPaint(boolean on){
  BorderPaint.setSelected(on);
}



/** get the current PointSize */
public int getPointSize(){
   return PointSize;
}

/** set the PointSize 
  * if size<1 then the PointSize is setted to 1
  * if size>250 then the PointSize is setted to 250
  */
public void setPointSize(int size){
  if(size<1) size =1;
  if(size>250) size=250;
  PointSize = size;
  PointSizeText.setText(""+PointSize);
}


private JButton OkBtn = new JButton("ok");
private JButton CancelBtn = new JButton("cancel");
private JButton ResetBtn = new JButton("reset");
private JButton ApplyBtn = new JButton("apply");
private JCheckBox AutoBoundingBox = new JCheckBox("auto");
private JCheckBox KeepProportions = new JCheckBox("keep proportions");
private JTextField ZeroX = new JTextField(8);
private JTextField ZeroY = new JTextField(8);
private JTextField DimX = new JTextField(8);
private JTextField DimY = new JTextField(8);
private JTextField PSX = new JTextField(8);
private JTextField PSY = new JTextField(8);
private JTextField BorderSize = new JTextField(8);
private JTextField PointSizeText = new JTextField(8);
private Vector MyApplyListeners = new Vector();
private BoundingBox2D BB= new BoundingBox2D();

private double BorderSizeValue = 0.0;
private boolean OrigAuto = false;
private boolean OrigProportion = false;
private int PointSize = 15;
private final double MINWIDTH = 10;
private final double MINHEIGHT = 10;

private JCheckBox BorderPaint = new JCheckBox("paint border of triangles");

}





