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

import java.awt.Button;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.media.j3d.PointLight;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JColorChooser;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel; 
import javax.swing.event.*;
import javax.swing.JSeparator;
import javax.swing.SwingConstants;
import javax.vecmath.Color3f;
import viewer.Spatial3DViewer;

/**
 * a SWING panel class for lighting in Spatial3DViewer
 *  
 */
public class LightPanel extends JPanel{
  
  private final int SPINNER_COLUMN_SIZE = 10;
  private final int MAX_LIGHT_NUM = 10;
  public final int STARTING_RADIUS = 500;
  
  private double xChangeFactor;
  private double yChangeFactor;
  private double zChangeFactor;
  
  private Spatial3DViewer sv;
  private JCheckBox enableBox;
  private JComboBox lightsBox;
  private JSlider radiusSlider;
  private JLabel radiusLabel;
  private JButton colorButton;
  private JButton delButton;
  private JButton delAllButton;
  private JSpinner verticalSpinner;
  private JSpinner horizontalSpinner;
  private JSpinner depthSpinner;
  private JLabel xPosLabel;
  private JLabel yPosLabel;
  private JLabel zPosLabel;
  private JLabel brightnessLabel;
  private JCheckBox mainSW;
  private JButton mainColorButton;
  private JSlider brightnessSlider;
  
