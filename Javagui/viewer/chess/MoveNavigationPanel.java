package viewer.chess;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import viewer.*;

/**
 * This class provides a panel for viewing and controling the moves of a chessgame. To achieve this MoveNavigationPanel is a direct subclass of JPanel and implements the MoveWatcher Interface.
 */
public class MoveNavigationPanel extends JPanel implements MoveWatcher
{

/**
 * This inner class provides functionality to switch through the moves of a chessgame in correct order automatically. ChessPlayer is a subclass of Thread. 
 */
  private class ChessPlayer extends Thread
  {
	public void run()
	{
		int selected = moveChooser.getSelectedIndex()+1; //get selected movenumber
		while (selected < moveChooser.getItemCount()  && playing) //as long as there is no interruption and there are more moves to be shown
		{
			moveChooser.setSelectedIndex(selected);  //set move selection 
			selected++;	
			try
			{
				Thread.sleep(1500); 
			}
			catch (InterruptedException e)
			{
						
			}
		}
		if(playing) //if there was no interruption of playing by the user
			play.doClick();
			
	}	
  }
  
  /**
   * combobox for selecting the move
   */
  private JComboBox moveChooser; 

  /**
   * buttons for controlling the current move
   */
  private JButton begin, end , fwd, back;

  /**
   * button for automatically showing all moves 
   */
  private JToggleButton play;

  /**
   * the view where the move is shown in
   */
  private GameView theGameView;

  /**
   * the game whose moves are shown at the moment
   */
  private GameData currentGame;

  /**
   * this field is checked by the ChessPlayer class and set/reset by the play button. While this is true the ChessPlayer - Thread shows the next move on a regular basis. 
   */
  private boolean playing;

  /**
   * constructor of MoveNavigationPanel needs a GameView object where the moves have to be shown in
   */
  public MoveNavigationPanel(GameView gv)
  {
    this.theGameView = gv;
    
    this.moveChooser = new JComboBox(); //set up the move chooser
    this.moveChooser.setMinimumSize(new Dimension(350,24));
    this.moveChooser.setPreferredSize(new Dimension(350,24));
    this.moveChooser.addActionListener(new ActionListener()
		    {
			    public void actionPerformed(ActionEvent e)
	    		    {
				showMove();    
			    }
			    
		    });

    JPanel movePanel = new JPanel();
    movePanel.setLayout(new GridLayout(1,4));


    this.begin = new JButton("<<"); //button to return to the first move
    begin.addActionListener(new ActionListener()
		    {
			    public void actionPerformed(ActionEvent e)
    			    { 
				    if(moveChooser.getItemCount()>0)
				    	moveChooser.setSelectedIndex(0);	    
			    }
		    });
    this.back = new JButton("<"); //button to go back one move
    back.addActionListener(new ActionListener()
		    {
			    public void actionPerformed(ActionEvent e)
    			    {
				    if (moveChooser.getItemCount()>0)
				    	moveChooser.setSelectedIndex(moveChooser.getSelectedIndex()>0?moveChooser.getSelectedIndex()-1:0);
			    }
		    });
    this.end = new JButton(">>"); //button to go to the last move
    end.addActionListener(new ActionListener()
		    {
			    public void actionPerformed(ActionEvent e)
	    		    {
				    moveChooser.setSelectedIndex(moveChooser.getItemCount()-1);
			    }	    
		    });
    this.fwd = new JButton(">"); //button to go to the next move
    fwd.addActionListener(new ActionListener()
		    {
			    public void actionPerformed(ActionEvent e)
	    		    {
				   moveChooser.setSelectedIndex(moveChooser.getSelectedIndex() < (moveChooser.getItemCount()-1)?moveChooser.getSelectedIndex()+1:moveChooser.getItemCount()-1); 
			    }
		    });
    this.play = new JToggleButton("play");
    play.addActionListener(new ActionListener()
		    {
			    public void actionPerformed(ActionEvent e)
			    {
				if(play.isSelected()) //if the button is selected 
				{
					play.setText("stop"); //the text on the button is changed and all other ways of manipulating the current move are disabled
					begin.setEnabled(false);
					back.setEnabled(false);
					end.setEnabled(false);
					fwd.setEnabled(false);
					moveChooser.setEnabled(false);
					playing = true;
					if(currentGame != null)
	    				{
						ChessPlayer cp = new ChessPlayer(); //a new Thread controls showing the moves 
						cp.start();
					}
				}
				else
				{
					play.setText("play"); //if not selected all other ways of manipulating the current move are enabled
					begin.setEnabled(true);
					back.setEnabled(true);
					end.setEnabled(true);
					fwd.setEnabled(true);
					moveChooser.setEnabled(true);
					playing = false;
				}	
			    }	    
		    });
    play.setPreferredSize(new Dimension(65,24));
    movePanel.add(begin);
    movePanel.add(back);
    movePanel.add(play);
    movePanel.add(fwd);
    movePanel.add(end);

  FlowLayout f = new FlowLayout(FlowLayout.LEFT, 2,2);
  this.setLayout(f);
  this.add(new JLabel("move: "));
  this.add(moveChooser);
  this.add(new JLabel("  control: "));
  this.add(movePanel);
    
  }

