package viewer;

import javax.swing.*;
import javax.swing.text.*;
import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import gui.SecondoObject;
import sj.lang.*;

/* this viewer shows all SecondoObjects as Lists */
public class FormattedViewer extends SecondoViewer {

 private static int LINELENGTH=80;
 private static int lastindex=0;

 private JScrollPane ScrollPane = new JScrollPane();
 private JTextField TextField = new JTextField(25);
 private JButton GoButton = new JButton("go");
 private JLabel SearchLabel = new JLabel("search",JLabel.RIGHT);
 private JTextComponent TextArea = new JTextArea();
 private JComboBox ComboBox = new JComboBox();
 private Vector ItemObjects = new Vector(10,5);
 private SecondoObject CurrentObject=null;
 private MenuVector MV = new MenuVector();
 private JMenuItem[] FontSizes;

/* create a new FormattedViewer */
 public FormattedViewer(){
   setLayout(new BorderLayout());
   JPanel Panel = new JPanel();
   Panel.add(SearchLabel);
   Panel.add(TextField);
   Panel.add(GoButton);
   JPanel TopPanel = new JPanel(new GridLayout(1,3));
   TopPanel.add(ComboBox);
   TopPanel.add(new JPanel());
   TopPanel.add(new JPanel());
   add(Panel,BorderLayout.SOUTH);
   add(BorderLayout.NORTH, TopPanel);
   //add(BorderLayout.NORTH,ComboBox);
   add(BorderLayout.CENTER,ScrollPane);
   ScrollPane.setViewportView(TextArea);
   TextField.setFont( new Font ("Monospaced", Font.PLAIN, 14));
   TextArea.setFont( new Font ("Monospaced", Font.PLAIN, 14));

   ComboBox.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
           showObject();
           if(VC !=null){
                int index = ComboBox.getSelectedIndex();
                if (index>=0){
                   try{
                       CurrentObject = (SecondoObject) ItemObjects.get(index);
                       VC.selectObject(FormattedViewer.this,CurrentObject);
                   }
                   catch(Exception e){}
                }
           }
     }});

    GoButton.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
	 try {
	     searchText();
	 }
	 catch(Exception e){}
     }
   });

   TextField.addKeyListener(new KeyAdapter(){
	   public void keyPressed(KeyEvent evt){
	       if( evt.getKeyCode() == KeyEvent.VK_ENTER )
		   searchText();
	   }
   });

   JMenu FontSizeMenu = new JMenu("Fontsize");
   ActionListener FS_Listener=new ActionListener(){
      public void actionPerformed(ActionEvent evt){
          JMenuItem S = (JMenuItem) evt.getSource();
	  try{
             int size = Integer.parseInt(S.getText().trim());
	     FormattedViewer.this.TextArea.setFont( new Font ("Monospaced", Font.PLAIN, size));
	     for(int i=0;i<9;i++)
	       FontSizes[i].setEnabled(true);
	     S.setEnabled(false);
	  }catch(Exception e){}
      }
   };

   FontSizes = new JMenuItem[9];
   for(int i=0;i<9;i++){
      FontSizes[i] = FontSizeMenu.add(""+(2*i+8));
      FontSizes[i].addActionListener(FS_Listener);
      if((i*2+8)==14)
        FontSizes[i].setEnabled(false);
   }

   MV.addMenu(FontSizeMenu);

 }

 /* searches for a string in the TextArea */
 public void searchText() {
     String content = TextArea.getText();
     String search = TextField.getText();
     int i1 = content.indexOf(search, lastindex +1);
     int i2 = i1 + search.length();
     lastindex = i1;
     if (i1 >= 0) {
	TextArea.setCaretPosition(i1);
	TextArea.moveCaretPosition(i2);
	TextArea.getCaret().setSelectionVisible(true);
     }
     else{
       i1 = 0;
       MessageBox.showMessage("end of text is reached");
     }
 }

 /* adds a new Object to this Viewer and display it */
 public boolean addObject(SecondoObject o){
   if (isDisplayed(o))
       selectObject(o);
   else{
      ItemObjects.add(o);
      ComboBox.addItem(o.getName());
      try{
         ComboBox.setSelectedIndex(ComboBox.getItemCount()-1);  // make the new object to active object
         showObject();
      }
      catch(Exception e){}
   }
   return true;
 }

 /* returns true if o a SecondoObject in this viewer */
 public boolean isDisplayed(SecondoObject o){
   return ItemObjects.indexOf(o)>=0;
 }

 /** remove o from this Viewer */
 public void removeObject(SecondoObject o){
    if (ItemObjects.remove(o))
        ComboBox.removeItem(o.getName());
 }


 /** remove all containing objects */
 public void removeAll(){
     ItemObjects.removeAllElements();
     ComboBox.removeAllItems();
     CurrentObject= null;
     if(VC!=null)
        VC.removeObject(null);
     showObject();
 }


 /* returns allways true (this viewer can display all SecondoObjects) */
 public boolean canDisplay(SecondoObject o){
    ListExpr LE = o.toListExpr();
    if(LE.listLength()!=2)
       return false;

    if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM  || !LE.first().symbolValue().equals("inquiry"))
       return false;

    ListExpr V = LE.second();
    if(V.listLength()!=2)
       return false;

    ListExpr T = V.first();
    if(T.atomType()!=ListExpr.SYMBOL_ATOM)
       return false;
    String Name = T.symbolValue();
    if(Name.equals("constructors") || Name.equals("operators") || Name.equals("algebra") ||
       Name.equals("algebras") || Name.equals("databases") || Name.equals("types") ||
       Name.equals("objects"))
       return true;
    return false;
 }


 /* returns the Menuextension of this viewer */
 public MenuVector getMenuVector(){
    return MV;
 }

 /* returns Formatted */
 public String getName(){
    return "FormattedViewer";
 }

 public double getDisplayQuality(SecondoObject SO){
    if(canDisplay(SO))
       return 1.0;
    else
       return 0;
 }




 /* select O */
 public boolean selectObject(SecondoObject O){
    int i=ItemObjects.indexOf(O);
    if (i>=0) {
       ComboBox.setSelectedIndex(i);
       showObject();
       return true;
    }else //object not found
      return false;
 }

 private static int maxHeaderLength( ListExpr type) {
     int max, len;
     String s;

     max = 0;
     while( !type.isEmpty() ) {
	 s = type.first().stringValue();
	 len = s.length();
	 if (len > max) {
	     max = len;
	 }
	 type = type.rest();
     }
     return max;
 }

 private static ListExpr concatLists(ListExpr le1, ListExpr le2) {
     if ( le1.isEmpty() ) return le2;
     else {
	 ListExpr first = le1.first();
	 ListExpr rest = le1.rest();
	 ListExpr second = concatLists( rest, le2);
	 ListExpr newnode = ListExpr.cons( first, second);
	 return newnode;
     }
 }

 private String displayDescriptionLines( ListExpr value, int maxNameLen) {
     ListExpr valueheader, valuedescr;
     String outstr, s, blanks, line, restline, printstr;
     boolean firstline, lastline;
     int position, lastblank, i;

     valueheader = value.second();
     valuedescr = value.third();
     blanks = "";
     outstr = "\n";
     for (i = 1; i <= maxNameLen-4; i++) blanks += " ";
     outstr += blanks + "Name: ";
     outstr += value.first().symbolValue() + "\n";
     printstr = "";
     while ( !valueheader.isEmpty() ) {
	 s = valueheader.first().stringValue();
	 blanks = "";
	 for (i = 1; i <= maxNameLen-s.length(); i++) blanks += " ";
	 printstr = blanks + s + ": ";
	 if ( valuedescr.first().isAtom() ) {
	     if ( valuedescr.first().atomType() == ListExpr.STRING_ATOM )
		 printstr += valuedescr.first().stringValue();
	     else
		 if( valuedescr.first().atomType() == ListExpr.TEXT_ATOM )
		     for (i = 0; i < 1 ; i++)
			 printstr += valuedescr.first().textValue();
	 }

     // check whether line break is necessary
	 if (printstr.length() <= LINELENGTH) {
	     outstr += printstr + "\n";
	 }
	 else {
	 firstline = true;
	 position = 0;
	 lastblank = -1;
	 line = "";
         for (i = 1; i <= printstr.length(); i++) {
	     line += printstr.charAt(i-1);
	     if(printstr.charAt(i-1) == ' ' ) lastblank = position;
	     position++;
	     lastline = (i == printstr.length());

	     if ( (firstline && (position == LINELENGTH)) || (!firstline &&
	       (position == (LINELENGTH-maxNameLen-2))) || lastline) {
		 if (lastblank > 0) {
		     if (firstline) {
			 if (lastline && (line.length() <= LINELENGTH)) {
			     outstr += line + "\n";
			 }
			 else outstr += line.substring(0, lastblank) + "\n";
			 firstline = false;
		     }
		     else {
			 blanks = "";
			 for (int j = 1; j <= maxNameLen+2; j++) blanks += " ";
			 if (lastline && (line.length() <= LINELENGTH))
			     outstr += blanks + line + "\n";
			 else outstr += blanks + line.substring(0,lastblank) + "\n";
		     }
		     restline = line.substring(lastblank+1, position);
		     line = "";
		     line += restline;
		     lastblank = -1;
		     position=line.length();
		 }
                 else {
		     if (firstline) {
			 outstr += line + "\n";
			 firstline = false;
		     }
		     else {
			 blanks = "";
			 for (int j = 1; j <= maxNameLen+2; j++) blanks += " ";
			 outstr += blanks + line + "\n";
		     }
		     line = "";
		     lastblank = -1;
		     position = 0;
		 }
	     }
	 }
         }
     valueheader = valueheader.rest();
     valuedescr = valuedescr.rest();
     }
     return outstr;
 }

 
 /** returns the display text for a list of database names */
 private String getDatabasesText(ListExpr Databases){
   String res = "\n--------------------\n";
   res += "Database";
   res += Databases.listLength()>1?"s\n":"\n";
   res += "--------------------\n\n";
   while(!Databases.isEmpty()){
      res+= "* "+Databases.first().symbolValue()+"\n";
      Databases = Databases.rest();
   }
   return res;
 }

 /** returns the display text for a list of algebra names */
 private String getAlgebrasText(ListExpr Algebras){
   String res = "\n--------------------\n";
   res += "Algebra";
   res += Algebras.listLength()>1?"s\n":"\n";
   res += "--------------------\n\n";
   while(!Algebras.isEmpty()){
      res+= "* "+Algebras.first().symbolValue()+"\n";
      Algebras = Algebras.rest();
   }
   return res;
 }

 /** return the displayText for object inquiry - just the nested list format */
 private String getObjectsText(ListExpr objects){
   objects = objects.rest(); // ignore symbol OBJECTS
   String  Text = "\n--------------------\n";
   Text += "Object";
   Text += objects.listLength()>1?"s\n":"\n";
   Text += "--------------------\n\n";
   if(objects.isEmpty())
      Text +="none";
   else
      Text += objects.writeListExprToString();
   return Text;
 }

 /** return the display text for a types inquiry as nested list */
