package  viewer;

import  javax.swing.*;
import  java.awt.*;
import  java.awt.event.*;
import  java.net.URL;
import  java.io.*;
import  javazoom.jl.player.*;
import  java.util.Properties;
import  java.util.Vector;
import  java.util.StringTokenizer;
import  sj.lang.ListExpr;
import  java.util.ListIterator;
import  javax.swing.event.*;
import  java.awt.geom.*;
import  javax.swing.border.*;
import  viewer.hoese.*;
import  gui.SecondoObject;
import  gui.idmanager.*;
import  project.*;
import  components.*;
import  java.util.TimerTask;
import  java.util.Timer;


/** The class MP3V implements a viewer for MP3, ID3 and Lyrics.
    It provides a player for MP3 songs and can display all available
    data in the ID3 tag. Lyrics can be displayed en bloc or line by line
    during playing the MP3. (in relations only) */
public class MP3V extends SecondoViewer 
    implements ActionListener, ListSelectionListener {
    /* Threads which play MP3 songs and display lyrics. */
    private PlayerThread player;
    private LyricsThread lyricsplayer;
    private Timer onetimer;
    
    /* In these text field an ID3 is shown. */
    private JTextField id3Version;
    private JTextField id3Title;
    private JTextField id3Artist;
    private JTextField id3Album;
    private JTextField id3Year;
    private JTextField id3Comment;
    private JTextField id3Genre;
    private JTextField id3TrackNo;

    /* In this text field general information about an MP3, ID3 
       or lyrics object is shown.
       
       MP3: version, frequency, number of frames, length in seconds, 
       and bitrate

       ID3: author, title, album, track, comment, songyear, genre

       lyrics: the actual lyrics line.*/
    private JTextArea infoarea;

    /* In this JList the whole lyrics text is shown. */

    private JList lyricsarea; 
    /* starts the playing of an MP3 */

    private JButton PlayButton;
    /* stops the playing of an MP3 */

    private JButton PauseButton;

    /* mychoice displays the elements of a relation in a JList. */
    private JList mychoice; 


    /* supported types for this viewer. */
    private static final int MP3TYPE = 1;
    private static final int ID3V10TYPE = 2;
    private static final int ID3V11TYPE = 3;
    private static final int RELTYPE = 4;
    private static final int STRINGTYPE = 5;
    private static final int INTTYPE = 6;
    private static final int LYRICSTYPE = 7;

    /* typeselect indicates supported types for this viewer. */
    private int typeselect;


    /* clist contains a copy of the whole nested list 
       that was provided by the JavaViewer
       this is necessary because otherwise the method 
       valuechanged could not read information
       from a nested list */
    private ListExpr clist;

    /* This ListExpr songbuffer contains the MP3 that has to be 
       provided for the MP3 player if needed. */
    private ListExpr songbuffer;
    /* The ListExpr lyricsbuffer contains the Lyrics data that
       has to be provided for the lyrics thread. */
    private ListExpr lyricsbuffer;

    private MenuVector MV = new MenuVector();

    /* these varaiables are only important for the use of relations
       selectedtuple and selectedattribute indicate which Tuple 
       and which Attribute were selected
       numberoftuple and numberofattr indicate the size 
       of the relation/tuple*/
    private int selectedtupel;
    private int selectedattribute;
    private int numberoftuple;
    private int numberofattr;
    
    /** implemented method of interface ActionListener
	This method is called if a user presses the
	start or stop button. */
    public void actionPerformed(ActionEvent event) {
	String command = event.getActionCommand();
	
	if (command.equals("stop")) {
	    /* stop a song */
	    StopPlay();
	    StopLyrics();
	} else if (command.equals("play")) {
	    /* start a song */
	    StartPlay();
	    StartLyrics();
	}
    }

    /** Implemented method of interface ListSelectionListener.
     This method is activated if the user activates an item of
     the relation. */
    public void valueChanged(ListSelectionEvent e) {
	/* first we have to find out which tuple and which 
	   attribute within the
	   relation were selected */
	selectedtupel = mychoice.getSelectedIndex() / (numberofattr+1) + 1;
	selectedattribute = mychoice.getSelectedIndex() % (numberofattr+1) + 1;
	
	if (selectedattribute > numberofattr)
	    return;
	
	ListExpr runner=clist.first().second().second();
	String typ = new String();
	int i;
	/* we have to determine the type of the selected 
	   attribute (e.g. mp3, lyrics, id3, int, bool, string, ...)
	   Afterwards type contains the type (coded as a string) */
	for(i = 1; i <= selectedattribute; i++) {
	    typ = runner.first().second().symbolValue();
	    runner = runner.rest();
	}
	

	/* now we determine the position of the selected 
	   attribute by its type
	   for example:
	   if we have a tuple that contains three mp3s 
	   and some other attributs
	   and the second mp3 was selected
	   then afterwards nroftype will have the value 2*/
	int nroftype=0;
	runner=clist.first().second().second();
	for (i=1; i<= selectedattribute; i++) {
	    if (runner.first().second().symbolValue().equals (typ) )
		nroftype++;
	    runner = runner.rest();   
	}
	
	/* we iterate through the tuples until we reach the selected tuple*/
	ListExpr tupel = clist.second();
	for (int j=1;j<selectedtupel;j++) {
	    tupel = tupel.rest();
	} 

	/* now we have to iterate within the tuple to find the 
	   correct attributes */
	ListExpr runnertupel = tupel.first();
	
	ListExpr runnerattr=clist.first().second().second();
	int counter=1;

	/* we have to stop any previous songs and lyrics (that might stil be playing)
	 */
	StopPlay ();
	StopLyrics ();
	songbuffer=null;
	lyricsbuffer=null;

	/* textareas and infofields have to be initialized*/
	lyricsarea.removeAll();
	lyricsarea.setListData(new Vector());
	infoarea.setText ("");
	id3Version.setText("");
	id3Title.setText("");
	id3Artist.setText("");
	id3Album.setText("");
	id3Year.setText("");
	id3Comment.setText("");
	id3Genre.setText("");
	id3TrackNo.setText("");



	/* first we have to search an mp3 object at the 
	   given position (nroftype) */
	for (i=1;i<=numberofattr;i++) {
	    if (runnerattr.first().second().symbolValue().equals ("mp3")) {  
		if (counter == nroftype) {
		    songbuffer = runnertupel.first();
		    StartPlay ();
		}
		counter++;
	    }
	    runnerattr = runnerattr.rest();   
	    runnertupel = runnertupel.rest();
	}
	
	/* then we seek an id3 at the given position (nroftype) */
	runnertupel = tupel.first();
	runnerattr = clist.first().second().second();
	counter = 1;
	for (i = 1; i <= numberofattr;i++) {
	    if (runnerattr.first().second().symbolValue().equals ("id3")) {  
		if (counter == nroftype)  
		    DisplayID3 (runnertupel.first() );
		counter++;
            }
	    runnerattr = runnerattr.rest();   
	    runnertupel = runnertupel.rest();
	}

	/* and finally we look for lyrics at the given position (nroftype) */
	runnertupel = tupel.first();
	runnerattr=clist.first().second().second();
	counter=1;
	for (i = 1; i <=numberofattr; i++) {
	    if (runnerattr.first().second().symbolValue().equals ("lyrics")) {  
		if (counter == nroftype){  
		    DisplayLyrics (runnertupel.first() );
		    lyricsbuffer = runnertupel.first();
		    StartLyrics ();
		}
		counter++;
            }
	    runnerattr = runnerattr.rest();   
	    runnertupel = runnertupel.rest();
	}
    }  
    
    /* create a label. A help method to build up the layout
     of the viewer. */
    private JLabel newJLabel(String text, String message) {
	return new JLabel(text);
    }

    /* create a text field which cannot be edited. */
    private JTextField newJTextField() {
	JTextField jtf = new JTextField();
	jtf.setEditable(false);
	jtf.setBackground(Color.WHITE);
	return jtf;
    }

    /* create a text area which cannot be edited. */
    private JTextArea newJTextArea() {
	JTextArea jta = new JTextArea("");
	jta.setEditable(false);
	return jta;
    }


    /* create a new JButton with an icon and which sends an 
       ActionCommand to its owner. A help method to build 
       up  the layout of the viewer. */
    private JButton newJButton(String message, String icon) {
	ImageIcon ii = new ImageIcon(icon);
	JButton result = new JButton(ii);
	result.setActionCommand(message);
	result.addActionListener(this);
	return result;
    }

    /* create a new JList with no elements and which
       sends a SelectionEvent to its owner. 
       A help method to build up the layout of the
       viewer. */
    private JList newJList() {
	JList result = new JList();
	result.addListSelectionListener(this);
	return result;
    }

    /* create a JPanel with a BorderLayout and put a
       JToolBar to the center component. Each member of comps
       array is inserted into the JToolBar. A help method to
       build up the layout of the viewer. */
    private JPanel newJToolBarJPanel(Component[] comps) {
	JToolBar jtb = new JToolBar();
	
	for (int i = 0; i < comps.length; i++) {
	    jtb.add(comps[i]);
	}

	jtb.putClientProperty("JToolBar.isRollover", Boolean.TRUE);
	jtb.setFloatable(false);
	jtb.addSeparator();

	JPanel result = new JPanel(new BorderLayout());
	result.add(jtb);
	
	return result;
    }

    /* create a JPanel with a BorderLayout and put a JSplitPane 
       into the center component. The orientation of this JSplltPane
       is determined by the first parameter. The other parameters
       specify which components are inserted into that JSplitPane. 
       A help method to build up the layout of the viewer. */
    private JPanel newJSplitJPanel(int orientation, 
				  Component lefttop,
				  Component rightdown) {
	JPanel panel = new JPanel(new BorderLayout());
	JSplitPane spane = new JSplitPane(orientation, lefttop, rightdown);
	panel.add(spane, BorderLayout.CENTER);
	
	return panel;
    }

    /* create a JPanel with a BorderLayout and put a JScrollPane 
       into the center component. The Component comp is inserted 
       into that JScrollPane. A help method to build up the layout
       of the viewer. */
    private JPanel newScrollJPanel(Component comp) {
	JScrollPane spane = new JScrollPane();
	JViewport port = spane.getViewport();
	port.add(comp);
	JPanel result = new JPanel(new BorderLayout());
	result.add(spane, BorderLayout.CENTER);
	
	return result;
    }

    /* create a JPanel with a BorderLayout and put the components north,
       south, east, west and center into this panel iff the component 
       reference != null. */
    private JPanel newBorderLayoutJPanel(Component north, Component south, 
					Component east, Component west,
					Component center) {
	JPanel p = new JPanel(new BorderLayout());
	
	if (north != null) p.add(north, BorderLayout.NORTH);
	if (south != null) p.add(south, BorderLayout.SOUTH);
	if (east != null) p.add(east, BorderLayout.EAST);
	if (west != null) p.add(west, BorderLayout.WEST);
	if (center != null) p.add(center, BorderLayout.CENTER);
	
	return p;
    }
    
    /* create a JPanel with a GridLayout and put all the components
       in the double array comps into this panel. The position of 
       each component in this panel depends on its position in the
       double array. 
       Precondition: The double array comps must be a rectangle.
       Attention: This is NOT checked. 
       For every component in the double array which is null an
       empty label is inserted instead to keep a cell empty. */
   private JPanel newGridLayoutJPanel(Component[][] comps) {
	int maxx = comps.length;
	int maxy = comps[0].length;

	JPanel p = new JPanel(new GridLayout(maxx, maxy));

	for (int i = 0; i < maxx; i++) 
	    for (int j = 0; j < maxy; j++) 
		if (p != null) p.add(comps[i][j]); 
		else p.add(new JLabel());

	return p;			     
    }

    /* creates a JPanel with a tabbed pane inside. The caller
       has to specify the orientation (TOP, BOTTOM, LEFT, RIGHT)
       and two arrays. The first array is an array of strings
       and contains the titles which are inserted into the
       tabs. The second array is an array of components which
       were inserted to the main area of the tabbed pane. 
       Precondition: The dimension of both arrays must be 
       equal.
       Attention: This is NOT checked. */
    private JPanel newTabbedJPanel(int tabPlacement, String[] titles, 
				  Component[] comps) {
	JTabbedPane tpane = new JTabbedPane(tabPlacement);
	
	for (int i = 0; i < titles.length; i++) {
	    tpane.addTab(titles[i], comps[i]);
	}
	
	JPanel result = new JPanel(new BorderLayout());
	result.add(tpane, BorderLayout.CENTER);
	return result;
    }

    /** Constructor. */
    public MP3V(){

	JMenu StdMenu = new JMenu("MP3V");
	JMenuItem MI_Remove = StdMenu.add("remove");
	MV.addMenu(StdMenu);

	MI_Remove.addActionListener( new ActionListener(){
		public void actionPerformed(ActionEvent e){
		    removeAll();
		}});


	setLayout(new BorderLayout());
	
	// north component
	add(
	    newJToolBarJPanel(new Component[] {
		PauseButton = newJButton("play", 
					 "MP3Images/play2_72.gif"),
		PlayButton = newJButton ("stop", 
					 "MP3Images/stop2_72.gif")
	    }),
	    BorderLayout.NORTH
	    );

	// center component
	add(
	    newBorderLayoutJPanel(
		// north 
		newBorderLayoutJPanel(
		    // north
		    null,
		    // south
		    null,
		    // east
		    null,
		    // west
		    newGridLayoutJPanel(new Component[][] {
			{newJLabel("Version", "")},
			{newJLabel("Title", "")},
			{newJLabel("Artist", "")},
			{newJLabel("Album", "")},
			{newJLabel("Year", "")},
			{newJLabel("Comment", "")},
			{newJLabel("Genre", "")},
			{newJLabel("Track No.", "")}
		    }), 
		    // center
		    newGridLayoutJPanel(new Component[][] {
			{
			    newGridLayoutJPanel(new Component[][] {
				{id3Version = newJTextField()},
				{id3Title = newJTextField()},
				{id3Artist = newJTextField()},
				{id3Album = newJTextField()},
				{id3Year = newJTextField()},
				{id3Comment = newJTextField()},
				{id3Genre = newJTextField()},
				{id3TrackNo = newJTextField()}
			    }),
			    newBorderLayoutJPanel(
				newJLabel("Info", ""),
				null,
				null,
				null,
				infoarea = newJTextArea()
				)
		    }})),
		// south
		null,
		// east
		null,
		// west
		null,
		// center
		newJSplitJPanel
		(JSplitPane.HORIZONTAL_SPLIT,
		 // left
		 newBorderLayoutJPanel(
		     // north
		     newJLabel("Relationen", ""),
		     // south
		     null,
		     // east
		     null,
		     // west
		     null,
		     // center
		     newScrollJPanel(
			 mychoice = newJList()
			 )
		     ),
		 // right
		 newBorderLayoutJPanel(
		     // north
		     newJLabel("Lyrics", ""),
		     // south
		     null,
		     // east
		     null,
		     // west
		     null,
		     // center
		     newScrollJPanel(
		     lyricsarea = new JList()
			 )))
		),
	    BorderLayout.CENTER
	    );
    }
    
    /** need for the Java Viewer. 
	Returns the name of this viewer/player.*/
    public String getName(){ 
	return "mp3V";
    }
    
    /* clears all components of the viewer. */
    private void clearAreas (){
	StopPlay();
	StopLyrics();
	songbuffer=null;
	lyricsbuffer=null;
	mychoice.removeAll();
	mychoice.setListData(new Vector());
	lyricsarea.removeAll();
	lyricsarea.setListData(new Vector());

	infoarea.setText ("");

	id3Version.setText("");
	id3Title.setText("");
	id3Artist.setText("");
	id3Album.setText("");
	id3Year.setText("");
	id3Comment.setText("");
	id3Genre.setText("");
	id3TrackNo.setText("");
    }
    
    /* starts the MP3 player. */
    private void StartPlay () {
	try {
	    StopPlay();
	    
	    /* if the mp3 is not defined we have to print 
	       "UNDEFINED" in the infoarea */
	    if (songbuffer != null 
		&& songbuffer.atomType() == ListExpr.SYMBOL_ATOM
		&& songbuffer.symbolValue().equals ("undef")){
		infoarea.setText ("Song: UNDEFINED");
	    }
	    else if (songbuffer!=null) {
		/* songbuffer is not empty. */
		player = new PlayerThread(songbuffer);
		player.start();
	    }
	}
	catch(Exception e){System.err.println("Error in player");}
    }

    /* stops the MP3 player. */
    private void StopPlay () {
	if (player!=null) {
	    player.close();
	    player=null;
	}
    }

    /* starts the Lyrics player. */
    private void StartLyrics (){
	try {
	    StopLyrics();
	    /* if the lyrics are not defined we have to print 
	       "UNDEFINED" in the infoarea */
	    if (lyricsbuffer != null 
		&& lyricsbuffer.atomType() == ListExpr.SYMBOL_ATOM 
		&& lyricsbuffer.symbolValue().equals ("undef")){
		infoarea.setText ("Lyrics: UNDEFINED");
	    }
	    else if (lyricsbuffer != null){
		/* lyricsbuffer is not empty. */
		onetimer = new Timer();
		onetimer.schedule (new LyricsThread (), 0, 1000);
	    }
	}
	catch(Exception e){
	    System.err.println("Error in Lyricsplayer");
	}
    }

    /* stops the Lyrics player. */
    private void StopLyrics () {
	if (onetimer != null) {
	    onetimer.cancel();
	    onetimer = null;
	}
    }


    
    /* displays an ID3 tag. LE contains the ID3 data in
	nested list format. */
    private void DisplayID3 (ListExpr LE) {
	if (LE.atomType()==ListExpr.SYMBOL_ATOM &&
	    LE.symbolValue().equals ("undef")){
	    /* the id3 is undefined*/
	    infoarea.setText ("ID3: UNDEFINED");
	    return;
	}

	id3Version.setText((LE.listLength() == 6) ? "1.0" : "1.1");
	id3Title.setText(LE.first().stringValue());
	id3Artist.setText(LE.second().stringValue());
	id3Album.setText(LE.third().stringValue());
	id3Year.setText("" + LE.fourth().intValue());
	id3Comment.setText(LE.fifth().stringValue());
	
	if (LE.listLength() == 6) {
	    /* we have to display an ID3 1.0 tag. */
	    id3Genre.setText(LE.sixth().stringValue());
	}
	if (LE.listLength() == 7) {
	    /* we have to display an ID3 1.1 tag. */
	    id3TrackNo.setText("" + LE.sixth().intValue());
	    id3Genre.setText(LE.rest().sixth().stringValue());
	}
    }

    /* displays lyrics tag. LE contains the lyrics data in
	nested list format. */
    private void DisplayLyrics (ListExpr LE) {
	ListExpr iter = LE;
	lyricsarea.removeAll ();

	if (LE.atomType()==ListExpr.SYMBOL_ATOM &&
	    LE.symbolValue().equals ("undef")){
	    /* the lyrics were undefined*/
	    infoarea.setText ("Lyrics: UNDEFINED");
	    return;
	}

	int len = LE.listLength() / 2;
	
	Vector lyricsData = new Vector();
	for (int i = 0; i < len; i++) {
	    int time = iter.first().intValue();
	    String text = iter.second().stringValue();
	    iter = iter.rest().rest(); // skip next entry.
	    lyricsData.add 
		("[" 
		 + ((time / 60) < 10 ? "0" : "") 
		 + time/60 + ":" 
		 + ((time % 60) < 10 ? "0" : "") 
		 + time % 60 + "] " + text);
	}
	lyricsarea.setListData (lyricsData);
	    
    }

    private void DisplayInt (ListExpr LE) {
	if (LE.atomType()==ListExpr.SYMBOL_ATOM &&
	    LE.symbolValue().equals ("undef")){
	    /* the int was undefined*/
	    infoarea.setText ("Int: UNDEFINED");
	    return;
	}
	infoarea.setText
	    ("Int: " 
	     + String.valueOf(LE.intValue()));
    }

    private void DisplayString (ListExpr LE) {
	if (LE.atomType()==ListExpr.SYMBOL_ATOM &&
	    LE.symbolValue().equals ("undef")){
	    /* the string was undefined*/
	    infoarea.setText ("String: UNDEFINED");
	    return;
	}
	infoarea.setText("String: " 
			 + LE.stringValue());
    }
    
    /** needed for the Java Viewer. The SecondoObject 
	obj contains a nested
	list of an MP3 or an ID3 or lyrics
	or strings or ints. */
    public boolean addObject(SecondoObject obj){
	clearAreas();
	clist = obj.toListExpr();
	
	if (!canDisplay(obj))
	    return false;
	else if (typeselect == MP3TYPE)
	{
	    /* This object contains an MP3 */
	    songbuffer = obj.toListExpr().second();
	    lyricsbuffer = null;
	    StartPlay ();
	}   
	else if (typeselect == ID3V10TYPE || typeselect == ID3V11TYPE)
	    /* This object is an ID3. (v1.0 or v1.1) */
	    DisplayID3 (obj.toListExpr().second());
	else if (typeselect == LYRICSTYPE) 
	    /* This object is a lyrics. */
	    DisplayLyrics(obj.toListExpr().second());
	else if (typeselect == STRINGTYPE) {
	    /* This object is a string. */
	    DisplayString (obj.toListExpr().second());
	} else if (typeselect == INTTYPE) {
	    /* This object is an int. */
	    DisplayInt (obj.toListExpr().second());
	} else if (typeselect == RELTYPE) {
	    /* This object is a relation. */

	    /* We have to iterate through all tuples in the relation. */
	    ListExpr iterTuple = obj.toListExpr().second();
	    numberoftuple = iterTuple.listLength();
	    /* Every tuple in the relation has the same number 
	       of attributes. */
	    numberofattr 
		= obj.toListExpr().first().second().second().listLength();
	    /* Remove all data from mychoice. 
	       New elements are inserted later. */
	    mychoice.removeAll();
	    /* Data for the mychoice. */
	    Vector choiceData = new Vector();
	    
	    /* Refill the vector choiceData. */
	    for (int i = 1; i <= numberoftuple; i++) {
		ListExpr iterAttr = obj.toListExpr().first().second().second();

		/* Iterate through all attributes of a tuple. */
		for (int j = 1; j <= numberofattr; j++) {

		    ListExpr iterAttrFF = iterAttr.first().first();
		    ListExpr iterAttrFS = iterAttr.first().second();

		    String oneline 
			= iterAttrFS.symbolValue() // type
			+" "
			+ iterAttrFF.symbolValue(); // name
		    
		    if (iterAttrFS.symbolValue().equals("string") 
			|| iterAttrFS.symbolValue().equals ("int")
			|| iterAttrFS.symbolValue().equals ("bool")) {
			
			/* We have to find the correct attribute 
			   within the selected tuple. */
			ListExpr iterAttrInTuple = iterTuple.first();
			for (int t = 1; t < j; t++) {
			    iterAttrInTuple = iterAttrInTuple.rest();
			}

			/* Now we have found the correct attribute 
			   inside the selected tuple. 
			   Now we have to add the existing data. */
			if (iterAttrInTuple.first().atomType() == ListExpr.SYMBOL_ATOM){
			    /* the variable was undefined */
			    oneline = oneline + " : UNDEFINED";
			}
			else
			{
			    /* the variable was defined, we have to read its value */
			    if (iterAttrFS.symbolValue().equals ("string"))
				oneline = oneline 
				    + " : "
				    + iterAttrInTuple.first().stringValue();
			    else if (iterAttrFS.symbolValue().equals ("int"))
				oneline = oneline 
				    + " : " 
				    + iterAttrInTuple.first().intValue();
			    else if (iterAttrFS.symbolValue().equals ("bool"))
				oneline = oneline 
				    + " : " 
				    + iterAttrInTuple.first().boolValue();
			}
		    } // if
		    /* oneline contains now the attribute type, 
		       attribute name and 
		       attribute value. */ 
		    choiceData.add(oneline);
		    iterAttr = iterAttr.rest();
		} // for j
		choiceData.add("---"); 
		iterTuple = iterTuple.rest();
	    } // for i

	    /* Now we can display the result. */
	    mychoice.setListData(choiceData);
	} // if relation
	
	/* The object could displayed. */
	return true;
    }

    /** needed for the Java viewer. */
    public void removeObject(SecondoObject o){
	/* our viewer can handle one object only (due to size of mp3s)*/
	removeAll();
    }

    /** needed for the Java viewer. */
    public void removeAll(){
	clearAreas();
	clist=null;
    }
    
    /** needed for the Java viewer. This viewer can
	display: MP3, ID3, Lyrics, int, string and relation of these types. */
    public boolean canDisplay(SecondoObject obj){
	ListExpr LE = obj.toListExpr();

	if(LE.listLength() == 2) {
	    /* our nested list must contain of a type 
	       information and the value. */
	    if (LE.first().atomType()==ListExpr.SYMBOL_ATOM) {
		if (LE.first().symbolValue().equals("mp3")) { 
		    if (LE.second().atomType()==ListExpr.TEXT_ATOM ||
			LE.second().atomType()==ListExpr.SYMBOL_ATOM )  {
			/* This nested list contains the MP3 data 
			   which can be displayed. */
			typeselect = MP3TYPE;
			return true;
		    } else {
			return false;
		    }
		} else if (LE.first().symbolValue().equals("id3") ) {
		    if (LE.second().listLength() == 6) {
			/* This nested list contains the ID3 data 
			   which is a nested list of six elements.
			   Therefore the ID3 version is 1.0 */
			typeselect = ID3V10TYPE;
			return true;
		    }
		    else if (LE.second().listLength() == 7) {
			/* this nested list contains the ID3
			   data which is a nested list of seven elements.
			   Therefore the ID3 version is 1.1 */
			typeselect = ID3V11TYPE;
			return true;
		    } 
		    else if (LE.second().atomType()==ListExpr.SYMBOL_ATOM){
			/* the id3 object is undefined*/
			typeselect = ID3V10TYPE;
			return true;
		    }
		    else
			return false;
		} else if (LE.first().symbolValue().equals("string")) {
		    if ( (LE.second().atomType() == ListExpr.STRING_ATOM) ||
			 (LE.second().atomType() == ListExpr.SYMBOL_ATOM && LE.second().symbolValue().equals ("undef") ) ){
			/* this nested list contains a string. */
			typeselect = STRINGTYPE;
			return true;
		    }
		    else 
			return false;
		} else if (LE.first().symbolValue().equals("int")) {
		    if ( (LE.second().atomType() == ListExpr.INT_ATOM) ||
			 (LE.second().atomType() == ListExpr.SYMBOL_ATOM && LE.second().symbolValue().equals("undef") ) ){
			/* this nested list contains an int. */
			typeselect = INTTYPE;
			return true;
		    }
		    else 
			return false;
		} else if (LE.first().symbolValue().equals("lyrics")) {
		    /* this nested list contains a lyrics. */
		    if (LE.second().atomType()==ListExpr.SYMBOL_ATOM ){
			/* the nested list contains undefined lyrics*/
			typeselect=LYRICSTYPE;
			return true;
		    }
		    typeselect = LYRICSTYPE;
		    return true;
		} else
		    return false;
	    }
	    /* LE.first() is not a symbol atom => LE.first() 
	       is a nested list. */
	    else if (LE.first().first().isAtom()) {
		if (LE.first().first().atomType() == ListExpr.SYMBOL_ATOM) {
		    if (LE.first().first().symbolValue().equals("rel") ||
                        LE.first().first().symbolValue().equals("mrel")) {
			/* LE contains a relation. */
			typeselect = RELTYPE;
			return true;
		    }
		    else 
			/* LE.first() contains a nested list but it contains
			   no relation. */
			return false;
		}
		else 
		    /* LE.first() is not a symbol atom. */
		    return false;
	    } else 
		/* no other object kinds supported by this viewer. */
		return false;
	
	}
	else
	    return false;
    }
    
    /** needed for the Java viewer. */
    public boolean isDisplayed(SecondoObject o){
	return false;
    }
    
    /** needed for the Java viewer. */
    public boolean selectObject(SecondoObject obj){
	return true;
    }

    /** needed for the Java viewer. */
    public MenuVector getMenuVector(){ 
	return MV;
    }
    
    /* This class is used to start a seperate thread witch plays 
       MP3 songs. */
    private class PlayerThread extends Thread implements Runnable {
	
	PlayerThread(ListExpr LE){
	    try {
		P = new Player(LE.decodeText());
	    } catch(Exception e){}
	}
	
	public void run(){
	    if(P != null)
		try {
		    P.play();
		} catch(Exception e){
		    e.printStackTrace();
		}
	}
	
	public void close(){
	    if(P != null)
		P.close();
	}
	
	Player P = null;
	
    }

    /* This class is used to start a seperate thread witch displays 
       MP3 lyrics while an mp3 is played. */
    private class LyricsThread extends TimerTask {
	/* indicates the position within the lyrics*/
	private int seconds=0;
	/* the corresponding line number*/
	private int linenumber=0;
	private String lyricsline="";

	LyricsThread(){
	    /* initianlization of variables*/
	    seconds=0;
	    linenumber=0;
	    lyricsline="";
	}
	public void run(){  		
	    /* calculate the number of entries within the lyrics nested list*/
	    int numberofentries = lyricsbuffer.listLength() / 2;
	    
	    /* runner is used to iterate through the lyrics nested list*/
	    ListExpr runner = lyricsbuffer;
	    
	    /* indicates whether the last line of the lyrics has 
	       already been displayed
	       if yes then the thread can be terminated*/
	    boolean canterminate=false;
	     	    
	    /* we now iterate through all lyricslines within 
	       the lyrics nested list
	       and check whether one line has to be display 
	       at the given point of time
	       (seconds) */
	    for (int i = 1; i <= numberofentries; i++){
		/* read the time stamp of one lyricsline*/
		int sec = runner.first().intValue();
		if (sec == seconds){
		    /* a lyricsline that corresponds to the given 
		       point of time (seconds)
		       was found */
		    if (i == numberofentries)
			/* we have found the last line
			   after it has been displayed the thread 
			   can be terminated */
			canterminate = true;
		    linenumber = i - 1;
		    /* copy the correct lyrics line */
		    lyricsline = runner.second().stringValue();
		}
		/* iterate to the next pair of timestamp and textatom*/
		runner = runner.rest().rest();
	    }
	    
	    /* display the time*/
	    infoarea.setText("Time: [" 
			     + ((seconds / 60) < 10 ? "0" : "") 
			     + seconds/60 + ":" 
			     + ((seconds % 60) < 10 ? "0" : "") 
			     + seconds % 60 + "] \n\n");
	    
	    /* display the corresponding lyricsline*/
	    infoarea.append (lyricsline);
	    
	    /* mark the correct line within the lyricsarea*/
	    lyricsarea.setSelectedIndex (linenumber);
	    
	    /* increases seconds*/
	    seconds++;

	    if (canterminate)
		cancel();		
	}	
    }
    
}
