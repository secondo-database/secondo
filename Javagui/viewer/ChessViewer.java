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

package viewer;

import javax.swing.*;
import java.util.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import gui.SecondoObject;
import sj.lang.ListExpr;
import tools.Reporter;
import viewer.chess.*;


/**
 * This class provides a viewer for the secondo database which shows chessgames and chessgames within relations.Most methods are inherited from the SecondoViewer class. Furthermore this class implements the viewer.chess.GameDeliverer interface, which delivers a vector of the currently shown chessgames.
 * */

public class ChessViewer extends SecondoViewer implements ChangeListener, GameDeliverer, PositionDeliverer, MetaDataDeliverer
{
  /**
  * current view which shows the current chessgames	
  */
  private GameView chessView;
  
  /**
  * panel to navigate through chessgames in the result
  */
  private GameNavigationPanel gameNavPanel;

  /**
  *panel to navigate through the moves of the current chessgame
  */
  private MoveNavigationPanel moveNavPanel;
  
  /**splitpanes to provide the layout*/
  private JSplitPane splitter,splitter2;

  /**scrollpane for the informationpanel*/
  private JScrollPane scroller;

  /**the current secondo object (which must contain a chessgame in any way), which is shown*/
  private SecondoObject currentObject;

  /**the informationpanel showing informations about the relation and the meta data of the current chessgame*/
  private InformationPanel infoPanel;

  /**tabbed pane to provide a layout for relations which have more than on attribute of type chessgame*/
  private JTabbedPane tabPane;

  /** contains the ChessViews needed for relations with more than one attribute of type chessgame*/
  private ArrayList chessViews;
  
  /** contains the chessgames if there is more than one chessgame to be shown **/
  private ArrayList chessGames;
  
  /** contains the other values in the relation if there are any. The values are put into HashMaps **/ 
  private ArrayList relationValues;
  
  /**panel which provides functioniality of building a query from a shown position. **/
  private QueryBuilder queryBuilder;
  
  /** ChessListParser which is used to parse the list format of a chessgame **/
  private ChessListParser listParser;
  
  /** Exporter which can export the currently shown chessgames*/
  private ChessFileExporter exporter;

  /** array to remember which game was lately selected in a tab*/
  private int[] selectedGamesHistory;

  /** remembers the last tab that was opened*/
  private int lastSelectedTab;
  
 /**
  * constructor which layouts the view of the ChessViewer.
  * */
public ChessViewer()
{	
  listParser = new ChessListParser();
  chessViews = new ArrayList();
  chessGames = new ArrayList();
  relationValues = new ArrayList();
  
  tabPane = new JTabbedPane(JTabbedPane.LEFT, JTabbedPane.WRAP_TAB_LAYOUT);
  chessView = new GameView();
  chessView.reset();
  chessViews.add(chessView);
  
  queryBuilder = new QueryBuilder(this, this);
  tabPane.addTab("chess game",chessView);
  tabPane.addChangeListener(this);
  infoPanel = new InformationPanel();
  moveNavPanel = new MoveNavigationPanel((GameView)chessViews.get(0));
  
  scroller = new JScrollPane(infoPanel);
  scroller.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
  scroller.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
  JScrollPane scroller2 = new JScrollPane(queryBuilder);
  scroller2.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
  scroller2.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
   this.setLayout(new BorderLayout());
  splitter = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, tabPane, scroller);
  setSplitterAttributes(splitter, true, 1.0, 0.5);
  splitter2 = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, scroller2, splitter);
  setSplitterAttributes(splitter2, false, 0.0, 0.0);
  splitter2.setEnabled(false);
  JToggleButton buildQuery = new JToggleButton("query...");
  buildQuery.addActionListener(new ActionListener()
		  {
			  public void actionPerformed(ActionEvent e)
	  		  {
				  if(splitter2.getDividerLocation() <= 1)
  				  {
					queryBuilder.showQueryBuilder();  
	  				splitter2.resetToPreferredSizes();
				  }
  				  else
  				  {
	  				splitter2.setDividerLocation(0.0);
				  }
			  }
		  });
  exporter = new ChessFileExporter(this, this);
  JButton export = new JButton("export...");
  gameNavPanel = new GameNavigationPanel(infoPanel, moveNavPanel, buildQuery, export);
  ActionListener exportListener = new ActionListener()
  {
	public void actionPerformed(ActionEvent e)
    	{
		exporter.showSaveDialog(gameNavPanel.getCurrentGame());			    
     	}
   };
  export.addActionListener(exportListener);  
  
  this.add(BorderLayout.CENTER, splitter2);
  this.add(BorderLayout.NORTH, gameNavPanel);
  this.add(BorderLayout.SOUTH, moveNavPanel);
  this.setVisible(true);
}

/**method to sum up changes on splitters*/
private void setSplitterAttributes(JSplitPane jsp, boolean oneTouch, double weight, double loc)
{
	jsp.setOneTouchExpandable(oneTouch);
	jsp.setDividerLocation(loc);
	jsp.setResizeWeight(weight);
}



