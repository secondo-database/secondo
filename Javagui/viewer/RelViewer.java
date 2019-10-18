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


import sj.lang.*;
import javax.swing.*;
import javax.swing.table.*;
import java.awt.*;
import java.util.Vector;
import gui.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.JFileChooser;
import tools.Reporter;
import java.util.StringTokenizer;



class TableTypePair{
  TableTypePair(JTable t, ListExpr[] types){
     this.table = t;
     this.types = types;
  }

   JTable table;
   ListExpr[] types;
}


public class RelViewer extends SecondoViewer{

 private JComboBox ComboBox;
 private JScrollPane ScrollPane;
 private JTable CurrentTable;
 private Vector<TableTypePair> Tables;
 private JPanel dummy;  // to show nothing
 private JButton exportBtn, importBtn, printBtn;
 private JFileChooser filechooser;

 // the value of MAX_TEXT_LENGTH must be greater then five
 private final static int MAX_TEXT_LENGTH=100;

 /** creates a new RelationViewer **/
 public RelViewer(){
   ComboBox = new JComboBox();
   ScrollPane = new JScrollPane();
   dummy = new JPanel();
   CurrentTable = null;
   Tables = new Vector<TableTypePair>();
   exportBtn = new JButton("export");
   importBtn = new JButton("import");
   printBtn = new JButton("print");

   JPanel p = new JPanel();
   p.add(exportBtn);
   p.add(importBtn);
   p.add(printBtn);
   
   
   filechooser = new JFileChooser(".");

   setLayout(new BorderLayout());
   add(ComboBox,BorderLayout.NORTH);
   add(ScrollPane,BorderLayout.CENTER);
   add(p,BorderLayout.SOUTH);


   ComboBox.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          showSelectedObject();
       }});
       
       
       
   exportBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          if (exportAsCSV()== false) {
          Reporter.showInfo("Export CSV file failed");
          }
       }
   });
   
   
   printBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          if (printTable()== false) {
          Reporter.showInfo("Export CSV file failed");
          }
       }
   });

   importBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          if(!importFromCSV()){
             Reporter.showInfo("Import CSV file failed");
           }
       }
   });
   
   

 }


 /** shows the table of the selected object **/
 private void showSelectedObject(){
    int index = ComboBox.getSelectedIndex();
    if(index>=0){
      CurrentTable = (JTable)Tables.get(index).table;
      ScrollPane.setViewportView(CurrentTable);
    }
    else
      ScrollPane.setViewportView(dummy);

 }
 
 
 
 
 private boolean importFromCSV(){
 
   // check whether table is selected
   int index = ComboBox.getSelectedIndex();   
   if(index < 0) { 
     Reporter.showInfo("no Table selected");
     return false;
   }
      
   // check whether attribute types are supported
   ListExpr[] types = Tables.get(index).types;
   
   
   if(!checkCSVTypes(types)) { 
     Reporter.showInfo("Unsupported Type in table");
     return false;
   }
      
   // open file
   if(filechooser.showOpenDialog(this)!=JFileChooser.APPROVE_OPTION) {   
       return true;
   }
   
   
   File file = filechooser.getSelectedFile();
   
   
   int choice = 0;
   Object[] opts = {"Update", "Insert", "Cancel"};
   
   int select = JOptionPane.showOptionDialog(null, "Please choose...(Return for Update)", "Select Update/Insert",
                                                            JOptionPane.DEFAULT_OPTION, 
                                                            JOptionPane.INFORMATION_MESSAGE, 
                                                            null, opts, opts[0]);
  choice = select; 
  if (choice == 2) 
   {
     Reporter.showInfo("Insert or update failure - User abort");
     return false;
   }
  
  
  String delim = JOptionPane.showInputDialog(null,"Please specify the delimiter (type \"tab\" for tabulator)", "Import: Delimiter input",
                                                             JOptionPane.PLAIN_MESSAGE);
                                                             
  if (delim.equals(""))
  { Reporter.showInfo("Empty delimiter inserted");
    return  false;                                                      
  }      
  
  if (delim.equals("tab"))
  { 
    delim = "\t";                                                      
  }      
  
                                                             
  String skip = JOptionPane.showInputDialog(null,"Insert number >= 0 of rows to be skipped-OK for 0 skips", "Import: Skip rows",JOptionPane.PLAIN_MESSAGE);
     
     
   
  ListExpr result = importCSV(file,types, delim, skip);
   
      
   
  if(result==null)
   { 
     Reporter.debug("csv file import");
     return false;
   }
   
   ListExpr relHeader = getHeader(types, Tables.get(index).table);   
   
   ListExpr objList = ListExpr.twoElemList(relHeader,result);      
   
   
   
   
   
   //Constructing the new table or insert into the table
   
   String update = "update";
   String let = "let";  
   String query = "query";
   String insert = "insert";
   String consume = "consume";
   String feed = "feed";
   
   
   String noitalerpmet = "noitalerpmet";
   
   String ins1 = "(query(consume(insert(feed noitalerpmet)";
   String ins3= ")))";   
   String que = "(query noitalerpmet)"; 
   String del = "delete noitalerpmet";   
   
   String assign = ":=";
   String assign2 = "=";
   
   String eingabe = JOptionPane.showInputDialog(null,
                          "Confirm update of the selected relation " +
                          "by typing in its name and pressing OK.\n " +
                          "Otherwise the update will not take place.",
                          "Relation Update",
                          JOptionPane.PLAIN_MESSAGE);
   
   String ins2 = eingabe; 
   
  ListExpr updaterel = ListExpr.fourElemList(ListExpr.symbolAtom(update), ListExpr.symbolAtom(eingabe), ListExpr.symbolAtom(assign), objList);
  ListExpr letrel = ListExpr.fourElemList(ListExpr.symbolAtom(let), ListExpr.symbolAtom(noitalerpmet), ListExpr.symbolAtom(assign2), objList);
   
   
  String ins = ins1 + ins2 + ins3;
  
   
   
   
   String text = updaterel.toString();
   String text2 = letrel.toString();   
 
     
   
  SecondoObject obj = new SecondoObject(file.getName(), objList);
  SecondoObject obj2 = new SecondoObject(file.getName(), objList); 
   
   
   
   boolean ok = false; 
   if (choice == 1)  {   
     VC.addObject(obj2);
     this.addObject(obj2);
     int rval = VC.execCommand(text2);
     int rval4 = VC.execCommand(ins);
     int rval3 = VC.execCommand(del);
   
     ok = (rval==0) && (rval4==0) && (rval4==0);
   } else { 
      VC.addObject(obj);
      this.addObject(obj);
      int rval2 = VC.execCommand(text);
      ok = rval2==0;
   }  
   if(!ok){
      Reporter.showInfo("Insert or update failure - Secondo error");
   } else if(VC.optimizerAvailable()){
     String r = VC.sendToOptimizer("updateRel("+ins2+")");
     if(r==null){
        Reporter.showInfo("Problem in updating the optimizer's knowledge base about relation " + ins2);
     }
   }
   return ok;
   
 }
 
 
 
 
 
 
 
 
 
 
 
