
package  gui;

import  java.awt.*;
import  java.awt.event.*;
import  javax.swing.*;
import  javax.swing.text.*;
import  javax.swing.event.*;
import  java.util.*;
import  sj.lang.*;

/**
 * The command area is a component of the GUI. Here the user
 * can input his database commands and read the status messages of the 
 * program. This class is based upon the JFC JScrollPane so that it may 
 * be scrolled. It offers copy'n'paste ability with Ctrl-C, Ctrl-V.
 * Mouse selection is possible too.When releasing the button the 
 * selected text will be copied. Enter finishes the input.
 * @author  Thomas Höse
 * @version 0.99 1.1.02
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

  /**
   * The constructor sets up the internal textarea.
   * @param   ResultViewer Link for show results
   * @see <a href="CommandPanelsrc.html#CommandPanel">Source</a>
   */
  public CommandPanel (ResultProcessor aRV) {
    super();
    RV = aRV;
    Secondointerface = new ESInterface();
    SystemArea = new JTextArea();
    SystemArea.setLineWrap(true);
    SystemArea.setWrapStyleWord(true);
    SystemArea.addKeyListener(new ReturnKeyAdapter());
    SystemArea.addCaretListener(new BoundMoveListener());
    Keymap keymap = SystemArea.getKeymap();
    KeyStroke key = KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0);
    keymap.addActionForKeyStroke(key, keymap.getDefaultAction());
    setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    showPrompt();
    setViewportView(SystemArea);
    SystemArea.setFont(new Font("Monospaced",Font.PLAIN,18));
  }


  /** returns the preferredSize of this Component as
    * 3/4 width and 1/3 height of Parent
    */
  public Dimension getPreferredSize(){
     if(RV==null)
        return new Dimension(400,300);
     else{
        Dimension ParentSize = RV.getSize();
        int myWidth = (ParentSize.width * 3 ) / 4;
        int myHeight = ParentSize.height / 4;
        return new Dimension(myWidth,myHeight);
     }
  } 
   
  /** returns the connection state from secondointerface */
  public boolean isConnected(){
    return Secondointerface.isConnected();
  }
  

  /**
   * Add code to the end of the textarea.
   * @param txt Text to append.
   */
  public void appendText (String txt) {
    //SystemArea.setForeground(Color.black);	
    SystemArea.append(txt);
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
    appendText("\nSec>");
    aktPos = SystemArea.getText().length();
    SystemArea.setCaretPosition(aktPos);
  }


  /* delete all entrys in the history */
  public void clearHistory(){
    History = new Vector(50,10);
  } 

 //    **********  moved from MainWindow ****************/
 /**
   * This method allows to any class to command to this SecondoJava object to
   * execute a Secondo command, and this object will execute the Secondo command
   * The result is send to the current ResultProcessor.
   * 
   * @param command The user command
   */
  public void execUserCommand (String command) {
    command = command.trim();
    if (command.equals("")){
       showPrompt();
       return;
    }
    
    if(command.startsWith("gui") & RV!=null){
       RV.execGuiCommand(command.substring(4));
       return;
    }

    ListExpr displayErrorList;
    int displayErrorCode;
    ListExpr resultList = new ListExpr();
    int commandLevel = 0;
    IntByReference errorCode = new IntByReference(0);
    IntByReference errorPos = new IntByReference(0);
    StringBuffer errorMessage = new StringBuffer();
    // First send an "echo" to the system panel with the received command.
    appendText("\n" + command + "...");
    // Builds the data to send to the server.

    if (command.startsWith("(")) {
      // if command is a list representation, then the command level to use
      // is EXEC_COMMAND_LISTEXPR_SYNTAX.
      commandLevel = Secondointerface.EXEC_COMMAND_LISTEXPR_SYNTAX;
    } 
    else {
      // if command is not a list representation, then the command level to
      // use is EXEC_COMMAND_SOS_SYNTAX.
      commandLevel = Secondointerface.EXEC_COMMAND_SOS_SYNTAX;
    }

    // Executes the remote command.
    if(Secondointerface.isInitialized()){
         Secondointerface.secondo(command,           //Command to execute.
         ListExpr.theEmptyList(),                    // we don't use it here.
         commandLevel, true,         // command as text.
         false,      // result as ListExpr.
         resultList, errorCode, errorPos, errorMessage);
         RV.processResult(command,resultList,errorCode,errorPos,errorMessage);
    }
    else{
      appendText("\n you are not connected to SecondoServer \n");
      showPrompt();
    }
     
  }



  /** sends command to the SecondoServer the result is ignored
    * @return the ErrorCode from Server
    **/ 
  public int internCommand (String command) {
    command = command.trim();
    ListExpr displayErrorList;
    int displayErrorCode;
    ListExpr resultList = new ListExpr();
    int commandLevel = 0;
    IntByReference errorCode = new IntByReference(0);
    IntByReference errorPos = new IntByReference(0);
    StringBuffer errorMessage = new StringBuffer();

    if (command.startsWith("(")) {
      // if command is a list representation, then the command level to use
      // is EXEC_COMMAND_LISTEXPR_SYNTAX.
      commandLevel = Secondointerface.EXEC_COMMAND_LISTEXPR_SYNTAX;
    } 
    else {
      // if command is not a list representation, then the command level to
      // use is EXEC_COMMAND_SOS_SYNTAX.
      commandLevel = Secondointerface.EXEC_COMMAND_SOS_SYNTAX;
    }

    // Executes the remote command.
    Secondointerface.secondo(command,           //Command to execute.
                      ListExpr.theEmptyList(),                    // we don't use it here.
                      commandLevel, true,         // command as text.
                      false,      // result as ListExpr.
                      resultList, errorCode, errorPos, errorMessage);
    return errorCode.value;

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

  public void setConnection(String User,String PassWd,String Host,int Port){
    Secondointerface.setUserName(User);
    Secondointerface.setPassWd(PassWd);
    Secondointerface.setHostname(Host);
    Secondointerface.setPort(Port);
  }

  public boolean connect(){
    return Secondointerface.connect();
  } 

  public void disconnect(){
     Secondointerface.terminate();
  }


  class ReturnKeyAdapter extends KeyAdapter {
    int HistoryPos;
    /**
     * Scans for ENTER key
     * @param e Eventdata
     * @see <a href="CommandPanelsrc.html#keypressed">Source</a>
     */
    public void keyPressed (KeyEvent e) {
      String com = "";
      if (e.getKeyCode() == KeyEvent.VK_ENTER) {
        try {
          com = SystemArea.getText(aktPos, SystemArea.getText().length() - 
              aktPos);
        } catch (Exception ex) {}
        History.add(com);
        HistoryPos=History.size();
        execUserCommand(com);
        //SystemArea.append("\n"+com+"\n");
        //showPrompt();
      } 
      else if ((e.getKeyCode() == KeyEvent.VK_BACK_SPACE) && (SystemArea.getCaretPosition()
          == aktPos)) {
        SystemArea.append(" ");
      }
      int keyCode = e.getKeyCode();
      int qrs=History.size();
      if (qrs==0) return;
	if ((keyCode==KeyEvent.VK_DOWN) &&(HistoryPos <qrs)) HistoryPos++;
	else if ((keyCode==KeyEvent.VK_UP) &&(HistoryPos >0))	HistoryPos--;
	else return;	
	SystemArea.select(aktPos,SystemArea.getText().length());
	if (HistoryPos==qrs)SystemArea.replaceSelection("");
	else SystemArea.replaceSelection((String)History.elementAt(HistoryPos));

    }
    /*	public void keyReleased(KeyEvent e){
     if (e.getKeyCode()==KeyEvent.VK_ENTER){
     }		
     }*/
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
      //Get the location in the text.
      int dot = e.getDot();
      int mark = e.getMark();
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



