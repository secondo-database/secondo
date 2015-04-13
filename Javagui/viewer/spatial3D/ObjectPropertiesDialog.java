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

import tools.Reporter;
import javax.media.j3d.*;
import javax.vecmath.*;

import java.awt.*;
import java.awt.Color;
import javax.swing.*;
import java.awt.BorderLayout;
import java.awt.event.*;
import viewer.Spatial3DViewer;

/**
 * This class opens a dialog to set properties on the objects in 
 * a relation
 */
public class ObjectPropertiesDialog extends JDialog{

  private JTextField scaleFactorField;
  private JButton changeColorBtn;
  private JButton changePointSizeBtn;
  private JButton cancelBtn;
  private JButton acceptBtn;
  private JButton resetBtn;

  private Spatial3DViewer viewer;
  private TextWindow window;
  private int itemIndex;
  private boolean colorChanged;
  private boolean pointSizeChanged;
  private Color oldColor;
  private Color currentColor ;
  private Color3f oldColor3f = new Color3f();
  private float oldPointSize;
  private float currentPointSize;
  

  /**
   * constructor
   */
  public ObjectPropertiesDialog(Spatial3DViewer sv, TextWindow win){
    super(sv.getMainFrame(), true);
    this.viewer=sv;
    this.window=win;
    setSize(350,120);
    getContentPane().setLayout(new BorderLayout());
    JPanel textPanel = new JPanel();
    textPanel.setLayout(new BoxLayout(textPanel, BoxLayout.Y_AXIS));
    textPanel.add(new JLabel("Change properties for the selected Object"));
    
    JPanel propButPanel = new JPanel();
    changeColorBtn = new JButton("Change Color");
    changePointSizeBtn= new JButton("Change Point Size");
    propButPanel.add(changeColorBtn);
    propButPanel.add(changePointSizeBtn);
    
    cancelBtn = new JButton("Cancel");
    acceptBtn = new JButton("Accept");
    resetBtn  = new JButton("set to Default");

    ActionListener al = new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        Object source = evt.getSource();
        if(source.equals(cancelBtn)){
          cancel();
          } else if(source.equals(acceptBtn)){
            accept();
            } else if(source.equals(changeColorBtn)){
              changeColor();
              } else if(source.equals(changePointSizeBtn)){
                changePointSize();
                } else if (source.equals(resetBtn)) {
                  setDefaultVal();
                }
        }
      };
      
    cancelBtn.addActionListener(al); 
    acceptBtn.addActionListener(al); 
    resetBtn.addActionListener(al);  
    changeColorBtn.addActionListener(al);
    changePointSizeBtn.addActionListener(al);
    
    KeyListener keyL = new KeyAdapter() {
      public void keyPressed(KeyEvent evt){
        int c = evt.getKeyCode();
        switch (c) {
          case KeyEvent.VK_ENTER:
            enterKey(evt);
            break;
          case KeyEvent.VK_ESCAPE:
            escKey(evt);
            break;
        }
      }
    };

    cancelBtn.addKeyListener(keyL); 
    acceptBtn.addKeyListener(keyL); 
    resetBtn.addKeyListener(keyL);  
    changeColorBtn.addKeyListener(keyL);
    changePointSizeBtn.addKeyListener(keyL);
    
    JPanel buttonPanel = new JPanel();
    buttonPanel.add(cancelBtn); 
    buttonPanel.add(acceptBtn); 
    buttonPanel.add(resetBtn);  

    getContentPane().setLayout(new BorderLayout());
    getContentPane().add(textPanel, BorderLayout.NORTH);
    getContentPane().add(propButPanel, BorderLayout.CENTER);
    getContentPane().add(buttonPanel, BorderLayout.SOUTH);
    
  }
  
  /**
   * enterKey
   * invoke if ENTER key is pressed
   */
  private void enterKey(KeyEvent evt) {
    Object source = evt.getSource();
    if(source.equals(cancelBtn)){
      cancel();
      } else if(source.equals(acceptBtn)){
        accept();
        } else if(source.equals(changeColorBtn)){
          changeColor();
          } else if(source.equals(changePointSizeBtn)){
            changePointSize();
            } else if (source.equals(resetBtn)) {
              setDefaultVal();
              }
  }
  
  /**
   * escKey
   * invoke if ESC is pressed
   */
  private void escKey(KeyEvent evt) {
    cancel();
  }

  /**
   * cancel()
   * cancel all changes 
   */
  private void cancel(){
    if (pointSizeChanged) {
      viewer.changeSinglePointSize(oldPointSize);
      pointSizeChanged = false;
    }
    if (colorChanged) {
      viewer.changeColor(oldColor);
      colorChanged=false;
    }
    setVisible(false);
  }

  /**
   * accept()
   * assumes all settings
   */
  private void accept(){
    if (colorChanged)
      viewer.changeColor(currentColor);
    if (pointSizeChanged)
      viewer.changeSinglePointSize(currentPointSize);
      
    pointSizeChanged = false;
    colorChanged=false;
    setVisible(false);
  }

  /**
   * setDefaultVal()
   * sets the default values 
   */
  private void setDefaultVal() {
    viewer.changeSinglePointSize((float)viewer.getPointSize());
    viewer.changeColor(viewer.getObjectColor().get());
    setVisible(false);
  }
  
  /**
   * changeColor()
   * set the color form current Object
   */
  private void changeColor() {
    viewer.getCurrentShape().getAppearance().getColoringAttributes().getColor(oldColor3f);
    oldColor=oldColor3f.get();
    currentColor = JColorChooser.showDialog(null, 
      "Choose Color for your Object!", oldColor);
    colorChanged=true;
  }
  
  /**
   * changePointSize()
   * change point size from current Point
   */
  private void changePointSize() {
    PointAttributes pntAtr =viewer.getCurrentShape().getAppearance().getPointAttributes();
    if (pntAtr !=null) {
      oldPointSize=pntAtr.getPointSize();
      currentPointSize=viewer.changeSinglePointSize();
      pointSizeChanged=true;
      }
    else{
        Reporter.showInfo("Your Object isn't a Point. "+
          "This option is only available for Points!");
    }
  }
  
  /**
   * setIndex
   */
 public void setIndex(int index) {
   itemIndex=index;
 }


}