  /**
   * constructor
   * @param Spatial3Dviewer object as parent
   */
  public LightPanel(final Spatial3DViewer _sv){
    
    this.setLayout(new GridBagLayout());
    sv = _sv;
    
    xChangeFactor = sv.getTranslateFactor();
    yChangeFactor = sv.getTranslateFactor();
    zChangeFactor = sv.getZoomFactor();
    
    GridBagConstraints constraint = new GridBagConstraints();
    constraint.anchor = GridBagConstraints.LINE_START;
    constraint.fill = GridBagConstraints.HORIZONTAL;
    
    //label for main light
    constraint.gridx = 0;
    constraint.gridy = 0;    
    this.add(new JLabel("global light"),constraint);
    
    // on/off switch for the ambient light
    mainSW = new JCheckBox();
    mainSW.addActionListener(new ActionListener(){
      @Override
      public void actionPerformed(ActionEvent event){
        sv.setMainLightSwitch(mainSW.isSelected());
      }
    });
    mainSW.setText("on / off");
    mainSW.setSelected(true);
    constraint.gridx = 0;
    constraint.gridy = 1;  
    this.add(mainSW,constraint);
    
    //color chooser for the ambient light
    mainColorButton = new JButton("light color");
    mainColorButton.addActionListener(new ActionListener(){
      @Override
      public void actionPerformed(ActionEvent event){
        Color3f curColor = sv.getLightColor();
        Color newColor = JColorChooser.showDialog(sv.getMainFrame(),"Choose the light's color",curColor.get()) ;
        if(newColor==null){
          Reporter.showError("Please choose a color!");
          return;
        }
        sv.setLightColor(newColor);
      }
    });
    constraint.gridx = 1;
    constraint.gridy = 1;
    this.add(mainColorButton,constraint);
    
    constraint.gridx=0;
    constraint.gridy=2;
    this.add(new JSeparator(JSeparator.HORIZONTAL),constraint);
    
    // individual lights
    constraint.gridx = 0;
    constraint.gridy = 3;
    this.add(new JLabel("Indiv. lights"),constraint);
    
    constraint.gridx = 1;
    constraint.gridy = 3;
    lightsBox = new JComboBox();
    lightsBox.addActionListener(new ActionListener(){
      @Override
      public void actionPerformed(ActionEvent event){
        JComboBox cb = (JComboBox)event.getSource();
        int index = cb.getSelectedIndex();
        setPanel(index);
      }
    });
    
    
    this.add(lightsBox,constraint);
    
    constraint.gridx = 0;
    constraint.gridy = 4;
    this.add(new JLabel("Enable"),constraint);
    
    enableBox = new JCheckBox();
    enableBox.addActionListener(new ActionListener(){
      @Override
      public void actionPerformed(ActionEvent event){
        LightBranchGroup light = sv.getLight(lightsBox.getSelectedIndex());
        if(light==null){
          Reporter.showError("wrong light lindex!");
          return;
        }
        light.setEnable(enableBox.isSelected());
      }
    });
    
    enableBox.setEnabled(false);
    constraint.gridx = 1;
    constraint.gridy = 4;
    this.add(enableBox,constraint);
    
    JButton addButton = new JButton("Add");
    addButton.addActionListener(new ActionListener(){
      @Override
      public void actionPerformed(ActionEvent arg0){
        if(lightsBox.getItemCount()> MAX_LIGHT_NUM){
        	Reporter.showError("There are already 10 lights. Please reduce!");
        	return;
        }
    	sv.addLight();
        lightsBox.addItem(makeObj("light " + lightsBox.getItemCount()));
        setVisible(true);
        activate(true);
        lightsBox.setSelectedIndex(lightsBox.getItemCount()-1);
        //reset();
      }
    });
    constraint.gridx = 0;
    constraint.gridy = 5;
    this.add(addButton,constraint);
    
    colorButton = new JButton("Change Color");
    colorButton.addActionListener(new ActionListener(){
      @Override
      public void actionPerformed(ActionEvent arg0) {
        if(lightsBox.countComponents() == 0) return;
        int index = lightsBox.getSelectedIndex();
        LightBranchGroup light = sv.getLight(index);
        if(light==null){
          Reporter.showError("wrong light lindex!");
          return;
        }  
        Color lastColor = light.getColor();
        Color newColor = JColorChooser.showDialog(sv.getMainFrame(),"Choose the light's color",lastColor) ;  
        if(newColor==null){
            Reporter.showError("Please choose a color!");
            return;
        }
        light.setColor(newColor);
        light.setEnable(true);
        enableBox.setSelected(true);
        //sv.setLightColor(newColor, index);
        
      }
    });
    colorButton.setEnabled(false);
    constraint.gridx = 1;
    constraint.gridy = 5;
    this.add(colorButton,constraint);
    
    delButton = new JButton("Delete");
    delButton.addActionListener(new ActionListener(){
      @Override
      public void actionPerformed(ActionEvent arg0){
        if(lightsBox.countComponents() == 0) return;
        int index = lightsBox.getSelectedIndex();
        sv.delLight(index);
        lightsBox.removeItemAt(index);
        if(lightsBox.getItemCount()==0) activate(false);
      }
    });
    delButton.setEnabled(false);
    constraint.gridx = 0;
    constraint.gridy = 6;
    this.add(delButton,constraint);
    
    delAllButton = new JButton("Delete All");
    delAllButton.addActionListener(new ActionListener(){
      @Override
      public void actionPerformed(ActionEvent arg0){
        if(lightsBox.countComponents() == 0) return;
        int indeces = lightsBox.getItemCount();
        sv.delAllLight(indeces);
        lightsBox.removeAllItems();
        //reset();
        activate(false);
        
      }
    });
    delAllButton.setEnabled(false);
    constraint.gridx = 1;
    constraint.gridy = 6;
    this.add(delAllButton,constraint);
    
    // slide bar for X position change
    constraint.gridwidth = 2;
    constraint.gridx = 0;
    constraint.gridy = 7;
    this.add(new JLabel("X-Pos"),constraint);
    
    SpinnerNumberModel hSpModel = new SpinnerNumberModel(0, Math.round(-Double.MAX_VALUE), Math.round(Double.MAX_VALUE),1);
    horizontalSpinner = new JSpinner(hSpModel);
  
    horizontalSpinner.addChangeListener(new ChangeListener(){
      @Override
      public void stateChanged(ChangeEvent e){
        JSpinner source = (JSpinner)e.getSource();
        //Reporter.writeInfo("X-Pos changed");
        //if(source.isFocusOwner()){
          double xps=((SpinnerNumberModel)source.getModel()).getNumber().doubleValue();
          int index = lightsBox.getSelectedIndex();
          LightBranchGroup light = sv.getLight(index);
          if(light==null){
            //Reporter.writeError("wrong light lindex!");
            return;
          }
          light.setX(xps);
          light.setEnable(true);
          enableBox.setSelected(true);
          //sv.setLightX(xps,index);
          //Reporter.writeInfo("X-Pos focused");
        //}
      }
    });
    
    ((JSpinner.NumberEditor)horizontalSpinner.getEditor()).getTextField().setColumns(SPINNER_COLUMN_SIZE);
    constraint.gridy = 8;
    horizontalSpinner.setEnabled(false);
    this.add(horizontalSpinner,constraint);
    
    
    // Spinner for Y position change
    constraint.gridy = 9;
    this.add(new JLabel("Y-Pos"),constraint);
    
    SpinnerNumberModel vSpModel = new SpinnerNumberModel(0, Math.round(-Double.MAX_VALUE), Math.round(Double.MAX_VALUE),1);
    verticalSpinner = new JSpinner(vSpModel);
    verticalSpinner.addChangeListener(new ChangeListener(){
      @Override
      public void stateChanged(ChangeEvent e){
        JSpinner source = (JSpinner)e.getSource();
        //Reporter.writeInfo("Y-Pos changed");
        //if(source.isFocusOwner()){
          double yps=((SpinnerNumberModel)source.getModel()).getNumber().doubleValue();
          int index = lightsBox.getSelectedIndex();
          LightBranchGroup light = sv.getLight(index);
          if(light==null){
            //Reporter.writeError("wrong light lindex!");
            return;
          }
          light.setY(yps);
          light.setEnable(true);
          enableBox.setSelected(true);
          //sv.setLightY(yps,index);
          //Reporter.writeInfo("Y-Pos focused");
        //}
      }
    });
    
    ((JSpinner.NumberEditor)verticalSpinner.getEditor()).getTextField().setColumns(SPINNER_COLUMN_SIZE);
    constraint.gridy = 10;
    verticalSpinner.setEnabled(false);
    this.add(verticalSpinner,constraint);
    
    // Spinner for Z position change
    constraint.gridy = 11;
    this.add(new JLabel("Z-Pos"),constraint);
    
    SpinnerNumberModel dSpModel = new SpinnerNumberModel(0, Math.round(-Double.MAX_VALUE), Math.round(Double.MAX_VALUE),1);
    depthSpinner = new JSpinner(dSpModel);
    depthSpinner.addChangeListener(new ChangeListener(){
      @Override
      public void stateChanged(ChangeEvent e){
        JSpinner source = (JSpinner)e.getSource();
        //Reporter.writeInfo("Z-Pos changed");
        //if(source.isFocusOwner()){
          double zps=(int)((SpinnerNumberModel)source.getModel()).getNumber().doubleValue();
          int index = lightsBox.getSelectedIndex();
          LightBranchGroup light = sv.getLight(index);
          if(light==null){
            //Reporter.writeError("wrong light lindex!");
            return;
          }
          light.setZ(zps);
          light.setEnable(true);
          enableBox.setSelected(true);
          //sv.setLightZ(zps,index);
          //Reporter.writeInfo("Z-Pos focused");
        //}
      }
    });
    
    
    ((JSpinner.NumberEditor)depthSpinner.getEditor()).getTextField().setColumns(SPINNER_COLUMN_SIZE);
    constraint.gridy = 12;
    depthSpinner.setEnabled(false);
    this.add(depthSpinner,constraint);
    
    constraint.gridy = 13;
    this.add(new JLabel("Position: 'Ctrl + mouse drag'"),constraint);
    
    // Brightness adjuster
    brightnessSlider = new JSlider(JSlider.HORIZONTAL, 0, LightBranchGroup.MAX_LEVEL, (LightBranchGroup.MAX_LEVEL/2));
    brightnessSlider.addChangeListener(new ChangeListener(){
      @Override
      public void stateChanged(ChangeEvent e){
        JSlider source = (JSlider)e.getSource();
        if(!source.getValueIsAdjusting()){
          int index = lightsBox.getSelectedIndex();
          int brightness = (int)source.getValue();
          LightBranchGroup light = sv.getLight(index);
          if(light==null){
            Reporter.showError("wrong light lindex!");
            return;
          }
          light.setEnable(true);
          enableBox.setSelected(true);
          light.setBrightness(brightness);
          //sv.setBrightness(brightness, index);
          brightnessLabel.setText("Brightness: " + brightness);
          
        }
      }
    });
    constraint.gridy = 14;
    brightnessSlider.setEnabled(false);    
    this.add(brightnessSlider,constraint);
    
    constraint.gridy = 15;
    brightnessLabel = new JLabel("Brightness: " + LightBranchGroup.MAX_LEVEL/2);  
    this.add(brightnessLabel,constraint);
    
    radiusSlider = new JSlider(JSlider.HORIZONTAL, 0, Integer.MAX_VALUE, STARTING_RADIUS);
    radiusSlider.addChangeListener(new ChangeListener(){
      @Override 
      public void stateChanged(ChangeEvent e) {
        JSlider source = (JSlider)e.getSource();
        if (!source.getValueIsAdjusting()) {
          int fps = (int)source.getValue();
          int index = lightsBox.getSelectedIndex();
          LightBranchGroup light = sv.getLight(index);
          if(light==null){
            Reporter.showError("wrong light lindex!");
            return;
          }
          light.setArea(fps);
          //sv.setLightArea(fps,index);
          light.setEnable(true);
          enableBox.setSelected(true);
          radiusLabel.setText("radius: " + (int)light.getArea());
        }    
      }
    });
    constraint.gridy = 16;
    radiusSlider.setEnabled(false);    
    this.add(radiusSlider,constraint);
    
    constraint.gridy = 17;
    radiusLabel = new JLabel("radius: " + STARTING_RADIUS);
    this.add(radiusLabel,constraint); 
  }
  
