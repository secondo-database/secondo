
package viewer;


import javax.swing.*;
import gui.*;
import sj.lang.ListExpr;
import java.awt.*;
import java.awt.event.*;
import tools.Reporter;
import java.util.*;


enum Type {STRING,INT,REAL,BOOL,TEXT,GENERIC};

/** This class is a SecondoViewer usuable for querying predefined functions.
  * In fact, this viewer displays no objects.
  **/
public class FunctionParamViewer extends SecondoViewer{

  private JPanel paramPanel;                     // panel including the params and the send button
  private JComboBox functions;                   // selection for functions 

  private ListExpr objectList;                   // lists of objects in the database
  private JPanel emptyPanel;                     // displayed if no function is selected
  private JPanel currentPanel;                   // pointer to the currently display panel
  private ActionListener theActionListener;      // actionListener for sending a qury to Secondo 
  private String funName;                        // name of the currently selected function 

  private JComboBox selectDBCB;                  // combobox for database selection
  private JComboBox selectFunDescCB;             // combobox for selecting the table containing functions descriptions
  private JComboBox selectParamDescCB;           // combobox for selecting the table containing parameter descriptions


  private String funDescTable;                    // name of the selected table with fucntion descriptions
  private HashMap<String,String> funDescMap;      // mapping from function name to function description 

  private String paramDescTable;                                    // name of the table for parameter descriptions
  private HashMap< String, HashMap<String, String> > paramDescMap;  // mapping function name -> (mapping param  name -> param description


  private tools.HTMLDialog helpDialog;
  private MenuVector mv;

  private final int maxFunDesc = 1000;
  private final int maxParamDesc = 10000;



