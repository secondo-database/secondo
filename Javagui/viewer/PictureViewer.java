/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"{u}]
//[ae] [\"{a}]
//[oe] [\"{o}]
//[TOC] [\tableofcontents]

[1] PictureViewer: Class Definitions

Dezember 2004 Christian Bohnebuck, Uwe Hartmann, Marion Langen and Holger
M[ue]nx during Prof. G[ue]ting's practical course
'Extensible Database Systems' at Fernuniversit[ae]t Hagen.

[TOC]

1 Introduction

See the documentation of ~PictureAlgebra.h~ for a general introduction to
the Picture algebra.

This module is part of a SECONDO viewer ~PictureViewer~ which shows ~picture~ objects and their ~histograms~.

	related files:

secondo/Javagui/viewer/pictureviewer/makefile

secondo/Javagui/viewer/pictureviewer/PictureViewable.java

secondo/Javagui/viewer/pictureviewer/PictureIcon.java

secondo/Javagui/viewer/pictureviewer/PictureTable.java

secondo/Javagui/viewer/pictureviewer/DisplayPicture.java

secondo/Javagui/viewer/makefile

secondo/Javagui/viewer/PictureViewer.java

secondo/Javagui/gui.cfg

secondo/Javagui/viewer/pictureviewer/DisplayHistogram.java



2 Includes and other preparations

*/





package viewer;

import viewer.pictureviewer.*;
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

3 Class PictureViewer

The class PictureViewer is an implementation of SecondoViewer.
It displays relations, pictures and histograms.

*/

public class PictureViewer
extends SecondoViewer
{

 private JComboBox 	ComboBox;

 private JScrollPane 	tableScroll;
 private JScrollPane 	pictureScroll;

 private JPanel		 picturePanel;
 private DisplayPicture  pictuPane;

 private JScrollPane	metaScroll;
 private JScrollPane	histogramScroll;

 private JTextArea 	 metaText;
 private JPanel		 histogramPanel;
 private DisplayHistogram histoPane;

 private JComboBox      zoomBox;

 private Vector 	Tables;
 private JPanel 	dummy = new JPanel();  // to show nothing
 private PictureTable 	CurrentTable;

 private int lastRow = -1;
 private int lastCol = -1;

 // the value of MAX_TEXT_LENGTH must be greater then five
 private final static int 	MAX_TEXT_LENGTH=100;



/*

3.1 Constructor ~PictureViewer~

It creates all displays for relations, pictures, histograms and jpeg-metadata.

*/

 public PictureViewer(){
/** creates a new PictureViewer **/

   Tables = new Vector();
   CurrentTable = null;

   tableScroll       = new JScrollPane();
   JPanel leftPane = new JPanel(new BorderLayout());
   pictureScroll   = new JScrollPane();
   picturePanel    = new JPanel();
   metaScroll      = new JScrollPane();

   //
   //	ComboBox - contains each command
   //
   ComboBox = new JComboBox();
   ComboBox.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          showSelectedObject();
       }});


   //
   //	Define objects for the graphics (Picture & Histogram)
   //	Is there any other method ?
   //
   histoPane = new DisplayHistogram();
   pictuPane = new DisplayPicture();

   //
   //	The lowPane component is divided in two parts:
   //	One component shows the metadata and the other
   //	my histograms !
   //
   JSplitPane lowPane =
       new JSplitPane( JSplitPane.HORIZONTAL_SPLIT,
		       metaScroll,
		       histoPane );
   lowPane.setOneTouchExpandable(true);
   lowPane.setResizeWeight( 0.6 );

   metaText = new JTextArea( "" );
   metaText.setEditable(false);
   metaText.setFont(new Font("Monospaced", Font.PLAIN, 10));
   metaScroll.setViewportView( metaText );

   //
   //	The left part of the Visual Panel
   //
   leftPane.add( tableScroll ,BorderLayout.CENTER);

   //
   //	The right part of the VisualPanel
   //	This component is also divided in two parts:
   //	The pictureScroll & lowPane part
   //
   JSplitPane rightPane =
       new JSplitPane(JSplitPane.VERTICAL_SPLIT,
		      pictureScroll,
		      lowPane);
   rightPane.setOneTouchExpandable(true);
   rightPane.setResizeWeight(0.65);

   //
   //	VisualPanel - the main Panel of the main frame !
   //	It is divided in two parts. The left & right part !
   //
   JSplitPane VisualPanel =
       new JSplitPane( JSplitPane.HORIZONTAL_SPLIT,
		       leftPane,
		       rightPane );
   VisualPanel.setOneTouchExpandable(true);
   VisualPanel.setResizeWeight(0.4);

   //
   //	The picturePanel component contains the Picture !
   //
   pictureScroll.setViewportView( picturePanel );
   picturePanel = new JPanel(new BorderLayout());
   picturePanel.add( pictuPane );

   //   Combo box for zooming
   zoomBox = new JComboBox(pictuPane.getZoomNames());
   zoomBox.setSelectedIndex(0);
   zoomBox.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
	   if (CurrentTable.getRowCount() == 1
	       && CurrentTable.getColumnCount()==1)
	       displayTableObject(0, 0);
	   else if (lastRow >= 0 && lastCol >= 0)
	       displayTableObject(lastRow, lastCol);
       }});

   //   JPanel for both combo boxes
   JPanel comboBoxPanel = new JPanel();
   comboBoxPanel.setLayout(new BorderLayout());
   comboBoxPanel.add(ComboBox, BorderLayout.CENTER);
   comboBoxPanel.add(zoomBox, BorderLayout.EAST);


   //
   //	Main Frame
   //
   setLayout(new BorderLayout());
   add(comboBoxPanel,BorderLayout.NORTH);
   add(VisualPanel,BorderLayout.CENTER);

   clearAllView();

}


