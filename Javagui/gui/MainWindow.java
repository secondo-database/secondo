package gui;

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import sj.lang.*;
import viewer.*;
import java.util.*;
import java.io.*;
import gui.idmanager.*;

public class MainWindow extends JFrame implements ResultProcessor,ViewerControl,SecondoChangeListener{

public final String CONFIGURATION_FILE="gui.cfg";
public final int MIN_FONTSIZE = 6;
public final int MAX_FONTSIZE = 24;

// shows additional informations if an error is occured
private boolean DEBUG_MODE = true;

private JPanel PanelTop;        // change to the desired components
private CommandPanel ComPanel;
private ObjectList OList;
private JPanel PanelTopRight;
private JSplitPane HSplitPane;
private JSplitPane VSplitPane;
private JOptionPane OptionPane;  // to Display Messages
private ServerDialog ServerDlg;
private Vector VCLs;


/* the current Viewer and all possible Viewers */
private SecondoViewer CurrentViewer;
private Vector AllViewers;
private Vector ViewerMenuItems;
private JFileChooser ViewerFileChooser;
private MenuVector CurrentMenuVector;

/* the Menubar with Menuitems */
private JMenuBar MainMenu;

private JMenu ProgramMenu;
private JMenuItem MI_FontSize_Console_Bigger;
private JMenuItem MI_FontSize_Console_Smaller;
private JMenuItem MI_FontSize_List_Bigger;
private JMenuItem MI_FontSize_List_Smaller;

private JMenuItem MI_ExecuteFile_HaltOnError;
private JMenuItem MI_ExecuteFile_IgnoreErrors;
private JMenuItem MI_SaveHistory;
private JMenuItem MI_ClearHistory;
private JMenuItem MI_ExtendHistory;
private JMenuItem MI_ReplaceHistory;
private JMenuItem MI_Close;

private JMenu ServerMenu;
private JMenuItem MI_Connect;
private JMenuItem MI_Disconnect;
private JMenuItem MI_Settings;

private JMenu Menu_ServerCommand;

private JMenu Menu_BasicCommands;
private JMenu Menu_Inquiries;
private JMenu Menu_Databases;
private JMenu Menu_Transactions;
private JMenu Menu_ImExport;


//Inquiries
private JMenuItem MI_ListDatabases;
private JMenuItem MI_ListTypes;
private JMenuItem MI_ListTypeConstructors;
private JMenuItem MI_ListObjects;
private JMenuItem MI_ListOperators;
private JMenuItem MI_ListAlgebras;
private JMenu AlgebraMenu;

// Databases
private JMenu OpenDatabaseMenu;
private JMenu DeleteDatabaseMenu;
private JMenuItem MI_UpdateDatabases;
private JMenuItem MI_CloseDatabase;
private JMenuItem MI_CreateDatabase;

// Transactions
private JMenuItem MI_BeginTransaction;
private JMenuItem MI_CommitTransaction;
private JMenuItem MI_AbortTransaction;

// Import Export
private JMenuItem MI_SaveDatabase;
private JMenuItem MI_RestoreDatabase;
private JMenuItem MI_SaveObject;
private JMenuItem MI_RestoreObject;

// Basic Commands
private JMenuItem MI_CreateType;
private JMenuItem MI_DeleteType;
private JMenuItem MI_CreateObject;
private JMenuItem MI_DeleteObject;
private JMenuItem MI_UpdateObject;
private JMenuItem MI_Let;
private JMenuItem MI_Query;


private JMenu HelpMenu;
private JMenuItem MI_ShowGuiCommands;
private JMenuItem MI_ShowSecondoCommands;
private HelpScreen MyHelp;


private JMenu Viewers;
private JMenuItem MI_ShowOnlyViewer;
private boolean onlyViewerShow = false;
private JMenuItem MI_AddViewer;
private Container DefaultContentPane;

private MenuListener BlendOutList; // a Menu cannot overlap a List ??


private String ObjectDirectory ="./"; // where search for Objects



private JFileChooser FC_History = new JFileChooser();
private JFileChooser FC_ExecuteFile = new JFileChooser();
private PriorityDialog PriorityDlg;



/* display a message in a new frame */
public void showMessage(String Text){
  OptionPane.showMessageDialog(this,Text);
}

/* return this */
public Frame getMainFrame(){ return this; }


/* creates a new MainWindow */
public MainWindow(String Title){
  super(Title);
  String StartScript=null;
  setSize(800,600);
  OptionPane = new JOptionPane();
  ServerDlg = new ServerDialog(this);
  MyHelp = new HelpScreen(this);
  this.getContentPane().setLayout(new BorderLayout());
  PanelTop = new JPanel(new BorderLayout(),true);
  ComPanel = new CommandPanel(this);
  ComPanel.addSecondoChangeListener(this);
  OList = new ObjectList(this,this);
  PanelTopRight = new JPanel();
  CurrentMenuVector = null;
  VCLs = new Vector(10);  // ViewerChangeListeners
  VSplitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT,true,
                              PanelTop,PanelTopRight);
  HSplitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,false,
                              ComPanel,OList);

  this.getContentPane().add(VSplitPane);     // add the splitpanes
  PanelTop.add(HSplitPane);


  ViewerFileChooser = new JFileChooser("."+File.separatorChar+"viewer");
  VSplitPane.setPreferredSize(new Dimension(600,400));
  VSplitPane.setDividerLocation(200);
  VSplitPane.setDividerSize(4);
  HSplitPane.setDividerLocation(500);
  HSplitPane.setDividerSize(4);

  addWindowListener(new WindowAdapter(){
       public void windowClosing(WindowEvent evt){
        ComPanel.disconnect();
        System.exit(0);
       }
  });

  PriorityDlg = new PriorityDialog(this);

  createMenuBar();
  CurrentViewer = null;
  ViewerMenuItems = new Vector(10);
  AllViewers = new Vector(10);
  DefaultContentPane = getContentPane();

  String UserName="";
  String PassWd="";
  String ServerName = "localhost";
  int ServerPort = 2550;
  boolean StartConnection = false;

  // try to read a configuration-File
  Properties Config = new Properties();
  File CF = new File(CONFIGURATION_FILE);
  boolean config_file_ok =true;
  if(!CF.exists()){
     System.err.println("Javagui: configuration file not found");
     config_file_ok = false;
  }

  try{
    FileInputStream CFG = new FileInputStream(CF);
    Config.load(CFG);
    CFG.close();
  } catch(Exception e){
    config_file_ok = false;
    if(DEBUG_MODE){
      System.err.println(e);
      e.printStackTrace();
    }
  }
 int maxStringLength=48;
 if(config_file_ok){
    String TMPServerName = Config.getProperty("SERVERNAME");
    if (TMPServerName==null)
      System.out.println("Servername not found in "+CONFIGURATION_FILE);
    else{
      ServerName = TMPServerName;
      System.out.println("set ServerName to "+ServerName);
    }

    String TMPServerPort = Config.getProperty("SERVERPORT");
    if(TMPServerPort==null)
       System.out.println("Serverport not found in "+CONFIGURATION_FILE);
    else{
       try{
          int PortInt = (new Integer(TMPServerPort)).intValue();
          if(PortInt <0)
            System.out.println("ServerPort in "+CONFIGURATION_FILE+" less than 0");
          else{
            System.out.println("set port to "+PortInt);
            ServerPort = PortInt;
          }
       }
        catch(Exception wrongport){
          System.out.println("error in ServerPort (not an Integer)");
        }
    }


    String Connection = Config.getProperty("START_CONNECTION");
    if(Connection==null)
       System.out.println("START_CONNECTION not found in "+CONFIGURATION_FILE);
    else{
       Connection=Connection.trim().toLowerCase();
       if(Connection.equals("true"))
           StartConnection = true;
       else if(Connection.equals("false"))
           StartConnection = false;
       else{
           System.out.println("START_CONNECTION has unknow value in "+CONFIGURATION_FILE);
           System.out.println("allowed values are  true  and false");
       }
    }


    String FontSize = Config.getProperty("COMMAND_FONTSIZE");
    if(FontSize==null)
       System.out.println("COMMAND_FONTSIZE NOT found in "+CONFIGURATION_FILE);
    else{
       try{
           int size = Integer.parseInt(FontSize.trim());
	   ComPanel.setFontSize(size);
       }
       catch(Exception e){
           System.out.println("COMMAND_FONTSIZE has no valid value (not an integer)");
       }

    }

    FontSize = Config.getProperty("LIST_FONTSIZE");
    if(FontSize==null)
        System.out.println("LIST_FONTSIZE not found in "+CONFIGURATION_FILE);
    else{
      try{
         int size = Integer.parseInt(FontSize.trim());
	 OList.setFontSize(size);
      }
      catch(Exception e){
         System.out.println("LIST_FONTSIZE has no valid value (not an integer)");
      }

    }

   String MaxStringLen = Config.getProperty("MAX_STRING_LENGTH");
   if(MaxStringLen!=null){
      try{
         int tmp = Integer.parseInt(MaxStringLen.trim());
	 if(tmp>0)
	    maxStringLength = tmp;
      } catch(Exception e){
         System.out.println("invalid value for MAX_STRING_LENGTH");
      }
   }

    String KnownViewers = Config.getProperty("KNOWN_VIEWERS");
    if(KnownViewers!=null){
        StringTokenizer View = new StringTokenizer(KnownViewers," ");
        Vector ViewerVector = new Vector(10);
        while(View.hasMoreTokens()){
            ViewerVector.add(View.nextToken());
        }
       String ViewerName;
        for(int i=0;i<ViewerVector.size();i++){
          ViewerName=(String) ViewerVector.get(i);
          String ClassName;
          try{
             if (ViewerName.startsWith("."))
                ClassName=ViewerName.substring(1);
             else
                ClassName="viewer."+ViewerName;
             Class VC = Class.forName(ClassName);
             Object Cand = VC.newInstance();
             if(Cand instanceof SecondoViewer){
                System.out.println("addViewer "+ViewerName);
                addViewer((SecondoViewer)Cand);
              }
             else
               System.out.println(ViewerName+" is not a SecondoViewer");
            }catch(Exception e){
            if(DEBUG_MODE)
               e.printStackTrace();
           System.out.println("cannot load viewer:"+ViewerName+"\n");
           }
        }
   }

   String Debug_Mode = Config.getProperty("DEBUG_MODE");
   if(Debug_Mode!=null){
      Debug_Mode = Debug_Mode.toLowerCase().trim();
      if(Debug_Mode.equals("false")){
         sj.lang.ListExpr.setDebugMode(false);
         DEBUG_MODE=false;
      }
      else{
         sj.lang.ListExpr.setDebugMode(true);
         DEBUG_MODE=true;
      }
   }


   String FS = System.getProperty("file.separator");
   if(FS==null){
      System.out.println("error in reading file separator");
      FS="/";
   }


   String SecondoHomeDir = Config.getProperty("SECONDO_HOME_DIR");
   String HistoryDirectory="";

   if(SecondoHomeDir==null){
     File F = new File("");
     String TMP = F.getAbsolutePath();
     TMP = TMP.substring(0,TMP.length()-8); // remove Javagui/
     F = new File(TMP);
     if(F.exists())
        SecondoHomeDir = F.getAbsolutePath();
   }

   String UseBinaryLists = Config.getProperty("USE_BINARY_LISTS");
   boolean use_binary_lists = false;
   if(UseBinaryLists!=null && UseBinaryLists.trim().toLowerCase().equals("true"))
      use_binary_lists = true;
   ComPanel.useBinaryLists(use_binary_lists);



   if(SecondoHomeDir!=null){
      ObjectDirectory = SecondoHomeDir+FS+"Data"+FS+"Guidatas"+FS+"gui"+FS+"objects";
      HistoryDirectory = SecondoHomeDir+FS+"Data"+FS+"Guidatas"+FS+"gui"+FS+"histories";
   }


   String TMPObjectDirectory= Config.getProperty("OBJECT_DIRECTORY");
   if(TMPObjectDirectory!=null){
      ObjectDirectory = TMPObjectDirectory.trim();
   }

   String TMPHistoryDirectory= Config.getProperty("HISTORY_DIRECTORY");
   if(TMPHistoryDirectory!=null)
        HistoryDirectory = TMPHistoryDirectory;


   System.out.println("set objectdirectory to "+ObjectDirectory);
   System.out.println("set historydirectory to "+ObjectDirectory);

   OList.setObjectDirectory(new File(ObjectDirectory));
   FC_History.setCurrentDirectory(new File(HistoryDirectory));


   StartScript = Config.getProperty("STARTSCRIPT");
  } // config -file readed




  ComPanel.setConnection(UserName,PassWd,ServerName,ServerPort);
  if (StartConnection){
      if (!ComPanel.connect())
         showMessage("I can't find a Secondo-server");
      else
         getServerInfos();
  }

  OList.setMaxStringLength(maxStringLength);
  ListExpr.setMaxStringLength(maxStringLength);

  if(StartScript!=null){
      StartScript = StartScript.trim();
      System.out.println("execute "+StartScript);
      if (StartScript.endsWith("-i")){
         StartScript = StartScript.substring(0,StartScript.length()-2).trim();
         executeFile(StartScript,true);
      }
      else
         executeFile(StartScript,false);
   }



   int fs = OList.getFontSize();
   if(fs<=MIN_FONTSIZE)
      MI_FontSize_List_Smaller.setEnabled(false);
   if(fs>=MAX_FONTSIZE)
      MI_FontSize_List_Bigger.setEnabled(false);

   fs = ComPanel.getFontSize();
   if(fs<=MIN_FONTSIZE)
      MI_FontSize_Console_Smaller.setEnabled(false);
   if(fs>=MAX_FONTSIZE)
      MI_FontSize_Console_Bigger.setEnabled(false);


}