private boolean checkCSVTypes(ListExpr[] types)
 {
   for(int i=0;i<types.length;i++)
   {
     if(!checkCSVType(types[i]))
     {
       return false;
     }
   }
   return true;
 }
 
 
 
 private boolean checkCSVType(ListExpr list)
 {
    if(list.atomType()!=ListExpr.SYMBOL_ATOM)
    {
       return false;
    }
    String sv = list.symbolValue();
    if(sv.equals("int")) return true;
    if(sv.equals("string")) return true;
    if(sv.equals("real")) return true;
    if(sv.equals("bool")) return true;
    if(sv.equals("text")) return true;
    if(sv.equals("date")) return true;
    
    // to be continued
    return false;
 }
 

 
  
 
 
 
 
 
 private ListExpr importCSV(File file, ListExpr[] types, String delm, String inskip)
 {
 
    try
    {
      ListExpr result = null;
      ListExpr last = null;
      BufferedReader in = new BufferedReader(new FileReader(file));
      BufferedReader test = new BufferedReader(new FileReader(file));
      
     
     String dummyheader;
     String dummycomment;
     int countcomments = 0;
     
     
     int readloop = -2;
     int read = test.read();      
     int read2 = test.read();
     int readbom = -2;
     int skip = 0;
     
     
     
     
     if (read == 65279)  
      {
      readbom = in.read();                                            //skipping the BOM in the in buffer
      }
   
   
   
     
   if (!inskip.equals(""))                                            //empty skip is a skip of 0 rows  
    {
     
     
      try
      {
       skip = Integer.parseInt(inskip);
      
      }
       catch(Exception e)
       {  
         Reporter.showInfo("Skip Rows: A number must be inserted");
         return null;
        
       }
     
     } 
     
   else   skip = 0;
     
          
     if (skip < 0) skip = 0;                                       // negativ skips are skips of 0 rows 
     
     
    
      //skipping inserted number of lines
      
      for (int i=1; i<=skip; i++)
       {
        dummyheader = in.readLine().trim();  
        
       if (!in.ready()) 
         {
          Reporter.showInfo("Skip Rows: You wanna skip more rows than you have");
          return null;
        
         }
        
        
       }
      
      
      
      
      // end of  skipping inserted number of lines
         
         
         
   
     
     
     //Skipping  #-comment lines  
     
    
     while ( (read == 35 && readloop==-2) || (read2 == 35 && readloop==-2) || readloop == 35)  //maybe there is a BOM before a "#"
      {       
       dummycomment = test.readLine().trim();
       readloop = test.read();       
       countcomments++;
      }
            
     
     if (skip<countcomments)
      {
            
      for (int i=1; i<=countcomments-skip; i++)
       {
        dummyheader = in.readLine().trim();                           //skipping the # rows
       }
      }
     
      
    // end of   #-comment lines  
      
      
    
      
      
      
      
      
      while(in.ready())                                                // going through the buffer lines
      { 
      
        String line = in.readLine().trim();
        if(line.length()>0)
        {
          ListExpr tuple = importCSVTuple(line,types,delm);
          
          if(tuple==null)
           {              
             in.close();
             test.close();
             return null;
           } 
           
          if(result==null)
           {
             result = ListExpr.oneElemList(tuple);
             last = result;
           } 
            
            else 
            {
             last = ListExpr.append(last,tuple);
            }
        }  
       
      }
      
      
      in.close();
      test.close();
      return result;
    } 
    
    catch(Exception e)
     {
       Reporter.debug(e);
       return null;
     }
 
 }
 

 
 
  
 
 
 
 
 
 private ListExpr importCSVTuple(String line, ListExpr[] types, String delim)
 {  
    
    char delimchar;
    char [] delimchararray;
    
    try
    {
     delimchararray = delim.toCharArray();
     delimchar = delimchararray[0];
    
          
     }
     catch(Exception e)
     {  
        Reporter.showInfo("Empty delimiter inserted");
        return  null;
     }
    
    
    
 
    MyStringTokenizer st = new MyStringTokenizer(line, delimchar);
    MyStringTokenizer lang = new MyStringTokenizer(line, delimchar);    
    StringTokenizer lang2 = new StringTokenizer(line, delim);    
    
    int count = 0;
    int count2 = lang2.countTokens();
    int typelenght = types.length;
    int delimnumber = CountDelims (line, delimchar);
    String tokentest = "dummy";     
    
    
    while (lang.hasMoreTokens())
     {     
      count++;
      tokentest = lang.nextToken();      
     }
    
       
    
     
   if ((count2 > typelenght) || ( (count2 == count) &&  (count > typelenght)) || (count > typelenght) 
                             ||  (delimnumber >= typelenght) || delimnumber < typelenght-1)              //Table mismatch
    
    
    {     
       Reporter.showInfo("Table mismatch or delimiter mismatch");
       return null;   
    }
    
      
      
    
    ListExpr res = null;
    ListExpr last = null;
    ListExpr attr = null;
    
    int i=0;
    int lenght=types.length;
    int trigger= 0;
    
    String token = "dummy";
    
    
    
    while (i<lenght)
    
    {  
              
      token = st.nextToken();    
      
     
      
        
      attr = importAttr(types[i],token);
       
              
       
       if(attr==null)
        {         
          return null;
        }
       
         
       
       if(res==null)
        {
          res = ListExpr.oneElemList(attr);
          last = res;
          
          
          
        }
        
       else {
              last = ListExpr.append(last,attr);
            }
            
            
      i++;  
      
    }
    
       
    return res;
 }
 
 

 
 
 
