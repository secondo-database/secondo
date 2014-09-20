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

package viewer.spacetimecube;

import java.awt.*;
import java.awt.event.*;
import gui.idmanager.*;
import java.lang.NumberFormatException;
import java.util.*;
import javax.swing.*;
import javax.vecmath.Color3f;
import tools.Reporter;
import viewer.SpaceTimeCubeViewer;

/**
 * Class representing the settings dialog from SpaceTimeCube viewer.
 * @author Franz Fahrmeier
 *
 */
public class SettingsDialog extends JFrame implements ActionListener, ItemListener {
	
	private JLabel labelDisplay, labelSO;
	private JLabel labelVertLines, labelColorSO, labelColorBackground, labelLineWidth;
	private JTextField tfSOred, tfSOgreen, tfSOblue, tfBackgroundRed, tfBackgroundGreen, 
		tfBackgroundBlue, tfLineWidth;
	private JCheckBox chbVertLines;
	private JComboBox cbSecondoObject;
	private JButton butOk, butCancel;
	private Hashtable<ID,float[]> objectColors;
		// holds color for each SecondoObject identified by a Secondo ID
		// float[] = values between 0 and 1
	private ID lastKey; // ID from the last SecondoObject handled in Combobox cbSecondoObject
	String lastSOred, lastSOgreen, lastSOblue;
	private SpaceTimeCubeViewer stcv;
	
	
	public SettingsDialog(SpaceTimeCubeViewer STCViewer) {
		stcv = STCViewer;
		
		objectColors = (Hashtable)stcv.getColorSO().clone();
		
		setTitle("SpaceTimeCube Settings");
		setSize(300,290);
		
		JSeparator sepHori1 = new JSeparator();
		
		// label section
		labelSO = new JLabel("SecondoObject settings:");
		labelSO.setFont(new Font("Arial", Font.BOLD, 14));
		labelDisplay = new JLabel("Display settings:");
		labelDisplay.setFont(new Font("Arial", Font.BOLD, 14));
		labelVertLines = new JLabel("Show vertical lines:");
		labelColorSO = new JLabel("Color (R/G/B):");
		labelColorBackground = new JLabel("Background color (R/G/B):");
		labelLineWidth = new JLabel("Line width:");
		//
		
		// combobox section
		cbSecondoObject = new JComboBox();
		//
		
		for (int i=0;i<stcv.getSecondoObjectList().size();i++) {
			cbSecondoObject.addItem(stcv.getSecondoObjectList().get(i).getName());
				// items are created out of SecondoObjects currently maintained in the STC viewer
		}
		
		cbSecondoObject.addItemListener(this);
		
		float[] actualSOCol = {0,0,0};
		if (stcv.getSecondoObjectList().size()>0) {
			ID key = stcv.getSecondoObjectList().get(0).getID();
			actualSOCol = objectColors.get(key); 
		}
		
		Color3f actualBackgroundCol = stcv.getColorCanvas(); // vector out of values between 0 and 1
		
		// textfield section
		String tmp;
		tfSOred = new JTextField();
		tfSOred.setPreferredSize(new Dimension(30, tfSOred.getPreferredSize().height));
		tmp = String.valueOf((int)(actualSOCol[0]*255)); // conversion to a RGB value necessary
		tfSOred.setText(tmp);
		lastSOred = tmp;
		tfSOgreen = new JTextField();
		tfSOgreen.setPreferredSize(new Dimension(30, tfSOgreen.getPreferredSize().height));
		tmp = String.valueOf((int)(actualSOCol[1]*255));
		tfSOgreen.setText(tmp);
		lastSOgreen = tmp;
		tfSOblue = new JTextField();
		tfSOblue.setPreferredSize(new Dimension(30, tfSOblue.getPreferredSize().height));
		tmp = String.valueOf((int)(actualSOCol[2]*255));
		tfSOblue.setText(tmp);
		lastSOblue = tmp;
		
		tfBackgroundRed = new JTextField();
		tfBackgroundRed.setPreferredSize(new Dimension(30, tfBackgroundRed.getPreferredSize().height));
		tfBackgroundRed.setText(String.valueOf((int)(actualBackgroundCol.getX()*255)));
		tfBackgroundGreen = new JTextField();
		tfBackgroundGreen.setPreferredSize(new Dimension(30, tfBackgroundGreen.getPreferredSize().height));
		tfBackgroundGreen.setText(String.valueOf((int)(actualBackgroundCol.getY()*255)));
		tfBackgroundBlue = new JTextField();
		tfBackgroundBlue.setPreferredSize(new Dimension(30, tfBackgroundBlue.getPreferredSize().height));
		tfBackgroundBlue.setText(String.valueOf((int)(actualBackgroundCol.getZ()*255)));
		
		tfLineWidth = new JTextField();
		tfLineWidth.setPreferredSize(new Dimension(10, tfLineWidth.getPreferredSize().height));
		tfLineWidth.setText(String.valueOf(stcv.getLineWidth()));
		//
		
		// checkbox section
		chbVertLines = new JCheckBox();
		chbVertLines.setSelected(stcv.isDrawVertLines());
		//
		
		// button section
		butOk = new JButton("OK");
		butOk.addActionListener(this);
		butCancel = new JButton("Cancel");
		butCancel.addActionListener(this);
		//
		
		JSeparator sepHori = new JSeparator();
		
		// layout section
		GridBagLayout gbl = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		gbc.fill=GridBagConstraints.HORIZONTAL;
		gbc.insets = new Insets(2,2,2,2);
		setLayout(gbl);
		
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.gridwidth = 4;
		gbc.gridheight = 1;
		gbl.setConstraints(labelSO, gbc);
		add(labelSO);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 4;
		gbc.gridheight = 1;
		gbl.setConstraints(cbSecondoObject, gbc);
		add(cbSecondoObject);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(labelColorSO, gbc);
		add(labelColorSO);
		
		gbc.gridx = 1;
		gbc.gridy += 0;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(tfSOred, gbc);
		add(tfSOred);
		
		gbc.gridx = 2;
		gbc.gridy += 0;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(tfSOgreen, gbc);
		add(tfSOgreen);
		
		gbc.gridx = 3;
		gbc.gridy += 0;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(tfSOblue, gbc);
		add(tfSOblue);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 4;
		gbc.gridheight = 1;
		gbl.setConstraints(sepHori1, gbc);
		add(sepHori1);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 4;
		gbc.gridheight = 1;
		gbl.setConstraints(labelDisplay, gbc);
		add(labelDisplay);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(labelVertLines, gbc);
		add(labelVertLines);
		
		gbc.gridx = 1;
		gbc.gridy += 0;
		gbc.gridwidth = 3;
		gbc.gridheight = 1;
		gbl.setConstraints(chbVertLines, gbc);
		add(chbVertLines);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(labelColorBackground, gbc);
		add(labelColorBackground);
		
		gbc.gridx = 1;
		gbc.gridy += 0;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(tfBackgroundRed, gbc);
		add(tfBackgroundRed);
		
		gbc.gridx = 2;
		gbc.gridy += 0;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(tfBackgroundGreen, gbc);
		add(tfBackgroundGreen);
		
		gbc.gridx = 3;
		gbc.gridy += 0;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(tfBackgroundBlue, gbc);
		add(tfBackgroundBlue);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(labelLineWidth, gbc);
		add(labelLineWidth);
		
		gbc.gridx = 1;
		gbc.gridy += 0;
		gbc.gridwidth = 1;
		gbc.gridheight = 1;
		gbl.setConstraints(tfLineWidth, gbc);
		add(tfLineWidth);
		
		JLabel tmpLabel = new JLabel("OpenGL only!");
		gbc.gridx = 2;
		gbc.gridy += 0;
		gbc.gridwidth = 2;
		gbc.gridheight = 1;
		gbl.setConstraints(tmpLabel, gbc);
		add(tmpLabel);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 4;
		gbc.gridheight = 1;
		gbl.setConstraints(sepHori, gbc);
		add(sepHori);
		
		JPanel panelBut = new JPanel(new FlowLayout());
		panelBut.add(butOk);
		panelBut.add(butCancel);
		
		gbc.gridx = 0;
		gbc.gridy += 1;
		gbc.gridwidth = 4;
		gbc.gridheight = 1;
		gbl.setConstraints(panelBut, gbc);
		add(panelBut);
		//
	}
	