/** this method parses a list expression to a chess game and manages its viewing*/
private void parseList(ListExpr newGame) throws Exception 
{

	this.clearAll(); //clean up
	gameNavPanel.clearAll();
	if(listParser.isChessGame(newGame)) //if only one chessgmae has to be shwon
	{
		chessGames = new ArrayList();
		ArrayList al = new ArrayList();
		chessGames.add(al);
		((ArrayList)chessGames.get(0)).add(listParser.parseChessGame(newGame));
		chessView = ((GameView)tabPane.getSelectedComponent());
		chessView.reset();
		moveNavPanel.setView(chessView);
		gameNavPanel.setGames((ArrayList)chessGames.get(0));
	}
	else //if there is more than one chessgame to be shown (relation)
	{
		ArrayList chessAttributeNames = new ArrayList();
		listParser.parseRelation(newGame, relationValues, chessGames, chessAttributeNames);
		tabPane.removeAll();
		Object[] chessNames = chessAttributeNames.toArray();
		for (int i = 0; i< chessNames.length;i++) //create tabs if multiple attributes of type chessgame are in one relation
		{
				GameView gv = new GameView();
				tabPane.addTab((String)chessNames[i], gv);
		}
		tabPane.setSelectedIndex(0);
		chessView = ((GameView)tabPane.getSelectedComponent());
		chessView.reset();
		moveNavPanel.setView(chessView);
		gameNavPanel.setRelations(relationValues);
		gameNavPanel.setGames((ArrayList)this.chessGames.get(0));
		selectedGamesHistory = new int[chessGames.size()];
		for (int i=0;i< selectedGamesHistory.length;i++)
			selectedGamesHistory[i] = 0;
		lastSelectedTab = 0;
		
	}
}

/**
 *  implementation of same method in SecondoViewer. Only objects which contain a chessgame are accepted
 */
public boolean addObject(SecondoObject o)
{
   if (isDisplayed(o))
       selectObject(o);
   else
   {
      if(!listParser.containsChessGame(o.toListExpr()))
	      return false;
      currentObject = o;
      try
      {
      	parseList(o.toListExpr());
      }
      catch(Exception e)
      {
	      e.printStackTrace();
	Reporter.showError(e.getMessage());   
	return false;      
      }
      return true;
   } 
   return true;
	
 }

/** 
 *Implemetation of same method in SecondoViewer.
 */
public boolean isDisplayed(SecondoObject o)
{
  if (currentObject == null)
	  return false;
  return currentObject.equals(o);

}

/**called when a object is to be removed or new SecondoObject hast to be displayed. Cleans up everything*/
private void clearAll()
{
	this.gameNavPanel.clearAll();
	this.moveNavPanel.clearMoves();
	this.infoPanel.clearAll();
	this.chessView.reset();
	this.chessViews.clear();
	this.relationValues.clear();
	this.chessViews.add(chessView);
	this.chessGames.clear();
	this.tabPane.removeAll();
	this.tabPane.addTab("chess game", chessView);	
	this.currentObject = null;
}

/**
 * Implemetation of same method in SecondoViewer
 */
public void removeObject(SecondoObject o)
{
	this.clearAll();
}

/**
 * Implemetation of same method in SecondoViewer. Returns 1.0 if o contains a chessgame 
 */
public double getDisplayQuality(SecondoObject o)
{
	if (listParser.containsChessGame(o.toListExpr()))
			return 1.0;
	return 0.0;
}

/**
 * Implemetation of same method in SecondoViewer
 */
public void removeAll()
{
	this.removeObject(this.currentObject);
}    

/**
 *  Implemetation of same method in SecondoViewer
*/
public boolean canDisplay(SecondoObject o)
{

    return listParser.containsChessGame(o.toListExpr());
}

/**
 *  Implemetation of same method in SecondoViewer. Returns null
*/
public MenuVector getMenuVector()
{
    return null;
}

/**
 *  Implemetation of same method in SecondoViewer
*/
public String getName()
{
    return "Chess Viewer";
}

 /**
 *  Implemetation of same method in SecondoViewer
*/
public boolean selectObject(SecondoObject o)
{
	if(currentObject.equals(o))
		return true;
	return false;
}

/**
 * method to get all chessgames of the current SecondoObject in one vector. Inherited from GameDeliverer
 */
public Vector getAllGames()
{
	Vector v = new Vector();
	for(int i =0;i< chessGames.size();i++)
	{
		ArrayList games = (ArrayList)chessGames.get(i);
		for(int j=0; j<games.size();j++)
		{
			v.add(games.get(j));
		}
	}
	return v;
}

/**
	cleans up everything, if needed from outside
*/
public void reset()
{
	this.clearAll();
}

public HashMap getMetaVals()
{
	return this.infoPanel.getValues();
}

public PositionData getCurrentPosition()
{
	return this.chessView.getCurrentPosition();
}


/**
 * Called when other tab is selected.
 */
public void stateChanged(ChangeEvent e)
{
	int selected = gameNavPanel.getSelectedGameIndex();
	if(selected  != -1)
	{
		this.gameNavPanel.clearGames();
		chessView = (GameView)this.tabPane.getSelectedComponent();
		chessView.clear();
		this.moveNavPanel.setView(chessView);
		this.gameNavPanel.setGames((ArrayList)this.chessGames.get(this.tabPane.getSelectedIndex()));
		try
		{
			selectedGamesHistory[lastSelectedTab]= selected;
			lastSelectedTab = tabPane.getSelectedIndex();
			this.gameNavPanel.setSelectedGameIndex(selectedGamesHistory[lastSelectedTab]);	
		}catch(Exception ex){} 
	}
}

}

