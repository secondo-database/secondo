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

package viewer.chess;
import java.io.*;
import javax.swing.*;
import java.util.*;
import java.awt.*;

/**
*This class provides functionality for exporting chess games to pgn files. 
*/
public class ChessFileExporter
{
/**
* constant for the maximum characters per line
*/
	public static final int MAX_LINE_LENGTH = 80;
/**
* the Game Deliverer to get a vector of the current games from 
*/
	private GameDeliverer gameDeliverer;
	
/**
* file chooser to choose the file to write the chessgames to
*/
	private JFileChooser chooser;
	
/**
* parent component to place the dialogues on
*/
	private JComponent parent;
	
/**
* boolean value whether a new file was created. Needed to clean up if a new file was created and the writing process is aborted in any way
*/
	private boolean createdNewFile = false;
	
/**
*list to choose the chessgames that will be written to the pgn-file from
*/
	private JList gameChooser;

/**
* box which contains everything to show the "select the chessgames you want to export"-dialogue
*/
	private Box gameChooserView;
	
/**
* the currently shown game.
*/
	private GameData currentGame;
	
	/**
	*constructor which needs a GameDeliverer to get the games which will be exported. Furthermore this needs a JComponent to locate the 
	*Dialogues on.
	*/
	public ChessFileExporter(GameDeliverer gameDeliverer, JComponent parent)
	{
		this.gameDeliverer = gameDeliverer;
		this.parent = parent;
		chooser = new JFileChooser();
		chooser.setMultiSelectionEnabled(false);
		chooser.setFileSelectionMode(JFileChooser.FILES_ONLY );
		gameChooser = new JList();
		gameChooser.setFixedCellWidth(300);
		gameChooser.setVisibleRowCount(5);
		gameChooser.setFixedCellHeight(24);
		JScrollPane scroll = new JScrollPane(gameChooser);
		gameChooserView = new Box(BoxLayout.Y_AXIS);
		JPanel p1 = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		JPanel p2 = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		JPanel p3 = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		JLabel lbl = new JLabel("Please select which chessgames you want to export.");
		JLabel lbl2=new JLabel("Note: Duplicated chessgames will be saved only once.");
		p1.add(lbl);
		p2.add(scroll);
		p3.add(lbl2);
		gameChooserView.add(p1);
		gameChooserView.add(p2);
		gameChooserView.add(p3);
		
	}
	
	/**
	* method to show an error. The String is the displayed error message
	*/
	private void showError(String errorMessage)
	{
		JOptionPane.showMessageDialog(parent,errorMessage,"error",JOptionPane.WARNING_MESSAGE );
	}
	
	/**
	* lets the user choose a file, adds the ending ".pgn" to the filename, if neccessary and creates a new file if neccessary
	*/
	private File fileCreation() throws IOException, Exception 
	{
		File file = chooser.getSelectedFile(); //file choosing
		if (!file.getName().toLowerCase().endsWith(".pgn")) //test if already has the right file extension
		{
			String path = file.getParent();
			String name = file.getName();
			if(path == null)
				file = new File(File.separatorChar+name+".pgn");
			else
				file = new File(path+File.separatorChar+name+".pgn");
		}
		createdNewFile = false;
		createdNewFile = file.createNewFile(); //create new file if neccessary
		int selection = JOptionPane.YES_OPTION;
		if(!createdNewFile) // question if existing file should be overwritten
		{
			selection = JOptionPane.showConfirmDialog(parent,"A file called "+file.getName()+" already exists in the current Directory, do you want to overwrite?","File already exists",JOptionPane.YES_NO_OPTION);
		}
		if(selection != JOptionPane.YES_OPTION)
			throw new Exception();
		return file;
	}
	
	/**
	* method to show a number of dialogues to export a chessgame. 'current' is the currently shown chessgame in the view
	*/
	public void showSaveDialog(GameData current)
  	{
		this.currentGame = current;
		if(chooser.showSaveDialog(parent)==JFileChooser.APPROVE_OPTION) // if the user decided which file the games will be written to
		{	
			File file = null;
			try
			{
				file = fileCreation(); // file is created
				try
				{
					saveChessGames(file); // chessgames are written to the file if possible
				}
				catch (Exception e)
				{
					if(createdNewFile)
					{
						try
						{
							file.delete();
						}
						catch (Exception e2)
						{
						}
					}
				}
			}
			catch(IOException ioe) // if an io Exception occurs the file cannot be written.
			{
				showError("Error: Cannot write file !");
			}
			catch (Exception ex)
			{
				showSaveDialog(currentGame);
			}
		}  
  	}
	
	/** method to show a dialogue in which the user can select the chessgames which should be exported. Returns an array of objects which contains these chessgames
	*/
	private Object[] selectChessGames(Vector v) throws Exception
	{
		Object[] selectedGames;
		gameChooser.setListData(v);
		int selection = JOptionPane.showConfirmDialog(parent,gameChooserView, "Please select chessgames", JOptionPane.OK_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE);
		if(selection == JOptionPane.CANCEL_OPTION)
		{
			throw new Exception();
		}
		selectedGames = gameChooser.getSelectedValues();
		if (selectedGames.length == 0)
		{
			showError("No selection was made, file saving abborted.");
			throw new Exception();
		}
		return selectedGames;
	}
	