/*

3.2 Function ~clearAllView~

This methode removes all displayed Objects from all displays

*/

private void clearAllView()
{

	histoPane.removeall( );
	pictuPane.removeall( );
   	metaText.setText("" );

	tableScroll.setViewportView(dummy);

	//
	//	Show no data
	//
	pictureScroll.setViewportView( picturePanel );
	metaScroll.setViewportView( metaText );
	histoPane.repaint();

	lastRow = -1;
	lastCol = -1;

}



/*

3.3 Function ~displayTableObject~

This methode displays picture and histogram of selected row and
selected column

*/

public void displayTableObject( int row, int col)
{

	if(CurrentTable==null)
		return;

	Vector phVec;

	Dimension dim = pictureScroll.getSize(null);

	if (CurrentTable.getColumnType(col).equals("picture") ){

	   //
	   //	Get the PictureIcon of the chosen picture. This
           //   PictureIcon contains the ListExpr and therefore
	   //   an image and the metadata could be displayed.
           //
	   PictureIcon pp = CurrentTable.getPictureIcon(row,col);
	   if (pp != null) {
	       pictuPane.setZoom(zoomBox.getSelectedIndex());
	       pictuPane.set(pp.getListExpr(), 
			     dim.getWidth(), 
			     dim.getHeight());
	       metaText.setText( pp.getMetaData() );
	   }
	   else
	   {
			pictuPane.removeall();
			metaText.setText( "" );
	   }


  	   phVec = CurrentTable.getHistogramColumns(col);


	} else if (CurrentTable.getColumnType(col).equals("histogram") ) {

	   //
	   //	Try to find the appripriate picture
	   //
	   int picCol = CurrentTable.getPictureColumn( col );

	   //
	   //	A picture was found!
	   //
	   if ( picCol >= 0 )
	   {
	   	PictureIcon pp = CurrentTable.getPictureIcon( row, picCol );
		if (pp != null) {
		    pictuPane.setZoom(zoomBox.getSelectedIndex());
		    pictuPane.set(pp.getListExpr(),
				  dim.getWidth(),
				  dim.getHeight());
		    metaText.setText( pp.getMetaData() );
		}
		else
		{
			pictuPane.removeall();
			metaText.setText( "" );
		}

	   	col = picCol;
		phVec = CurrentTable.getHistogramColumns(col);
	   }
	   else
	   {
		//
		//	There's no picture, nevertheless, show the
		//	histogram (only this one)!
		//
		phVec = CurrentTable.getHistogramColumns(col, true);

		//      Remove currently displayed picture.& meta data
		pictuPane.removeall();
		metaText.setText("" );
	   }
	}
	else
	{
	   //	there is nothing to do !
	   //
	   return;
	}

	//
	//	Ok, show them my histograms !
	//
	//	Try to find the appropriate histograms. These
        //	elements should be displayed as well !
	//
	//Vector phVec = CurrentTable.getHistogramColumns(col);

 	//
 	//	Show all histogram which were found
        //

	histoPane.init();
 	for ( int i=0; i<phVec.size(); i++ )
 	{
		Integer histoCol = (Integer) phVec.get(i);
		PictureIcon pi =
			CurrentTable.getPictureIcon( row, histoCol.intValue() );
		if (pi != null) histoPane.set( pi.getListExpr() );
	}
	histoPane.repaint();

	//
	//	Show the new data
  	//
	pictureScroll.setViewportView( picturePanel );
  	metaScroll.setViewportView(metaText);
	histoPane.repaint();

	lastRow = row;
	lastCol = col;

}



