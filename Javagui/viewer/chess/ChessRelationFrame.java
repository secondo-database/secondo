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

package  viewer.chess;

import javax.swing.*;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.*;
import java.util.Vector;
import javax.swing.event.*; 
import sj.lang.ListExpr;
import tools.Reporter;

 
public class ChessRelationFrame extends ChessObject implements ActionListener {
	private JSlider slide;
	private JButton autofor;
	private JButton stopp;
	private JButton faster;
	private JButton slower;
	private JButton oneback;
	private JButton onefor;
	private JTextField no;
	//private Border line = BorderFactory.createLineBorder(Color.black);
	//private Border empty = BorderFactory.createEmptyBorder();
	//private Border raisedbevel = BorderFactory.createRaisedBevelBorder();
	//private Border matte2Border = BorderFactory.createMatteBorder(2, 2, 2, 2, Color.white);
	private int showElem=0;
	private int maxElem=0;
	private Vector gameIncluded = new Vector();
	private boolean paused = false;
	private long waitTime = 1000;
	private Thread mythread;
	private int[] tupType;
	private String[] tupName;
	private int attrCount=0;
	private JTabbedPane subtabpane;
	private ChessObject co = null;
	private ListExpr types;
	
	public ChessRelationFrame(ListExpr tupletypes, ListExpr value) {
		try {
		    UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
		} catch (Exception exc) {
		    Reporter.debug("Error loading L&F: " , exc);
		}
		//System.out.println("hier relation");
		this.types = tupletypes;
		val = value;
		subtabpane = new JTabbedPane();
		this.parseTupleTypes(tupletypes);
		if (value.listLength() >0) this.maxElem = value.listLength()-1;
		JPanel buttonPanel = new JPanel();
		onefor = new JButton(" + ");
		oneback = new JButton(" - ");
		slower = new JButton("<<");
		faster = new JButton(">>");
		autofor = new JButton(" > ");
		stopp = new JButton(" || ");
		autofor.setEnabled(true);
		slower.setEnabled(false);
		faster.setEnabled(false);
		oneback.setEnabled(false);
		onefor.setEnabled(true);
		stopp.setEnabled(false);
		autofor.setToolTipText("show the relation tuple by tuple");
		slower.setToolTipText("show the relation slower");
		faster.setToolTipText("show the relation faster");
		oneback.setToolTipText("show the previous tuple");
		onefor.setToolTipText("show the next tuple");
		autofor.setBorder(ChessObject.compound2);
		slower.setBorder(ChessObject.compound2);
		faster.setBorder(ChessObject.compound2);
		oneback.setBorder(ChessObject.compound2);
		onefor.setBorder(ChessObject.compound2);
		stopp.setBorder(ChessObject.compound2);
		no = new JTextField(3);
		no.setToolTipText("number of actual tuple");
		no.setText(" 0");
		no.setEditable(true);
		no.setBorder(ChessObject.linecompound2);
		no.setHorizontalAlignment(JTextField.RIGHT);
		slide = new JSlider(SwingConstants.HORIZONTAL);
		slide.setMinorTickSpacing(1);
		slide.setMajorTickSpacing(10);
		slide.setPaintTicks(true);
		slide.setSnapToTicks(true);
		slide.setEnabled(true);
		slide.setMaximum(maxElem);
		//slide.setPreferredSize(new Dimension(300, 25));
		buttonPanel.add(oneback);
		buttonPanel.add(onefor);
		buttonPanel.add(autofor);
		buttonPanel.add(stopp);
		buttonPanel.add(faster);
		buttonPanel.add(slower);
		buttonPanel.add(no);
		createPanes(value.first());
		oneback.addActionListener(this);
		onefor.addActionListener(this);
		faster.addActionListener(this);
		slower.addActionListener(this);
		autofor.addActionListener(this);
		stopp.addActionListener(this);
		no.addActionListener(new NoListener());
		slide.addChangeListener(new SlideListener());
		//System.out.println("rel building ready"+ maxElem ); 
		//GridBagLayout gridbag = new GridBagLayout();
        //GridBagConstraints c = new GridBagConstraints();
		JPanel northPanel = new JPanel(new GridLayout(0,2));
		//c.fill = GridBagConstraints.BOTH;
		//c.gridwidth = GridBagConstraints.RELATIVE;
        //c.weightx = 1.0;
		northPanel.add(buttonPanel);
		//c.weightx = 0.0;  
		//c.fill = GridBagConstraints.HORIZONTAL;
		//c.gridwidth = GridBagConstraints.REMAINDER;
		northPanel.add(slide);
		this.setLayout(new BorderLayout());
		this.add(northPanel, BorderLayout.NORTH);
		this.add(subtabpane, BorderLayout.CENTER);
	}
	
