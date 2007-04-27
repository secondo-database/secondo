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

package  viewer;

import viewer.chess.*;

import  javax.swing.*;
import javax.swing.border.Border;
import  java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.LinkedList;
import java.util.Vector;
import java.io.*;
import  sj.lang.ListExpr;
import  gui.SecondoObject;
import  tools.Reporter;

 /**
 * this is a viewer for spatial and temporal spatial objects
 * but this viewer can display other query results
 */
public class ChessViewer extends SecondoViewer implements ActionListener {
	//private JTabbedPane tabPane;
	private Border compound1 = BorderFactory.createCompoundBorder(
			BorderFactory.createEmptyBorder(3,3,3,3),
	 		BorderFactory.createRaisedBevelBorder());
	
	private int typeOfObject;
	//private int idx = -1;
	private JComboBox comboBox = new JComboBox();
	private JPanel pane = new JPanel();
	private JButton edit, saveAs, cancel,addToQuery,makeQuery,export;
	private String objName;
	private ChessObject empty = new ChessText(ListExpr.stringAtom("no chessobject to show"));
	private ChessObject comp = empty;
   //0-chessgame,1-chessmove,2-chessmaterial,3-chessposition,4-relation,99-can't display
	private LinkedList objects = new LinkedList();
	private Vector sqlQuerys = new Vector();
	private Vector secQuerys = new Vector();
	private QueryCreateDialog queryDlg = null;
	private QuerySelectGameDialog gameDlg = null;
  /**
   * Creates a MainWindow with all its components, initializes Secondo-Server, loads the
   * standard-categories, etc.
   */
public ChessViewer() {  
	try {
      	UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
	} catch (Exception exc) {
      	Reporter.debug("Error loading L&F: " , exc);
	}
	setLayout(new BorderLayout());
	//comboBox.setBorder(BorderFactory.createEmptyBorder(2,2,2,2));
	comboBox.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent evt){
			if(VC !=null){
			int idx = comboBox.getSelectedIndex();
				if (idx>=0){
					try{
						SecondoObject CurrentObject = (SecondoObject)objects.get(idx);
						VC.selectObject(ChessViewer.this,CurrentObject);
						showObject(CurrentObject);
	                   	}
					catch(Exception e){}
				}
			}
	}});
	comboBox.setBorder(compound1);
	edit = new JButton(" edit ");
	edit.setEnabled(false);
	edit.setBorder(ChessObject.compound5);
	edit.addActionListener(this);
	edit.setToolTipText("change to edit-modus");
	saveAs = new JButton("save as");
	saveAs.addActionListener(this);
	saveAs.setEnabled(true);
	saveAs.setBorder(ChessObject.compound5);
	saveAs.setToolTipText("save object and leave edit-modus if edited");
	cancel = new JButton("cancel");
	cancel.setEnabled(false);
	cancel.addActionListener(this);
	cancel.setBorder(ChessObject.compound5);
	cancel.setToolTipText("leave edit-modus without saving");
	addToQuery = new JButton("add query");
	addToQuery.setEnabled(true);
	addToQuery.setBorder(ChessObject.compound5);
	addToQuery.addActionListener(this);
	addToQuery.setToolTipText("add object to query");
	makeQuery = new JButton("query");
	makeQuery.setEnabled(true);
	makeQuery.setBorder(ChessObject.compound5);
	makeQuery.addActionListener(this);
	makeQuery.setToolTipText("edit or execute the query");
	export = new JButton("export");
	export.setEnabled(false);
	export.setBorder(ChessObject.compound5);
	export.addActionListener(this);
	export.setToolTipText("export object to PGN-file");
	JPanel buttonPanel1 = new JPanel();
	JPanel buttonPanel2 = new JPanel();
	buttonPanel1.add(edit);
	buttonPanel1.add(saveAs);
	buttonPanel1.add(cancel);
	buttonPanel2.add(addToQuery);
	buttonPanel2.add(makeQuery);
	buttonPanel2.add(export);
	JPanel TopPanel = new JPanel(new GridLayout(1,4)); 
	TopPanel.add(comboBox);
	TopPanel.add(new JPanel());
	TopPanel.add(buttonPanel1);
	TopPanel.add(buttonPanel2);
	TopPanel.setBorder(BorderFactory.createMatteBorder(0, 0, 1, 0, Color.BLACK));
	add(BorderLayout.NORTH, TopPanel);
	add(BorderLayout.CENTER, pane);
	pane.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
	pane.setLayout(new BorderLayout());
	pane.add(comp,BorderLayout.CENTER);
	this.validate();
	this.setVisible(true);
}

  /**
  * The name is used in the menu of MainWindow.
  * @return the name of this viewer, used by MainWindow's title bar and menu.
  */