  /**
   * inherited from chess.MoveWatcher, sets the current game whose moves are displayed.
   */
  public void setCurrentGame(GameData g)
  {
	if (playing)
		play.doClick(); //stop showing the current moves if necessary
	if(g != null)
	{
		theGameView.clear();		//clear the view
		int selected = g.getCurrentMoveIndex();
		this.currentGame = g;
		moveChooser.removeAllItems(); //remove everything
		Object[] movesOfGame = g.getMoves();
		MoveData initialMove = new MoveData();
		moveChooser.addItem(initialMove);
		for(int i = 0; i< movesOfGame.length;i++)
		{
			moveChooser.addItem((MoveData)movesOfGame[i]);
		}
		if(selected != -1) //if possible show the last move that was shown before of this game
			moveChooser.setSelectedIndex(selected);
		else
			moveChooser.setSelectedIndex(0);
		currentGame.setCurrentMoveIndex(selected);
	}
	else
	{
		currentGame = g;
	}	
  }
  
  /**
   * add m to the movechooser
   */
  public void addMove(MoveData m) 
  {
	  this.moveChooser.addItem(m);
  }

  /***
   * set the selected move
   */
  public void setSelected(MoveData m)
  {
	  this.moveChooser.setSelectedItem(m);
  }

  /**
   * clear the MoveNavigationPanel
   */
  public void clearMoves()
  {
	  currentGame = null;
	  this.moveChooser.removeAllItems();
  }

  /**
   * set the GameView where the moves are shown in. This is especially useful when there is more than one chessgame attribute in a relation
   */
  public void setView(GameView gv)
  {
	  this.theGameView = gv;
  }

  /**
   * get the move which is shown at the moment
   */
  public MoveData getCurrentMove()
  {
	  return (MoveData)this.moveChooser.getItemAt(this.moveChooser.getSelectedIndex());
  }

  /**
   * show the move in the viewer. Called when a new move is selected
   */
  private void showMove()
  {
	  if(moveChooser.getSelectedItem()!= null)
	  {
		  showMoveInView(moveChooser.getSelectedIndex());
		  theGameView.repaint();
		  moveChooser.setToolTipText(moveChooser.getSelectedItem().toString());
		  if(currentGame != null)
		  	currentGame.setCurrentMoveIndex(moveChooser.getSelectedIndex()); //save the index of the current move to the chessgame
	  }
  }

  /**
   * this method shows the move with index 'index' in the GameView. 
   */
  private void showMoveInView(int index)
  {
	  //since the showMove method in the GameView class throws an IllegalArgumentException if the move is not the direct next move 
	  try 
	  {
		  //the method first tries to show the move of index
		  theGameView.showMove((MoveData)moveChooser.getItemAt(index), index);
	  }
	  catch (IllegalArgumentException e)
	  {
		  //if that fails the move before index is shown calling this method recursively. Hence all the moves between index and the current move will be calculated in the viewer
		  showMoveInView(index-1);
		  //now the move can be shown!
		  theGameView.showMove((MoveData)moveChooser.getItemAt(index), index);
	  }
  }
}
