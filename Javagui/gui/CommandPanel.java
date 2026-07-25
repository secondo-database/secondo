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
  // The optimizer runs inside the connected Secondo server, so two things
  // have to be true for it to be usable: the user asked for it
  // (optimizerWanted, from the configuration file or the Optimizer menu) and
  // the server we are currently connected to actually has one
  // (optimizerEnabled, established by probing that server). The wish outlives
  // a connection, the answer does not -- see connect().
  private boolean optimizerWanted = false;
  private boolean optimizerEnabled = false;
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
            updateCatalogIfWanted();
        }
        // deleted or create type
        public void typesChanged(){
            updateCatalogIfWanted();
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


  /** Lets the optimizer re-read the catalog after the set of objects or types
    * has changed, if the user asked for that.
    * Only sensible while the optimizer is enabled -- without that check every
    * object change would report a failed updateCatalog.
    */
  private void updateCatalogIfWanted(){
     if(!autoUpdateCatalog || !useOptimizer()){
        return;
     }
     if(sendToOptimizer("updateCatalog")==null){
        Reporter.writeError("updateCatalog failed");
     }
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


  /** Splits a command into at most maxTok lower cased tokens.
    * The delimiter set is shared with the TTY clients
    * (SecondoTTY.cpp, sqlTokens), so that "let r5=select ..." and
    * "create table t(a)" tokenize identically here and there.
    */
  private String[] sqlTokens(String command, int maxTok){
     StringTokenizer st = new StringTokenizer(command, " \t\n\r\f\u000b\b([{=.,;");
     Vector toks = new Vector();
     while(st.hasMoreTokens() && toks.size()<maxTok){
        toks.add(st.nextToken().toLowerCase());
     }
     return (String[]) toks.toArray(new String[toks.size()]);
  }


  /** Tells whether a command is written in the SQL dialect and therefore has
    * to be optimized by the server (command level 2) instead of being
    * interpreted by the kernel directly.
    * Deciding this is not a matter of the first keyword alone: "delete",
    * "create", "update" and "let" exist in both languages and are told apart
    * by the following token(s). Anything not recognized here falls through to
    * the kernel -- the fallback is the kernel in every client, so the same
    * typed command behaves the same in the GUI and in SecondoTTY/SecondoCS.
    * The rule set is the one of SecondoTTY.cpp, looksLikeSql.
    */
  private boolean isSqlCommand(String command){
     command = command.trim();
     // nested list command or command sequence: always the kernel
     if(command.startsWith("(") || command.startsWith("{")){
        return false;
     }
     String[] t = sqlTokens(command,3);
     if(t.length==0){
        return false;
     }
     // unambiguous openers
     if(   t[0].equals("sql") || t[0].equals("select") || t[0].equals("union")
        || t[0].equals("intersection") || t[0].equals("drop")){
        return true;
     }
     // ambiguous with kernel commands: need the second token
     if(t.length<2){
        return false;
     }
     if(t[0].equals("delete") && t[1].equals("from")) return true;
     if(t[0].equals("insert") && t[1].equals("into")) return true;
     if(t[0].equals("create") && (t[1].equals("table") || t[1].equals("index"))){
        return true;
     }
     // ... or the third
     if(t.length<3){
        return false;
     }
     if(t[0].equals("update") && t[2].equals("set")) return true;
     // "let <ident> = select|union|intersection ...": an SQL right hand side.
     // The server splits the prefix off and re-wraps the generated plan.
     if(t[0].equals("let") && (   t[2].equals("select") || t[2].equals("union")
                               || t[2].equals("intersection"))){
        return true;
     }
     return false;
  }


  /** Unwraps the answer of a level 2 (SQL) command, the list
    * (plan result costs), and returns the half to be rendered.
    * The generated plan is echoed when the user asked for it or when only the
    * plan was requested; the result half is then handed to the usual result
    * renderer, so an optimized query looks exactly like a kernel query.
    * For create/drop the optimizer carried the command out itself while
    * translating: the plan is the atom "done" and there is no result.
    */
  private ListExpr unwrapSqlAnswer(ListExpr answer, boolean planOnly){
     if(answer==null || answer.listLength()<2){
        return answer;   // not the expected shape, show it unchanged
     }
     ListExpr planExpr = answer.first();
     String plan = planExpr.atomType()==ListExpr.TEXT_ATOM
                     ? planExpr.textValue().trim()
                     : planExpr.toString().trim();
     if(plan.equals("done")){
        appendText("\nExecuted by the optimizer (no plan to run).");
        return ListExpr.theEmptyList();
     }
     if(showRewrittenOptimizerQuery || planOnly){
        appendText("\nOptimized plan: " + plan);
        // The costs were appended to the answer, so a server that does not
        // send them still works.
        if(answer.listLength()>=3 && answer.third().atomType()==ListExpr.REAL_ATOM){
           double costs = answer.third().realValue();
           if(costs>0.0){
              appendText("\nEstimated costs: " + costs);
           }
        }
     }
     if(planOnly){
        appendText("\nPlan only -- not executed.");
        return ListExpr.theEmptyList();
     }
     return answer.second();
  }


  /** Checks the preconditions for sending an SQL command and reports the
    * reason it cannot be sent. Returns true if it may be sent.
    */
  private boolean canSendSql(String command){
     if(!useOptimizer()){
        appendText("\noptimizer not available");
        showPrompt();
        return false;
     }
     if(OpenedDatabase.length()==0){
        appendText("\nno database open");
        showPrompt();
        return false;
     }
     StringBuffer buf = new StringBuffer();
     if(!checkBrackets(command,buf)){
        appendText("\n\n"+buf.toString());
        showPrompt();
        return false;
     }
     return true;
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

  
    // A command addressed to the optimizer explicitly. What follows the
    // prefix decides what it means, exactly as in the TTY clients:
    //   "optimizer <sql>"  -> optimize only, report the plan, execute nothing
    //   "optimizer <goal>" -> run an optimizer control directive
    if(command.startsWith(OptString)){
       if(!useOptimizer()){
          appendText("\noptimizer not available");
          showPrompt();
          if(!isTest){
             return false;
          } else{
             return !success;
          }
       }
       String optCommand = command.substring(OptString.length()).trim();

       if(isSqlCommand(optCommand)){
          return execServerCommand(optCommand,true,isTest,success,epsilon,
                                   isAbsolute,expectedResult);
       }

       long starttime=0;
       if(tools.Environment.MEASURE_TIME)
          starttime = System.currentTimeMillis();

       String answer = sendToOptimizer(optCommand);

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
       } else {
          appendText("\n"+answer);
          showPrompt();
          if(!isTest){
             return true;
          }else{
             return success;
          }
       }
    }

    return execServerCommand(command,false,isTest,success,epsilon,isAbsolute,
                             expectedResult);
  }


  /** Sends one command to the connected server and processes its result.
    * An SQL command (auto detected) is optimized by the server at command
    * level 2; anything else is sent unchanged at level 0/1. With planOnly the
    * server stops after optimizing, so the plan is reported and nothing runs.
    * @see #execUserCommand(String,boolean,boolean,double,boolean,ListExpr)
    */
  private boolean execServerCommand (String command,
                                     boolean planOnly,
                                     boolean isTest,
                                     boolean success,
                                     double epsilon,
                                     boolean isAbsolute,
                                     ListExpr expectedResult) {

    ListExpr displayErrorList;
    int displayErrorCode;
    ListExpr resultList = new ListExpr();
    IntByReference errorCode = new IntByReference(0);
    IntByReference errorPos = new IntByReference(0);
    StringBuffer errorMessage = new StringBuffer();
    // Builds the data to send to the server.

    // Executes the remote command.
    if(Secondointerface.isInitialized()){

         int commandLevel = SecondoInterface.DERIVE_COMMAND_LEVEL;
         if(isSqlCommand(command)){
            if(!canSendSql(command)){
               if(!isTest){
                 return false;
               } else{
                  return !success;
               }
            }
            commandLevel = 2;   // SQL dialect: the server optimizes it
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
                                   errorMessage,
                                   commandLevel,
                                   planOnly);

           if(commandLevel==2 && errorCode.value==0){
              // the answer is (plan result costs); render the result half
              resultList.setValueTo(unwrapSqlAnswer(resultList,planOnly));
           }

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
        IntByReference errorPos=new IntByReference();
        if(!sendCommandToServer(cmd,resultList,errorCode,errorPos,errorMessage)){
           return false;
        }
        return errorCode.value==0;
  }


  /** Sends a command to the server at the level it belongs to: an SQL command
    * (auto detected) is optimized by the server at command level 2, anything
    * else is sent unchanged. A level 2 answer is unwrapped to its result half,
    * so the caller sees the same shape as for a kernel command.
    * Returns false when an SQL command could not be sent at all; the reason
    * has then been written to the panel.
    */
  private boolean sendCommandToServer(String command,
                                      ListExpr resultList,
                                      IntByReference errorCode,
                                      IntByReference errorPos,
                                      StringBuffer errorMessage){
     boolean isSql = isSqlCommand(command);
     if(isSql && !canSendSql(command)){
        return false;
     }
     Secondointerface.secondo(command,resultList,errorCode,errorPos,errorMessage,
                              isSql?2:SecondoInterface.DERIVE_COMMAND_LEVEL,
                              false);
     if(isSql && errorCode.value==0){
        resultList.setValueTo(unwrapSqlAnswer(resultList,false));
     }
     return true;
  }


  /** sends command to the SecondoServer the result is ignored
    * @return the ErrorCode from Server
    **/
  public int internCommand (String command) {
    command = command.trim();
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
    if(!sendCommandToServer(command,resultList,errorCode,errorPos,errorMessage)){
       return -1;
    }
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
    command = command.trim();
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
    if(!sendCommandToServer(command,resultList,errorCode,errorPos,errorMessage)){
       return null;
    }
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
    // Whether the optimizer is available is a property of the server, so it
    // cannot be carried over from an earlier connection.
    optimizerEnabled = false;
    boolean ok = Secondointerface.connect();
    if(ok){
       if(optimizerWanted){
          optimizerEnabled = Secondointerface.optimizerAvailable();
          if(!optimizerEnabled){
             appendText("\nthe optimizer is not available on this server");
          }
       }
       informListeners("connect");
    }
    return ok;
  }

  /** disconnect from Secondo */
  public void disconnect(){
     // there is no optimizer without a server to run it; the user's wish
     // survives, so reconnecting brings it back where the server allows it
     optimizerEnabled = false;
     Secondointerface.terminate();
     informListeners("disconnect");
  }



  /** enables the optimizer
    * returns true if successful false otherwise
    */
  public boolean enableOptimizer(){
    optimizerWanted = true;
    if(!Secondointerface.isInitialized()){
       // Switched on from the configuration file before the connection is
       // made; connect() probes the server and decides then.
       return true;
    }
    if(!optimizerEnabled){
      // The optimizer belongs to the connected server; if that server has
      // none, it cannot be switched on here.
      if(!Secondointerface.optimizerAvailable()){
         Reporter.debug("the connected server provides no optimizer");
         return false;
      }
      optimizerEnabled = true;
    }
    return true;
  }


  /** runs the given optimizer control directive (a Prolog goal such as
    * "showOptions", "setOption(subqueries)", "updateCatalog") in the server's
    * optimizer and returns the text it printed
    * returns null if not successful
    */
  public String sendToOptimizer(String cmd){
     if(!useOptimizer())
        return null;
     return Secondointerface.optimizerCommand(cmd);
  }

  /** disables the use of the optimizer */
  public void disableOptimizer(){
      optimizerWanted = false;
      optimizerEnabled = false;
  }


  /** returns true if the optimizer is enabled */
  public boolean useOptimizer(){
     return optimizerEnabled;
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
         cmd.startsWith("update ") ||
         // SQL dialect: "drop table/index ..." and "insert into ..."
         cmd.startsWith("drop ") || cmd.startsWith("insert "))
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




}