public void addViewerChangeListener(ViewerChangeListener VCL){
  if(VCLs.indexOf(VCL)<0)
     VCLs.add(VCL);
}

public void removeViewerChangeListener(ViewerChangeListener VCL){
  VCLs.remove(VCL);
}


/** send a viewerChanged Message to all
  * registred ViewerChangeListener
  */
private void viewersChanged(){
  Object o;
  for(int i=0;i<VCLs.size();i++){
    o = VCLs.get(i);
    if(o!=null)
       ((ViewerChangeListener) o).viewerChanged();
  }
}

/** add a new Viewer*/
private void addViewer(SecondoViewer NewViewer){
   if (AllViewers.indexOf(NewViewer)<0){  // really a new Viewer
      AllViewers.add(NewViewer);
      JMenuItem MI_Viewer = new JMenuItem(NewViewer.getName());
      ViewerMenuItems.add(MI_Viewer);
      Viewers.insert(MI_Viewer,AllViewers.size()-1);
      NewViewer.setViewerControl(this);
      NewViewer.setDebugMode(DEBUG_MODE);
      MI_Viewer.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent e){
            int index = ViewerMenuItems.indexOf(e.getSource());
            if (index>=0)
                MainWindow.this.setViewerindex(index);
         }});
      PriorityDlg.addViewer(NewViewer);
      viewersChanged();
   }
   setViewer(NewViewer);
}