/*

3.4 Function ~interpretTuple~

This Function gets names and types of the tupleattributes from ListExpr.

 	This methods expects a correct ListExpr. Hence, the structure
 	of the ListExpr should be checked in the canDisplay() function

 	It is possible that ListExpr does not contain
 	any values ( in case of no result). Nevertheless, there should
	be attribute names, at least one.

*/

private void interpretTuple( ListExpr LE, Vector names, Vector types )
{

	//
	//	QUESTION: Is it possible that the ListExpr doesn't contain
	//	any attributenames ? It is possible that it doesn't contain
	//	any values ( in case of no result). Nevertheless, there should
	//	be attribute names, at least one !
	//
	if ( LE.listLength() != 2 )
		return;

	ListExpr first = LE.first();
	if ( first.listLength() != 2 )
		return;

	ListExpr fsecond = first.second().second();

	while( ! fsecond.isEmpty() )
	{
		ListExpr attribute = fsecond.first();
		fsecond = fsecond.rest();

		names.add( attribute.first().writeListExprToString().trim());
		types.add( attribute.second().writeListExprToString().trim());
	}

	return;
}


/*

3.5 Function ~getExprHistogram~

This methode creates an element for the table to store a histogram

*/

private PictureIcon getExprHistogram(ListExpr LE)
{
	double histo[] = new double[258];
	for (int i=0; i<258; i++)
		histo[i] = 0.0;
	if (LE.listLength() != 3){
		Reporter.debug(" Histogram len=" + (LE.listLength()) );
		return null;
	}
	if (    (LE.first().atomType() != ListExpr.INT_ATOM) ||
		(LE.second().atomType() != ListExpr.REAL_ATOM ) ||
		(LE.third().atomType() != ListExpr.NO_ATOM) ||
		(LE.third().listLength()!=256) )
	{
		 Reporter.debug(" Histogram bad Types" );
		 return null;
	}
	int colorchannel = LE.first().intValue();
	double maxvalue = LE.second().realValue();

	ListExpr le = LE.third();
	for (int i=0; (!le.isEmpty()) && (i<256); i++){
	    histo[i]= le.first().realValue()/maxvalue;
	    le = le.rest();
	}

        return new PictureIcon( histo, colorchannel, LE );
}




/*

3.6 Function ~getTupleValues~

This methode reads all values from the specified ListExpr.
 	Notice, that an empty result is possible !

*/

private Vector getTupleValues(ListExpr LE, Vector vtypes )
{
     Vector values = new Vector();

     if ( LE.listLength() != 2 )
	return values;

     ListExpr tuplesExpr = LE.second();


     //
     //		It is possible that there is no result
     //
     if ( tuplesExpr.listLength() == 0 )
	return values;


     //
     //		It seems so that there might be some tuples, at least one !
     //
     int index = 0;
     while( ! tuplesExpr.isEmpty() )
     {
	ListExpr oneTupleExpr = tuplesExpr.first();

	tuplesExpr = tuplesExpr.rest();

	//
	//	get the values of each tuple
	//
	Vector tupleVec = new Vector();
	index = 0;
	while( ! oneTupleExpr.isEmpty() )
	{
		ListExpr  attributeValue = oneTupleExpr.first();



		oneTupleExpr = oneTupleExpr.rest();

		//
		//	interpret the attributValue
		//
		String type = (String) vtypes.elementAt(index++);

		if ( type.equals( "picture" ))
		{
			tupleVec.add( getExprPicIcon( attributeValue ));
		}
		else
		if ( type.equals( "histogram" ))
		{
			tupleVec.add( getExprHistogram( attributeValue ));
		}
		else
		{
			tupleVec.add( attributeValue.writeListExprToString() );
		}
	}
	values.add( tupleVec );

     }

     return values;

}