	 /**
	  * Called when AWT itemstate changed.
	  */
	 public void itemStateChanged(ItemEvent e) {
		 // itemstate in secondoobject combobox changed
		 if (e.getSource() == cbSecondoObject) {
			 
			 if (checkEntries()) {
				 float SORedVal = (float)(Integer.parseInt(tfSOred.getText()))/255.0f;
				 float SOGreenVal = (float)(Integer.parseInt(tfSOgreen.getText()))/255.0f;
				 float SOBlueVal = (float)(Integer.parseInt(tfSOblue.getText()))/255.0f;
				 float[] colorSO = {SORedVal, SOGreenVal, SOBlueVal};
				 
				 if (lastKey != null) { // if an item has already been changed
					 objectColors.put(lastKey, colorSO); // store color changes per SecondoObject
				 }
				 else { // if no item has been changed yet
					 objectColors.put(stcv.getSecondoObjectList().get(0).getID(), colorSO);
				 }
				 
				 ID key = stcv.getSecondoObjectList().get(cbSecondoObject.getSelectedIndex()).getID();
				 lastKey = key;
				 float[] col = objectColors.get(key);
				 tfSOred.setText(String.valueOf((int)(col[0]*255)));
				 tfSOgreen.setText(String.valueOf((int)(col[1]*255)));
				 tfSOblue.setText(String.valueOf((int)(col[2]*255)));
				 
				 lastSOred = tfSOred.getText();
				 lastSOgreen = tfSOgreen.getText();
				 lastSOblue = tfSOblue.getText();
			 }
			 else { // values are reset
				 tfSOred.setText(lastSOred);
				 tfSOgreen.setText(lastSOgreen);
				 tfSOblue.setText(lastSOblue);
			 }
		 }
	 }
	
