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

package  gui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.event.*;
import java.util.*;
import sj.lang.*;
import communication.optimizer.*;
import tools.Reporter;
import java.io.File;
import mmdb.MMDBUserInterfaceController;

/**
 * The command area is a component of the GUI. Here the user
 * can input his database commands and read the status messages of the
 * program. This class is based upon the JFC JScrollPane so that it may
 * be scrolled. It offers copy'n'paste ability with Ctrl-C, Ctrl-V.
 * Mouse selection is possible too.When releasing the button the
 * selected text will be copied. Enter finishes the input.
 * @author  Thomas Hoese
 * @version 0.99 1.1.02
 *
 * modified by Thomas Behr
 *
 */

public class CommandPanel extends JScrollPane {
  /**
  * The intern swing component for text output with the ability to scroll.
  */
  public JTextArea SystemArea;
  private ResultProcessor RV;
  private int aktPos;
  private Vector History=new Vector(50,10);
  private ESInterface Secondointerface;
  private ReturnKeyAdapter ReturnKeyListener;
  private Vector ChangeListeners = new Vector(3);
  private String OpenedDatabase = "";
  private OptimizerInterface OptInt = new OptimizerInterface();
  private OptimizerSettingsDialog OptSet = new OptimizerSettingsDialog(null);
  private Object SyncObj = new Object();
  private boolean ignoreCaretUpdate=false;
  private boolean autoUpdateCatalog = true;
  private boolean showRewrittenOptimizerQuery = false;



  public void setShowRewrittenOptimizerQuery(boolean on){
     showRewrittenOptimizerQuery = on;
  }


  private StoredQueriesDialog favouredQueries = new StoredQueriesDialog(null);

  /**
   * The constructor sets up the internal textarea.
   * @param   ResultViewer Link for show results
   */
  public CommandPanel (ResultProcessor aRV,String user, String passwd) {
    super();
    RV = aRV;
    Secondointerface = new ESInterface();
    Secondointerface.setUserName(user);
    Secondointerface.setPassWd(passwd);
    SystemArea = new JTextArea();
    SystemArea.setLineWrap(true);
    SystemArea.setWrapStyleWord(true);
    ReturnKeyListener = new ReturnKeyAdapter();
    SystemArea.addKeyListener(ReturnKeyListener);
    SystemArea.addCaretListener(new BoundMoveListener());
    Keymap keymap = SystemArea.getKeymap();
    KeyStroke key = KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0);
    keymap.addActionForKeyStroke(key, keymap.getDefaultAction());
    setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    appendText("Sec>"); // show the initially prompt
    aktPos = SystemArea.getText().length();
    SystemArea.setCaretPosition(aktPos);
    setViewportView(SystemArea);
    SystemArea.setFont(new Font("Monospaced",Font.PLAIN,18));

