/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"{u}]
//[ae] [\"{a}]
//[oe] [\"{o}]
//[TOC] [\tableofcontents]

[1] PictureTable: Class Definitions

Dezember 2004 Christian Bohnebuck, Uwe Hartmann, Marion Langen and Holger
M[ue]nx during Prof. G[ue]ting's practical course
'Extensible Database Systems' at Fernuniversit[ae]t Hagen.

[TOC]

1 Introduction

See the documentation of ~PictureAlgebra.h~ for a general introduction to
the Picture algebra.

This module is part of a SECONDO viewer ~PictureViewer~ which shows ~picture~ objects and their ~histograms~.



2 Includes and other preparations

*/

package viewer.pictureviewer;

import viewer.pictureviewer.*;
import viewer.PictureViewer;
import sj.lang.*;
import javax.swing.*;
import javax.swing.table.*;
import java.awt.*;
import java.util.*;
import gui.*;
import java.awt.event.*;
import javax.swing.event.*;
import tools.*;
import java.io.*;
import java.awt.image.*;
import java.lang.*;



/*

3 Class PictureTable

This is used to store Icons or strings and display them.
The real data - picture or histogram - is stored inside the icon.

*/

public class PictureTable
extends JTable
{
	private	PictureViewer 	destinationofEvent;
        //private PictureIcon	picdummy;

	private Vector		vnames = new Vector();
	private Vector		vtypes = new Vector();
	static  private	ImageIcon icon = new ImageIcon();

    	static String histogramPostfixes[] = { "_hist_red",
					   "_hist_green",
					   "_hist_blue",
					   "_hist_brightness" };



/*

3.1 Constructor ~PictureTable~

Create a new table with all data , names and types.
Send an event to PictureViewer if a cell is selected with the mouse.

*/

	public PictureTable( Vector data,
			     Vector names,
			     Vector columntypes,
			     PictureViewer pV )
	{
		super( data, names );

		setNames( names );
		setTypes( columntypes );

		setCellSelectionEnabled( true );
		setSelectionMode( ListSelectionModel.SINGLE_SELECTION);
		setRowHeight( 44 );

		destinationofEvent = pV;
		//picdummy = new PictureIcon();
	}


/*

3.1 Function ~setNames~

Store the names of the columns.

*/

	private void setNames( Vector names )
	{
		for ( int i=0; i< names.size() ; i++ )
			vnames.add( names.elementAt(i) );
	}


/*

3.1 Function ~setTypes~

Store the types of the columns.

*/

	private void setTypes( Vector types )
	{
		for ( int i=0; i< types.size() ; i++ )
			vtypes.add( types.elementAt(i) );
	}


/*

3.1 Function ~isCellEditable~

No cell is editable.

*/

	public boolean isCellEditable( int roe, int col)
	{
		return false;
	}


/*

3.1 Function ~getColumnType~

Returns the type of column with index ix.

*/

	public String getColumnType(int ix)
	{
		int index = ix;
		String columnName = getColumnName( ix );

		for ( int i=0; i<vnames.size() ; i++ )
		{
			if ( columnName.equals( vnames.elementAt(i) ) )
			{
				index = i;
				break;
			}
		}

		String datatype = (String) vtypes.elementAt( index );
		return datatype;
	}


/*

3.1 Function ~getColumnClass~

Returns the class of column with index ix.
JTable can only handle strings and icons.

*/

	public Class getColumnClass( int ix )
	{
		String columnType = getColumnType( ix );

        	if ( ( columnType.equals("picture")) ||
             		(columnType.equals("histogram")) )
        	{
            		return icon.getClass();
        	}

		//
		//	Everything else is a String-Object
		//
		String s = new String();
        	return s.getClass();
	}


/*

3.1 Function ~getPictureIcon~

Returns the icon at position (row,col).

*/

	public PictureIcon getPictureIcon( int row, int col )
	{
 	    //if (row<0 || col<0) return picdummy;
	    if (row<0 || col<0) return null;

	    Object ob = getValueAt(row,col);
	    //if (ob==null) return picdummy;
	    if (ob==null) return null;

	    if (ob.getClass().getName().equals(
		    "viewer.pictureviewer.PictureIcon"))
		return (PictureIcon)ob;
	    
	    //return picdummy;
	    return null;
	}


/*

3.1 Function ~getValueAt~

Returns super.getValueAt, but might be changed in future.

*/

	public Object getValueAt( int row, int col)
	{
		Object ob = super.getValueAt(row,col);
		return ob;
	}

/*

3.1 Function ~getBasenameFromHistogramName~

This Function extracts the picturename from a histogramname.

*/

    private String getBasenameFromHistogramName(String histogramName) {
	//System.err.println("getBasenameFromHistogramName() histogramName="
	//		   +histogramName);

	//
	//      Find _hist_X postfix and return -1 unless found
	int postfixPos = -1;
	for (int i = 0; i < histogramPostfixes.length; i++) {
	    postfixPos = histogramName.indexOf(histogramPostfixes[i]);
	    if (postfixPos != -1
		&& histogramName.endsWith(histogramPostfixes[i])) {
		String basename = histogramName.substring( 0, postfixPos);
		//System.err.println("getBasenameFromHistogramName() basename="
		//		   +basename);
		return basename;
	    }
	}

	return null;
    }


/*

3.1 Function ~getPictureColumn~

This function gets the column index of the picture which
match the histogram at the specified column index col.
It returns the column index of the picture

*/

    public int getPictureColumn( int col ) {
	String basename = getBasenameFromHistogramName(getColumnName(col));

	if (basename == null) return -1;

	for (int i=0; i<getColumnCount(); i++)
	    if (basename.equals(getColumnName(i))) return i;

	return -1;
    }


/*

3.1 Function ~getHistogramColumns~

This function gets all column index of histograms which
matches the ~basename~ .
It returns all column indexes by a vector

*/

    private Vector getHistogramColumns(String basename) {
	//System.err.println("getHistogramColumns()-1 basename="+basename);

	Vector res = new Vector();

	String columnName;
	for ( int i=0; i<getColumnCount(); i++ ) {
	    columnName = getColumnName(i);

	    //System.err.println("getHistogramColumns()-1 columnName="
	    //		       +columnName);

	    for (int j = 0; j < histogramPostfixes.length; j++)
		if (columnName.equals(basename+histogramPostfixes[j])) {
		    //System.err.println("getHistogramColumns()-1   matched!");
		    res.add(new Integer(i));
		}
	}

	return res;
    }


/*

3.1 Function ~getHistogramColumns~

This function gets all column index of histograms which
matches the picture at specified column index col.
It returns all column indexes by a vector

*/

    public Vector getHistogramColumns( int col ) {
	return getHistogramColumns(getColumnName(col));
    }


/*

3.1 Function ~getHistogramColumns~

This function gets all column index of histograms which
matches the histogram at specified column index col.
It returns all column indexes by a vector

*/

    public Vector getHistogramColumns( int col, boolean nothing ) {
	String basename = getBasenameFromHistogramName(getColumnName(col));

	if (basename == null) {
	    Vector res = new Vector();
	    res.add(new Integer(col));
	    return res;
	}

	return getHistogramColumns(basename);
    }


/*

3.1 Function ~valueChanged~

The User selects a cell.
JTable only knows columnselection, but we get row and column of
the selection.
Inform PictureViewer to paint a new Picture with histograms.

*/

   public void valueChanged(ListSelectionEvent e)
   {
      	int row = getSelectedRow();
      	int col = getSelectedColumn();

      	if ( (row<0) || (col<0) || e.getValueIsAdjusting())
      	 	return;

	//
	//	Rearranging the columns can make problems. The
	//	TableModel doesn't realise any changes. But this
	//	class knows the new name of the chosen column.
	//
	clearSelection();
	resizeAndRepaint();
	destinationofEvent.displayTableObject(row,col);

   }


}