/** returns all loaded viewers */
public SecondoViewer[] getViewers(){
   SecondoViewer[] Viewers = new SecondoViewer[AllViewers.size()];
   for(int i=0;i<AllViewers.size();i++)
      Viewers[i] = (SecondoViewer) AllViewers.get(i);
   return Viewers;
}



/** removes the current Viewer and shows nothing
  * use it with setViewer
 **/
private void removeCurrentViewer(){
      cleanMenu();
      CurrentMenuVector = null;
      MainMenu.revalidate();
      if(onlyViewerShow)
        getContentPane().removeAll();
      else
        VSplitPane.setRightComponent(PanelTopRight);

}


/** set the current Viewer to AllViewers[index]); */
private void setViewerindex(int index){
 SecondoViewer SV;
 try {
     SV = (SecondoViewer)AllViewers.get(index);
     }
 catch(Exception e) {SV=null;}
 setViewer(SV);
}


/** set the current viewer to SV **/
private void setViewer(SecondoViewer SV){
 if (SV!=null) {
    removeCurrentViewer();
    if (onlyViewerShow){
       getContentPane().removeAll();
       getContentPane().add(SV);
    }
    else
       VSplitPane.setRightComponent(SV);
    // extend the Menu
    MenuVector MenuExtension = SV.getMenuVector();
    CurrentMenuVector = MenuExtension; 
    if (MenuExtension!=null)
       for(int i=0;i<MenuExtension.getSize();i++){
           MenuExtension.get(i).addMenuListener(BlendOutList);     
           MainMenu.add(MenuExtension.get(i));
       } 

    invalidate();
    validate();
    repaint();
    CurrentViewer = SV;
    SV.setViewerControl(this);
    CurrentViewer.revalidate();  
    MainMenu.revalidate();
    OList.updateMarks();
    setTitle("Secondo-GUI ("+SV.getName()+")");
 }
  MainMenu.revalidate(); 
}


/** executes a gui-command 
  *
  * don't forget to extend the HelpScreen if added a new command
  * 
  * available commands :
  * exit 
  * clearAll
  * addViewer <ViewerName>
  * selectViewer <ViewerName>
  * clearHistory
  * saveHistory
  * loadHistory [-r]
  * showObject <ObjectName>
  * hideObject <ObjectName>
  * removeObject <ObjectName>
  * clearObjectList
  * saveObject <ObjectName>
  * loadObject
  * setObjectDirectory <directory>
  * loadObjectFrom <FileName>
  * storeObject <ObjectName>
  * connect
  * disconnect
  * serverSettings
  * renameObject <oldName> -> <newName>
  * onlyViewer
  * listCommands
  * showAll
  * hideAll
  * executeFile [-i] <FileName>
  */