public String getName(){
	return "ChessViewer";
}   
 /** Adds a <code>SecondoObject</code> to the viewer.
  * @param o the object to be added.
  * @return <code>true</code> if this viewer can display o otherwise <code>false</code>.
  */ 
public boolean addObject(SecondoObject o){
	if (!canDisplay(o)) return false;
	//System.out.println("can display");
	if (isDisplayed(o))
		selectObject(o);
	else {
		comboBox.addItem(o.getName());
		comboBox.setSelectedItem(o.getName());
		objects.add(o);
		showObject(o);
		
	}
	this.paintImmediately(this.getVisibleRect());
	return  true;
} 
 /** Removes o from viewer if displayed. 
  * @param o the object to be removed.
  **/
 public void removeObject(SecondoObject o){
	if (isDisplayed(o)) {
		comboBox.removeItem(o.getName());
		objects.remove(o);
		if(objects.isEmpty()) {
			 pane.remove(comp);
			 comp = empty; 
			 edit.setEnabled(false);
		}
		else {
			 this.showObject((SecondoObject)objects.getFirst());
		} 
	}
} 

 /*
  *  Remove all objects from viewer.
  */
 public void removeAll(){
	 objects.clear();
	 comboBox.removeAll();
	 pane.remove(comp);
	 comp = empty; 
	 edit.setEnabled(false);
	 if(VC!=null)
		VC.removeObject(null);
 }

 /** 
  * Check if this viewer can display o.
  */
public boolean canDisplay(SecondoObject o) {
	ListExpr LE = o.toListExpr();
	typeOfObject=99;
	if(LE==null) {
		//TypeOfObject=99;
		return false;
	}   
	else{
		if(LE.listLength()!=2){
		//TypeOfObject=99;
			return false;
		}   
		ListExpr type = LE.first();
		if (type.isAtom() && type.atomType()== ListExpr.SYMBOL_ATOM) {
			if (type.symbolValue().equals("chessgame")){
				typeOfObject=0;
				return true;
			}
			if (type.symbolValue().equals("chessmove")){
				typeOfObject=1;
				return true;
			}
			if (type.symbolValue().equals("chessmaterial")){
				typeOfObject=2;
				return true;
			}
			if (type.symbolValue().equals("chessposition")){
				typeOfObject=3;
				return true;
			}
		} 
		if (type.listLength()==2 && (type.first().symbolValue().equals("rel"))) {
		// it is a relation
			ListExpr tupletype = type.second();
			//System.out.println(tupletype.writeListExprToString());
		// analyse Tuple
			if (tupletype.listLength()==2 && tupletype.first().symbolValue().equals("tuple")) {
		// it is a tuple
				ListExpr tuplelist = tupletype.second();	
				int l =tuplelist.listLength(); 
        while(!tuplelist.isEmpty()){
					if (tuplelist.first().second().isAtom()) {
						ListExpr elem = tuplelist.first().second();
						if (elem.atomType()== ListExpr.SYMBOL_ATOM &&
						(elem.symbolValue().equals("chessgame") ||
						elem.symbolValue().equals("chessmove") ||
						elem.symbolValue().equals("chessmaterial") ||
						elem.symbolValue().equals("chessposition"))) 
						{
							typeOfObject=4;
							//System.out.println("ready with true");
							// can display if any chessthing found
							return true;
						}
					}
				}
        tuplelist= tuplelist.rest();
			}
		}
	}
//System.out.println("ready with false");
//TypeOfObject=99;
return false;  
}


