
package viewer;

import javax.swing.*;
import javax.swing.text.*;
import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import gui.SecondoObject;
import sj.lang.*;

public class InquiryViewer extends SecondoViewer{

 // define supported subtypes
 private static final String DATABASES = "databases";
 private static final String CONSTRUCTORS="constructors";
 private static final String OPERATORS = "operators";
 private static final String ALGEBRAS = "algebras";
 private static final String ALGEBRA = "algebra";
 private static final String TYPES = "types";
 private static final String OBJECTS ="objects";

 private JScrollPane ScrollPane = new JScrollPane();
 private JEditorPane  HTMLArea = new JEditorPane();
 private JComboBox ComboBox = new JComboBox();
 private Vector ObjectTexts = new Vector(10,5);
 private Vector SecondoObjects = new Vector(10,5);
 private SecondoObject CurrentObject=null;

 private String HeaderColor = "silver";
 private String CellColor ="white";
 private MenuVector MV = new MenuVector();
 private JTextField SearchField = new JTextField(20);
 private JButton SearchButton = new JButton("Search");
 private int LastSearchPos =0;
 private JCheckBox CaseSensitive = new JCheckBox("Case Sensitive");


/* create a new InquiryViewer */
 public InquiryViewer(){
   setLayout(new BorderLayout());
   add(BorderLayout.NORTH,ComboBox);
   add(BorderLayout.CENTER,ScrollPane);
   HTMLArea.setContentType("text/html");
   HTMLArea.setEditable(false);
   ScrollPane.setViewportView(HTMLArea);

   JPanel BottomPanel = new JPanel();
   BottomPanel.add(CaseSensitive);
   BottomPanel.add(SearchField);
   BottomPanel.add(SearchButton);
   add(BottomPanel,BorderLayout.SOUTH);
   CaseSensitive.setSelected(true);

   ComboBox.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
	showObject();
     }});

    SearchField.addKeyListener(new KeyAdapter(){
        public void keyPressed(KeyEvent evt){
	   if( evt.getKeyCode() == KeyEvent.VK_ENTER )
		   searchText();
    }});

    SearchButton.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           searchText();
       }});


    JMenu SettingsMenu = new JMenu("Settings");
    JMenu HeaderColorMenu = new JMenu("header color");
    JMenu CellColorMenu = new JMenu("cell color");
    SettingsMenu.add(HeaderColorMenu);
    SettingsMenu.add(CellColorMenu);





    ActionListener HeaderColorChanger = new ActionListener(){
      public void actionPerformed(ActionEvent evt){
          JMenuItem S = (JMenuItem) evt.getSource();
		HeaderColor = S.getText().trim();
		reformat();
       }
    };
    ActionListener CellColorChanger = new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          JMenuItem S = (JMenuItem) evt.getSource();
		CellColor = S.getText().trim();
		reformat();
       }
    };

    HeaderColorMenu.add("white").addActionListener(HeaderColorChanger);
    HeaderColorMenu.add("silver").addActionListener(HeaderColorChanger);
    HeaderColorMenu.add("gray").addActionListener(HeaderColorChanger);
    HeaderColorMenu.add("aqua").addActionListener(HeaderColorChanger);
    HeaderColorMenu.add("blue").addActionListener(HeaderColorChanger);
    HeaderColorMenu.add("black").addActionListener(HeaderColorChanger);
    CellColorMenu.add("white").addActionListener(CellColorChanger);
    CellColorMenu.add("yellow").addActionListener(CellColorChanger);
    CellColorMenu.add("aqua").addActionListener(CellColorChanger);
    CellColorMenu.add("lime").addActionListener(CellColorChanger);
    CellColorMenu.add("silver").addActionListener(CellColorChanger);

    MV.addMenu(SettingsMenu);
 }

 /** returns the html formatted string representation for an atomic list */
 private String getStringValue(ListExpr atom){
   int at = atom.atomType();
   String res = "";
   switch(at){
      case ListExpr.NO_ATOM : return "";
      case ListExpr.INT_ATOM : return ""+atom.intValue();
      case ListExpr.BOOL_ATOM : return atom.boolValue()?"TRUE":"FALSE";
      case ListExpr.REAL_ATOM : return ""+atom.realValue();
      case ListExpr.STRING_ATOM: res = atom.stringValue();break;
      case ListExpr.TEXT_ATOM: res = atom.textValue();break;
      case ListExpr.SYMBOL_ATOM: res = atom.symbolValue();break;
      default : return "";
   }

   res = replaceAll("&",res,"&amp");
   res = replaceAll("<",res,"&lt;");
   res = replaceAll(">",res,"&gt;");
   return res;
 }


 /** include for using older Java-versions */
 private static String replaceAll(String what, String where, String ByWhat){
   StringBuffer res = new StringBuffer();
   int lastpos = 0;
   int len = what.length();
   int index = where.indexOf(what,lastpos);
   while(index>=0){
       if(index>0)
          res.append(where.substring(lastpos,index));
       res.append(ByWhat);   
       lastpos = index+len;
       index = where.indexOf(what,lastpos);
   }
   res.append(where.substring(lastpos));
   return res.toString();
 }