  private Object makeObj(final String item)  {
       return new Object() { public String toString() { return item; } };
  }
  
  private void reset(){
    enableBox.setSelected(true);
    radiusSlider.setValue(STARTING_RADIUS);
    brightnessSlider.setValue(LightBranchGroup.MAX_LEVEL/2);
    verticalSpinner.setValue(0);
    horizontalSpinner.setValue(0);
    depthSpinner.setValue(0);
    radiusLabel.setText("radius: " + STARTING_RADIUS);
    brightnessLabel.setText("Brightness: " + LightBranchGroup.MAX_LEVEL/2);
    
  }
  
  private void setPanel(int index){
    LightBranchGroup light = sv.getLight(index); 
    if(light==null)return;
    sv.setLightMouseMove(index);
    if(light==null){
      Reporter.showError("wrong light lindex!");
      return;
    }
    enableBox.setSelected(light.getEnable());
    radiusSlider.setValue((int)light.getArea());
    radiusLabel.setText("radius: " + (int)light.getArea());
    brightnessSlider.setValue(light.getBrightness());
    brightnessLabel.setText("Brightness: " + light.getBrightness());
    verticalSpinner.setValue(light.getPosition().y);
    horizontalSpinner.setValue(light.getPosition().x);
    depthSpinner.setValue(light.getPosition().z);

  }
  