private String getTypesText(ListExpr types){
   types = types.rest(); // ignore symbol TYPES
   String  Text = "\n--------------------\n";
   Text += "Type";
   Text += types.listLength()>1?"s\n":"\n";
   Text += "--------------------\n\n";
   if(types.isEmpty())
      Text += "none";
   else
      Text += types.writeListExprToString();
   return Text;
 }

/** returns the display text for operators inquiry */
private String getOperatorsText(ListExpr operators){
   String Text = "\n--------------------\n";
   Text += "Operator";
   Text += operators.listLength()>1?"s\n":"\n";
   Text += "--------------------\n\n";
   ListExpr LE = operators;
   ListExpr headerlist = operators;
   //ListExpr concatenatedlist = nl.theEmptyList();
   ListExpr concatenatedlist = headerlist.first().second();
   while (!headerlist.isEmpty()) {
       concatenatedlist =
	   concatLists(concatenatedlist, headerlist.first().second());
            headerlist = headerlist.rest();
   }
   int maxHeadNameLen = maxHeaderLength( concatenatedlist );

   while ( !LE.isEmpty() ) {
       Text += this.displayDescriptionLines( LE.first(), maxHeadNameLen );
       LE = LE.rest();
   }
   return Text;
}

/** returns the display text for constructors inquiry */
private String getConstructorsText(ListExpr constructors){
   String Text = "\n--------------------\n";
   Text += "Type Constructor";
   Text += constructors.listLength()>1?"s\n":"\n";
   Text += "--------------------\n\n";
   ListExpr LE = constructors;
   ListExpr headerlist = constructors;
   ListExpr concatenatedlist = headerlist.first().second();
   while (!headerlist.isEmpty()) {
       concatenatedlist =
	   concatLists(concatenatedlist, headerlist.first().second());
            headerlist = headerlist.rest();
   }
   int maxHeadNameLen = maxHeaderLength( concatenatedlist );
   while ( !LE.isEmpty() ) {
       Text += this.displayDescriptionLines( LE.first(), maxHeadNameLen );
       LE = LE.rest();
   }
   return Text;
}