public boolean execGuiCommand(String command){
  ComPanel.appendText("\n"); 
  command=command.trim();
  boolean success=true;

  if(command.startsWith("exit")){
     setVisible(false);
     ComPanel.disconnect();
     System.exit(0);
  } else
  if(command.startsWith("addViewer")){
     String ViewerName = command.substring(9).trim();  // command without "addviewer"
     try{
        String ClassName;
        if (ViewerName.startsWith("."))
           ClassName=ViewerName.substring(1);
        else
           ClassName="viewer."+ViewerName;
        Class VC = Class.forName(ClassName);
        Object Cand = VC.newInstance();
        if(Cand instanceof SecondoViewer)
           addViewer((SecondoViewer)Cand);
        else{
           ComPanel.appendText("this is not a SecondoViewer");
           success=false;
        }
     }catch(Exception e){
        ComPanel.appendText("cannot load viewer:"+ViewerName+"\n");
        success=false;
        if(DEBUG_MODE)
           e.printStackTrace();
     }
     ComPanel.showPrompt();
  }
  else if(command.startsWith("selectViewer")) {
     String ViewerName=command.substring(12).trim();  // command without "selectViewer"
     boolean found=false;
     // check first for ViewerName
     int i=0;
     while(i<AllViewers.size() && !found)     
        if (((SecondoViewer) AllViewers.get(i)).getName().equals(ViewerName))
             found=true;
        else
            i++;

     if (found)
        setViewer((SecondoViewer)AllViewers.get(i));
     else{  // search the ClassName
        i=0;
        ViewerName = "viewer."+ViewerName;
        while(i<AllViewers.size() && !found)     
            if (((SecondoViewer) AllViewers.get(i)).getClass().getName().equals(ViewerName))
               found=true;
            else
               i++;
        if(found) 
            setViewer((SecondoViewer)AllViewers.get(i));
        else  {
             ComPanel.appendText("I can't find the Viewer \""+ViewerName+"\"\n");
             success=false;
        }     
     }
     ComPanel.showPrompt();
  } else if(command.startsWith("clearHistory")){
     ComPanel.clearHistory();
     ComPanel.showPrompt();
  } else if(command.startsWith("showObject")){
      if(OList.showObject(command.substring(10)))
         ComPanel.appendText("OK");
      else{
         ComPanel.appendText("ObjectName not found");
         success=false;
      }   
      ComPanel.showPrompt();
  } else if(command.startsWith("hideObject")){
      if(OList.hideObject(command.substring(10)))
         ComPanel.appendText("OK");
      else{
         ComPanel.appendText("ObjectName not found");
         success=false;    
      }   
      ComPanel.showPrompt();
  } else if(command.startsWith("removeObject")){
      if(OList.removeObject(command.substring(12)))
         ComPanel.appendText("OK");
      else{
         ComPanel.appendText("ObjectName not found");
         success=false;
      }   
      ComPanel.showPrompt();
  } else if(command.startsWith("clearObjectList")){
      ComPanel.appendText("OK");
      OList.clearList();
      ComPanel.showPrompt();
  } else if(command.startsWith("saveObject")){
      if(OList.saveObject(command.substring(10)))
         ComPanel.appendText("OK");
      else{
         ComPanel.appendText("ObjectName not found");
         success=false;
      }   
      ComPanel.showPrompt();
  } else if(command.startsWith("loadObject") & !command.startsWith("loadObjectFrom")){
      OList.loadObject();
      ComPanel.showPrompt();
  } else if(command.startsWith("storeObject")){
      if(OList.storeObject(command.substring(11)))
         ComPanel.appendText("OK");
      else{
         ComPanel.appendText("ObjectName not found");
         success=false;
      }   
      ComPanel.showPrompt();
  }
  else if(command.startsWith("connect")){
    if(!ComPanel.isConnected());
       if(ComPanel.connect()){
          ComPanel.appendText("you are connected to a secondo server");
	  getServerInfos();
       }
       else{
          ComPanel.appendText("i can't connect to secondo server (are the settings correct?)");
          success=false;
       }
    ComPanel.showPrompt();
  }
  else if(command.startsWith("disconnect")){
      ComPanel.disconnect();
      ComPanel.appendText("you are disconnected from secondo server");
      ComPanel.showPrompt();
  }
  else if(command.startsWith("serverSettings")){
     showServerSettings();
     ComPanel.showPrompt();

  }else if(command.startsWith("renameObject")){
     command = command.substring(12).trim(); //remove "rename"
     int pos = command.indexOf("->");
     if(pos <0){ // no correct input
        ComPanel.appendText("usage:  \"gui rename <oldname> -> <newName>\"");
        success=false;
     }
     else{
       String oldName = command.substring(0,pos).trim();
       String newName = command.substring(pos+2).trim();
       if(oldName.equals("") || newName.equals("")){
          ComPanel.appendText("usage:  \"gui rename <oldname> -> <newName>\"");
          success=false;
       }
       else{
         int EC = OList.renameObject(oldName,newName);
         ComPanel.appendText(OList.getErrorText(EC));
       }
     }
     ComPanel.showPrompt();
  } else if(command.startsWith("onlyViewer")){
     onlyViewerSwitch();
     ComPanel.showPrompt();
  } else if(command.startsWith("hideAll")){
    OList.hideAll();
    ComPanel.showPrompt();
  } else if(command.startsWith("showAll")){
    OList.showAll();
    ComPanel.showPrompt();
  } else if(command.startsWith("executeFile")){
    String crest = command.substring(11).trim();
    int errors = 0;
    if (crest.startsWith("-i"))
       errors = executeFile(crest.substring(2).trim(),true);
    else
       errors = executeFile(crest,false);
    if(errors>0){
       ComPanel.appendText("there are "+errors+" errors");
       success=false;
    }
    else
       ComPanel.appendText("executeFile successful");

    ComPanel.showPrompt();
  } else if(command.startsWith("setObjectDirectory")){
     String dir = command.substring(18).trim();
     String sep = System.getProperties().getProperty("file.separator");
     if(dir.endsWith(sep))
        dir = dir.substring(1,dir.length()-1);  // remove a fileseparator
     ObjectDirectory = dir;
     OList.setObjectDirectory(new File(dir));   
     ComPanel.appendText("ObjectDirectory ="+dir);
     ComPanel.showPrompt();
  } else if(command.startsWith("loadObjectFrom")){
     String sep = System.getProperties().getProperty("file.separator");
     String Name = command.substring(14).trim();
     if(!Name.startsWith(sep))
        if(ObjectDirectory.endsWith(sep))
          Name = ObjectDirectory+Name;
        else
          Name = ObjectDirectory+sep+Name;
     if(OList.loadObject(new File(Name)))
        ComPanel.appendText("Object loaded");
     else
        ComPanel.appendText("i can't load this object");
     ComPanel.showPrompt();   
  } else if(command.startsWith("saveHistory")){
     saveHistory();
  } else if(command.startsWith("loadHistory")){
     String Param = command.substring(11).trim();
     if(Param.equals(""))
        loadHistory(false);
     else if(Param.equals("-r"))
         loadHistory(true);
     else
         ComPanel.appendText("unknow parameter\n");
     ComPanel.showPrompt();    
  } else if(command.startsWith("clearAll")){
     clearAll();
     //ComPanel.showPrompt();
  }
  else {
    ComPanel.appendText("unknow gui command \n show help to get a list of available commands");
    ComPanel.showPrompt();
    success=false;
  }
  return success;
}


/** executes all commands in a file */
private int executeFile(String FileName,boolean ignoreErrors){
  BufferedReader BR = null;
  try{
     BR = new BufferedReader(new FileReader(FileName));
  }
  catch(Exception e){
    ComPanel.appendText("File \""+FileName+"\" not found\n");
    return 1;
  }
  int errors =0;
  try{
    String Line=BR.readLine();
    boolean ok=true;
    while(Line!=null & ok){
       Line = Line.trim();
       ComPanel.appendText(Line+"\n");
       if (!ComPanel.execUserCommand(Line)){
          errors++;
          if(!ignoreErrors)
             ok=false;
       }
       Line = BR.readLine();
    }
  
  }
  catch(Exception e){
    ComPanel.appendText("a IO error is occurred\n");
    if(DEBUG_MODE){
       System.out.println(e);
       e.printStackTrace();
    }   
    errors++;
  }
  finally{
    try{
      if(BR!=null)
        BR.close();
    }
    catch(Exception e){}
  }
  return errors;

}


/** make Menu appropriate to MenuVector from CurrentViewer */
public void updateMenu(){
   cleanMenu();
   CurrentMenuVector = null;
   // add the new Menu
   if (CurrentViewer!=null){  
     try{
        CurrentMenuVector = CurrentViewer.getMenuVector();
        if (CurrentMenuVector!=null)
           for(int i=0;i<CurrentMenuVector.getSize();i++){
              CurrentMenuVector.get(i).addMenuListener(BlendOutList);
              MainMenu.add(CurrentMenuVector.get(i));
           }
        CurrentViewer.revalidate();
        MainMenu.revalidate();
        OList.updateMarks();
     }
     catch(Exception e) {
        if(DEBUG_MODE)
           e.printStackTrace();
        showMessage("error when update the menu");
     }
   }  
}

/** if So exists in the Objectlist (identified by name) then 
  * the object is updated otherwise SO is inserted to
  * objectlist
  **/
public void updateObject(SecondoObject SO){
   OList.updateObject(SO);
}


/** adds a new Object to the ObjectList */
public boolean addObject(SecondoObject SO){
   OList.addEntry(SO);
   return true;
}

/* the main function to start program */
public static void main(String[] args){
  System.setErr(System.out);
  MainWindow SecGui = new MainWindow("Secondo-GUI");
  SecGui.setVisible(true);
  SecGui.ComPanel.requestFocus();
}


