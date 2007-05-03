package viewer.chess;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import viewer.*;

/**
 * This class provides a panel which offers functionality to navigate through a list of games
 */
public class GameNavigationPanel extends JPanel
{
  /**
   * combobox to choose the game to be shown
   */
  private JComboBox gameChooser;

  /**
   * button for exporting a game
   */
  private JButton export;

  /**
   * toggle button to start the QueryBuilder
   */
  private JToggleButton createQuery;

  /**
   * the InformationPanel which shows further information of the current relation
   */
  private InformationPanel infoPanel;

  /**
   * arraylist of hashmaps which contain  further values in this relation
   */
  private ArrayList relationVals;

  /**
   * the QueryBuilder which will be shown by pressing the createQuery button
   */
  private QueryBuilder queryBuilder;

  /**
   * the MoveWatcher which shows the moves of the game
   */
  private MoveWatcher moveNavi;

  /**
   * constructor which needs a InformationPanel to show further values of the relation, a MoveWatcher to show the moves of the current game and two buttons for further functionality
   */
  public GameNavigationPanel(InformationPanel infoPanel, MoveWatcher m, JToggleButton createQuery, JButton export)
  {
    this.createQuery = createQuery;
    relationVals = null;
    moveNavi = m;

    this.infoPanel = infoPanel;
    
    this.gameChooser = new JComboBox();
    this.gameChooser.setMinimumSize(new Dimension(350,24));
    this.gameChooser.setPreferredSize(new Dimension(350,24));
    this.gameChooser.addActionListener(new ActionListener()
		    {
			    public void actionPerformed(ActionEvent e)
	    		    {
				showGame();     
			    }
		    });
    this.export = export;
    this.export.setEnabled(false);
  FlowLayout f = new FlowLayout(FlowLayout.LEFT, 2,2);
  this.setLayout(f);
  this.add(new JLabel("game: "));
  this.add(gameChooser);
  this.add(new JLabel("  options: "));
  this.add(this.export);
  this.add(createQuery);
    
  }

  /**
   * Setter for the currently selected game
   */
  public void setSelected(GameData g)
  {
	  this.gameChooser.setSelectedItem(g);
  }

  /**
   * adds the game g to the list of games
   */
  public void addGame(GameData g)
  {
	  this.gameChooser.addItem(g);
  }

  /**
   * removes all games from the list
   */
  public void clearGames()
  {
	  this.gameChooser.removeAllItems();
  }

  /**
   * returns the index of the currently selected game
   */
  public int getSelectedGameIndex()
  {
	  return gameChooser.getSelectedIndex();
  }

  /**
   * sets the currently selected game to the game with the specified index
   */
  public void setSelectedGameIndex(int index)
  {
	  this.gameChooser.setSelectedIndex(index);
  }

  /**
   * resets the GameNavigationPanel
   */
  public void clearAll()
  {
	  this.clearGames();
	  relationVals = null;
	  export.setEnabled(false);
  }

  /**
   * sets the list of shown games to the games in the arraylist
   */
  public void setGames(ArrayList games)
  {
	  Object[] o = games.toArray();
	  clearGames();
	  for (int i=0; i< o.length; i++)
	  {
		  ((GameData)o[i]).setNo(i+1);
		  this.gameChooser.addItem((GameData)o[i]);
	  }
	  if (o.length>0)
		  export.setEnabled(true);
  }

  /**
   * sets the list of further relationValues to relationVals
   */
  public void setRelations(ArrayList relationVals)
  {
	  this.relationVals = relationVals;
  }

  /**
   * returns the currently shown game
   */
  public GameData getCurrentGame()
  {
	  return (GameData)this.gameChooser.getItemAt(this.gameChooser.getSelectedIndex());
  }

  /**
   * this method is called when another chessgame is selected
   */
  private void showGame()
  {
	  GameData current = (GameData)gameChooser.getSelectedItem(); //get the newly selected game
	  if(current != null)
	  {
		  moveNavi.setCurrentGame(current); //show the moves of the current game
		  infoPanel.showMetaData(current.getEvent(), current.getSite(), current.getDate(),current.getRound(),current.getWhite(),current.getBlack(),current.getResult(),current.getTags()); //show tag- values in the InformationPanel
		  if (relationVals != null) //if possible show other relation values in the InformationPanel
		  	infoPanel.showRelationValues((HashMap)relationVals.get(gameChooser.getSelectedIndex()));
		  gameChooser.setToolTipText(current.toString()); 
	  }
  }

  

 
}