  /**
   * set activate/deactivate panel
   * @para on/off
   */
  public void activate(boolean act){
    enableBox.setEnabled(act);
    //lightsBox.setEditable(act);
    radiusSlider.setEnabled(act);
    brightnessSlider.setEnabled(act);
    colorButton.setEnabled(act);
    delButton.setEnabled(act);
    delAllButton.setEnabled(act);
    verticalSpinner.setEnabled(act);
    horizontalSpinner.setEnabled(act);
    depthSpinner.setEnabled(act);
    
  }
  
  /**
   * to set x-position value the light
   * @param x value
   */
  public void setX(double value){
    ((SpinnerNumberModel)horizontalSpinner.getModel()).setValue((double)value);
  }
  
  /**
   * to set y-position value the light
   * @param y value
   */
  public void setY(double value){
    ((SpinnerNumberModel)verticalSpinner.getModel()).setValue((double)value);
  }
  
  /**
   * to set z-position value the light
   * @param z value
   */
  public void setZ(double value){
    ((SpinnerNumberModel)depthSpinner.getModel()).setValue((double)value);
  }
  
  /**
   * to set changefactor of x-movement
   * @param factor
   */
  public void setXChangeFactor(double factor){
  	xChangeFactor = factor;
  }
  
  /**
   * to set changefactor of y-movement
   * @param factor
   */
  public void setYChangeFactor(double factor){
  	yChangeFactor = factor;
  }
  
  /**
   * to set changefactor of z-movement
   * @param factor
   */
  public void setZChangeFactor(double factor){
  	zChangeFactor = factor;
  }
  
  /**
   * getter changefactor of X
   * @return change factor of x-movement
   */
  public double getXChangeFactor(){
	  return xChangeFactor;
  }
  
  /**
   * getter changefactor of Y movement
   * @return change factor of y-movement 
   */
  public double getYChangeFactor(){
	  return yChangeFactor;
  }
  
  /**
   * getter changefactor of Z movement
   * @return change factor of z-movement
   */
  public double getZChangeFactor(){
	  return zChangeFactor;
  }
}