/** searchs the text in the textfield in the document and
  * marks its if found 
  */
private void searchText(){
  String Text = SearchField.getText();
  if(Text.length()==0){
    MessageBox.showMessage("no text to search");
    return;
  }
  try{
     Document Doc = HTMLArea.getDocument();
     String DocText = Doc.getText(0,Doc.getLength());
     if(!CaseSensitive.isSelected()){
        DocText = DocText.toUpperCase();
	Text = Text.toUpperCase();
     }
     int pos = DocText.indexOf(Text,LastSearchPos);
     if(pos<0){
        MessageBox.showMessage("end of text is reached");
        LastSearchPos=0;
        return;
     }
     pos = pos;
     int i1 = pos;
     int i2 = pos+Text.length();
     LastSearchPos = pos+1;
     HTMLArea.setCaretPosition(i1);
     HTMLArea.moveCaretPosition(i2);
     HTMLArea.getCaret().setSelectionVisible(true);
  } catch(Exception e){
    if(DEBUG_MODE)
       e.printStackTrace();
    MessageBox.showMessage("error in searching text");

  }

}




/** returns the html string for a single 	entry for
	* type constructors or operators
	*/
 private String formatEntry(ListExpr LE){
   if(LE.listLength()!=3){
     System.err.println("InquiryViewer : error in list (listLength() # 3");
     return "";
   }
   ListExpr Name = LE.first();
   ListExpr Properties = LE.second();
   ListExpr Values = LE.third();
   if(Properties.listLength()!= Values.listLength()){
      System.err.println("InquiryViewer : Warning: lists "+
			"have different lengths ("+Name.symbolValue()+")");
   }

   String res =" <tr><td class=\"opname\" colspan=\"2\">" +
		Name.symbolValue() + "</td></tr>\n";
   while( !Properties.isEmpty() & ! Values.isEmpty()){
     res = res + "   <tr><td class=\"prop\">" +
		getStringValue(Properties.first())+"</td>" +
		"<td class=\"value\">" +
		getStringValue(Values.first())+"</td></tr>\n";
     Properties = Properties.rest();
     Values = Values.rest();
   }

   // handle non empty lists
   // if the lists are correct this never should occur
   while( !Properties.isEmpty()){
     res = res + "   <tr><td class=\"prop\">" +
		getStringValue(Properties.first())+"</td>" +
		"<td>   </td></tr>\n";
     Properties = Properties.rest();
   }
   while(!Values.isEmpty()){
     res = res + "   <tr><td> </td>" +
		"<td class=\"value\">" +
		getStringValue(Values.first())+"</td></tr>\n";
     Values = Values.rest();
   }

   return res;
 }


 /** create the html head for text representation
   * including the used style sheet
   */
 private String getHTMLHead(){
    StringBuffer res = new StringBuffer();
    res.append("<html>\n");
    res.append("<head>\n");
    res.append("<title> inquiry viewer </title>\n");
    res.append("<style type=\"text/css\">\n");
    res.append("<!--\n");
    res.append("td.opname { background-color:"+HeaderColor+
		 "; font-family:monospace;"+
		 "font-weight:bold; "+
		 "color:green; font-size:x-large;}\n");
    res.append("td.prop  {background-color:"+CellColor+
		 "; font-family:monospace; font-weight:bold; color:blue}\n");
    res.append("td.value {background-color:"+CellColor+
		 "; font-family:monospace; color:black;}\n");
    res.append("-->\n");
    res.append("</style>\n");
    res.append("</head>\n");
    return res.toString();
  }


 /** get the html formatted html Code for type contructors
   */
 private String getHTMLCode_Constructors(ListExpr ValueList){
    StringBuffer res = new StringBuffer();
    if(ValueList.isEmpty())
       return "no type constructors are defined <br>";
    res.append("<table border=\"2\">\n");
     while(!ValueList.isEmpty() ){
       res.append(formatEntry(ValueList.first()));
       ValueList = ValueList.rest();
    }
    res.append("</table>\n");
    return res.toString();
 }

 /** returns the html-code for  operators */
 private String getHTMLCode_Operators(ListExpr ValueList){
   if(ValueList.isEmpty())
       return "no operators are defined <br>";
   // the format is the same like for constructors
   return getHTMLCode_Constructors(ValueList);
 }


 /** returns the html for an Algebra List */
 private String getHTMLCode_Databases(ListExpr Value){
   // the valuelist for algebras is just a list containing
   // symbols representing the database names
   if(Value.isEmpty())
      return "no database exists <br>";
   StringBuffer res = new StringBuffer();
   res.append("<ul>\n");
   while (!Value.isEmpty()){
     res.append("<li> "+Value.first().symbolValue() + " </li>");
     Value = Value.rest();
   }
   res.append("</ul>");
   return res.toString();
 }
 
 /** returns the html code for objects */
 private String getHTMLCode_Objects(ListExpr Value){
   ListExpr tmp = Value.rest(); // ignore  "SYMBOLS"
   if(tmp.isEmpty())
      return "no existing objects";
   StringBuffer res = new StringBuffer();
   res.append("<h2> Objects - short list </h2>\n ");
   res.append("<ul>\n");
   while(!tmp.isEmpty()){
      res.append("  <li>"+tmp.first().second().symbolValue()+ " </li> \n");
      tmp = tmp.rest();
   }
   res.append("</ul><br><hr><br>");
   res.append("<h2> Objects - full list </h2>\n");
   res.append("<pre>\n"+Value.rest().writeListExprToString() +"</pre>");
   return res.toString();
 }

 /** returns the html code for types */
 private String getHTMLCode_Types(ListExpr Value){
   ListExpr tmp = Value.rest(); // ignore  "TYPES"
   if(tmp.isEmpty())
      return "no existing type";
   StringBuffer res = new StringBuffer();
   res.append("<h2> Types - short list </h2>\n ");
   res.append("<ul>\n");
   while(!tmp.isEmpty()){
      res.append("  <li>"+tmp.first().second().symbolValue()+ " </li> \n");
      tmp = tmp.rest();
   }
   res.append("</ul><br><hr><br>");
   res.append("<h2> Types - full list </h2>\n");
   res.append("<pre>\n"+Value.rest().writeListExprToString() +"</pre>");
   return res.toString();
 }



 /** returns a html formatted list for algebras */
 private String getHTMLCode_Algebras(ListExpr Value){
   // use the same format like databases
   if(Value.isEmpty())
      return "no algebra is included <br> please check your Secondo installation <br>";
   return getHTMLCode_Databases(Value);
 }

 /** returns the formatted html code for a algebra inquiry */
 private String getHTMLCode_Algebra(ListExpr Value){
    // the format is
    // (name ((constructors) (operators)))
    // where constructors and operators are formatted like in the
    // non algebra version
    StringBuffer res = new StringBuffer();
    res.append("<h1> Algebra "+Value.first().symbolValue()+" </h1>\n");
    res.append("<h2> type constructors of algebra: "+
		Value.first().symbolValue()+" </h2>\n");
    res.append( getHTMLCode_Constructors(Value.second().first()));
    res.append("<br>\n<h2> operators of algebra: "+
	        Value.first().symbolValue()+"</h2>\n");
    res.append( getHTMLCode_Operators(Value.second().second()));
    return res.toString();
 }

 /** returns the html code for a given list */
 private String getHTMLCode(ListExpr VL){
      StringBuffer Text = new StringBuffer();
      Text.append(getHTMLHead());
      Text.append("<body>\n");
      String inquiryType = VL.first().symbolValue();
      if (inquiryType.equals(DATABASES)){
         Text.append("<h1> Databases </h1>\n");
	 Text.append(getHTMLCode_Algebras(VL.second()));
      }
      else if(inquiryType.equals(ALGEBRAS)){
         Text.append("<h1> Algebras </h1>\n");
	 Text.append(getHTMLCode_Algebras(VL.second()));
      }
      else if(inquiryType.equals(CONSTRUCTORS)){
        Text.append("<h1> Type Constructors </h1>\n");
	Text.append(getHTMLCode_Constructors(VL.second()));
      }
      else if(inquiryType.equals(OPERATORS)){
	Text.append("<h1> Operators </h1>\n");
	Text.append(getHTMLCode_Operators(VL.second()));
      }
      else if(inquiryType.equals(ALGEBRA)){
	Text.append(getHTMLCode_Algebra(VL.second()));
      }
      else if(inquiryType.equals(OBJECTS)){
	Text.append("<h1> Objects </h1>\n");
	Text.append(getHTMLCode_Objects(VL.second()));
      }
      else if(inquiryType.equals(TYPES)){
	Text.append("<h1> Types </h1>\n");
	Text.append(getHTMLCode_Types(VL.second()));
      }
      Text.append("\n</body>\n</html>\n");
      return Text.toString();
 }



 /* adds a new Object to this Viewer and display it */
 public boolean addObject(SecondoObject o){
   if(!canDisplay(o))
      return false;
   if (isDisplayed(o))
       selectObject(o);
   else{
      ListExpr VL = o.toListExpr().second();
      ObjectTexts.add(getHTMLCode(VL));
      ComboBox.addItem(o.getName());
      SecondoObjects.add(o);
      try{
         ComboBox.setSelectedIndex(ComboBox.getItemCount()-1);
         showObject();
      }
      catch(Exception e){
        if(DEBUG_MODE)
	   e.printStackTrace();
      }
   }
   return true;
 }

 /** write all htmls texts with a new format */
 private void reformat(){
   int index = ComboBox.getSelectedIndex();
   ObjectTexts.removeAllElements();
   for(int i=0;i<SecondoObjects.size();i++){
      SecondoObject o = (SecondoObject) SecondoObjects.get(i);
      ListExpr VL = o.toListExpr().second();
      String inquiryType = VL.first().symbolValue();
      ObjectTexts.add(getHTMLCode(VL));
   }
   if(index>=0)
     ComboBox.setSelectedIndex(index);
 }



 /* returns true if o a SecondoObject in this viewer */
 public boolean isDisplayed(SecondoObject o){
   return SecondoObjects.indexOf(o)>=0;
 }

 /** remove o from this Viewer */
 public void removeObject(SecondoObject o){
    int index = SecondoObjects.indexOf(o);
    if(index>=0){
        ComboBox.removeItem(o.getName());
        SecondoObjects.remove(index);
        ObjectTexts.remove(index);
    }
 }


 /** remove all containing objects */
 public void removeAll(){
     ObjectTexts.removeAllElements();
     ComboBox.removeAllItems();
     SecondoObjects.removeAllElements();
     CurrentObject= null;
     if(VC!=null)
        VC.removeObject(null);
     showObject();
 }