// Hilfsmethoden
 
 
  private int CountDelims (String input, char c) 
  {
    int count = 0;
    for (char act : input.toCharArray()) 
    {
     if (act == c) 
      {
       count++;
      }
    }
    return count;
  }
 
 
 
 
 
 
 
 
 

 
 //------------------------------------------------------
 
  
 
 
 private ListExpr importAttr(ListExpr type, String value)
 {
  
  if (value.trim().equals(""))
   {
   return ListExpr.symbolAtom("undef");
   }
   
  else
  
  { 
  
    String t = type.symbolValue();
    
    if(value.trim().equals("undef")  || value.trim().equals("undefined"))
    
    {
      return ListExpr.symbolAtom("undefined");
    }
    
    
    
    
    if(t.equals("int"))
    
    { 
      try
       {      
        int v = Integer.parseInt(value.trim());
     
        return ListExpr.intAtom(v);
       }
      
        catch(NumberFormatException e)
        {
         return ListExpr.symbolAtom("undef");       
        }
     } 
     
   
   
   
   
   
   if(t.equals("real"))
    { 
     
     try
      {
     
        double v = Double.parseDouble(value.trim());
      
        return ListExpr.realAtom(v);             
      }    
    
      catch(NumberFormatException e)
        {        
         return ListExpr.symbolAtom("undef");
        }
        
     }
     
     
     
     
     
    
    if (t.equals("bool"))
    
    { 
     
     try
      { 
     
        Boolean v = Boolean.parseBoolean(value.trim());
      
        return ListExpr.boolAtom(v);   
        
      }    
    
      catch(NumberFormatException e)
        {
         return ListExpr.symbolAtom("undef");
        }
       
    
     }
    
    
    
    
    
    if(t.equals("string"))
     
    {
    
     try
      {      
          
       return  ListExpr.stringAtom(value.trim());
        
      }    
    
      catch(NumberFormatException e)
        {
         return ListExpr.symbolAtom("undef");
        }
    
     } 
    
    
    
    if(t.equals("text"))
     
    {
    
     try
      {      
          
       return  ListExpr.textAtom(value.trim());
        
      }    
    
      catch(NumberFormatException e)
        {
         return ListExpr.symbolAtom("undef");
        }
    
     } 
    
    
    
   if(t.equals("date"))
     
    {
    
     try
      {      
          
       return  ListExpr.stringAtom(value.trim());
        
      }    
    
      catch(NumberFormatException e)
        {
         return ListExpr.symbolAtom("undefined");
        }
    
     } 
    
    
  
  
  } //end of else
    
    
    return null; // unsupported type
}
 
 
 
 
 
 
 
 

 
 
 
 
 
private ListExpr getHeader(ListExpr[] types, JTable table)
 {
    ListExpr res = null;
    ListExpr last = null;
    
    String tuple = "tuple";
    String relation = "rel";
    
    
    for(int i=0;i<types.length;i++){
      String name = table.getColumnName(i).toString();
      ListExpr attr = ListExpr.twoElemList( ListExpr.symbolAtom(name), types[i]);
      if(res==null){
        res = ListExpr.oneElemList(attr);
        last = res;
      } else {
        last = ListExpr.append(last,attr);
      }
    }
    
    ListExpr tup = ListExpr.twoElemList(ListExpr.symbolAtom(tuple), res);
    ListExpr rel = ListExpr.twoElemList(ListExpr.symbolAtom(relation), tup);
    
    
     
    return rel;
    
    
    
    
 }
 
 
 
 
 
 
 
 

 
 
 
 
 
 
 
 
 
 
 
 
 

 public  String getName(){
   return "RelationsViewer";
 }

 /** exports the currently selected table into a file in
   * csv format.
   */
 private boolean  exportAsCSV()
 {
    int index = ComboBox.getSelectedIndex();
    char delimchar;
    char [] delimchararray;
    
    if(index<0) {
      tools.Reporter.showError("No table is selected");
      return true;
    }
    
    
    JTable theTable = (JTable) Tables.get(index).table;
    if(filechooser.showSaveDialog(this)==JFileChooser.APPROVE_OPTION) {
       File file = filechooser.getSelectedFile();
       if(file.exists()){
         if(tools.Reporter.showQuestion("File already exists\n overwrite it?")!=tools.Reporter.YES){
             return true;
         }
       }
       
       
       String delim = JOptionPane.showInputDialog(null,"Please specify the delimiter-(type \"tab\" for tabulator)", "Export: Delimiter input",
                                                             JOptionPane.PLAIN_MESSAGE);   
      
        try {
        
           if (delim.equals("tab"))
             { 
               delim = "\t";   
               
             }      
             
           delimchararray = delim.toCharArray();
           
           
           delimchar = delimchararray[0];
        } catch(Exception e) {  
           Reporter.showInfo("Empty delimiter inserted");
           return  false;
        }
    
        exportTable(theTable,file, delimchar);
      
        return true;
    } 
    return true;
 }


 private boolean printTable(){
    int index = ComboBox.getSelectedIndex();
    if(index<0) {
      tools.Reporter.showError("No table is selected");
      return true;
    }
    JTable theTable = (JTable) Tables.get(index).table;
    try{
       boolean complete = theTable.print();
       if(complete){
         Reporter.showInfo("printing complete");
       } else {
          Reporter.showInfo("printing canceled");
       }
    } catch(Exception e){
        Reporter.debug(e);
        return false;
    }
    return true; 
 }



 
 
 
 
 
 
 
 
 /** function supporting expostAsCSV
   */
  private void exportTable(JTable table, File file, char delimchar){
    try{
       PrintStream out = new PrintStream(new FileOutputStream(file));
    
       // print out the header
       for(int j=0;j<table.getColumnCount();j++) { 
         if(j==0)  {
           out.print("#");
         }
         
         if(j>0) {
          out.print(delimchar);
         }
         out.print((""+table.getColumnModel().getColumn(j).getIdentifier()).replaceAll("\n",""));
        
       }
       out.println("");
    
       for(int i=0;i<table.getRowCount(); i++){
          for(int j=0;j<table.getColumnCount();j++){
            if(j>0){
               out.print(delimchar);
            }
            out.print((""+table.getValueAt(i,j)).replaceAll("\n",""));
          }
          out.println("");
       } 
       out.close();
    } catch(IOException e){
       tools.Reporter.debug(e);
       tools.Reporter.showError("Error in writing file");
    }
  }



 public boolean addObject(SecondoObject o){
   if (isDisplayed(o)){
      selectObject(o);
      return false;
   }
   else{
      TableTypePair pair = createTableFrom(o.toListExpr());
      if(pair==null)
          return false;
      else{
         Tables.add(pair);
         ComboBox.addItem(o.getName());
         selectObject(o);
         ScrollPane.setViewportView(pair.table);
         return true;
       }
    }
 }



 /** created a new JTable from LE, if LE not a valid
   * ListExpr for a relation null ist returned
   **/
 private TableTypePair createTableFrom(ListExpr LE){
 boolean result = true;
 JTable NTable = null;
 ListExpr[] types;
  if (LE.listLength()!=2)
     return null;
  else{
    ListExpr type = LE.first();
    ListExpr value = LE.second();
    // analyse type
    ListExpr maintype = type.first();
    if (type.listLength()!=2 || !maintype.isAtom() || maintype.atomType()!=ListExpr.SYMBOL_ATOM
        || !(maintype.symbolValue().equals("rel") || maintype.symbolValue().equals("mrel") 
        || maintype.symbolValue().equals("trel") || maintype.symbolValue().equals("memoryRelObject")))
       return null; // not a relation
    ListExpr tupletype = type.second();
    // analyse Tuple
    ListExpr TupleFirst=tupletype.first();
    if (tupletype.listLength()!=2 || !TupleFirst.isAtom() ||
         TupleFirst.atomType()!=ListExpr.SYMBOL_ATOM ||
	 !(TupleFirst.symbolValue().equals("tuple") | TupleFirst.symbolValue().equals("mtuple")))
       return null; // not a tuple
    ListExpr TupleTypeValue = tupletype.second();
    // the table head
    String[] head=new String[TupleTypeValue.listLength()];
    types= new ListExpr[head.length];
    for(int i=0;!TupleTypeValue.isEmpty()&&result;i++) {
        ListExpr TupleSubType = TupleTypeValue.first();
        if (TupleSubType.listLength()!=2)
           result = false;
        else{
           head[i] = TupleSubType.first().writeListExprToString();
          // remove comment below if Type is wanted
          types[i] = TupleSubType.second();
        }
        TupleTypeValue = TupleTypeValue.rest();
    }

   if (result){
     // analyse the values
     ListExpr TupleValue;
     Vector<String[]> V = new Vector<String[]>();
     String[] row;
     int pos;
     ListExpr Elem;
     while (!value.isEmpty()){
         TupleValue = value.first();
         row = new String[head.length];
         pos = 0;
         while(pos<head.length & !TupleValue.isEmpty()){
           Elem = TupleValue.first();
           if (Elem.isAtom() && Elem.atomType()==ListExpr.STRING_ATOM)
              row[pos] = Elem.stringValue();
           else if((Elem.isAtom() && Elem.atomType()==ListExpr.TEXT_ATOM) ||
	           (!Elem.isAtom() && Elem.listLength()==1 && Elem.first().isAtom() &&
		     Elem.first().atomType()==ListExpr.TEXT_ATOM)){
	     if(!Elem.isAtom())
	        Elem = Elem.first();
	     if(Elem.textLength()<MAX_TEXT_LENGTH)
	        row[pos] = Elem.textValue();
	     else
	        row[pos] = Elem.textValue(); //.substring(0,MAX_TEXT_LENGTH-4)+" ...";
           }
	   else
              row[pos] = TupleValue.first().writeListExprToString();
           pos++;
           TupleValue = TupleValue.rest();
          }
         V.add(row);
         value = value.rest();
     }

     String[][] TableDatas = new String[V.size()][head.length];
     for(int i=0;i<V.size();i++)
          TableDatas[i]=(String[]) V.get(i);

     NTable = new JTable(TableDatas,head);
    }
  }

  if(result) 
      return new TableTypePair(NTable, types);
   else 
      return null;
}












 /** returns the index in ComboBox of S,
   * if S not exists in ComboBox -1 is returned 
   */
 private int getIndexOf(String S){
   int count =  ComboBox.getItemCount();
   int pos = -1;
   for(int i=0;i<count;i++){
     if( (ComboBox.getItemAt(i)).equals(S)) pos=i;
   }
   return pos;
 }

 /** removes a object from this viewer **/
 public void removeObject(SecondoObject o){
    int index = getIndexOf(o.getName());
    if(index>=0){
       ComboBox.removeItemAt(index);
       Tables.remove(index);
       showSelectedObject();
    }
 }
 
 public void removeAll(){
   ComboBox.removeAllItems();
   Tables.clear();
   showSelectedObject();
 
 }

 public boolean canDisplay(SecondoObject o){
   ListExpr LE = o.toListExpr();
   if(LE.listLength()!=2)
      return false;
   else{
     LE = LE.first();
     if(LE.isAtom() || LE.isEmpty())
       return false;
     else{
       LE = LE.first();
       if(LE.isAtom() && LE.atomType()==ListExpr.SYMBOL_ATOM &&
          (LE.symbolValue().equals("rel") | LE.symbolValue().equals("mrel") | LE.symbolValue().equals("trel") | LE.symbolValue().equals("memoryRelObject")) )
           return true;
       else
           return false;
     }
   }  
 }
 
 

 /** check if o displayed in the moment **/
 public boolean isDisplayed(SecondoObject o){
   return getIndexOf(o.getName())>=0;
 }

/** hightlighting of o **/
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

 /** get the MenuExtension for MainWindow
   *
   **/
 public MenuVector getMenuVector(){ return null;}


 /** set the Control for this viewer **/
 public void setViewerControl(ViewerControl VC){
      this.VC = VC;
 }

}