public double getDisplayQuality(SecondoObject o){
    if(canDisplay(o))
       return 1.0;
    else
       return 0;
}

 /** check if o displayed in the moment **/
public boolean isDisplayed(SecondoObject o){
	
	return (objects.contains(o));
}

 /** hightlighting of o **/
public boolean selectObject(SecondoObject o) {
	if (isDisplayed(o) && canDisplay(o)) {
		comboBox.setSelectedItem(o.getName());
		this.showObject(o);
		edit.setEnabled(true);
		return true;
	}
	else return false;
}

 /** Get the MenuExtension for MainWindow.
  *  This method should be overwritten if there is need for an own menu.
  *  @return The menu vector; we return null as we don't have one here.
  */
 public MenuVector getMenuVector(){
	 return null;
 }
 
 private void showObject(SecondoObject o) {
	if (canDisplay(o) && !objects.isEmpty()) {
		pane.remove(comp);
		if (comp.getEditModus()) {
			int i;
			boolean ok = false;
			StringBuffer message = new StringBuffer();
			do {
				Object[] options = { "OK", "CANCEL" };
				i = JOptionPane.showOptionDialog(this, "object is edited\n save?", "save", 
					JOptionPane.YES_NO_OPTION , JOptionPane.QUESTION_MESSAGE, 
					null, options, options[0]);
				if (i==0) { // let's save
					StringBuffer name = new StringBuffer(objName);
					ok = ChessObject.saveObject(message,name,comp.getType(), comp.getListExpr().writeListExprToString());
					if (ok) {
						if (name.equals(objName)) {
							// has been same objectname so update the object in the list
							ListExpr myLE = ListExpr.twoElemList(ListExpr.symbolAtom(comp.getType()), comp.getListExpr());
							((SecondoObject)objects.get(comboBox.getSelectedIndex())).fromList(myLE);
						}
						Reporter.showInfo(message.toString());
					}
					else {
						Reporter.showError(message.toString());
					}
				}	
			} while (!(ok || i == 1));
		}
		try {
			if (typeOfObject != 99 ) {
				ListExpr LE = o.toListExpr();
				//System.out.println(LE.writeListExprToString());
				pane.remove(comp);
				switch (typeOfObject) {
				case 0: comp = new ChessGameFrame(LE.second());break;
				case 1: comp = new ChessMoveFrame(LE.second());
					/*long myTime  = System.currentTimeMillis();
					while (System.currentTimeMillis() < myTime+60);
					((ChessMoveFrame)comp).showMove();*/
					break;
				case 2: comp = new ChessMaterialFrame(LE.second());break; 
				case 3: comp = new ChessPositionFrame(LE.second());break; 
				case 4: 
					comp = 
					new ChessRelationFrame(LE.first().second().second(),LE.second());
					break;
				default: break;
				}
				pane.add(comp,BorderLayout.CENTER);
				objName = o.getName().replaceFirst("query","").trim();
				cancel.setEnabled(false);
				edit.setEnabled(comp.canEdit());
				export.setEnabled(comp.canExport());
			}
		}
		catch (Exception e) {
			edit.setEnabled(false);
			export.setEnabled(false);
			e.printStackTrace();
		}
 		this.validate();
 		this.repaint();
 	}
}

	public void actionPerformed(ActionEvent e) {
		StringBuffer message = new StringBuffer();
		boolean ok = false;
		if (e.getActionCommand().equals(" edit ")) {
			// change to edit modus
			comp.changeToEdit(true);
			edit.setEnabled(false);
			cancel.setEnabled(true);
		}
		
		if (e.getActionCommand().equals("save as")) {
			StringBuffer name = new StringBuffer(objName);
			String typeLE = null;
			if (comp.getType().equals("rel")) {
				typeLE = ((ChessRelationFrame)comp).getListExprTypes();
				//System.out.println(typeLE);
				ok = ChessObject.saveObject(message,name, typeLE , comp.getListExpr().writeListExprToString());
			}
			else 
				ok = ChessObject.saveObject(message,name, comp.getType() , comp.getListExpr().writeListExprToString());	
			if (ok) {
				edit.setEnabled(true);
				cancel.setEnabled(false);
				comp.changeToEdit(false);
				if (name.equals(objName)) {
					//System.out.println("new"+ name + "ori"+ objName);
					// has been same objectname so update the object in the list
					SecondoObject o = (SecondoObject)objects.get(comboBox.getSelectedIndex());
					ListExpr myLE = ListExpr.twoElemList(o.toListExpr().first(), comp.getListExpr());
					((SecondoObject)objects.get(comboBox.getSelectedIndex())).fromList(myLE);
				}
				else {
					// new objectname show the old object again 
					showObject((SecondoObject)objects.get(comboBox.getSelectedIndex()));
				}
				Reporter.showInfo(message.toString());
			}
			else 
				Reporter.showError(message.toString());
		}
		
		if (e.getActionCommand().equals("cancel")) {
			// leave edit modus without save
			comp.changeToEdit(false);
			showObject((SecondoObject)objects.get(comboBox.getSelectedIndex()));
			edit.setEnabled(true);
			cancel.setEnabled(false);
		}
		
		if (e.getActionCommand().equals("add query")) {
			StringBuffer buf = new StringBuffer();
			if (comp.getType().equals("chessgame")) {
				if (gameDlg == null)
					gameDlg = new QuerySelectGameDialog(sqlQuerys, secQuerys, (ChessGameFrame)comp);
				else
					gameDlg.setComp((ChessGameFrame)comp);
				gameDlg.setLocationRelativeTo(null);
				gameDlg.setModal(true);
				gameDlg.setVisible(true);
			}
			else {
				if (comp.getType().equals("chessmove")) {
					for (int i = 0; i < 7; i++){
						String s = ((ChessMoveFrame)comp).getQuery(i);
						if (s!=null)
						sqlQuerys.add(s);
					}
					Reporter.showInfo("chessmove to query-list added");
				}
				else {
					if (comp.getType().equals("chessposition")) {
						StringBuffer name = new StringBuffer(objName);
						if (comp.getEditModus()) {
							ok =  ChessObject.saveObject(message,name,comp.getType(), comp.getListExpr().writeListExprToString());
//							leave edit modus
							if (ok) {
								if (name.equals(objName)) {
									// has been same objectname so update the object in the list
									ListExpr myLE = ListExpr.twoElemList(ListExpr.symbolAtom(comp.getType()), comp.getListExpr());
									((SecondoObject)objects.get(comboBox.getSelectedIndex())).fromList(myLE);
								}
								else {
									// new objectname show the old object again 
									showObject((SecondoObject)objects.get(comboBox.getSelectedIndex()));
								}
								comp.changeToEdit(false);
								edit.setEnabled(true);
								cancel.setEnabled(false);
							}
						}	
						else {
							name = new StringBuffer(objName);
						}	
						if (name.length()!=0) {
							buf.append(name);
							buf.append(" includes pos");
							sqlQuerys.add(buf.toString());
							Reporter.showInfo(message.insert(0,"\nchessposition to query-list added").toString());
						}
						else 
							Reporter.showWarning(message.insert(0,"nothing added \n no pos selected").toString());
					}
					else {
						if (comp.getType().equals("chessmaterial")) {
							StringBuffer name= new StringBuffer(objName);
							if (comp.getEditModus()) {
								ok= ChessObject.saveObject(message, name, comp.getType(), comp.getListExpr().writeListExprToString());
//								 leave edit modus
								if (ok) {
									if (name.equals(objName)) {
										// has been same objectname so update the object in the list
										ListExpr myLE = ListExpr.twoElemList(ListExpr.symbolAtom(comp.getType()), comp.getListExpr());
										((SecondoObject)objects.get(comboBox.getSelectedIndex())).fromList(myLE);
									}
									else {
										// new objectname show the old object again 
										showObject((SecondoObject)objects.get(comboBox.getSelectedIndex()));
									}
									comp.changeToEdit(false);
									edit.setEnabled(true);
									cancel.setEnabled(false);
								}
							}	
							else {
								name = new StringBuffer(objName);
							}	
							if (name.length()!=0) {
								buf.append("(pieces(pos)) = ");
								buf.append(name);
								sqlQuerys.add(buf.toString());
								Reporter.showInfo(message.insert(0,"\nchessmaterial to query-list added").toString());
							}
							else 
								Reporter.showWarning(message.insert(0,"\nnothing added").toString());
						}
						else {
							if (comp.getType().equals("rel")) {	
//				get the actual component 
								ChessObject attrComp = ((ChessRelationFrame)comp).getAttrType();
								if (attrComp.getType().equals("chessgame")) {
									if (gameDlg == null)
										gameDlg = new QuerySelectGameDialog(sqlQuerys, secQuerys, (ChessGameFrame)attrComp);
									else
										gameDlg.setComp((ChessGameFrame)attrComp);
									gameDlg.setLocationRelativeTo(null);
									gameDlg.setModal(true);
									gameDlg.setVisible(true);
								}
								else {
									if (attrComp.getType().equals("chessmove")) {
										for (int i =0; i <7; i++){
											String s = ((ChessMoveFrame)attrComp).getQuery(i);
											if (s!=null)
											sqlQuerys.add(s);
										}
										Reporter.showInfo("chessmove to query-list added");
									}
									else {
										if (attrComp.getType().equals("chessposition")) {
											ListExpr lePos = attrComp.getListExpr();
											StringBuffer name = new StringBuffer();
											ok = ChessObject.saveObject(message, name, "chessposition", lePos.writeListExprToString());
											if (ok && name.length()!=0) {
												buf.append(name);
												buf.append(" includes pos");
												sqlQuerys.add(buf.toString());
												Reporter.showInfo(message.insert(0,"\nchessposition to query-list added").toString());
											}
											else 
												Reporter.showWarning(message.insert(0, "\nnothing added").toString());
										}
										else {
											if (attrComp.getType().equals("chessmaterial")) {
												ListExpr leMat = attrComp.getListExpr();
												StringBuffer name = new StringBuffer();
												ok = ChessObject.saveObject(message, name, "chessmaterial", leMat.writeListExprToString());
												if (ok && name.length()!=0) {
													buf.append("(pieces(pos)) = ");
													buf.append(name);
													sqlQuerys.add(buf.toString());
													Reporter.showInfo(message.insert(0,"\nchessmaterial to query-list added").toString());
												}
												else 
													Reporter.showWarning(message.insert(0, "nothing added").toString());
											}
											else Reporter.showError("can't create query from this");
										}
									}
								}
							}
							else Reporter.showError("can't create query from this");
						}
					}
				}
			}
		}	
		
		if (e.getActionCommand().equals("query")) {
			if (queryDlg == null) 
				queryDlg= new QueryCreateDialog(sqlQuerys, secQuerys,VC);
			queryDlg.setLocationRelativeTo(null);
			queryDlg.setModal(true);
			queryDlg.setVisible(true);
		}
		
		if (e.getActionCommand().equals("export")) {
			String filename = JOptionPane.showInputDialog(this,
            		"Insert a filename :", objName+".pgn");
			if(comp.getType().equals("chessgame")) {
				if (savePGNgame(comp.getListExpr(), filename)) // and Export
					Reporter.showInfo("export chessgame success");
				else	
					Reporter.showError("export chessgame failed");
			}	
			if(comp.getType().equals("rel")) {
				if (savePGNrel(comp.getListExpr(), ((ChessRelationFrame)comp).getGameNo(), filename)) // and Export
					Reporter.showInfo("export chessrelation success");
				else	
					Reporter.showError("export chessrelation failed");
			}
		}
	}
	
	/*
	 * exports a relation to pgn-file 
	 * only the attributes of chessgames will be exported, 
	 * other attributes will be ignored
	 */
	private boolean savePGNrel(ListExpr rel, Vector attrNo, String filename) {
		// attrNo numbers of indexes with game !
		boolean retVal = true;
    while(!rel.isEmpty()){
			ListExpr ll = rel.first();
      rel = rel.rest();
			for (int j =0; j < attrNo.size(); j++) {
				retVal = savePGNgame(ll.first(), filename);
        ll = ll.rest();
				if (!retVal) break;
			}
		}
		return retVal;
	}
	
	/*
	 * exports a single chessgame to pgn-file 
	 *  
	 */
	private boolean savePGNgame(ListExpr game, String filename) {
		boolean ret = true;
		ListExpr sixMeta = game.first();
		//second are the rest five metadatas
		ListExpr fiveMeta = game.second();
		ListExpr gameMoves = game.third().first();
		try {
			//Open file
			PrintWriter out = new PrintWriter(new FileWriter(filename));
			
			//Metadata
			out.println("[Event" + sixMeta.first().writeListExprToString().replace('\n',' ') + "]");
			out.println("[Site" + sixMeta.second().writeListExprToString().replace('\n',' ') + "]");
			out.println("[Date" + sixMeta.third().writeListExprToString().replace('\n',' ') + "]");
			out.println("[Round" + sixMeta.fourth().writeListExprToString().replace('\n',' ') + "]");
			out.println("[White" + sixMeta.fifth().writeListExprToString().replace('\n',' ') + "]");
			out.println("[Black" + sixMeta.sixth().writeListExprToString().replace('\n',' ') + "]");
			out.println("[Result" + fiveMeta.first().writeListExprToString().replace('\n',' ') + "]");
			out.println("[WhiteElo" + fiveMeta.second().writeListExprToString().replace('\n',' ') + "]");
			out.println("[BlackElo" + fiveMeta.third().writeListExprToString().replace('\n',' ') + "]");
			out.println("[EventDate" + fiveMeta.fourth().writeListExprToString().replace('\n',' ') + "]");
			out.println("[ECO" + fiveMeta.fifth().writeListExprToString().replace('\n',' ') + "]");
			
			out.println();
			
			int nr = 1;
			ListExpr actMove;
			//The moves
			while(gameMoves.listLength() >= 1) {
				//white turn
				actMove = gameMoves.first();
				gameMoves = gameMoves.rest();
			
				out.print(Integer.toString(nr) + ". ");
				out.print(getPGNMove(actMove.first().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""),
						     actMove.second().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""),
					             actMove.third().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""),
						     actMove.fourth().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""),
						     actMove.fifth().writeListExprToString().replaceAll("\n", "").replaceAll("\"", "")
						    ) + " ");
				
				//black turn
				if(gameMoves.listLength() >= 1) {
					actMove = gameMoves.first();
					gameMoves = gameMoves.rest();
								
					out.print(getPGNMove(
						     actMove.first().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""),
						     actMove.second().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""),
					             actMove.third().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""),
						     actMove.fourth().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""),
						     actMove.fifth().writeListExprToString().replaceAll("\n", "").replaceAll("\"", "")
						  ) + " ");
				}
				
				nr++;
			
			}
			//Add result
			out.print(fiveMeta.first().writeListExprToString().replaceAll("\n", "").replaceAll("\"", ""));
			
			out.println("");
			out.close();
			
		}catch(Exception e) {System.out.println(e.getMessage()); ret = false;}
		
		
		return ret; 
		// return true if success false if error happend
	}
	
	/*
	 * creates a pgn-formatted-String from the movesdates
	 *   
	 */
	private String getPGNMove(String figurename, String startpos, String endpos, String action, String capturedFigure) {
		String ret = "";
		//no castling
		if(action.charAt(2) == '-') {
			//add figure name
			if(!figurename.equals("p") && !figurename.equals("P")) 
				ret = figurename.toUpperCase();
			
			//add startposition
			ret = ret + startpos;
		
			//captured Figure. add x
			if(!capturedFigure.equals("-"))
				ret = ret + "x";
			
			//add endposition
			ret = ret + endpos;
			
			//pawn Promotion
			if(action.charAt(1) != '-') {
				ret = ret + "=" + action.charAt(1);
			}
			//check
			if(action.charAt(0) == 'c')
				ret = ret + "+";
			
			//checkmate
			if(action.charAt(0) == 'm')
				ret = ret + "#";
		}
		//short side castling
		else if(action.charAt(2) == 's')
			ret = "O-O";
		//long side casting
		else if(action.charAt(3) == 'l')
			ret = "O-O-O";
		 
		return ret;
	}

	

 
}