  /** Constructs a new FunctionParamViewer. **/
  public FunctionParamViewer(){
      paramPanel = new JPanel();
      setLayout(new BorderLayout());
      add(paramPanel, BorderLayout.CENTER);
  		JPanel controlPanel = new JPanel();
      
      controlPanel.setLayout(new GridLayout(2,5));
      controlPanel.add(new JLabel("Update DB list"));
      controlPanel.add(new JLabel("Database"));  
      controlPanel.add(new JLabel("fun descriptions"));
      controlPanel.add(new JLabel("param descriptions"));
      controlPanel.add(new JLabel("use function"));
      
      JButton updateDbBtn = new JButton("update DB");
      controlPanel.add(updateDbBtn);
      updateDbBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
           updateDB();
        }
      });
	
      selectDBCB = new JComboBox();
      selectDBCB.addItem("none");
      controlPanel.add(selectDBCB); 
      selectDBCB.addItemListener(new ItemListener(){
         public void itemStateChanged(ItemEvent evt){
             if(evt.getStateChange() == ItemEvent.SELECTED){
                  Object o = evt.getItem();
                  if(o==null){
                      selectDB(null);
                  }  else {
                      selectDB(o.toString());
                  }
             }  
         } 
      });

      selectFunDescCB = new JComboBox();
      selectFunDescCB.addItem("none");
      controlPanel.add(selectFunDescCB);
      selectFunDescCB.addItemListener(new ItemListener(){
         public void itemStateChanged(ItemEvent evt){
             if(evt.getStateChange() == ItemEvent.SELECTED){
                  Object o = evt.getItem();
                  if(o==null){
                    selectFunDesc(null);
                  } else {
                    selectFunDesc(o.toString());
                  }
             }  
         } 
      });
      
      selectParamDescCB = new JComboBox();
      selectParamDescCB.addItem("none"); 
      controlPanel.add(selectParamDescCB);
      selectParamDescCB.addItemListener(new ItemListener(){
         public void itemStateChanged(ItemEvent evt){
             if(evt.getStateChange() == ItemEvent.SELECTED){
                  Object o = evt.getItem();
                  if(o==null){
                    selectParamDesc(null);
                  } else {
                    selectParamDesc(o.toString());
                  }
             }  
         } 
      });
       
      functions = new JComboBox();
      functions.addItem("none");
      controlPanel.add(functions); 
      functions.addItemListener(new ItemListener(){
          public void itemStateChanged(ItemEvent evt){
             if(evt.getStateChange()==ItemEvent.SELECTED){
                 buildPanel();
             } else {
                setEmpty();
            }
          }             
      });

      add(controlPanel,BorderLayout.SOUTH);


      emptyPanel = new JPanel();
      currentPanel = emptyPanel;
      paramPanel.add(currentPanel);

      theActionListener = new ActionListener(){
          public void actionPerformed(ActionEvent evt){
              doQuery();
          }
      };
      helpDialog = new tools.HTMLDialog("Help");
      helpDialog.setText(getHelpText());

      JMenu menu = new JMenu("FunctionParamViewer");
      JMenuItem helpItem = new JMenuItem("Help");
      helpItem.addActionListener(new ActionListener(){
          public void actionPerformed(ActionEvent evt){
             helpDialog.setVisible(true);
          }
      });
      menu.add(helpItem);
      mv = new MenuVector();
      mv.addMenu(menu);

  }

   private String getHelpText(){
       String res =     "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
                    +    "\"http://www.w3.org/TR/html4/loose.dtd\">";
       res += "<html>\n";
       res += "<head>\n";
       res += "<style TYPE=\"text/css\">\n";
       res += "<!--\n";
       res += " h1{color:red;} \n"
            + " h2{color:blue;}\n"
            + "body{ background:#efffef;}";
       res += "-->\n";
       res += "</style>";
       res += "</head>\n";
       res += "<body>\n";
       
       res += "<h1> FunctionParamViewer </h1>\n";
       res += "<h2> Propose of this Viewer </h2>\n";
       res +=" <div>\n";
       res +=   "This viewer is intended to perform predefined functions stored in "
              + " a database. If the database is well prepared, the user will find some "  
              + "functions as well as table describing the functionality of these functions "
              + " and there parameters.<br>\n"
              + " The viewer does not display any Secondo objects. If a query is send to "
              + " the Secondo DBMS, an appropriate viewer for this result will be chosen."; 
       res +="</div>\n";

       res += "<h2> Usage of this Viewer </h2>\n";
       res += "<div>\n";
       res += "This viewer can be used in several steps: <br>"
             + "<ol>"
             + "  <li> Press the update Button <br>"
             + "       This retrieves the avaiable databases of Secondo and inserts them into the selection box </li>\n"
             + "  <li> Select the database <br>"
             + "       After selecting a database, this database is scanned for function objects and tables"
             + "       providing desctiptions for functions and parameters. All candidate tables are "
             + "       inserted into the corresponding selection boxes.</li>"
             + "  <li> Selection of function and parameter descriptions <br>"
             + "       Using the selection boxes, one can choose the tables containing the descriptions"
             + "       for fucntions and their parameters. </li>"
             + "  <li> Selection of the function <br>"
             + "       During selection of the fucntion, tool tips are provided giving information about the "
             + "       the functions where the mouse is over. If there is no tool tip, either the is no "
             + "       table selected containing function descriptions or the selected table does not "
             + "       have any information about this function. <br>"
             + "       After selecting a certain function, the main area is filled with control elements"
             + "       for entering the parameters of the selected function. </li>"
             + "  <li> Entering Parameters <br>"
             + "       After selecting a function, the main area contains for each parameter of this function"
             + "       a row consisting of four columns. <br>"
             + "       The first columns contains the name of this parameter, the second column is a text field"
             + "       for entering constant values, the third column is a selection box for selecting database objects as parameter,"
             + "       and the last column describes the type of this parameter. <br>"
             + "       <b> Note: </b> The content of the text field is only used, if the database object selection is &quot;const&quot;."
             + "       The &quot;send query&quot; called the function stored in the Secondo system using the entered parameter.  </li> " 
             + "</ol>";
        
       res += "</div>\n";

       res += "<h2> For Database Administrators </h2>\n";
       res += " To use a database with this viewer for querying, at least one function must be provided. "
           + " Each function should be described in a table. Even auxiliary functions should be described, e.g. "
           + " with &quot;For internal use only&quot;. <br>"
           + " For function descriptions a table with two attributes is used. The first attribute has to be of "
           + " type string and represents the name of the function to describe."
           + " The second attribute can be of type string or of type text and contains the description of this function. <br>"
           + " Parameter descriptions are stored in another table with three attributes. "
           + " The first two attributes have to be of type string and refer to the function name and the parameter name, respectively.<br>"
           + " The third attribute contains the description of this parameter and has to be of type string or text. <br><br>"
           + " For reasons of storage management, the size of describing tables is restricted. "
           + " The maximum size of the table for function descriptions is " + maxFunDesc + " and the size of the table "
           + " describing the parameters of all functions is restricted to " + maxParamDesc+".<br>"
           + " The viewer only uses the table scheme for detecting if it contains fucntion or parameter descriptions. "
           + " It's recommended to have only the describing tables with this type or these table have speaking names. ";
       res += "</body>\n";
       res += "</html>\n";
       return res;
   }


   /** retrieves the available databases,
    * and resets all other contents. 
    **/
   private void updateDB(){
       // clean all combo boxes
       selectDBCB.removeAllItems();
       selectFunDescCB.removeAllItems();
       selectParamDescCB.removeAllItems();
       functions.removeAllItems();

       selectDBCB.addItem("none");
       selectFunDescCB.addItem("none");
       selectParamDescCB.addItem("none");
       functions.addItem("none");       
       
       ListExpr databases  = VC.getCommandResult("list databases");
       if(databases==null){
          Reporter.showError("Retrieving available databases failed. \n May be you are not connected to a Secondo server");
          return;
       }
       databases = databases.second().second();  // ignore inquiry and databases
       while(!databases.isEmpty()){
           selectDBCB.addItem(databases.first().toString());
           databases = databases.rest(); 
       }
       selectDBCB.setSelectedIndex(0); // select "none"
   }

   /** Select a database. 
     * The objects stored in the database are get from the server and 
     * analysed to be a description table or a function.
     **/ 
   private void selectDB( String dbName){
       selectFunDescCB.removeAllItems();
       selectParamDescCB.removeAllItems();
       functions.removeAllItems();
       selectFunDescCB.addItem("none");
       selectParamDescCB.addItem("none");
       functions.addItem("none");       
       if(dbName==null || dbName.equals("none")){
         return; 
       }
       int openOk = VC.execCommand("{close database | open database "+dbName+" }");
       if(openOk!=0){
          Reporter.showError("Could not open database " + dbName);
          return;
       }
      
       // get content of the database
       ListExpr objects = VC.getCommandResult("list objects");
       if(objects==null){
         Reporter.showError("Could not retrieve function. \n May be, no database is opened");
         return;
       }  
       objectList = objects.second().second().rest();
       retrieveFunDesc();
       retrieveParamDesc();
       retrieveFunctions();
   }



   /** Retrieves all objects having type rel(tuple( string, string)) or rel(tuple(string, text)) 
     * and inserts the names to fundescCB
     * when calling this functions, objectList must be up to date.
     */
  private void retrieveFunDesc(){
     ListExpr olist = objectList;
     selectFunDescCB.removeAllItems();
     selectFunDescCB.addItem("none");
     while(!olist.isEmpty()){
        ListExpr o = olist.first();
        olist = olist.rest();
        ListExpr name = o.second();
        ListExpr type = o.fourth();
        if(isFunDescType(type)){
          selectFunDescCB.addItem(name.toString()); 
        }
     }
  } 

  /** Checks whether a type corresponds to a function description table.
    * This means, the type must be a relation having tuples with two attributes.
    * The first attribute is the name of the fucntion and must be of type string.
    * The second attribute is assumed to be a description for the function and must
    * be of type string or of type text.
    **/
  private boolean isFunDescType(ListExpr type){
      if(type.listLength()==1){
         type = type.first();
      }
      if(type.listLength()==2 && isSymbol(type.first(),"rel")){
          type = type.second();
          if(type.listLength()==2 && isSymbol(type.first(),"tuple")){
             ListExpr attrList = type.second();
             if(attrList.listLength()==2){
                 ListExpr attr1 = attrList.first();
                 ListExpr attr2 = attrList.second();
                 if(    attr1.listLength()==2 && attr2.listLength()==2 
                     && isSymbol(attr1.second(),"string")  
                     && (isSymbol(attr2.second(),"string") || isSymbol(attr2.second(),"text"))){
                     return true;
                 }
             }
          }       
      }
      return false;
  }

  /** Scans the list of objects in the database and inserts
    * possible parameter description tables into the according combobox.
    **/
  private void retrieveParamDesc(){
     ListExpr olist = objectList;
     selectParamDescCB.removeAllItems();
     selectParamDescCB.addItem("none");
     while(!olist.isEmpty()){
        ListExpr o = olist.first();
        olist = olist.rest();
        ListExpr name = o.second();
        ListExpr type = o.fourth();
        if(isParamDescType(type)){
        selectParamDescCB.addItem(name.toString()); 
        }
     }
  }


  /** Checks whether a given type is possible a parameter description.
    * In this case, the type must be a relation containing tuples with three
    * attributes. The first attribute is the name of the function and has to 
    * be of type string. The second attribute is the name of the parameter
    * of this function and has also to be of type string. The third attribute
    * contains the description of this parameter and has to be of type string 
    * or of typoe text. 
    **/
  private boolean isParamDescType(ListExpr type){
        if(type.listLength()==1){
           type = type.first();
        }
        if(type.listLength()==2 && isSymbol(type.first(),"rel")){
            type = type.second();
            if(type.listLength()==2 && isSymbol(type.first(),"tuple")){
               ListExpr attrList = type.second();
               if(attrList.listLength()==3){
                   ListExpr attr1 = attrList.first();
                   ListExpr attr2 = attrList.second();
                   ListExpr attr3 = attrList.third();
                   if(    attr1.listLength()==2 && attr2.listLength()==2 && attr3.listLength()==2 
                       && isSymbol(attr1.second(),"string")  
                       && isSymbol(attr2.second(),"string")
                       && (isSymbol(attr3.second(),"string") || isSymbol(attr3.second(),"text"))){
                       return true;
                   }
               }
            }       
        }
        return false;
  }

  /** Returns the string value of a list of type string, text or symbol. **/
  private static String getText(ListExpr e){
     if(e.atomType()==ListExpr.STRING_ATOM){
        return e.stringValue();
     }
     if(e.atomType()==ListExpr.TEXT_ATOM){
        return e.textValue();
     }
     if(e.atomType()==ListExpr.SYMBOL_ATOM){
        return e.symbolValue();
     }
     return null;
  }

  /** Selects the table used for function descriptions.
    * All functions in the function combobox are extended
    * by a tooltip text if a description of this function is 
    * available.
    **/
  private void selectFunDesc(String tableName){
      funDescTable = null;
      funDescMap = null;
      functions.setToolTipText(null);
      if(tableName == null || tableName.equals("none")){
         return;
      }

      ListExpr tableCount = VC.getCommandResult("query " + tableName + " count");
      if(tableCount==null){
         Reporter.showError("Could not retrieve size of function description table " + tableName);
         return;
      }
      int tc = tableCount.second().intValue();
      funDescTable = tableName;
      funDescMap = new HashMap<String,String>();
      if(tc <= maxFunDesc){ // retrieve the complete table
         ListExpr table = VC.getCommandResult("query " + tableName);
         if(table==null){
            Reporter.writeError("Could not retrieve table " + tableName);
            return;
         }
         ListExpr type = table.first();
         ListExpr value = table.second();
         if(!isFunDescType(type)){
             Reporter.writeError("Table " + tableName + " has a wrong type \n (press update button and open the db again)");
             return;
         }
            
         while(!value.isEmpty()){
            ListExpr tuple = value.first();
            value = value.rest();
            String fn = getText(tuple.first());
            String fd = getText(tuple.second());
            funDescMap.put(fn,fd);
         }
      }  else {
          Reporter.writeError("The table for function descriptions " + tableName + " is too big");
          return;
      }
      // addToolTips ad the functions combobox
      DefaultListCellRenderer functionsTT = new DefaultListCellRenderer(){
         public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus){
             JComponent comp = (JComponent) super.getListCellRendererComponent(list,value,index,isSelected, cellHasFocus);
             if(index > -1 && value!=null && !value.toString().equals("none") && funDescMap!=null) {
                 String desc = funDescMap.get(value.toString().trim());
                 list.setToolTipText(desc);
             } else {
                 list.setToolTipText(null);
             }
             return comp;
         }

      };
      Object o = functions.getSelectedItem();
      if(o==null){
         functions.setToolTipText(null);
      }  else {
         String d = funDescMap.get(o.toString().trim()); 
         functions.setToolTipText(d);
      }
      functions.setRenderer(functionsTT); 
  }

 
  /** Selects the table of the parameter descriptions.
    * The content of this table is collected in the paramDescMap mapping.
    **/ 
  private void selectParamDesc(String tableName){
      if(tableName == null || tableName.equals("none")){
         paramDescTable = null;
         paramDescMap = null;
         changeParamToolTips();
         return;
      }
      paramDescTable = tableName;
      paramDescMap = new HashMap<String, HashMap<String,String> >();
      
      ListExpr tableCount = VC.getCommandResult("query " + tableName + " count");
      if(tableCount==null){
         Reporter.showError("Could not retrieve size of parameter description table " + tableName);
         changeParamToolTips();
         return;
      }
      int tc = tableCount.second().intValue();
			if(tc <= maxParamDesc){ // retrieve the complete table
         ListExpr table = VC.getCommandResult("query " + tableName);
         if(table==null){
            Reporter.writeError("Could not retrieve table " + tableName);
            changeParamToolTips();
            return;
         }
         ListExpr type = table.first();
         ListExpr value = table.second();
         if(!isParamDescType(type)){
             Reporter.writeError("Table " + tableName + " has a wrong type \n (press update button and open the db again)");
             changeParamToolTips();
             return;
         }
            
         while(!value.isEmpty()){
            ListExpr tuple = value.first();
            value = value.rest();
            String fn = getText(tuple.first());
            String an = getText(tuple.second());
            String ad = getText(tuple.third());
            HashMap<String,String> funMap = paramDescMap.get(fn);
            if(funMap!=null){
               funMap.put(an,ad);
            } else {
               funMap = new HashMap<String,String>();
               funMap.put(an,ad);
               paramDescMap.put(fn,funMap); 
            }
         }
         changeParamToolTips();
      }  else {
          Reporter.writeError("The table for parameter descriptions " + tableName + " is too big");
          changeParamToolTips();
          return;
      }
       
  }

  /** Changes the tooltips for parameters according to the selection. **/
   private void changeParamToolTips(){
     int c = currentPanel.getComponentCount();
     for(int i=0;i<c;i++){
         Component p = currentPanel.getComponent(i);
         if(p instanceof ParamPanel){
            ParamPanel pp = (ParamPanel) p;
            pp.changeToolTips();
         }
     } 

   }



  /** Builds a query from the current values of parameters and sends
   * this query to the Secondo DBMS.
   **/
  private void doQuery(){
     int c = currentPanel.getComponentCount();
     String query = " query " + funName + "(";
     int params = 0;
     for(int i=0;i<c;i++){
         Component p = currentPanel.getComponent(i);
         if(p instanceof ParamPanel){
            ParamPanel pp = (ParamPanel) p;
            if(!pp.checkContent()){
                Reporter.showInfo("Invalid content for parameter " + pp.getName());
                return;
            }
            if(params>0){
               query += ", ";
            }
            query += pp.getParamString();
            params++;
         }
     } 
     query += ")";
     if(!VC.execUserCommand(query)){
         Reporter.showInfo("problem in query found");
     }
  }

  /** Scans the list of objects for functions and inserts there 
    * names into the corresponding combo box.
    **/
  private void retrieveFunctions(){
     ListExpr olist = objectList;
     while(!olist.isEmpty()){
         ListExpr o = olist.first();
         ListExpr name =  o.second();
         ListExpr type = o.fourth();
         if(type.listLength()>0 ){
           ListExpr mt = type.first();
           if(mt.listLength()>2){
              mt = mt.first();
              if(mt.atomType()==ListExpr.SYMBOL_ATOM && mt.symbolValue().equals("map")){
                 String n = name.toString();
                 functions.addItem(n); 
              }
           } 
         }
         olist=olist.rest();
     }
     functions.setSelectedIndex(0); 
  }

 /** Calls validate recursively. 
   **/
 public static void validate(Container c){
      c.invalidate();
      Component[] childs = c.getComponents();
      for(int i=0;i<childs.length;i++){
          Component cs = childs[i];
          if(cs instanceof Container){
              Container csc = (Container) cs;
              validate(csc); 
          } else {
             cs.invalidate();
             cs.validate();
         }    
      }    
      c.validate();
  }

  private void setEmpty(){
       paramPanel.remove(currentPanel);
       paramPanel.add(emptyPanel);
       currentPanel = emptyPanel;
       validate(this);;
       repaint();
  }

  /** creates a panel holding the parameters for the selected function. **/ 
  private void buildPanel(){
     Object fno = functions.getSelectedItem();
     if(fno==null || fno.toString().trim().equals("none")){
       setEmpty();
       return;
     }
     funName = functions.getSelectedItem().toString().trim();
     ListExpr map = VC.getCommandResult("query " + funName);
     if(map==null){
        Reporter.showError("Querying " + funName + " failed");
        setEmpty();
        return;
     } 
     if(map.listLength()!=2){
        Reporter.showError("Type of " + funName + " seems to be no function, may the the database has been changes, Try an update.");
        setEmpty();
        return;
     }
     ListExpr type = map.first();
     ListExpr fun = map.second();
     if(type.listLength()<2 || !isSymbol(type.first(),"map")){
        Reporter.showError("Type of " + funName + " seems to be no function, may the the database has been changes, Try an update.");
        setEmpty();
        return;
     }
     if(fun.listLength()<2 || !isSymbol(fun.first(),"fun")){
        Reporter.showError("Type of " + funName + " seems to be no function, may the the database has been changes, Try an update.");
        setEmpty();
        return;
     }
     JPanel completePanel = new JPanel();
     completePanel.setLayout(new BoxLayout(completePanel,BoxLayout.Y_AXIS));
     completePanel.setBorder(new javax.swing.border.TitledBorder("Function: "+ funName));

     functions.setToolTipText(null);
     if(funDescMap!=null ){
         String d = funDescMap.get(funName);
         if(d!=null){
            functions.setToolTipText(d);
         }
     }

     fun = fun.rest(); // ignore fun keyword
     int len = fun.listLength();
     for(int i=0;i<len-1; i++){ // the last element is the return type of the function
         ListExpr param = fun.first();
         ParamPanel  p = createParamPanel(param, funName);
         if(p!=null){
            completePanel.add(p);
         }    
         fun = fun.rest();
     }
     JButton queryBtn = new JButton("send query");
     queryBtn.addActionListener(theActionListener);
     completePanel.add(queryBtn);
     paramPanel.remove(currentPanel);
     paramPanel.add(completePanel);
     currentPanel = completePanel;
     validate(this);
     repaint();
  }


 
  public boolean addObject(SecondoObject o){ // this viewer display no objects
      return false;
  } 

  public boolean isDisplayed(SecondoObject o){ return false; }

  public void removeAll(){}
  public void removeObject(SecondoObject o){}

  public boolean canDisplay(SecondoObject o){ return false; }

  public String getName(){ return "FunctionParam"; }

  public boolean selectObject(SecondoObject o){ return false; }


  public MenuVector getMenuVector(){
    return mv;
  }



  /** returns a panel for a single parameter. **/
  private ParamPanel createParamPanel(ListExpr param, String funName){
     if(param.listLength()!=2){
       return null;
     } else {
        return new ParamPanel(param.first(), param.second(), funName);
     }
  }

  /** Checks whether a ListExpr is a symbol with given content.**/
  public static boolean isSymbol(ListExpr list, String content){
       return list.atomType()==ListExpr.SYMBOL_ATOM && list.symbolValue().equals(content);
  }

 

  /** Class for a single parameter representation. **/
  private class ParamPanel extends JPanel{
    private String funName;
    private String name;           // name of the parameter
    private Type stype;            // special type treatment
    private ListExpr exactType;    // the exact type 

    private JLabel nameLabel;        // label for the parameter name
    private JLabel typeLabel;        // label for the parameter type
    private JTextField constField;   // textfield for input of const values
    private JComboBox dbObjects;     // combobox for selecting db objects or "const"
   
    /** Constructs a new parameter panel **/
    public ParamPanel(ListExpr name, ListExpr type, String funName){
       super();
       this.name = name.symbolValue();
       this.funName = funName;
       this.exactType = type;
       if(isSymbol(exactType,"string")){
           this.stype = Type.STRING;
       } else if(isSymbol(exactType,"int")){
           this.stype=Type.INT;
       } else if(isSymbol(exactType,"real")){
           this.stype=Type.REAL;
       } else if(isSymbol(exactType,"bool")){
           this.stype=Type.BOOL;
       } else if(isSymbol(exactType,"text")){
           this.stype=Type.TEXT;
       } else {
           this.stype = Type.GENERIC;
       }
       setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
       nameLabel = new JLabel(this.name+" :   ");
       constField = new JTextField(20);
       dbObjects = new JComboBox();

        typeLabel = new JLabel("  ( "+ mainType(exactType) + ")");

       add(nameLabel);
       add(constField);
       add(dbObjects);
       add(typeLabel);
       dbObjects.addItem("const");
       ListExpr olist =  FunctionParamViewer.this.objectList;
       while(!olist.isEmpty()){
          ListExpr o = olist.first();
          olist= olist.rest();
          ListExpr otype = o.fourth();
          if(otype.first().equals(exactType)){
             dbObjects.addItem(o.second().toString());
          }
       }
       dbObjects.addItemListener(new ItemListener(){
          public void itemStateChanged(ItemEvent evt){
              if(evt.getStateChange()==ItemEvent.SELECTED){
                 changeObject();
              }
          }
       });
       dbObjects.setSelectedIndex(0); 
       changeToolTips();
    }

    public void changeToolTips(){
       if(paramDescMap==null){
          nameLabel.setToolTipText(null);
          typeLabel.setToolTipText(null);
          return;
       }
       HashMap<String, String> funMap = paramDescMap.get(funName);
       if(funMap == null){
          return;
       }
       String d = funMap.get(name);
       nameLabel.setToolTipText(d);
       typeLabel.setToolTipText(d);
    }




    /** Called when the selected db objects has been changed. **/
    private void changeObject(){
        String s = dbObjects.getSelectedItem().toString();
        if(s.equals("const")){
           constField.setEnabled(true);
        } else {
           constField.setEnabled(false);
        }
    }

    /** returns the parameter for embedding in a query. **/
    public String getParamString() {
        String s = dbObjects.getSelectedItem().toString();
        if(!s.equals("const")){
           return s;
        }
        String c = constField.getText();
        switch(stype){
           case STRING : c = c.replaceAll("\\\"","\\\\\""); return "\""+c+"\"";
           case INT    : return c.trim(); 
           case REAL   : return c.trim();
           case BOOL   : return  c.toUpperCase().trim(); 
           case TEXT   : c = c.replaceAll("'","\\\\'"); return "'"+c+"'";
           default     :  return "[const " + exactType+" value " + c+ "]";
        }
    }

    /** checks whether the content is ok. **/
    public boolean checkContent(){
        if(!dbObjects.getSelectedItem().toString().equals("const")){
          return true;
        }
        String c = constField.getText();
        switch(stype){
           case STRING : return c.length() < 48;
           case INT    : try{Integer.parseInt(c.trim()); return true;} catch(Exception e){ return false; }
           case REAL   : try{Double.parseDouble(c.trim()); return true;} catch(Exception e){ return false; }
           case BOOL   : c = c.toUpperCase().trim(); return c.equals("TRUE") || c.equals("FALSE");
           case TEXT   : return true;
           default     : ListExpr l = new ListExpr();
                         return l.readFromString(c)==0; 
        }
    }

    /** extracts  the main type from a type description. **/
    String mainType(ListExpr l){
       while(l.atomType()!=ListExpr.SYMBOL_ATOM){
          l = l.first();
       }
       return l.toString();
    }
    
    /** Returns the name of this parameter. **/
    public String getName() {
          return name;
    }



  }
}