	/**
	 * Called when AWT action is performed.
	 */
	public void actionPerformed(ActionEvent e) {
		
		// OK button clicked
		if (e.getSource() == butOk) {
			
			if (checkEntries()) {
				
				float SORedVal = (float)(Integer.parseInt(tfSOred.getText()))/255.0f;
				float SOGreenVal = (float)(Integer.parseInt(tfSOgreen.getText()))/255.0f;
				float SOBlueVal = (float)(Integer.parseInt(tfSOblue.getText()))/255.0f;
				float[] colorSO = {SORedVal, SOGreenVal, SOBlueVal};
				ID key = stcv.getSecondoObjectList().get(cbSecondoObject.getSelectedIndex()).getID();
				objectColors.put(key, colorSO);
					// currently entered SO color values need to be stored
				
				float backgroundRedVal = (float)(Integer.parseInt(tfBackgroundRed.getText()))/255.0f;
				float backgroundGreenVal = (float)(Integer.parseInt(tfBackgroundGreen.getText()))/255.0f;
				float backgroundBlueVal = (float)(Integer.parseInt(tfBackgroundBlue.getText()))/255.0f;
				Color3f colorBackground = new Color3f(backgroundRedVal, backgroundGreenVal, backgroundBlueVal);
				
				float lineWidth = (float)(Float.parseFloat((tfLineWidth.getText())));
				
				// effective setting of the values
				stcv.setColorSO(objectColors);
				stcv.setDrawVertLines(chbVertLines.isSelected());
				stcv.setColorCanvas(colorBackground);
				stcv.setLineWidth(lineWidth);
				
				stcv.recompute(); // mandatory
				
				setVisible(false);
			}
		}
		
		// Cancel button clicked
		if (e.getSource() == butCancel) {
			setVisible(false);
		}
	}
	
	/*
	 * Checks all currently entered values and
	 * displays an exception dialog if necessary.
	 */
	private boolean checkEntries() {
		String msg = "";
		
		// check SO color entry
		try {			
			int SOred = Integer.parseInt(tfSOred.getText());
			int SOgreen = Integer.parseInt(tfSOgreen.getText());
			int SOblue = Integer.parseInt(tfSOblue.getText());
			
			if (SOred < 0 || SOred > 255 || SOgreen < 0 || SOgreen > 255 || 
					SOblue < 0 || SOblue > 255) {
				msg += "One or more RGB values for SO color not between 0 and 255.\n";
			}	
		} catch (NumberFormatException e) {
			msg += "One or more RGB values for SO color not between 0 and 255.\n";
		}
		
		// check background color entry
		try {
			int backgroundRed = Integer.parseInt(tfBackgroundRed.getText());
			int backgroundGreen = Integer.parseInt(tfBackgroundGreen.getText());
			int backgroundBlue = Integer.parseInt(tfBackgroundBlue.getText());
			
			if (backgroundRed < 0 || backgroundRed > 255 || backgroundGreen < 0 || 
					backgroundGreen > 255 || backgroundBlue < 0 || backgroundBlue > 255) {
				msg += "One or more RGB values for background color not between 0 and 255.\n";
			}
		} catch (NumberFormatException e) {
			msg += "One or more RGB values for background color not between 0 and 255.\n";
		}
		
		// check line width entry
		try {
			float lineWidth = Float.parseFloat(tfLineWidth.getText());
			
			if (lineWidth < 1.0f || lineWidth > 10.0f) {
				msg += "Line width value not between 1.0 and 10.0.\n";
			}
		} catch (NumberFormatException e) {
			msg += "Line width value not between 1.0 and 10.0.\n";
		}
		
		if (msg != "") {
			Reporter.reportWarning(msg, null, false, false, false);
			return false;
		} else {
			return true;
		}
	}

}