	public void actionPerformed(ActionEvent arg0) {
		if (arg0.getActionCommand().equals(" + ")) {
			// one for
			if (this.showElem < this.maxElem) {
				this.showElem++;
				slide.setValue(this.showElem);
				no.setText(Integer.toString(this.showElem));
				updatePanes(theNthElement(val,this.showElem+1));
			}
			oneback.setEnabled(this.showElem > 0);
			onefor.setEnabled(this.showElem < this.maxElem);
		}
		if (arg0.getActionCommand().equals(" - ")) {
			// one back
			if (this.showElem > 0) {
				this.showElem--;
				slide.setValue(this.showElem);
				no.setText(Integer.toString(this.showElem));
				updatePanes(theNthElement(val,this.showElem+1));
			}
			oneback.setEnabled(this.showElem > 0);
			onefor.setEnabled(this.showElem < this.maxElem);
		}
		if (arg0.getActionCommand().equals(" > ")) {
			// auto play
			if (!paused ) {
				showElem=0;
				slide.setValue(this.showElem);
				no.setText(Integer.toString(this.showElem));
			}
			autofor.setEnabled(false);
			slower.setEnabled(true);
			faster.setEnabled(true);
			oneback.setEnabled(false);
			onefor.setEnabled(false);
			stopp.setEnabled(true);
			paused = false;
			mythread = new PlayThread();
			mythread.start();
		}
		if (arg0.getActionCommand().equals(" || ")) {
			// auto play paused
			paused = true;
			autofor.setEnabled(true);
			slower.setEnabled(false);
			faster.setEnabled(false);
			oneback.setEnabled(this.showElem > 0);
			onefor.setEnabled(this.showElem < this.maxElem);
			stopp.setEnabled(false);
			if (mythread.isAlive()) mythread.interrupt();
		}
		if (arg0.getActionCommand().equals(">>")) {
			// faster
			if ( mythread.isAlive()) waitTime = (long)waitTime*2;
		}
		if (arg0.getActionCommand().equals("<<")) {
			// slower
			if ( mythread.isAlive()) waitTime = (long)waitTime/2;
		}
	}
	
	private void createPanes(ListExpr tupleValue) {
		if (!(tupleValue==null || tupleValue.isEmpty())) {
		for (int i =0; i < attrCount; i++) {
			ListExpr attrValue = theNthElement(tupleValue,i+1);
			//System.out.println( attrValue.writeListExprToString());
			switch (tupType[i]) {
				case 0: 
					co = new ChessGameFrame(attrValue);
					break;
				case 1: 
					co = new ChessMoveFrame(attrValue);
					break;
				case 2: 
					co = new ChessMaterialFrame(attrValue);
					break; 
				case 3: 
					co = new ChessPositionFrame(attrValue);	
					break;
				case 4:
					co = new ChessText(attrValue);
					break;
				default: 
			}   
			subtabpane.addTab(tupName[i], co); 
		}
		}
		slide.setValue(0);
		no.setText(Integer.toString(0));
	}

  ListExpr theNthElement(ListExpr orig, int n){
   ListExpr tmp = orig;
   for(int i=1;i<n;i++){
       tmp = tmp.rest();
   }
   return tmp.first();

  }


	
	private void updatePanes(ListExpr tupleValue) {
		for (int i =0; i < attrCount; i++) {
			ListExpr attrValue = theNthElement(tupleValue,i+1);
			//System.out.println(attrValue.writeListExprToString());
			try {				
				((ChessObject)subtabpane.getComponentAt(i)).update(attrValue);
			}
			catch (IndexOutOfBoundsException e) {
				e.printStackTrace();
			} 
		}
	}
	