/* show the result of a command */
public void processResult(String command,ListExpr ResultList,IntByReference ErrorCode,
                        IntByReference ErrorPos,StringBuffer ErrorMessage){
  if (ErrorCode.value!=0){
    ComPanel.appendText("Error: " + ErrorMessage);
    ComPanel.appendText(ServerErrorCodes.getErrorMessageText(ErrorCode.value));
    if (ErrorCode.value==5){
       StringBuffer SB=new StringBuffer();
       if (ResultList.writeToString(SB)==0)
           ComPanel.appendText(""+SB);
    }
  }
  else
  {  ComPanel.appendText("successful  \n");
     if (ResultList.isEmpty()){
        ComPanel.appendText("no result");
     }
     else{
         SecondoObject o = new SecondoObject(IDManager.getNextID());
         o.setName(command);
         o.fromList(ResultList);
         OList.addEntry(o);
         if(CurrentViewer!=null){
            try{
              SecondoViewer SV = PriorityDlg.getBestViewer(CurrentViewer,o);
              if(SV==null)
                showMessage("no viewer found to display the result");
              else{
                  if(SV!=CurrentViewer)
                     setViewer(SV);
                  CurrentViewer.addObject(o);
                  OList.updateMarks();
               }
  
            } catch(Exception e){
               if(DEBUG_MODE)
                  e.printStackTrace();
               showMessage("an error is occurred (in current viewer)");
            }
         }
         ComPanel.appendText("see result in object list");
     }
  }
  ComPanel.showPrompt();
}


/** tests if the current Viewer can display SO **/
public boolean canActualDisplay(SecondoObject SO){
  if ((SO==null) || (CurrentViewer==null))
      return false;
  else
    try{return CurrentViewer.canDisplay(SO);}
    catch(Exception e){
       if(DEBUG_MODE)
          e.printStackTrace();
       System.out.println("error in method canDisplay in the current Viewer");
       return false;
    }
}

// tests if SO displayed in the current Viewer **/
public boolean isActualDisplayed(SecondoObject SO){
  if ((SO==null) || (CurrentViewer==null))
     return false;
  else
     try{return CurrentViewer.isDisplayed(SO);}
     catch(Exception e){ 
        if(DEBUG_MODE)
           e.printStackTrace();
        System.out.println("error in current Viewer "+CurrentViewer+" method: isDisplayed");
        return false;
     }
} 

/** shows SO in current Viewer if possible **/
public boolean showObject(SecondoObject SO){
  if (CurrentViewer==null) 
      return false;
  else
     try{
        if(CurrentViewer.canDisplay(SO))
           return CurrentViewer.addObject(SO);
        else{
          SecondoViewer TheBest = PriorityDlg.getBestViewer(CurrentViewer,SO);
          if(TheBest==null){
             showMessage("no Viewer found to display this object");
             return false;
          }
          else{
             setViewer(TheBest);
             return CurrentViewer.addObject(SO);
          }  
        }  
     }      
     catch(Exception e){
        System.out.println("error in Viewer :"+CurrentViewer+" method addObject");
        if(DEBUG_MODE)
           e.printStackTrace();
        return false;
     }
}


/** hide this Object in all Viewers and update the
  * marks in the ObjectList 
  */
public void hideObject(Object Sender,SecondoObject SO){
  if(Sender instanceof SecondoViewer)
     OList.updateMarks();
  else
     for(int i=0;i<AllViewers.size();i++){
        try{
          ((SecondoViewer)AllViewers.get(i)).removeObject(SO);
        }
        catch(Exception e){
           if(DEBUG_MODE)
              e.printStackTrace();
           System.out.println("an Exception is occured in removeObject-Method of a Viewer");
        }
     }
}


/** send the remove Command to ObjectList */
public void removeObject(SecondoObject SO){
  if(SO!=null)
    OList.removeObject(SO);
  OList.updateMarks();
}


/** select SO in Current Viewer if Sender is the ObjectList and vice versa */
public void selectObject(Object Sender,SecondoObject SO){
  if (Sender instanceof SecondoViewer){
     OList.selectObject(SO);
  }
  else{
     if (CurrentViewer!=null){
       try{
         CurrentViewer.selectObject(SO);
          }
       catch(Exception e){
          if(DEBUG_MODE)
            e.printStackTrace();
          showMessage("Exception in current viewer (method selectObject)");
       }
     }
  }
}


/** creates the standardmenubar   this means a
  * menubar without extensions by a viewer
  */