/** returns the display text for algebra xxx inquiry */
private String getAlgebraText(ListExpr algebra){
   String AlgebraName = algebra.first().symbolValue();

   String Text = "\n--------------------\n";
   Text += "Algebra "+AlgebraName+"\n";
   Text += "--------------------\n\n";

   ListExpr Constructors = algebra.second().first();
   ListExpr Operators = algebra.second().second();

   ListExpr headerlist = Constructors;
   ListExpr concatenatedlist = headerlist.first().second();
   while (!headerlist.isEmpty()) {
       concatenatedlist =
	   concatLists(concatenatedlist, headerlist.first().second());
            headerlist = headerlist.rest();
   }
   headerlist = Operators;
   while (!headerlist.isEmpty()) {
       concatenatedlist =
	   concatLists(concatenatedlist, headerlist.first().second());
            headerlist = headerlist.rest();
   }
   int maxHeadNameLen = maxHeaderLength( concatenatedlist );

   Text += "\n--------------------\n";
   Text += "Constructor";
   Text += Constructors.listLength()>1?"s\n":"\n";
   Text += "--------------------\n\n";
   if(Constructors.isEmpty())
       Text+="   none   \n";
   else
      while ( !Constructors.isEmpty() ) {
           Text += this.displayDescriptionLines( Constructors.first(), maxHeadNameLen );
           Constructors = Constructors.rest();
      }

   Text += "\n\n\n--------------------\n";
   Text += "Operator";
   Text += Operators.listLength()>1?"s\n":"\n";
   Text += "--------------------\n\n";
   if(Operators.isEmpty())
       Text+="   none   \n";
   else
      while ( !Operators.isEmpty() ) {
           Text += this.displayDescriptionLines( Operators.first(), maxHeadNameLen );
           Operators = Operators.rest();
      }


   return Text;
}



 private void showObject(){

    ListExpr nl, headerlist, concatenatedlist, temp1, temp2;
    int maxHeadNameLen;
    String Text;

    Text = "";
    //TextArea.setText(Text);
    int index = ComboBox.getSelectedIndex();
    if(index<0){
       TextArea.setText("");
       return;
    }

    try{
       CurrentObject = (SecondoObject) ItemObjects.get(index);
       ListExpr LEx = CurrentObject.toListExpr().second(); // ignory "inquiry"-entry
       String type = LEx.first().symbolValue();
       ListExpr value = LEx.second();
       if(type.equals("databases")){
          Text += getDatabasesText(value);
       } else if(type.equals("algebras")){
           Text += getAlgebrasText(value);
       } else if(type.equals("objects")){
           Text += getObjectsText(value);
       } else if(type.equals("types")){
           Text += getTypesText(value);
       } else if(type.equals("operators")){
           Text += getOperatorsText(value);
       }
       else if(type.equals("constructors")){
           Text += getConstructorsText(value);
       } else if(type.equals("algebra")){
           Text += getAlgebraText(value);
       }
       else{
           Text = "unknow inquiry type";
       }

       TextArea.setText(Text);

    }
    catch(Exception e){}
    }



}