	/**
	* writes the chessgames to the file. Gets the games whih should be written and the PrintWriter which writes the files
	*/ 
	private void writeGames(Object[] selectedGames, PrintWriter p) throws Exception 
	{
		ArrayList alreadyWritten = new ArrayList();
		for(int i = 0;i<selectedGames.length;i++)
		{
			try
			{	
				if(!alreadyWritten.contains(selectedGames[i])) // tries to write a file
				{
					writePGNtoFile(p, (GameData)selectedGames[i]);
					alreadyWritten.add(selectedGames[i]);
				}
			}	
			catch (ClassCastException cce) //if class cast Exception occurs it is the special entry which says "current game"
			{
				if(!alreadyWritten.contains(currentGame))
				{
					writePGNtoFile(p, currentGame);
					alreadyWritten.add(currentGame);
				}
			}
		}	
		p.flush();
		JOptionPane.showMessageDialog(parent, "PGN file  succesfully written.","Success", JOptionPane.INFORMATION_MESSAGE);
	}

	/**
	* manages the writing of the chessgames to the given file. Returns an Exception if writing is not possible. 
	*/
 	private void saveChessGames(File f) throws Exception
  	{
		Vector v = gameDeliverer.getAllGames();
		v.add(0,"selected Chessgame");
		Object[] selectedGames = v.toArray();
		if (v.size()> 1)
		{
			selectedGames = selectChessGames(v); // gets the games that will be selected
		}
		FileWriter fileWriter = null;
		PrintWriter p = null;
		try
		{
			fileWriter = new FileWriter(f, false); //creation of writers /opening file and writing the file
			p = new PrintWriter(fileWriter);
			writeGames(selectedGames,p);
		}
		catch (Exception exc)
		{
			showError("Error: Could not write file."); // if problems occur while writing
			throw new Exception();			
		}
		finally
		{
			try
			{
				fileWriter.close(); // closing of the writers if possible
				p.close();
			}
			catch(Exception e){}
		}
  	}
  
	/**
	* writes a tag pair to the file with the PrintWriter p. Key is the key of the tag pair and value the value.
	*/
  	private void writeTag(PrintWriter p, String key, String value) throws Exception
  	{
		if(!value.equals(""))
		{
			StringBuffer out = new StringBuffer();
			out.append('[');
			out.append(key);
			out.append(' ');
			out.append('"');
			out.append(value);
			out.append('"');
			out.append(']');
			p.println(out.toString());
		}
  	}
	
	/**
	* writes all the tags of the chessgame g with the PrintWriter p to the file. This method manages writing at least the most common tags of a chessgames to the pgn file 
	*/
	private void writeAllTags(PrintWriter p, GameData g) throws Exception
	{
		writeTag(p, ChessToolKit.EVENT_KEY, g.getEvent());
		writeTag(p, ChessToolKit.SITE_KEY, g.getSite());
		writeTag(p, ChessToolKit.DATE_KEY, g.getDate());
		writeTag(p, ChessToolKit.ROUND_KEY, g.getRound());
		writeTag(p, ChessToolKit.WHITE_KEY, g.getWhite());
		writeTag(p, ChessToolKit.BLACK_KEY, g.getBlack());
		writeTag(p, ChessToolKit.RESULT_KEY, g.getResult());
		
		HashMap tags = g.getTags();
		if(tags != null)
		{
			Object[] keys = g.getKeys();
			for(int i=0;i<keys.length;i++)
			{
				writeTag(p, (String)keys[i],(String)tags.get(keys[i]));
			}
		}
	}
	
	/**
	* this methods writes all the moves of the Game g with the PrintWriter p to the file. 
	* Checks all the time if the line length is not to long (<80 characters)
	*/
	private void writeAllMoves(PrintWriter p, GameData g) throws Exception
	{
		int lineLength = 0;
		int movePos = 0;
		int moveNumber = 1;
		String pgn;
		MoveData currentMove;
		Object[] moves = g.getMoves();
		StringBuffer out = new StringBuffer();
		while(movePos < moves.length) // writing moves
		{
			currentMove = (MoveData)moves[movePos];
			pgn = currentMove.getPgn();
			if(currentMove.isWhite())
			{
				if((lineLength+2+((""+moveNumber).length()))>=ChessFileExporter.MAX_LINE_LENGTH)
				{
					p.println(out.toString());
					out.delete(0,out.length());
					lineLength = 0;
				}
				out.append(moveNumber);
				out.append('.');
				out.append(' ');
				moveNumber++;
				lineLength = lineLength+2+((""+moveNumber).length());
			}
			if((lineLength + pgn.length()+1) >=ChessFileExporter.MAX_LINE_LENGTH)
			{
				p.println(out.toString());
				out.delete(0,out.length());
				lineLength = 0;
			}
			out.append(pgn);
			out.append(' ');
			lineLength = lineLength+pgn.length()+1;
			movePos++;
		}
		if(lineLength+g.getResult().length()+1 >=80) // writing game result
		{
			p.println(out.toString());
			out.delete(0,out.length());
		}
		out.append(g.getResult());
		p.println(out.toString());	
	}

	/**
	* method which manages writing the chessgame g with the PrintWriter p to the file. 
	**/
  	private void writePGNtoFile(PrintWriter p, GameData g) throws Exception
	{
		writeAllTags(p, g);	
		p.println();
		writeAllMoves(p,g);
		p.println();
	} 
}