private void createMenuBar(){
   MainMenu = new JMenuBar();
   ProgramMenu = new JMenu("Program");
   MainMenu.add(ProgramMenu);

   JMenuItem MI_Clear = ProgramMenu.add("New ");
   MI_Clear.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
          clearAll();
      }
   });


   // create MenuItem for fontsize of console and object list
   JMenu MI_FontSize = new JMenu("FontSize");
   ProgramMenu.add(MI_FontSize);
   JMenu MI_FontSize_Console = new JMenu("Console");
   MI_FontSize_Console_Bigger = new JMenuItem("Bigger");
   MI_FontSize_Console_Smaller = new JMenuItem("Smaller");
   JMenu MI_FontSize_List = new JMenu("Object list");
   MI_FontSize_List_Bigger = new JMenuItem("Bigger");
   MI_FontSize_List_Smaller = new JMenuItem("Smaller");
   // create the hierarchy of this menuitems
   MI_FontSize.add(MI_FontSize_Console);
   MI_FontSize.add(MI_FontSize_List);
   MI_FontSize_List.add(MI_FontSize_List_Bigger);
   MI_FontSize_List.add(MI_FontSize_List_Smaller);
   MI_FontSize_Console.add(MI_FontSize_Console_Bigger);
   MI_FontSize_Console.add(MI_FontSize_Console_Smaller);
   // enable - disable Items

   ActionListener FontSizeAL = new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         Object src = evt.getSource();
	 int fs;
	 if(src.equals(MI_FontSize_Console_Bigger)){
	   fs = ComPanel.getFontSize()+2;
           ComPanel.setFontSize(fs);
	   if (fs>=MAX_FONTSIZE)
	       MI_FontSize_Console_Bigger.setEnabled(false);
	   MI_FontSize_Console_Smaller.setEnabled(true);
	 }

	 if(src.equals(MI_FontSize_Console_Smaller)){
             fs = ComPanel.getFontSize()-2;
	     ComPanel.setFontSize(fs);
	     if(fs<=MIN_FONTSIZE)
	        MI_FontSize_Console_Smaller.setEnabled(false);
	     MI_FontSize_Console_Bigger.setEnabled(true);
	 }

	 if(src.equals(MI_FontSize_List_Bigger)){
	    fs = OList.getFontSize()+2;
            OList.setFontSize(fs);
	    if (fs>=MAX_FONTSIZE)
	       MI_FontSize_List_Bigger.setEnabled(false);
	     MI_FontSize_List_Smaller.setEnabled(true);
          }

	 if(src.equals(MI_FontSize_List_Smaller)){
	     fs = OList.getFontSize()-2;
	     OList.setFontSize(fs);
	     if(fs<=MIN_FONTSIZE)
	        MI_FontSize_List_Smaller.setEnabled(false);
	     MI_FontSize_List_Bigger.setEnabled(true);
	 }
      }};

    MI_FontSize_Console_Bigger.addActionListener(FontSizeAL);
    MI_FontSize_Console_Smaller.addActionListener(FontSizeAL);
    MI_FontSize_List_Bigger.addActionListener(FontSizeAL);
    MI_FontSize_List_Smaller.addActionListener(FontSizeAL);


   JMenu MI_ExecuteFile = new JMenu("Execute file");
   ProgramMenu.add(MI_ExecuteFile);
   MI_ExecuteFile_HaltOnError = new JMenuItem("Halt on error");
   MI_ExecuteFile_IgnoreErrors = new JMenuItem("Ignore errors");
   MI_ExecuteFile.add(MI_ExecuteFile_HaltOnError);
   MI_ExecuteFile.add(MI_ExecuteFile_IgnoreErrors);

   ActionListener ExecuteListener= new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         if(FC_ExecuteFile.showSaveDialog(MainWindow.this)==JFileChooser.APPROVE_OPTION){
            Object Source = evt.getSource();
            if(Source.equals(MI_ExecuteFile_HaltOnError))
                executeFile(FC_ExecuteFile.getSelectedFile().getPath(),false);
            else
                executeFile(FC_ExecuteFile.getSelectedFile().getPath(),true);
         }
      }
   };

   MI_ExecuteFile_HaltOnError.addActionListener(ExecuteListener);
   MI_ExecuteFile_IgnoreErrors.addActionListener(ExecuteListener);


   JMenu HistoryMenu = new JMenu("History");
   ProgramMenu.add(HistoryMenu);
   MI_SaveHistory=HistoryMenu.add("Save history");
   MI_ClearHistory=HistoryMenu.add("Clear history");
   JMenu LoadHistoryMenu = new JMenu("Load");
   HistoryMenu.add(LoadHistoryMenu);
   MI_ExtendHistory = LoadHistoryMenu.add("Append");
   MI_ReplaceHistory = LoadHistoryMenu.add("Replace");


   ActionListener HistoryListener = new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          Object Source = evt.getSource();
          if(Source.equals(MI_ClearHistory))
              ComPanel.clearHistory();
          else if(Source.equals(MI_SaveHistory))
             saveHistory();
          else if(Source.equals(MI_ExtendHistory))
             loadHistory(false);
          else if(Source.equals(MI_ReplaceHistory))
             loadHistory(true);
       }
   };

   MI_SaveHistory.addActionListener(HistoryListener);
   MI_ClearHistory.addActionListener(HistoryListener);
   MI_ExtendHistory.addActionListener(HistoryListener);
   MI_ReplaceHistory.addActionListener(HistoryListener);


   MI_Close = ProgramMenu.add("Exit");
   MI_Close.addActionListener( new ActionListener(){
     public void actionPerformed(ActionEvent E){
        ComPanel.disconnect();
        System.exit(0);
     }});



   // Create the Server-Menu
   ServerMenu = new JMenu("Server");
   ServerMenu.addMenuListener(new MenuListener(){
     public void menuSelected(MenuEvent evt){
        if(ComPanel.isConnected()){
           MI_Connect.setEnabled(false);
           MI_Disconnect.setEnabled(true);
        }
        else{
           MI_Connect.setEnabled(true);
           MI_Disconnect.setEnabled(false);
        }
     }
     public void menuDeselected(MenuEvent evt){}
     public void menuCanceled(MenuEvent evt){}
   } );



   MainMenu.add(ServerMenu);
   MI_Connect = ServerMenu.add("Connect");
   MI_Disconnect = ServerMenu.add("Disconnect");
   MI_Settings = ServerMenu.add("Settings");

   MI_Connect.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          if(ComPanel.connect())
	     getServerInfos();
       }});
   MI_Disconnect.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          ComPanel.disconnect();
       }});

   MI_Settings.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
       showServerSettings();
    }});

   Menu_ServerCommand = new JMenu("Command");
   ServerMenu.add(Menu_ServerCommand);

   Menu_BasicCommands = new JMenu("Basic Commands");
   Menu_Inquiries = new JMenu("Inquiries");
   Menu_Databases = new JMenu("Databases");
   Menu_Transactions = new JMenu("Transactions");
   Menu_ImExport = new JMenu("Import / Export");

   Menu_ServerCommand.add(Menu_BasicCommands);
   Menu_ServerCommand.add(Menu_Inquiries);
   Menu_ServerCommand.add(Menu_Databases);
   Menu_ServerCommand.add(Menu_Transactions);
   Menu_ServerCommand.add(Menu_ImExport);


   // Inquiries
   MI_ListDatabases=Menu_Inquiries.add("list databases");
   MI_ListTypes = Menu_Inquiries.add("list types");
   MI_ListTypeConstructors = Menu_Inquiries.add("list type constructors");
   MI_ListObjects = Menu_Inquiries.add("list objects");
   MI_ListOperators = Menu_Inquiries.add("list operators");
   MI_ListAlgebras = Menu_Inquiries.add("list algebras");
   AlgebraMenu = new JMenu("list algebra");
   Menu_Inquiries.add(AlgebraMenu);

   // databases
   MI_UpdateDatabases = new JMenuItem("Update");
   Menu_Databases.add(MI_UpdateDatabases);
   MI_CreateDatabase = new JMenuItem("~create database~");
   Menu_Databases.add(MI_CreateDatabase);
   OpenDatabaseMenu = new JMenu("open database");
   Menu_Databases.add(OpenDatabaseMenu);
   MI_CloseDatabase = new JMenuItem("close database");
   Menu_Databases.add(MI_CloseDatabase);
   DeleteDatabaseMenu = new JMenu("delete database");
   Menu_Databases.add(DeleteDatabaseMenu);


   // Transactions
   MI_BeginTransaction = Menu_Transactions.add("begin");
   MI_AbortTransaction = Menu_Transactions.add("abort");
   MI_CommitTransaction = Menu_Transactions.add("commit");

   // Import Export
   MI_SaveDatabase = Menu_ImExport.add("~save database~");
   MI_RestoreDatabase = Menu_ImExport.add("~restore database~");
   MI_SaveObject = Menu_ImExport.add("~save object~");
   MI_RestoreObject = Menu_ImExport.add("~restore object~");

   // Basic Commands
   MI_CreateType = Menu_BasicCommands.add("~create type~");
   MI_DeleteType = Menu_BasicCommands.add("~delete type~");
   MI_CreateObject = Menu_BasicCommands.add("~create object~");
   MI_UpdateObject = Menu_BasicCommands.add("~update~");
   MI_DeleteObject = Menu_BasicCommands.add("~delete object~");
   MI_Let = Menu_BasicCommands.add("~let~");
   MI_Query = Menu_BasicCommands.add("~query~");


   Command_Listener Com_Listener = new Command_Listener();
   MI_ListDatabases.addActionListener(Com_Listener);
   MI_ListTypes.addActionListener(Com_Listener);
   MI_ListTypeConstructors.addActionListener(Com_Listener);
   MI_ListObjects.addActionListener(Com_Listener);
   MI_ListOperators.addActionListener(Com_Listener);
   MI_ListAlgebras.addActionListener(Com_Listener);
   MI_UpdateDatabases.addActionListener(Com_Listener);
   MI_CloseDatabase.addActionListener(Com_Listener);
   MI_BeginTransaction.addActionListener(Com_Listener);
   MI_AbortTransaction.addActionListener(Com_Listener);
   MI_CommitTransaction.addActionListener(Com_Listener);
   MI_CreateDatabase.addActionListener(Com_Listener);
   MI_SaveDatabase.addActionListener(Com_Listener);
   MI_RestoreDatabase.addActionListener(Com_Listener);
   MI_SaveObject.addActionListener(Com_Listener);
   MI_RestoreObject.addActionListener(Com_Listener);
   MI_CreateType.addActionListener(Com_Listener);
   MI_DeleteType.addActionListener(Com_Listener);
   MI_CreateObject.addActionListener(Com_Listener);
   MI_UpdateObject.addActionListener(Com_Listener);
   MI_DeleteObject.addActionListener(Com_Listener);
   MI_Let.addActionListener(Com_Listener);
   MI_Query.addActionListener(Com_Listener);

   HelpMenu = new JMenu("Help");
   MI_ShowGuiCommands=HelpMenu.add("Show gui commands");
   MI_ShowSecondoCommands = HelpMenu.add("Show secondo commands");
   MI_ShowGuiCommands.addActionListener(new ActionListener(){
           public void actionPerformed(ActionEvent evt){
               MyHelp.setMode(MyHelp.GUI_COMMANDS);
               MyHelp.setVisible(true);
           }
        });

   MI_ShowSecondoCommands.addActionListener(new ActionListener(){
           public void actionPerformed(ActionEvent evt){
               MyHelp.setMode(MyHelp.SECONDO_COMMANDS);
               MyHelp.setVisible(true);
           }
        });

   MainMenu.add(HelpMenu);

   Viewers = new JMenu("Viewers");
   Viewers.addSeparator();

   JMenuItem ViewerPriorities = Viewers.add("Set priorities");
   ViewerPriorities.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         PriorityDlg.setVisible(true);
      }
   });


   MI_ShowOnlyViewer = Viewers.add("Show only viewer");
   MI_ShowOnlyViewer.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           onlyViewerSwitch();
       }
   });


   MI_AddViewer = Viewers.add("Add Viewer");
   MI_AddViewer.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
         if (ViewerFileChooser.showOpenDialog(MainWindow.this)==JFileChooser.APPROVE_OPTION){
           String Name = ViewerFileChooser.getSelectedFile().getPath();
           // a Viewer must be in  "viewer" package and so in the viewer-directory
           int pos = Name.lastIndexOf("viewer");
           if (pos<0){
             showMessage("a viewer must be in the viewer directory");
           }
           else{
                Name = Name.substring(pos);
                char FileSep = File.separatorChar;
                Name = Name.replace(FileSep,'.');
                if (!Name.endsWith(".class")){
                   showMessage("this is no class file");
                }
                else{
                   Name= Name.substring(0,Name.length()-6);  // remove ".class" extension
                   try{
                        Class NViewerClass = Class.forName(Name);
                        Object O = NViewerClass.newInstance();
                        if (!(O instanceof SecondoViewer)){
					showMessage("the selected class is not an SecondoViewer");
                        }
                        else{  // is allright
                           addViewer((SecondoViewer)O);
                        }
                   }
                   catch(Exception e){
                        ComPanel.appendText(""+e);
                        ComPanel.showPrompt();
     				    showMessage("cannot load the given Viewer\n see commandPanel for details");
     				    if(DEBUG_MODE)
                           e.printStackTrace();

                   }
                }
           }
         }
     }
   });

   MainMenu.add(Viewers);
   setJMenuBar(MainMenu);
}