	private void parseTupleTypes(ListExpr tupleTypes) {
		//ListExpr tupleTypes = this.types;
		this.attrCount = tupleTypes.listLength();
		//System.out.println(tupleTypes.writeListExprToString());
		this.tupType = new int[this.attrCount];
		this.tupName = new String[this.attrCount];
		for(int i=0;i<this.attrCount;i++) {
			ListExpr SubType = tupleTypes.first();
			tupName[i] = SubType.first().writeListExprToString();
			if (SubType.second().symbolValue().equals("chessgame")) {
				tupType[i] = 0;
				gameIncluded.addElement(new Integer(i));
			}	
			else	
			if (SubType.second().symbolValue().equals("chessmove") )
				tupType[i] = 1;
			else	
			if (SubType.second().symbolValue().equals("chessmaterial"))
				tupType[i] = 2;
			else
			if (SubType.second().symbolValue().equals("chessposition"))
				tupType[i] = 3;
			else 
				tupType[i] = 4;
		tupleTypes = tupleTypes.rest();
		}
		//System.out.println("parse types ready");
	}
	
	private class PlayThread extends Thread {
		public void run() {
			boolean interrupt = false;
			while (showElem <= maxElem && !interrupt){
				try {
					slide.setValue(showElem);
					no.setText(Integer.toString(showElem));
					updatePanes(theNthElement(val,showElem+1));
					showElem++;
					Thread.sleep(waitTime);
				} catch (InterruptedException e) {
					interrupt = true;
				}
			}
			autofor.setEnabled(true);
			slower.setEnabled(false);
			faster.setEnabled(false);
			oneback.setEnabled(showElem > 0);
			onefor.setEnabled(showElem < maxElem);
			stopp.setEnabled(false);
		}
	}

	private class SlideListener implements ChangeListener {
		public void stateChanged(ChangeEvent arg0) {
			int i= slide.getValue();
			if (showElem != i) { 
				showElem =i;	
				no.setText(Integer.toString(showElem));
				updatePanes(theNthElement(val,showElem+1));
				oneback.setEnabled(showElem > 0);
				onefor.setEnabled(showElem < maxElem);
			}
		}
	}

	private class NoListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			try {  // test if no input is a integer
				int actNo = Integer.parseInt(no.getText());
				if (actNo >maxElem) {
					actNo = maxElem;
				}
				if (actNo < 0) {
					actNo = 0;
					showElem=0;
				}
				updatePanes(theNthElement(val,showElem+1));
				no.setText(Integer.toString(actNo));
				slide.setValue(actNo);
				oneback.setEnabled(showElem > 0);
				onefor.setEnabled(showElem < maxElem);
			} catch (Exception e) { // if not set to old value
				no.setText(Integer.toString(showElem));
			}
		}
	}

	public boolean changeToEdit(boolean edit) {
		editModus = false;
		// TODO Auto-generated method stub
		return false;
	}

	public String getType() {
		return "rel";
	}

	public ListExpr getListExpr() {
		return val;
	}
	
	public String getListExprTypes() {
		StringBuffer ret = new StringBuffer();
		ListExpr myTypes = this.types;
		ret.append("rel(tuple([");
		for (int i= 0; i< myTypes.listLength();i++) {
			ListExpr SubType = myTypes.first();
			ret.append(SubType.first().writeListExprToString());
			ret.append(": ");
			ret.append(SubType.second().symbolValue());
			myTypes = myTypes.rest();
		}
		ret.append("]))");
		return ret.toString();
	}
	
	public void update(ListExpr value) {
		// TODO Auto-generated method stub
		val = value;

	}

	public boolean canEdit() {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean canExport() {
		// TODO Auto-generated method stub
		return (!gameIncluded.isEmpty());
	}
	
	public Vector getGameNo() {
		return gameIncluded;
	}

	public ChessObject getAttrType() {
		return (ChessObject)subtabpane.getComponentAt(subtabpane.getSelectedIndex());
	}
}