/*

3.7 Function ~getExprType~

This methode returns the type of ListExpr as String

*/

private String getExprType(ListExpr LE)
{
   if(LE.listLength()!=2)
      	return "";

   LE = LE.first();

   if(LE.isAtom())
   	if (LE.atomType()==ListExpr.SYMBOL_ATOM)
	       	return LE.symbolValue().trim();

   if ( LE.listLength() >= 1 )
   {
   	LE = LE.first();

   	if(LE.isAtom() )
   		if( LE.atomType()==ListExpr.SYMBOL_ATOM)
			return LE.symbolValue().trim();
   }

   return "";
}



/*

3.8 Function ~getExprPicIcon~

This methode creates an element for the table to store as picture

*/

private PictureIcon getExprPicIcon(ListExpr Elem)
{
    if (Elem.listLength() != 5) return null;

    return new PictureIcon(Elem.fifth().textValue(), Elem);
}




/*

3.9 Function ~tuplereport~

This methode creates a new table as result of ListExpr.

*/

private PictureTable tuplereport(ListExpr LE)
{
	String reltyp = getExprType( LE );

	//
	//	The specified ListExpr can be a picture, a histogram
	//	or a relation. Everything else is not interesting !
	//
	if ( (reltyp.equals("picture") )){

		Vector stt = new Vector();
		stt.add( "picture" );

		PictureIcon pi = getExprPicIcon( LE.second() );

		Vector v = new Vector();
		v.add( pi );

		Vector vv= new Vector();
		vv.add(v);

		return new PictureTable( vv, stt, stt, this);
	}
	else
	if ( (reltyp.equals("histogram") )){

		Vector v = new Vector();
		v.add( getExprHistogram( LE.second() ) );

		Vector vv= new Vector();
		vv.add(v);

		Vector stty = new Vector();
		stty.add( "histogram" );

		return new PictureTable( vv, stty, stty, this);
	}
	else
	if ( !(reltyp.equals("rel") || reltyp.equals("mrel") ) )
		return null;

	//
	//	In case it is a relation
	//
	Vector names = new Vector();
	Vector types = new Vector();
	interpretTuple( LE, names, types);
	Vector values = getTupleValues( LE, types);

	return new PictureTable(values, names, types, this);
}





/*

3.9 Function ~showSelectedObject~

This methode shows the table of the selected object

*/

 private void showSelectedObject(){
    clearAllView();
    int index = ComboBox.getSelectedIndex();
    if(index>=0)
    {
        CurrentTable = (PictureTable)Tables.get(index);
      	tableScroll.setViewportView(CurrentTable);
    	if (CurrentTable!=null)
       		if (CurrentTable.getRowCount()==1
		    && CurrentTable.getColumnCount()==1)
		{
		    displayTableObject( 0, 0);
		}
    } else {
        tableScroll.setViewportView(dummy);
    }


    lastRow = -1;
    lastCol = -1;

 }






/*

3.10 Function ~addObject~

This methode adds an Objects to the viewer, stores it and displays it.

*/

 public boolean addObject(SecondoObject o){


   if (!canDisplay(o))
	  return false;


   if (isDisplayed(o)){
      selectObject(o);
   }
   else
   {
      PictureTable NTable = tuplereport(o.toListExpr());
      if(NTable==null)
          return false;
      else{
        Tables.add(NTable);
        ComboBox.addItem(o.getName());
        selectObject(o);
      }
    }
	return true;
 }






/*

3.11 Function ~getIndexOf~

 This methode returns the index in ComboBox of S,
    if S not exists in ComboBox -1 is returned

*/

 private int getIndexOf(String S){
   int count =  ComboBox.getItemCount();
   int pos = -1;
   for(int i=0;i<count;i++){
     if( ((String)ComboBox.getItemAt(i)).equals(S)) pos=i;
   }
   return pos;
 }



/*

3.12 Function ~removeObject~

 This methode removes a object from this viewer

*/

 public void removeObject(SecondoObject o){
    int index = getIndexOf(o.getName());
    if(index>=0){
       ComboBox.removeItemAt(index);
       Tables.remove(index);
       showSelectedObject();
    }
 }



/*

3.13 Function ~removeAll~

 This methode removes all objects from this viewer

*/

 public void removeAll(){
   ComboBox.removeAllItems();
   Tables.clear();
   showSelectedObject();

 }