/** clear the History, the ObjectList and the Content of all Viewers */
public void clearAll(){
  ComPanel.clear();
  OList.clearList();
  for(int i=0;i<AllViewers.size();i++)
     ((SecondoViewer) AllViewers.get(i)).removeAll();
}



/** load a new History,
  * if replace is true, the old History is make clear before
  * load the new History
  */
public void loadHistory(boolean replace){

  if(FC_History.showOpenDialog(this)==JFileChooser.APPROVE_OPTION){
      // first try to load the file content
      boolean ok = true;
      Vector TMP=new Vector();
      BufferedReader BR=null;
      try{
        BR = new BufferedReader(new FileReader(FC_History.getSelectedFile()));
        String Line = BR.readLine();
        while(Line!=null){
           TMP.add(Line);
           Line = BR.readLine();
        }
      } catch(Exception e){
        ComPanel.appendText("load history failed \n");
        if(DEBUG_MODE)
           e.printStackTrace();
        ok = false;
      }
      finally{
        try{
          if(BR!=null)
             BR.close();
         }catch(Exception e){}
      }
      if(ok){
        if(replace)
           ComPanel.clearHistory();
        for(int i=0;i<TMP.size();i++)
           ComPanel.addToHistory((String)TMP.get(i));
      }
  }
}




/** switch to display
  * only the viewer or viewer commandpanel and objectlist */
public void onlyViewerSwitch(){
  if(onlyViewerShow){
     MI_ShowOnlyViewer.setText("Show only viewer");
     DefaultContentPane.removeAll();
     DefaultContentPane.add(VSplitPane);
     onlyViewerShow = false;
     setViewer(CurrentViewer);
     DefaultContentPane.validate();
   }
   else{
     if(CurrentViewer==null)
        showMessage("there is no viewer to show");
     else{
        MI_ShowOnlyViewer.setText("Show all");
        DefaultContentPane.removeAll();
        DefaultContentPane.add(CurrentViewer);
        DefaultContentPane.validate();
        onlyViewerShow = true;
     }
  }
}


/* returns the actual viewer */
public SecondoViewer getCurrentViewer(){
   return CurrentViewer;
 }


/* executes cmd and ignoring the result */
public int internCommand(String cmd){
   return ComPanel.internCommand(cmd);
}


/* show the ServerSetting dialog */
private void showServerSettings(){
       ServerDlg.set(ComPanel.getHostName(),ComPanel.getPort());
       ServerDlg.show();
       if (ServerDlg.getResultValue()==ServerDialog.OK){
          ComPanel.setConnection("","",ServerDlg.getHostName(),ServerDlg.getPortAddress());
          if (!ComPanel.connect()){
             showMessage("I can't find a SecondoServer ");
          }
	  else{
	    getServerInfos();
	  }
       }
}


/** open a FileChooser and save the current History to the
  * selected File
  */
private void saveHistory(){
    if(FC_History.showSaveDialog(this)==JFileChooser.APPROVE_OPTION){
       File F = FC_History.getSelectedFile();
       FileWriter FW =null;
       try{
          FW = new FileWriter(F);
          for(int i=0;i<ComPanel.getHistorySize();i++)
              FW.write(ComPanel.getHistoryEntryAt(i)+"\n");
       }
       catch(Exception e){
          if(DEBUG_MODE)
            e.printStackTrace();
          ComPanel.appendText("IO error");
       }
       finally{
         try{
          if(FW!=null)
             FW.close();
         }
         catch(Exception e2){}
       }

    }
}



/* cleans the MenuBar (MenuBar without Viewer-Extension */
private void cleanMenu(){
   MainMenu.removeAll();
   MainMenu.add(ProgramMenu);
   MainMenu.add(ServerMenu);
   MainMenu.add(HelpMenu);
   MainMenu.add(Viewers);
}


/* get infos from server
   - known algebras, databases
 */