    // create a changelistener for autoupdatecatalog
    SecondoChangeListener autoUpdateListener = new SecondoChangeListener(){
        public void databasesChanged(){}
        // deleted or created or updated object
        public void objectsChanged(){
            if(sendToOptimizer("updateCatalog")==null){
               Reporter.writeError("updateCatalog failed");
            }
            //if(sendToOptimizer("closedb")==null){
            //   Reporter.writeError("close database failed"); 
            //} 
        }
        // deleted or create type
        public void typesChanged(){
            if(sendToOptimizer("updateCatalog")==null){
               Reporter.writeError("updateCatalog failed");
            } 
        }
         // a database is opened
        public void databaseOpened(String DBName){}
        // a database is closed
        public void databaseClosed(){}
        // the connection is opened
        public void connectionOpened(){}
         // the connection is closed
        public void connectionClosed(){}
        
    };
    addSecondoChangeListener(autoUpdateListener);

  }

  /** adds a new MessageListener **/
  public void addMessageListener(MessageListener ml){
    Secondointerface.addMessageListener(ml);
  }

  public boolean setHeartbeat(int heart1, int heart2){
    return Secondointerface.setHeartbeat(heart1, heart2);
  }

  public void setAutoUpdateCatalog(boolean auc){
     autoUpdateCatalog = auc;
  }


  /** Reopends the currently opened database.
   **/
  private boolean reopenDatabase(){
    //System.out.println("reopen database called");
    if(OpenedDatabase.length()==0){
      return false;
    }
    String db = OpenedDatabase;
    if(!execUserCommand("close database")){
        return false;
    }
    return execUserCommand("open database " + db);
  }


   /* retrieves the name of the currently open database
    * directly from Secondo. If Secondo is not available or 
    * no database is open, null is returned.
    */
   public String retrieveDBName(){
      if(!Secondointerface.isInitialized()){
         return null;
      }
      ListExpr resultList = new ListExpr();
      IntByReference errorCode = new IntByReference(0);
      IntByReference errorPos = new IntByReference(0);
      StringBuffer errorMessage = new StringBuffer();
      Secondointerface.secondo("query getDatabaseName()",
                                   resultList,
                                   errorCode, errorPos, errorMessage);
       if(errorCode.value!=0){
          return null;
       }
       if(resultList.listLength()!=2){
          return null;
       }
       if(resultList.second().atomType() != ListExpr.STRING_ATOM){
          return null;
       }  
       String name = resultList.second().stringValue().trim();
       return name.length()==0?null:name;  

   } 




  /** sets the connection properties for the optimizer server
    * the changes holds for the next connection with a optimizer
    */
  public void setOptimizer(String Host,int Port){
     OptSet.setConnection(Host,Port);
     OptInt.setHost(Host);
     OptInt.setPort(Port);
  }

  /** Disconnects and connects to the optimizer if already connected **/
  public void reconnectOptimizer(){
     if(OptInt.isConnected()){
         OptInt.disconnect();
         OptInt.connect(); 
     }
  }



  /** set a new FontSize for this CommanPanel;
    * the Size should be in [6,50]
    * if the Size is not in this invervall then Size is
    * fir to this inteval
    */
  public void setFontSize(int Size){
   if(Size<6) Size=6;
   if(Size>50) Size=50;
   SystemArea.setFont(new Font("Monospaced",Font.PLAIN,Size));
   SystemArea.repaint();
  }

  /* get the actual Fontsize */
  public int getFontSize(){
    return SystemArea.getFont().getSize();
  }


  /** returns the preferredSize of this Component as
    * 3/4 width and 1/3 height of Parent
    */
  public Dimension getPreferredSize(){
     if(RV==null)
        return new Dimension(600,300);
     else{
        Dimension ParentSize = RV.getSize();
        int myWidth = (ParentSize.width * 3 ) / 4;
        int myHeight = ParentSize.height / 4;
        return new Dimension(myWidth,myHeight);
     }
  }
  
  /** returns a small interface needed by the 'UpdateViewer'*/
  public UpdateInterface getUpdateInterface(){
	return Secondointerface;
  }

  /** returns the connection state from secondointerface */
  public boolean isConnected(){
    return Secondointerface.isConnected();
  }

  /** set the focus to the SystemArea */
  public void requestFocus(){
     SystemArea.requestFocus();
  }

  /** adds a SecondoChangeListener */
  public void addSecondoChangeListener(SecondoChangeListener SCL){
    if(SCL==null) return;
    if(!ChangeListeners.contains(SCL))
       ChangeListeners.add(SCL);
  }

  /** removes a SecondoChangeListener */
  public void removeSecondoChangeListener(SecondoChangeListener SCL){
    ChangeListeners.remove(SCL);
  }

  /**
   * Add code to the end of the textarea.
   * @param txt Text to append.
   */
  public void appendText (String txt) {
    SystemArea.append(txt);
    if(tools.Environment.TESTMODE != tools.Environment.NO_TESTMODE ){
      Reporter.writeInfo(txt);
    }
  }

  /**
   * Formats the output so that it can be recognized at error.
   * @param txt Errortext to add.
   * @see <a href="CommandPanelsrc.html#appendErr">Source</a>
   */
  public void appendErr (String txt) {
    SystemArea.append("*****" + txt);
  }

  /**
   * Simulate a prompt at the end of last light.
   * @see <a href="CommandPanelsrc.html#showPrompt">Source</a>
   */
  public void showPrompt () {
    //if(aktPos!=SystemArea.getText().length()){ // no prompt in the moment
       appendText("\nSec>");
       aktPos = SystemArea.getText().length();
       synchronized(SyncObj){
          SystemArea.setCaretPosition(aktPos);
       }
   // }
  }


  /* delete all entrys in the history */
  public void clearHistory(){
    History.clear();
  }

  /* use binary list for client server communication */
  public void useBinaryLists(boolean ubl){
    Secondointerface.useBinaryLists(ubl);
  }



  /** make clean the TextArea and the History */
  public void clear(){
     clearHistory();
     aktPos=0;
     SystemArea.setText("");
     appendText("Sec>"); // show the first prompt
     aktPos = SystemArea.getText().length();
     SystemArea.setCaretPosition(aktPos);;
  }

  private boolean isWhiteSpace(char c){
    return c==' ' || c=='\n' || c=='\t' || c=='\r' ;
  }

  private boolean isWordSep(char c){
    return isWhiteSpace(c) || c=='('  || c==')' || c=='[' || c==']' || c=='+'; // .... 
  }

  private boolean isSymbolStart(char c){
     return    (c>='a' && c<='z') 
            || (c>='A' && c<='Z')
            || (c=='_'); 
  }

  private boolean isSymbolElement(char c){
     return isSymbolStart(c) || (c>='0' && c<='9');
  }




  /** Changes the format of error messages coming from the optimizer.
    * The Error messages from the optimizer are not nice formatted.
    * For example line breaks are marked by \n. This is changed by this function.
  */
  private String formatOptimizerError(String errmsg){
    errmsg = errmsg.replaceAll("\\\\n","\n");
    errmsg = errmsg.replaceAll("\\\\t","\t");
    errmsg = errmsg.replaceAll("\\\\'","'");
    if(errmsg.startsWith("'") && errmsg.endsWith("'") && errmsg.length()>1){
       errmsg = errmsg.substring(1,errmsg.length()-1);
    }
    errmsg=errmsg.replaceAll("''","\"");
    return errmsg;
  }



  private char toLower(char c){
     if(c>='A' && c<='Z'){
        return (char)(c - 'A' + 'a'); 
     }
     return c;
  }

  private boolean isLetter(char c){
     return ((c>='A') && (c<='Z') ) || ((c>='a' && c<='z'));
  }

  private boolean isUpperCase(char c){
     return (c>='A') && (c<='Z');
  }


  private boolean isDigit(char c){
    return c>='0' && c <= '9';
  }

  private boolean isIdentChar(char c){
    return isLetter(c) || isDigit(c) || (c == '_');
  }

  /*
   Changes the first letter of symbols outside quotes starting with an upper case 
   to lower case. Words insie quotes or words starting with a lower case
   are keept as there are.

  */

  private static char getClosing(char bracket){
     if(bracket=='(') return ')';
     if(bracket=='{') return '}';
     if(bracket=='[') return ']';
     return bracket;
  }


  private boolean checkBrackets(String str, StringBuffer errMsg){
    Stack<Character> stack = new Stack<Character>();
    int state = 0; // 0 : normal, 1 : double quoted string, 2: single quoted string
    int line = 1;
    int pos = 0; // pos within line
    for(int i=0;i<str.length();i++){
       char c =  str.charAt(i);
       pos++;
       if(c=='\n'){
         line++;
         pos = 0;
       } 
       switch(state){
         case 0 : 
           if(c == '"'){
             state = 1;
           } else if (c== '\''){
             state = 2;
           } else if ( c == '(' || c == '{' || c == '['){
             stack.push(getClosing(c));
           } else if ( c == ')' || c == '}' || c == ']'){
             if(stack.empty()){
                errMsg.append("In line " +line + " at position " + pos +
                              " is a closing bracket '" + c + "' that is not " +
                              "opened before");
                return false;
             }
             char t = stack.pop().charValue();
             if(t!=c){
                errMsg.append("In line " + line + " at position " + pos +
                         " a '" + c + "' is found but '"+t + " is expected");
                return false;
             } 
           }
           break;
         case 1 : {
             if( c == '"'){
                state = 0;
             }
           }
           break;
         case 2 : {
             if( c == '\''){
                state = 0;
             }
           }
           break;
       }
    }
    if(!stack.empty()){
       errMsg.append("There are unclosed brackets");
       return false;
    }
    if(state != 0){
      errMsg.append("Unclosed string");
      return false;
    }
    return true;
  }


  private  String[] rewriteForOptimizer(String str){

    String prefix ="query ";
    String suffix="";
    if(str.matches("let +[a-z,A-Z][a-z,A-Z,0-9,_]*\\s*=.*")){
       int ass = str.indexOf('=');
       prefix = str.substring(0,ass); // without =
       str = str.substring(ass+1,str.length());
       String[] cmdVar = prefix.split("\\s+");
       prefix = "let "+cmdVar[1]+" = ";
    }

    String res = varToLowerCase(str);

    res = replacePoint(res);

    res = res ;

    if(showRewrittenOptimizerQuery){
      appendText("\nrewritten query: \n"+ res + "\n\n");
    }
    String[] r = {prefix,res,suffix};
    return r;
  }


  /** Replaces variables having format a.b by a:b.
   **/
  private String replacePoint( String str){

    // states
    // 0 start state
    // 1 within double quoted string
    // 2 within single quoted string
    // 3 within symbol

    StringBuffer buf = new StringBuffer();
    int state = 0;

    for(int i=0;i<str.length(); i++){
      char c = str.charAt(i);
      switch(state){
         case 0:
              if(c == '"'){
                 state = 1;
                 buf.append(c);
              } else if(c == '\''){
                 state = 2;
                 buf.append(c);
              } else if(isLetter(c)){
                 state = 3;
                 buf.append(c);
              } else {
                 buf.append(c);
              }
              break;

          case 1:
              if( c == '"'){
                state = 0;
              } 
              buf.append(c);
              break;

          case 2:
              if(c == '\''){
                 state = 0;
              }         
              buf.append(c);
              break;

         case 3:
              if(isLetter(c) || isDigit(c) || c == '_'){
                 buf.append(c);
              } else if(c == '"'){
                 state = 1;
                 buf.append(c);
              } else if(c == '\''){
                 state = 2;
                 buf.append(c);
              } else if( c == '.'){
                  if((i < str.length()-1) && isLetter(str.charAt(i+1))){
                     buf.append(':');
                  } else {
                    buf.append(c);
                  }
                  state = 0;
              } else {
                 state = 0;
                 buf.append(c);
              }
              break;
          default: System.err.println("invalid state reached");
      }
    }
    return buf.toString();
  }



  private String varToLowerCase(String str) {
     StringBuffer buf = new StringBuffer();
     int state = 0; //normal = 0, inDoublequotes = 1 in quotes = 2
     int pos = 0;


     // the states are the following
     // state 0 : begin of a symbol or something other
     // state 1 : within a double quoted string
     // state 2 : within a single quoted text
     // state 3 : within a symbol starting with a capital, all chars are converted to lower case
     // state 4 : within a symbol starting with a lower case, all chars are keept

     for(int i=0;i<str.length();i++){
        char c = str.charAt(i);
        switch(state){
          case 0: { // normal, first character of a symbol 

             if(c=='"'){ // begin of a string constant
               state = 1;
               buf.append(c);
             } else if(c=='\''){ //begin of a text constant
               state = 2;
               buf.append(c); 
             } else if(isLetter(c)){ // start of a symbol
               if(isUpperCase(c)){
                 buf.append(toLower(c));
                 state = 3;
               } else {
                 buf.append(c);
                 state = 4;
               }

             } else {  // something other
                buf.append(c);
             }
             break;
           }
          case 1: { // string constant
            if(c=='"'){
               state = 0;
            }
            buf.append(c);
            break;
          }
          case 2: { // text constant
            if(c=='\''){
              state = 0;
            }
            buf.append(c);
          }
          break;
          case 3: { // within an indentifier, convert all
            if(isIdentChar(c)){
              //buf.append(toLower(c));
              buf.append(c);
            } else {
              buf.append(c);
              if(c=='"'){
                 state = 1;
              } else if(c=='\''){
                 state = 2;
              } else {
                 state = 0;
              }
            }
          }
          break;
          case 4: { // within an identifier, convert nothing
            if(isIdentChar(c)){
              buf.append(c);
            } else {
              buf.append(c);
              if(c=='"'){
                 state = 1;
              } else if(c=='\''){
                 state = 2;
              } else {
                 state = 0;
              }
            }
          }
          break;

          default : { 
             Object o = null;
             if(o.equals(null)){ // force a null pointer exception
                System.err.println("Bad idea");
             }
          }
        }
     }
     return buf.toString();
  }


  /** optimizes a command if optimizer is enabled */
  private String optimize(String command){

   // check for secondo command
   command = command.trim();

   // special kernel commands
   if(command.startsWith("(") || command.startsWith("{")) return command; // command in nl format or command sequence

   StringTokenizer st = new StringTokenizer(command,  " \t\n\r\f([{=.,;");
   if(!st.hasMoreTokens()){
     return command;
   }
   String start = st.nextToken();
   // check for kernel command
   String[] keywords = {"query","derive","type","kill","if","while","open","close","begin","commit", "abort",
                        "save", "restore","list"};
   // note "update", "create", and "delete"  can be both, part of the kernel and part of the optimizer
   for(int i=0;i<keywords.length;i++){
     if(keywords[i].equals(start)){
         return command;
     }
   }
   if(start.equals("let") && !command.matches("let +[a-z,A-Z][a-z,A-Z,0-9,_]* *= *select.*")){
     return command;
   }
   if(start.equals("update") && !command.matches("update +[a-z][a-z,A-Z,0-9,_]* *set.*")){
     return command;
   }
   if(start.equals("create") && !command.matches("create +table .*") && !command.matches("create +index .*")) {
     return command;
   }
   if(start.equals("delete") && !command.matches("delete +from.*")){
     return command;
   }

   // now is should be an optimizer command
   if(!useOptimizer()){ // error select clause found but no optimizer enabled
      appendText("optimizer not available");
      showPrompt();
      return "";
   }


   IntObj Err = new IntObj();

   boolean catChanged = false;

   // check whether the catalog is changed -> requires reopen of database (hotfix)
   if(command.matches("create +table .*")){
      catChanged = true;
   } else if(command.matches("create +index .*")){
      catChanged = true;
   } else if(command.startsWith("drop ")){
      catChanged = true;
   } 

   
   String[] pre_command_suf = rewriteForOptimizer(command);
   StringBuffer buf = new StringBuffer();
   command = pre_command_suf[1];

   if(!checkBrackets(command,buf)){
      appendText("\n\n"+buf.toString());
      showPrompt();
      return "";
   }

   //System.out.println("to " + command);
   if(OpenedDatabase.length()==0){
      appendText("\nno database open");
      showPrompt();
      return "";
   }
   String opt = OptInt.optimize_execute(command,OpenedDatabase,Err,false);
   if(Err.value!=ErrorCodes.NO_ERROR){  // error in optimization
      appendText("\nerror in optimization of this query");
      showPrompt();
      return "";
   }else if(opt.startsWith("::ERROR::")){
      appendText("\nproblem in optimization: \n");
      appendText(formatOptimizerError(opt.substring(9))+"\n");
      showPrompt();
      return "";
   } else if(catChanged){
        boolean ok = reopenDatabase();
        appendText("reopen database ");
        if(ok){
           appendText("successful \n");
        } else {
           appendText("failed \n");
        }
        showPrompt();
        return "";
   } else {
       return pre_command_suf[0] + opt + pre_command_suf[2];
   }
 }



 /**
   * This method allows to any class to command to this SecondoJava object to
   * execute a Secondo command, and this object will execute the Secondo command
   * The result is send to the current ResultProcessor.
   *
   * @param command The user command
   */
   public boolean execUserCommand(String command){
      return execUserCommand(command,false,false,0,true,null);
   }
  

  /** This functions executes a command and performs some test
    * if isTest is set to true.
    * @param command: the command to execute
    * @param isTest:  if set to true, make a test using the remaining parameters
    * @param success: expected success of this command
    * @param epsilon: precision for comparing lists
    * @param expectedResult: the result expected for this query
    **/
  public boolean execUserCommand (String command,
                                  boolean isTest,
                                  boolean success,
                                  double epsilon,
                                  boolean isAbsolute,
                                  ListExpr expectedResult) {
    // empty commands are successful 
 //   command = command.replaceAll("\n"," ").trim();
      command = command.trim();

    if (command.equals("")){
       showPrompt();
       if(isTest){
          return success;
       } else{
          return true;
       }
    }

    // process commands designates for the gui
    if(command.startsWith("gui ") & RV!=null){
       boolean res = RV.execGuiCommand(command.substring(4));
       if(!isTest){
          return res;
       } else{ // testMode
          if(res!=success){
            return false;
          } 
          if(!success){
            return true;
          }
          return expectedResult==null;
       }
    }

 		if (command.startsWith("mmdb ")) {
      addToHistory(command);
      try {
        MMDBUserInterfaceController.getInstance().processMMDBQuery(command, Secondointerface);
      } catch (Exception e) {
         appendText("\n\n  Unexpected Exception caught on MMDB query execution: "
                    + e.getMessage());
         showPrompt();
      }
      return true;
		}

  
    // command designates for the optimizer 
    boolean eval=false; 
    if((eval = command.startsWith(EvalString)) || command.startsWith(OptString)){
       if(!useOptimizer()){
          appendText("\noptimizer not available");
          showPrompt();
          if(!isTest){
             return false;
          } else{
             return !success;
          }
       }
       long starttime=0;
       if(tools.Environment.MEASURE_TIME)
          starttime = System.currentTimeMillis();

       int OptCommandLength = eval?EvalString.length():OptString.length();

       String answer = sendToOptimizer(command.substring(OptCommandLength));

       if(tools.Environment.MEASURE_TIME)
          Reporter.writeInfo("used time for optimizing: "+(System.currentTimeMillis()-starttime)+" ms");

       if(answer==null){
          appendText("\nerror in optimizer command");
           showPrompt();
           if(!isTest){
               return  false;
           } else{
               return !success;
           }
       }
       else{
         if(!eval){ 
             appendText("\n"+answer);
             showPrompt();
             if(!isTest){
                return true;
             }else{
                return success;
             }
         }else{ // execute the plan
             // remove the "VARNAME =  " from the answer
             int pos = answer.indexOf("=");
             if(pos >= 0){
                 answer = "query " + answer.substring(pos+1);
             }
             if(answer.startsWith(EvalString)){
                 appendText("\npossible infinite recursion detected");
                 appendText("\nsuppress execution of the optimized result");
                 appendText("\n the result is:\n"+answer);
                 showPrompt();
                 if(!isTest){
                     return false; 
                 } else{
                     return !success;
                 }
             } else{
                 appendText("\nevaluate the query:\n"+answer+"\n"); 
                 addToHistory(answer);
                 return execUserCommand(answer,isTest,success,epsilon,isAbsolute,expectedResult); 
             }
         }
       }
    }
   
    // normal command

    ListExpr displayErrorList;
    int displayErrorCode;
    ListExpr resultList = new ListExpr();
    IntByReference errorCode = new IntByReference(0);
    IntByReference errorPos = new IntByReference(0);
    StringBuffer errorMessage = new StringBuffer();
    // Builds the data to send to the server.

    // Executes the remote command.
    if(Secondointerface.isInitialized()){

         command = optimize(command);
         if(command.equals("")){
             if(!isTest){
               return false;
             } else{
                return !success;
             }
          }
          appendText("\n" + command + "...");
          long starttime=0;
          if(tools.Environment.MEASURE_TIME){
               starttime = System.currentTimeMillis();
          }

           Secondointerface.secondo(command,      
                                   resultList, 
                                   errorCode, 
                                   errorPos, 
                                   errorMessage);

           if(tools.Environment.MEASURE_TIME){
                 Reporter.writeInfo("used time for query: "+
                 (System.currentTimeMillis()-starttime)+" ms");
            }

            RV.processResult(command,resultList,errorCode,
                             errorPos,errorMessage);
           boolean succ = errorCode.value==0;
           String tcommand = command.trim();
           boolean isSequence = tcommand.startsWith("{") && tcommand.endsWith("}");
           if(succ || isSequence){
               informListeners(command);
            }
            else if(!Secondointerface.isConnected()){ // connection lost
               informListeners("disconnect");
            }
            if(!isTest){
               return succ;
            }else{ // testmode
               if(succ!=success){ // not the expected result
                  return false;
               }
               if(!success){ // this case was expected
                  return true;
               }
               if(expectedResult!=null){
                   // if the resultList is a single symbolAtom, it is interpreted as
                   // a database objects which has to be load from the currently open database
                   if(expectedResult.atomType()==ListExpr.SYMBOL_ATOM){
                        String resCommand = "query "+expectedResult.symbolValue();
                        ListExpr testResult = new ListExpr();
                        IntByReference testErrorCode = new IntByReference(0);
                        IntByReference testErrorPos = new IntByReference(0);
                        StringBuffer testErrorMessage = new StringBuffer();
                        Secondointerface.secondo(resCommand,testResult,testErrorCode,
                                                 testErrorPos,testErrorMessage);
                        if(testErrorCode.value!=0){
                           Reporter.writeError(  "can't load the expected testresult '"
                                               + expectedResult.symbolValue()
                                               + "' from the database \n"
                                               + "the error message is '"
                                               + testErrorMessage +"'");
                           return false; // test unsuccessful
                        } else { // replace the expected result by the list get from the database
                           expectedResult.destroy(); // not longer needed
                           expectedResult = testResult;
                        }
                   }

                   Reporter.writeInfo("compare expected result with actual result");
                   boolean res = resultList.equals(expectedResult,epsilon,isAbsolute);
                   
                   if(!res){
                      Reporter.writeError("failed comparison");   
                   }else {
                      Reporter.writeInfo("the resultlist is equal to the expected result");
                   }
                   return res;
               } else{
                   return true;
               }
            }
    }
    else{
      appendText("\n you are not connected to SecondoServer");
      showPrompt();
      if(!isTest){
         return false;
      } else{
         return !success;
      }
    }
  }

  /** Executes a command in sos syntax **/
  public boolean execCommand(String cmd, IntByReference errorCode, ListExpr resultList,
                             StringBuffer errorMessage){
        if(!Secondointerface.isInitialized()){
                errorMessage.append("You are not connected to a Secondo Server");
                return false;
        }
        cmd = optimize(cmd);
        IntByReference errorPos=new IntByReference();
        Secondointerface.secondo(cmd, resultList,errorCode,errorPos,errorMessage);
        return errorCode.value==0;
  }


  /** sends command to the SecondoServer the result is ignored
    * @return the ErrorCode from Server
    **/
  public int internCommand (String command) {
    command = optimize(command.trim());
    if(command.equals("")) return -1;
    ListExpr displayErrorList;
    int displayErrorCode;
    ListExpr resultList = new ListExpr();
    IntByReference errorCode = new IntByReference(0);
    IntByReference errorPos = new IntByReference(0);
    StringBuffer errorMessage = new StringBuffer();

    long starttime=0;
    if(tools.Environment.MEASURE_TIME)
        starttime = System.currentTimeMillis();

     // Executes the remote command.
    Secondointerface.secondo(command,           //Command to execute.
                             resultList,
                             errorCode, 
                             errorPos, 
                             errorMessage);
    if(tools.Environment.MEASURE_TIME){
       Reporter.writeInfo("used time for query: "+(System.currentTimeMillis()-starttime)+" ms");
    }

    int res = errorCode.value;
    String tcommand = command.trim();
    boolean  isSequence = tcommand.startsWith("{") && tcommand.endsWith("}");
    if(res==0 || isSequence)
       informListeners(command);
    else if(!Secondointerface.isConnected()) // connection lost
       informListeners("disconnect");
    return res;
  }


    /** sends command to the SecondoServer the result is ignored
      * returns the resultList from SecondoServer,
      * if an error occurs, null is returned
    **/
  public ListExpr getCommandResult (String command) {
    command = optimize(command.trim());
    if(command.equals("")) return ListExpr.theEmptyList();
    ListExpr displayErrorList;
    int displayErrorCode;
    ListExpr resultList = new ListExpr();
    IntByReference errorCode = new IntByReference(0);
    IntByReference errorPos = new IntByReference(0);
    StringBuffer errorMessage = new StringBuffer();

    long starttime=0;
    if(tools.Environment.MEASURE_TIME)
       starttime = System.currentTimeMillis();

    // Executes the remote command.
    Secondointerface.secondo(command,           //Command to execute.
                             resultList, 
                             errorCode, errorPos, errorMessage);
    if(tools.Environment.MEASURE_TIME){
       Reporter.writeInfo("used time for query: "+(System.currentTimeMillis()-starttime)+" ms");
    }
    String tcommand = command.trim();
    boolean isSequence = tcommand.startsWith("{") && tcommand.endsWith("}");
    if(errorCode.value!=0){
       if(isSequence){
         informListeners(command);
       }
       if(!Secondointerface.isConnected())
          informListeners("disconnect");
       return  null;
    }
    else{
       informListeners(command);
       return resultList;
    }
  }



  public String getHostName(){
     return Secondointerface.getHostname();
  }

  public int getPort(){
     return Secondointerface.getPort();
  }

  public String getUserName(){
     return Secondointerface.getUserName();
  }

  public String getPassWd(){
    return Secondointerface.getPassWd();
  }

  /** set the values for connection with SECONDO */
  public void setConnection(String User,String PassWd,String Host,int Port){
    Secondointerface.setUserName(User);
    Secondointerface.setPassWd(PassWd);
    Secondointerface.setHostname(Host);
    Secondointerface.setPort(Port);
  }

  /** connect the commandpanel to SECONDO */
  public boolean connect(){
    boolean ok = Secondointerface.connect();
    if(ok)
       informListeners("connect");
    return ok;
  }

  /** disconnect from Secondo */
  public void disconnect(){
     Secondointerface.terminate();
     informListeners("disconnect");
  }



  /** enables the optimizer
    * returns true if successful false otherwise
    */
  public boolean enableOptimizer(){
    if(!useOptimizer())
      return OptInt.connect();
    return true;
  }


  /** sends the given command to the optimizer
    * returns null if not successful
    */
  public String sendToOptimizer(String cmd){
     if(!OptInt.isConnected())
        return null;
     IntObj Err = new IntObj();
     String res = OptInt.optimize_execute(cmd,OpenedDatabase,Err,true);
     if(Err.value!=ErrorCodes.NO_ERROR)
        return null;
     else
        return  res;
  }

  /** disables the use of the optimizer */
  public void disableOptimizer(){
      if(useOptimizer()){
        IntObj err = new IntObj();
        OptInt.optimize_execute("secondo('close database')",OpenedDatabase, err, true);
      }
      OptInt.disconnect();
  }


  /** returns true if the opttimizer is enabled
    * if the optimizer was enabled but the connection is broken,
    * the function will return false
    */
  public boolean useOptimizer(){
     return OptInt.isConnected();
  }

  /** shows a dialog for making settings for optimizer
    */
  public void showOptimizerSettings(){
      if(OptSet.showDialog()){
         OptInt.setHost(OptSet.getHost());
         OptInt.setPort(OptSet.getPort());
     }
  }





  public String getOpenedDatabase(){
     return  OpenedDatabase;
  }


  /** returns the size if the history*/
  public int getHistorySize(){
     return History.size();
  }

  /** returns the entry on pos i in the history,
    * if index i dont exists then null is returned
    */
  public String getHistoryEntryAt(int i){
    if(i<0)
      return null;
    if(i>=History.size())
      return null;
    return (String)History.get(i);

  }

  public void addToHistory(String S){
    if(S!=null){
       boolean store = true;
       if(History.size()>0){
          String last = (String) History.get(History.size()-1);
          store = !last.equals(S);
       }

       if(store)
          History.add(S);
       ReturnKeyListener.HistoryPos=History.size();
    }
  }


  /** informs all SecondoChangeListeners about changes in Secondo */
  private void informListeners(String cmd){
    cmd = cmd.trim();



    if(cmd.startsWith("{") && cmd.endsWith("}")){

       for(int i=0;i<ChangeListeners.size();i++){
       SecondoChangeListener SCL = (SecondoChangeListener) ChangeListeners.get(i);
         // a sequence command, it's not clear what all happened during this command
         // may be some of the subcommands failed, other was successful
         String newDBName = retrieveDBName();
         if(OpenedDatabase!=null && OpenedDatabase.length()!=0){
            if(newDBName!=null){
                SCL.databaseOpened(newDBName);
                OpenedDatabase = newDBName;
            } else { // database was closed and is closed after that command sequence
               OpenedDatabase = "";
            }
         }  else{
            if(newDBName==null){ // no open database
                OpenedDatabase="";
                SCL.databaseClosed();
            } else if(!newDBName.equals(OpenedDatabase)){
                // another database was opened
                SCL.databaseClosed();
                OpenedDatabase = newDBName;
                SCL.databaseOpened(newDBName);
            } else { // the same database
                SCL.objectsChanged(); // may be some objects are deleted, inserted or modified
                SCL.databasesChanged(); // may be a database is created, removed
            }
         }
         
      }
      return;
   }


    if(cmd.startsWith("("))
       cmd = cmd.substring(1).trim();

    if(cmd.equals("")) return;
    for(int i=0;i<ChangeListeners.size();i++){
      SecondoChangeListener SCL = (SecondoChangeListener) ChangeListeners.get(i);
      if(cmd.indexOf(" type ")>=0||cmd.startsWith("type ")){
         SCL.typesChanged();
      }else
      if(cmd.indexOf(" database ")>=0 && (cmd.startsWith("create") || cmd.startsWith("delete")))
         SCL.databasesChanged();
      else
      if(cmd.indexOf(" database ")>=0 && cmd.startsWith("open")){
         int index = cmd.lastIndexOf(" ");
         String DBName = cmd.substring(index+1);
         SCL.databaseOpened(DBName);
         OpenedDatabase=DBName;
      } else if(cmd.startsWith("restore") && cmd.indexOf(" database ")>0){
         int index1 = cmd.indexOf("database")+9;
         int index2 = cmd.indexOf("from")-1;
         if(index1<0 | index2<0)
      	    return;
         String DBName = cmd.substring(index1,index2).trim();
         SCL.databaseOpened(DBName);
         OpenedDatabase=DBName;
      } else if(cmd.endsWith(" database") && cmd.startsWith("close")){
         OpenedDatabase="";
         SCL.databaseClosed();
      } else if(cmd.startsWith("create ") || cmd.startsWith("delete ") || cmd.startsWith("let ") ||
         cmd.startsWith("update "))
         SCL.objectsChanged();
      else if(cmd.equals("connect"))
          SCL.connectionOpened();
          if(cmd.equals("disconnect"))
             SCL.connectionClosed();
    }
  }


  /** Saves all favoured queries into a file **/
  public void saveQueries(File f){
    saveQueries(f.getAbsolutePath());
  }

  public void saveQueries(String f){
    if(! favouredQueries.saveToFile(f)){
        Reporter.showError("Cannot save favoured queries");
    } 
  }

  public void loadQueries(File f){
    loadQueries(f.getAbsolutePath());
  }

  public void loadQueries(String f){
    if(favouredQueries.readFromFile(f)>0){
     Reporter.showError("Errors in loading favoured queries");
    }
  }

  public String getLastCommand(){
    return (String)History.get(History.size()-1);
  }

  public void addLastQuery(){
    if(History.size()==0){
      Reporter.showError("no last query exists");
      return;
    }
    String name = JOptionPane.showInputDialog("Please enter a name for that query");
    if(name==null){
       return;
    }
    if(favouredQueries.contains(name)){
       if(JOptionPane.showConfirmDialog(this,"name already exists,\n overwrite command?")== JOptionPane.YES_OPTION){
          favouredQueries.remove(name);
          favouredQueries.add(name,getLastCommand());
       }
    } else {
       favouredQueries.add(name,getLastCommand());
    }
  }

  public void showQueries(){
    String q = favouredQueries.showQueries();
    if(q!=null){
      showPrompt();
      appendText(q);
    }
  }

  class ReturnKeyAdapter extends KeyAdapter {
    int HistoryPos;
    /**
     * Scans for ENTER key
     * @param e Eventdata
     * @see <a href="CommandPanelsrc.html#keypressed">Source</a>
     */
    public void keyPressed (KeyEvent e) {
       int keyCode = e.getKeyCode();
       int mod = e.getModifiersEx();
       if(keyCode==KeyEvent.VK_ENTER){
           if(gui.Environment.TTY_STYLED_COMMAND){
              processReturnInTTYMode(e);
           } else{
              processReturnInGuiMode(e);
           }
           return;
       }
        
       // other keys are processed in the same way
       Caret C = SystemArea.getCaret();
       int p1 = C.getDot();
       int p2 = C.getMark();
       int pos = SystemArea.getCaretPosition();
       // selected area crosses the begin of the new command

       if(p1<aktPos | p2<aktPos){
          if( (mod&KeyEvent.CTRL_DOWN_MASK)==0 ){ // no key allowed
              C.moveDot(aktPos);
              C.setDot(aktPos);
         }
       else {  // ctrl is pressed allow C and Control
            if(keyCode!=KeyEvent.VK_C & keyCode!=KeyEvent.VK_CONTROL){
              C.moveDot(aktPos);
              C.setDot(aktPos);
            }
         }
       }
       // try to go back over the prompt
       if( ( ( (mod&KeyEvent.CTRL_DOWN_MASK)!=0 & keyCode==KeyEvent.VK_H ) |
            ( keyCode==KeyEvent.VK_BACK_SPACE))
          && SystemArea.getCaretPosition()==aktPos){
        SystemArea.insert(" ",aktPos);

      }

      // avoid selection using keyboard
      if(((mod&KeyEvent.SHIFT_DOWN_MASK)!=0) & (keyCode==KeyEvent.VK_LEFT | keyCode==KeyEvent.VK_UP)){
         if(pos==aktPos){
             e.setKeyCode(0);
             return;
       	 }
      }
      // try to select crossing the prompt
      if(keyCode==KeyEvent.VK_HOME | keyCode==KeyEvent.VK_PAGE_UP){
         if((mod&KeyEvent.SHIFT_DOWN_MASK)!=0)
            C.moveDot(aktPos);
         else{
            C.setDot(aktPos);
         }
         e.setKeyCode(0);
         return;
      }
      // pressing home => go to the prompt 
      if(keyCode==KeyEvent.VK_HOME){
         SystemArea.setCaretPosition(aktPos);
         e.setKeyCode(0);
         return;
      }

      // pressing tab without any modifiers
      if((keyCode==KeyEvent.VK_TAB) &&  
         (mod&KeyEvent.SHIFT_DOWN_MASK)!=0){
            String query = SystemArea.getText().substring(aktPos);
            e.setKeyCode(0);
            if(query.length()==0){
               return;
            }            
            int pos1 = query.lastIndexOf(' ');
            if(pos1<0){
              pos1 = 0;
            }else{
              pos1++;
            }
            int pos2 = query.lastIndexOf('\t');
            if(pos2<0){
              pos2 = 0;
            } else {
              pos2++;
            }
            // handle more separators here
            String word = query.substring(Math.max(pos1,pos2));
            Vector ext = gui.Environment.getExtensions(word);
            int size = ext.size();
            if(size==0){ // no extension found 
               return;
            } 
            if(size==1){ // a unique extension found
               String complete = (String) ext.get(0);
               String rest = complete.substring(word.length());
               if(rest.length()==0){ // word is an entry
                 rest = " "; // insert a space
               }
               appendText(rest);
               return;
            }
            String common = commonprefix(ext);
            common = common.substring(word.length());
            if(common.length()>0){ // extend so far as possible
                appendText(common);
                return;
            } 
            // no common part found, show extensionlist
            String allExt = "";
            for(int i=0;i<ext.size();i++){
               allExt += ext.get(i)+"\n";
            } 
            Reporter.showInfo("possible extensions:\n"+allExt);
      }


      // do not allow selection using the keyboard
      if((mod&KeyEvent.SHIFT_DOWN_MASK)!=0){
         if(keyCode==KeyEvent.VK_DOWN || keyCode==KeyEvent.VK_PAGE_DOWN ||
            keyCode==KeyEvent.VK_UP || keyCode==KeyEvent.VK_PAGE_UP) {
             e.setKeyCode(0);
         }
         return;
      }


      // replace the current command by a history entry
      int qrs=History.size();
      if (qrs==0) return;
      if ((keyCode==KeyEvent.VK_DOWN) &&(HistoryPos <qrs)) HistoryPos++;
      else if ((keyCode==KeyEvent.VK_UP) &&(HistoryPos >0))	HistoryPos--;
      else return;

      SystemArea.select(aktPos,SystemArea.getText().length());
      if (HistoryPos==qrs){
          SystemArea.replaceSelection("");
      }
      else{
        synchronized(SyncObj){
          SystemArea.replaceSelection((String)History.elementAt(HistoryPos));
        }
        synchronized(SyncObj){ 
          int length = SystemArea.getDocument().getLength();
          SystemArea.setCaretPosition(length);
        }
      }
      e.setKeyCode(0);
 

    }

    private String commonprefix(Vector strings){
      Trie t = new Trie();
      for(int i=0;i<strings.size();i++){
         t.insert((String)strings.get(i));
      }
      return t.commonPrefix();
    }


    /** processes a key event e. 
      * Returns true if a command was executed 
      **/
    private boolean processReturnInTTYMode(KeyEvent e){
        int keyCode = e.getKeyCode();
        // only process the return key
        if(keyCode!=KeyEvent.VK_ENTER){
           return false;
        }
        int lastLine = SystemArea.getLineCount();
        int position = SystemArea.getCaretPosition();
        int currentLine=-1;
        try{
           currentLine = SystemArea.getLineOfOffset(position);
        }catch(Exception ex){
           Reporter.debug(ex);
        }
        if(currentLine!=lastLine-1){ // only process when command end in the last line
           // insert a newLine
           String text = SystemArea.getText();
           int cursor = SystemArea.getCaretPosition();
           text = text.substring(0,cursor)+"\n"+text.substring(cursor,text.length());
           ignoreCaretUpdate=true; 
           SystemArea.setText(text);
           SystemArea.setCaretPosition(cursor+1);
           ignoreCaretUpdate=false;
           e.setKeyCode(0);
           return false;
        }
        // get the command 
        String command = "";
        try{
            command = SystemArea.getText(aktPos,SystemArea.getText().length()-aktPos);
        }catch(Exception ex){
            Reporter.debug(ex);
        }
        boolean complete = false;
        if(command.trim().startsWith("gui")){
          complete=true;
        } else if(command.endsWith(";")){ // command finished
           command = command.substring(0,command.length()-1).trim();
           complete = true;
        } else if(command.endsWith("\n")){ // command ends with empty line
           command = command.substring(0,command.length()-1).trim();
           complete = true;
        }
        if(complete && command.length()>0) {
           History.add(command);     
           HistoryPos = History.size();
           execUserCommand(command); 
           return true;
        }else{
           appendText("\n");
        }
        return false;
    }

    private boolean processReturnInGuiMode(KeyEvent e){
      String com = "";
      int keyCode = e.getKeyCode();
      int mod = e.getModifiersEx();
      if (keyCode == KeyEvent.VK_ENTER ) {
        if((mod&KeyEvent.SHIFT_DOWN_MASK)!=0){
           // Note: Pressing the return key together with the 
           // shift key has no affect in the JTextArea. Unfortunately,
           // the setModifier method is deprecated since java 1.1.4
           // For this reasong, we have to insert a newline manually at
           // the place under the cursor
           String text = SystemArea.getText();
           int cursor = SystemArea.getCaretPosition();
           text = text.substring(0,cursor)+"\n"+text.substring(cursor,text.length());
           ignoreCaretUpdate=true; 
           SystemArea.setText(text);
           SystemArea.setCaretPosition(cursor+1);
           ignoreCaretUpdate=false;
           e.setKeyCode(0);
           return false;
        }
        try {
          com = SystemArea.getText(aktPos, SystemArea.getText().length() -
              aktPos);
          if(com.trim().length()>0){
             History.add(com);
             HistoryPos=History.size();
          }
          execUserCommand(com);
          return true;
          
        } catch (Exception ex) {}
       }
       return false;

    }
  

 }
 /** This class controls the caret-movement */
  class BoundMoveListener
      implements CaretListener {

    /**
     * Watches the changing of the caret cursor
     * @param e Eventdata
     * @see <a href="CommandPanelsrc.html#caretupdate">Source</a>
     */
    public void caretUpdate (CaretEvent e) {
      if(ignoreCaretUpdate)
          return;
      synchronized(SyncObj){
	      //Get the location in the text.
	      int dot = e.getDot();
	      int mark = e.getMark();
	      int CPos = Math.min(Math.min(SystemArea.getCaretPosition(),dot),mark);
	      if (dot == mark) {        // no selection
		if (dot < aktPos)
		  SystemArea.setCaretPosition(aktPos);
	      }
	      else if (mark < aktPos) {
		appendText(SystemArea.getSelectedText());
		SystemArea.setCaretPosition(SystemArea.getText().length());
	      }
	    }
      }
  }

  class Interval{
     public Interval(int x, int y){
        min=x;
	max = y;
     }

     int min=0;
     int max=0;
  }

// define strings for special treatment when a command begins with it
private static final String OptString ="optimizer "; 
private static final String EvalString="eval ";




}