/** check if this viewer can display the given object */
public boolean canDisplay(SecondoObject o){
    ListExpr LE = o.toListExpr();
    if(LE.listLength()!=2)
       return false;
    if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM ||
       !LE.first().symbolValue().equals("inquiry"))
       return false;
    ListExpr VL = LE.second();
    if(VL.listLength()!=2)
       return false;
    ListExpr SubTypeList = VL.first();
    if(SubTypeList.atomType()!=ListExpr.SYMBOL_ATOM)
      return false;
    String SubType = SubTypeList.symbolValue();
    if(SubType.equals(DATABASES) || SubType.equals(CONSTRUCTORS) ||
       SubType.equals(OPERATORS) || SubType.equals(ALGEBRA) ||
       SubType.equals(ALGEBRAS)  || SubType.equals(OBJECTS) ||
       SubType.equals(TYPES))
       return true;
    return false;
  }


 /* returns the Menuextension of this viewer */
 public MenuVector getMenuVector(){
    return MV;
 }

 /* returns InquiryViewer */
 public String getName(){
    return "InquiryViewer";
 }

 public double getDisplayQuality(SecondoObject SO){
    if(canDisplay(SO))
       return 0.9;
    else
       return 0;
 }


 /* select O */
 public boolean selectObject(SecondoObject O){
    int i=SecondoObjects.indexOf(O);
    if (i>=0) {
       ComboBox.setSelectedIndex(i);
       showObject();
       return true;
    }else //object not found
      return false;
 }

 private void showObject(){
    String Text="";
    int index = ComboBox.getSelectedIndex();
    if (index>=0){
       HTMLArea.setText((String)ObjectTexts.get(index));
    }   else {
       // set an empty text
       HTMLArea.setText(" <html><head></head><body></body></html>");
    }
    LastSearchPos = 0;

 }
}