private void getServerInfos(){

  ListExpr Algebras = ComPanel.getCommandResult("list algebras");
  AlgebraMenu.removeAll();
  if (Algebras==null){
     updateDatabases();
     return;
  }
  JMenuItem[] MI_Algebras= new JMenuItem[Algebras.listLength()];
  int index = 0;
  while(!Algebras.isEmpty()){
    String Name = Algebras.first().symbolValue();
    MI_Algebras[index] = new JMenuItem(Name);
    MI_Algebras[index].addActionListener(new ActionListener(){
                      public void actionPerformed(ActionEvent evt){
		         String cmd = "list algebra "+((JMenuItem)evt.getSource()).getText();
			 MainWindow.this.ComPanel.appendText(cmd);
			 MainWindow.this.ComPanel.addToHistory(cmd);
		         MainWindow.this.ComPanel.execUserCommand(cmd);
		      }});
    Algebras=Algebras.rest();
    index++;
  }
  for(int i=0;i<MI_Algebras.length;i++)
     AlgebraMenu.add(MI_Algebras[i]);

  updateDatabases();
}


/* includes all databases in the "open|delete databases" menu */
public boolean updateDatabases(){
  ListExpr Databases = ComPanel.getCommandResult("list databases");
  OpenDatabaseMenu.removeAll();
  DeleteDatabaseMenu.removeAll();
  if(Databases==null)
    return false;
  JMenuItem[] MI_OpenDatabases = new JMenuItem[Databases.listLength()];
  JMenuItem[] MI_DeleteDatabases = new JMenuItem[Databases.listLength()];
  int index = 0;
  while(!Databases.isEmpty()){
    String Name = Databases.first().symbolValue();
    MI_OpenDatabases[index] = new JMenuItem(Name);
    MI_OpenDatabases[index].addActionListener(new ActionListener(){
                           public void actionPerformed(ActionEvent evt){
			      String cmd = "open database "+((JMenuItem)evt.getSource()).getText();
			      MainWindow.this.ComPanel.appendText(cmd);
			      MainWindow.this.ComPanel.addToHistory(cmd);
			      MainWindow.this.ComPanel.execUserCommand(cmd);
			   }});
    MI_DeleteDatabases[index] = new JMenuItem(Name);
    MI_DeleteDatabases[index].addActionListener(new ActionListener(){
                           public void actionPerformed(ActionEvent evt){
			      String db = ((JMenuItem)evt.getSource()).getText();
			      String cmd = "delete database "+db;
			      int c = JOptionPane.showConfirmDialog(null,"really delete the database "+db+"?",
			                                           "Confirm",JOptionPane.YES_NO_OPTION);
			      if(c==JOptionPane.YES_OPTION){
			         MainWindow.this.ComPanel.appendText(cmd);
			         MainWindow.this.ComPanel.addToHistory(cmd);
			         MainWindow.this.ComPanel.execUserCommand(cmd);
			      }
			   }});
    Databases = Databases.rest();
    index++;
   }
   for(int i=0;i<MI_OpenDatabases.length;i++){
      OpenDatabaseMenu.add(MI_OpenDatabases[i]);
      DeleteDatabaseMenu.add(MI_DeleteDatabases[i]);
   }


  return true;

}


/* the following methods implements the interface SecondoChangeListener */
public void databasesChanged(){
  updateDatabases();
}

public void objectsChanged(){}

public void typesChanged(){}

public void databaseOpened(){}

public void databaseClosed(){}



/** the Listener for the SecondoCommands in MainMenu */
class Command_Listener implements ActionListener{
        public void actionPerformed(ActionEvent evt){
             if (evt.getSource() instanceof JMenuItem){
                JMenuItem Source = (JMenuItem) evt.getSource();
                boolean ok=false;
                String cmd = "";
                if(Source.equals(MainWindow.this.MI_CreateDatabase)){
		   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("create database <dbname>");
		   return;
		}
                if(Source.equals(MainWindow.this.MI_SaveDatabase)){
                   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("save database to <filename>");
		   return;
		}
                if(Source.equals(MainWindow.this.MI_RestoreDatabase)){
                   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("restore database <dbname> from <filename>");
		   return;
		}
                if(Source.equals(MainWindow.this.MI_SaveObject)){
                   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("save <objname> to <filename>");
		   return;
		}
                if(Source.equals(MainWindow.this.MI_RestoreObject)){
                   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("restore <objname> from <filename>");
		   return;
		}
		if(Source.equals(MainWindow.this.MI_CreateType)){
  		   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("type <name> = <type expr>");
		   return;
         	}
		if(Source.equals(MainWindow.this.MI_DeleteType)){
  		   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("delete type <name>");
		   return;
         	}
		if(Source.equals(MainWindow.this.MI_CreateObject)){
  		   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("create <objname> : <type expr>");
		   return;
         	}
		if(Source.equals(MainWindow.this.MI_UpdateObject)){
  		   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("update <objname> := <value expr>");
		   return;
         	}
		if(Source.equals(MainWindow.this.MI_DeleteObject)){
  		   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("delete <objname>");
		   return;
         	}
		if(Source.equals(MainWindow.this.MI_Let)){
  		   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("let <objname> = <value expr>");
		   return;
         	}
		if(Source.equals(MainWindow.this.MI_Query)){
  		   ok = false;
                   MainWindow.this.ComPanel.showPrompt();
		   MainWindow.this.ComPanel.appendText("query <value expr>");
		   return;
         	}
		if(Source.equals(MainWindow.this.MI_UpdateDatabases)){
                   ok = false; // execute no Secondo command
		   if(!MainWindow.this.updateDatabases())
		      MessageBox.showMessage("error in reading databases");
		   return;
		}

                if (Source.equals(MainWindow.this.MI_ListDatabases)){
                    ok = true;
                    cmd = "list databases";
                }else
                if (Source.equals(MainWindow.this.MI_ListTypes)){
                    ok = true;
                    cmd ="list types";
                } else
                if (Source.equals(MainWindow.this.MI_ListTypeConstructors)){
                    ok = true;
                    cmd ="list type constructors";
                } else
                if (Source.equals(MainWindow.this.MI_ListObjects)){
                    ok = true;
			  cmd ="list objects";
                } else
                if(Source.equals(MainWindow.this.MI_ListOperators)){
                    ok = true;
			  cmd="list operators";
                } else
                if(Source.equals(MainWindow.this.MI_ListAlgebras)){
		   ok = true;
                   cmd = "list algebras";
		}else
		if(Source.equals(MainWindow.this.MI_CloseDatabase)){
		   ok = true;
		   cmd = "close database";
		} else
		if(Source.equals(MainWindow.this.MI_BeginTransaction)){
		   ok = true;
		   cmd = "begin transaction";
		} else
        	if(Source.equals(MainWindow.this.MI_AbortTransaction)){
		   ok = true;
		   cmd = "abort transaction";
		}else
		if(Source.equals(MainWindow.this.MI_CommitTransaction)){
		   ok = true;
		   cmd = "commit transaction";
		}
		if (ok) {
		    MainWindow.this.ComPanel.showPrompt();
                    MainWindow.this.ComPanel.appendText(cmd);
		    MainWindow.this.ComPanel.addToHistory(cmd);
                    MainWindow.this.ComPanel.execUserCommand(cmd);
                }
             }
        }
       }


}