/*

3.14 Function ~getDisplayQuality~

 This methode return the ability of this viewer for SO.

*/

 public double getDisplayQuality(SecondoObject SO){

    if(!canDisplay(SO)){
       return 0;
    }
    // the list structure is checked at this point
    // thereby we can avoid a lot of tests
    ListExpr type = SO.toListExpr().first();
    // picture or attribute
    if(type.isAtom()){
      return 0.9;
    }
    // relation
    ListExpr attributes = type.second().second();
    while(!attributes.isEmpty()){
      String attrtype = attributes.first().second().symbolValue();
      if(attrtype.equals("histogram") || 
         attrtype.equals("picture")){
         return 0.9;
      }
      attributes=attributes.rest();
    }
    return 0.2; // not appropriate for only standard-realations
 }





/*

3.15 Function ~canDisplay~

 This methode returns true for realtions, pictures and histograms.

*/


 public boolean canDisplay(SecondoObject o){

	ListExpr LE = o.toListExpr();

	if (LE.listLength() != 2)
		return false;


	ListExpr first = LE.first();
	ListExpr second = LE.second();

	//
	//	We can also show Picture- and Histogram-Objects !
	//
	if ( first.isAtom())
	{
		if ( first.symbolValue().equals("picture") ||
			 first.symbolValue().equals("histogram" ))
			return true;
		else
			return false;
	}

    //
    //	Check for relation
    //
	if ( first.listLength() != 2 )
		return false;

	ListExpr rel = first.first();
	ListExpr rfirst = first.second();

	if ( rfirst.listLength() != 2 )
		return false;

	ListExpr tuple = rfirst.first();
	ListExpr attrlist = rfirst.second();

	if ( ( rel.atomType() != ListExpr.SYMBOL_ATOM ) ||
		 ( ! rel.symbolValue().equals("rel") )      ||
		 ( tuple.atomType() !=ListExpr.SYMBOL_ATOM ) ||
		 ( ! tuple.symbolValue().equals("tuple"))    ||
  		 ( attrlist.atomType() != ListExpr.NO_ATOM ) )
	{
		Reporter.debug( "canDisplay(): rel expected" );
		return false;
	}

	if ( attrlist.listLength() == 0 )
	{
		Reporter.debug( "canDisplay(): no attributes" );
		return false;
	}

	boolean parameterOK = true;
	while( ! attrlist.isEmpty() )
	{
		ListExpr elem = attrlist.first();

		if (!((elem.second().atomType() == ListExpr.SYMBOL_ATOM ) &&
			  ((elem.second().symbolValue().equals("int")) ||
			  (elem.second().symbolValue().equals("real")) ||
			  (elem.second().symbolValue().equals("bool")) ||
			  (elem.second().symbolValue().equals("string")) ||
			  (elem.second().symbolValue().equals("picture")) ||
			  (elem.second().symbolValue().equals("histogram")) ||
			  (elem.second().symbolValue().equals("instant")) )))
		{
			Reporter.debug( "canDisplay(): unexpected parameter" );
			parameterOK = false;
			break;
		}
		attrlist = attrlist.rest();
	}

	if ( !parameterOK )
		return false;

		Reporter.debug( "canDisplay(): Each Parameter is valid" );

	return true;
}





/*

3.16 Function ~isDisplayed~

 This methode checks whether o displayed

*/

 public boolean isDisplayed(SecondoObject o){
   return getIndexOf(o.getName())>=0;
 }




/*

3.17 Function ~selectObject~

  hightlighting of o

*/

 public  boolean selectObject(SecondoObject O){
      int index = getIndexOf(O.getName());
      if (index<0)
         return false;
      else{
         ComboBox.setSelectedIndex(index);
         showSelectedObject();
         return true;
      }
 }




/*

3.18 Function ~getMenuVector~

 This Methode returns that no menuevector is used.

*/

 public MenuVector getMenuVector()
 {
 	return null;
 }


/*

3.19 Function ~setViewerControl~

 This Methode sets the Control for this viewer.

*/

 public void setViewerControl(ViewerControl VC){
      this.VC = VC;
 }



/*

3.21 Function ~getName~

 This Methode returns the name of this Viewer.

*/

 public  String getName(){
   return "PictureViewer";
 }






}

