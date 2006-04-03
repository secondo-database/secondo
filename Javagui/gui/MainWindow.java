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
import java.awt.image.BufferedImage;
import java.awt.geom.*;
import tools.Reporter;

public class MainWindow extends JFrame implements ResultProcessor,ViewerControl,SecondoChangeListener{

public final static String CONFIGURATION_FILE="gui.cfg";
public final static String AUTO_HISTORY_FILE=".gui_history";
// the last ... entries of the history are stored
public final static int AUTO_HISTORY_LENGTH=100;
public final static int MIN_FONTSIZE = 6;
public final static int MAX_FONTSIZE = 24;

// shows additional informations if an error is occured
private boolean DEBUG_MODE = true;
/*
The version-line of the history 

This line is used as the first line in a hisrory file 
starting with version 2.0. 
*/
private final static String HIST_VERSION20_LINE = "# VER 2.0";

private JPanel PanelTop;        // change to the desired components
private static CommandPanel ComPanel;
private ObjectList OList;
private JPanel PanelTopRight;
private JSplitPane HSplitPane;
private JSplitPane VSplitPane;
private ServerDialog ServerDlg;
private Vector VCLs;

private Vector ListOfObjects=null;
// contains all objects of the currently opened database



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
private JMenuItem MI_Snapshot;

private JMenu ServerMenu;
private JMenuItem MI_Connect;
private JMenuItem MI_Disconnect;
private JMenuItem MI_Settings;

private JMenu OptimizerMenu;
private JMenuItem MI_OptimizerEnable;
private JMenuItem MI_OptimizerDisable;
private JMenu OptimizerCommandMenu;
private JMenu UpdateRelationsMenu;
private JMenu UpdateIndexMenu;
private JMenuItem MI_UpdateRelationList;
private JMenuItem MI_UpdateIndexList;
// flag indicating whether the entropy menu parts should be included
private boolean useEntropy;
private JMenuItem MI_EnableEntropy;
private JMenuItem MI_DisableEntropy;
private JMenuItem MI_OptimizerSettings;


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
private JMenu Menu_RestoreDatabase;
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
private JFileChooser FC_Database = new JFileChooser();
private JFileChooser FC_Snapshot = new JFileChooser();
private PriorityDialog PriorityDlg;


/* return this */
public Frame getMainFrame(){ return this; }


/* creates a new MainWindow */
public MainWindow(String Title){
  super(Title);
  String StartScript=null;
  setSize(800,600);
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

  this.addWindowListener(new WindowAdapter(){
       public void windowClosing(WindowEvent evt){
          ComPanel.disableOptimizer();
	  ComPanel.disconnect();
	  saveHistory(new File(AUTO_HISTORY_FILE),false,AUTO_HISTORY_LENGTH);
	  System.exit(0);
       }
  });

  PriorityDlg = new PriorityDialog(this);

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
     Reporter.writeError("Javagui: configuration file not found \n" +
                          "searched configuration file: "+CF.getAbsolutePath());
     config_file_ok = false;
  }

  if(config_file_ok){
    try{
      Reporter.writeInfo("load configuration data from: "+CF.getAbsolutePath());
      FileInputStream CFG = new FileInputStream(CF);
      Config.load(CFG);
      CFG.close();
    } catch(Exception e){
      config_file_ok = false;
      Reporter.debug(e);
    }
  }
 int maxStringLength=48;
 useEntropy=false;
 if(config_file_ok){
     String UseEntropy = Config.getProperty("USE_ENTROPY");
     if(UseEntropy!=null){
        if(UseEntropy.toLowerCase().trim().equals("true"))
           useEntropy=true;
     }
     UseEntropy=null;

     String UseFormattedText = Config.getProperty("FORMATTED_TEXT");
     if(UseFormattedText!=null){
        if(UseFormattedText.toLowerCase().trim().equals("true")){
           Environment.FORMATTED_TEXT=true;
        }
     }
     UseFormattedText=null;

     String OldObjectStyle = Config.getProperty("OLD_OBJECT_STYLE");
     Environment.OLD_OBJECT_STYLE=OldObjectStyle!=null && 
                                  OldObjectStyle.toLowerCase().trim().equals("true");
     OldObjectStyle=null;

     String TraceServerCommands = Config.getProperty("TRACE_SERVER_COMMANDS");
     if(TraceServerCommands!=null && TraceServerCommands.trim().toLowerCase().equals("true")){
        Environment.TRACE_SERVER_COMMANDS = true;  
     }

 }
 createMenuBar();
 if(config_file_ok){
    String TMPServerName = Config.getProperty("SERVERNAME");
    if (TMPServerName==null){
      Reporter.writeError("Servername not found in "+CONFIGURATION_FILE);
    }
    else{
      ServerName = TMPServerName;
      Reporter.writeInfo("set ServerName to "+ServerName);
    }

    String TMPServerPort = Config.getProperty("SERVERPORT");
    if(TMPServerPort==null){
       Reporter.writeError("Serverport not found in "+CONFIGURATION_FILE);
    }
    else{
       try{
          int PortInt = (new Integer(TMPServerPort)).intValue();
          if(PortInt <0){
            Reporter.writeError("ServerPort in "+CONFIGURATION_FILE+" less than 0");
          }
          else{
            Reporter.writeInfo("set port to "+PortInt);
            ServerPort = PortInt;
          }
       }
        catch(Exception wrongport){
          Reporter.writeError("error in ServerPort (not an Integer)");
        }
    }


    String Connection = Config.getProperty("START_CONNECTION");
    if(Connection==null){
       Reporter.writeError("START_CONNECTION not found in "+CONFIGURATION_FILE);
    }
    else{
       Connection=Connection.trim().toLowerCase();
       if(Connection.equals("true"))
           StartConnection = true;
       else if(Connection.equals("false"))
           StartConnection = false;
       else{
           Reporter.writeError("START_CONNECTION has unknown value in "+CONFIGURATION_FILE);
           Reporter.writeInfo("allowed values are  true  and false");
       }
    }


    String FontSize = Config.getProperty("COMMAND_FONTSIZE");
    if(FontSize==null){
       Reporter.writeError("COMMAND_FONTSIZE NOT found in "+CONFIGURATION_FILE);
    }
    else{
       try{
           int size = Integer.parseInt(FontSize.trim());
	   ComPanel.setFontSize(size);
       }
       catch(Exception e){
           Reporter.writeError("COMMAND_FONTSIZE has no valid value (not an integer)");
       }

    }

    FontSize = Config.getProperty("LIST_FONTSIZE");
    if(FontSize==null){
       Reporter.writeError("LIST_FONTSIZE not found in "+CONFIGURATION_FILE);
    }
    else{
      try{
         int size = Integer.parseInt(FontSize.trim());
         OList.setFontSize(size);
      }
      catch(Exception e){
         Reporter.writeError("LIST_FONTSIZE has no valid value (not an integer)");
      }

    }

   String MaxStringLen = Config.getProperty("MAX_STRING_LENGTH");
   if(MaxStringLen!=null){
      try{
         int tmp = Integer.parseInt(MaxStringLen.trim());
	 if(tmp>0)
	    maxStringLength = tmp;
      } catch(Exception e){
         Reporter.writeError("invalid value for MAX_STRING_LENGTH");
      }
   }

   String NLCache = Config.getProperty("NL_CACHE");
   if(NLCache!=null){
      try{
        int tmp = Integer.parseInt(NLCache);
        Reporter.writeInfo("initialize NLCache : "+ tmp);
        ListExpr.initialize(tmp);
      } catch(Exception e){
         Reporter.writeError("invalid value for NLCACHE");
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
                Reporter.writeInfo("addViewer "+ViewerName);
                addViewer((SecondoViewer)Cand);
                ((SecondoViewer)Cand).enableTestmode(Environment.TESTMODE);
              }
             else{
               Reporter.writeError(ViewerName+" is not a SecondoViewer");
             }
            }catch(Exception e){
               Reporter.debug("cannot load viewer ",e);
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
   Environment.DEBUG_MODE = DEBUG_MODE;

   String ShowCommand = Config.getProperty("SHOW_COMMAND");
   if(ShowCommand!=null && ShowCommand.toLowerCase().equals("true")){
       Environment.SHOW_COMMAND = true;
   }
   Reporter.writeInfo("ShowCommand " + Environment.SHOW_COMMAND); 


   String Measure_Time = Config.getProperty("MEASURE_TIME");
   Environment.MEASURE_TIME=Measure_Time!=null &&
                            Measure_Time.toLowerCase().trim().equals("true");
   if(Environment.MEASURE_TIME){
     Reporter.writeInfo("Enable messages about used time.");
   }

   String Measure_Memory = Config.getProperty("MEASURE_MEMORY");
   Environment.MEASURE_MEMORY=Measure_Memory!=null &&
                            Measure_Memory.toLowerCase().trim().equals("true");
   if(Environment.MEASURE_MEMORY){
      Reporter.writeInfo("Enable messages about the used memory.");
   }

   String FS = System.getProperty("file.separator");
   if(FS==null){
      Reporter.writeError("error in reading file separator");
      FS="/";
   }


   String SecondoHomeDir = Config.getProperty("SECONDO_HOME_DIR");
   String HistoryDirectory="";
   String DatabaseDirectory="";
   String SnapshotDirectory="";

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

   
   String TMPDIR = Config.getProperty("TMP_DIR");
   File TMPF = new File("tmp");
   if(TMPDIR!=null){
      TMPF = new File(TMPDIR);
   } 
   ListExpr.setTempDir(TMPF.getAbsolutePath());

   String USE_PERSISTENT_TEXT = Config.getProperty("USE_PERSISTENT_TEXT");
   boolean persistentText = false;
   if(USE_PERSISTENT_TEXT!=null){
       USE_PERSISTENT_TEXT=USE_PERSISTENT_TEXT.toLowerCase().trim();
       persistentText = USE_PERSISTENT_TEXT.equals("true");
   } 
   ListExpr.usePersistentText(persistentText);

   String MAX_INTERNAL_TEXT_LENGTH = Config.getProperty("MAX_INTERNAL_TEXT_LENGTH");
   int maxTextLength = 256;
   if(MAX_INTERNAL_TEXT_LENGTH!=null){
       try{
          int tmpMaxTextLength = Integer.parseInt(MAX_INTERNAL_TEXT_LENGTH.trim());
          if(tmpMaxTextLength<1){

          } else{
             maxTextLength=tmpMaxTextLength;
             if(persistentText){
                Reporter.writeInfo("Swap texts with length greater than "+maxTextLength+
                                   " to file");
             }
          }
       }catch(Exception e){
           Reporter.writeError("MAX_INTERNAL_STRING_LENGTH must be an positive integer \n"+
                               " the actual value is :"+MAX_INTERNAL_TEXT_LENGTH+"\n"+
                               " use default value   :"+maxTextLength);
       }
   } 
   ListExpr.setMaxInternalTextLength(maxTextLength);


   if(SecondoHomeDir!=null){
      ObjectDirectory = SecondoHomeDir+FS+"Data"+FS+"GuiData"+FS+"gui"+FS+"objects";
      HistoryDirectory = SecondoHomeDir+FS+"Data"+FS+"GuiData"+FS+"gui"+FS+"histories";
      DatabaseDirectory = SecondoHomeDir+FS+"Data"+FS+"Databases";
      SnapshotDirectory = SecondoHomeDir+FS+"Data"+FS+"GuiData"+FS+"gui"+FS+"snapshots";
   }


   String TMPObjectDirectory= Config.getProperty("OBJECT_DIRECTORY");
   if(TMPObjectDirectory!=null){
      ObjectDirectory = TMPObjectDirectory.trim();
   }

   String TMPDatabaseDirectory= Config.getProperty("OBJECT_DIRECTORY");
   if(TMPDatabaseDirectory!=null){
      DatabaseDirectory = TMPObjectDirectory.trim();
   }

   String TMPHistoryDirectory= Config.getProperty("HISTORY_DIRECTORY");
   if(TMPHistoryDirectory!=null)
        HistoryDirectory = TMPHistoryDirectory;

   String TMPSnapshotDirectory= Config.getProperty("SNAPSHOT_DIRECTORY");
   if(TMPSnapshotDirectory!=null)
        SnapshotDirectory = TMPSnapshotDirectory;

   Reporter.writeInfo("set objectdirectory to "+ObjectDirectory);
   Reporter.writeInfo("set historydirectory to "+HistoryDirectory);
   Reporter.writeInfo("set databasedirectory to "+DatabaseDirectory);
   Reporter.writeInfo("set snapshotdirectory to "+SnapshotDirectory);

   OList.setObjectDirectory(new File(ObjectDirectory));
   FC_History.setCurrentDirectory(new File(HistoryDirectory));
   FC_Database.setCurrentDirectory(new File(DatabaseDirectory));
   FC_Snapshot.setCurrentDirectory(new File(SnapshotDirectory));
   javax.swing.filechooser.FileFilter filter = new javax.swing.filechooser.FileFilter(){
        public boolean accept(File PathName){
             if(PathName==null) return false;
             if(PathName.isDirectory())
                 return true;
             if(PathName.getName().endsWith(".png"))
                  return true;
             else
                 return false;
             }
             public String getDescription(){
               return "PNG images";
             }
        };
    FC_Snapshot.setFileFilter(filter);


   StartScript = Config.getProperty("STARTSCRIPT");

   // optimizer settings
   String OptHost = Config.getProperty("OPTIMIZER_HOST");
   if(OptHost==null){
      OptHost ="localhost";  // the default value
      Reporter.writeError("OPTIMIZER_HOST not defined, use default: "+OptHost);
   }
   String OptPortString = Config.getProperty("OPTIMIZER_PORT");
   int OptPort = 1235; // default value
   if(OptPortString!=null){
      try{
        int P = Integer.parseInt(OptPortString);
        if(P<=0){
          Reporter.writeError("optimizer-port has no valid value");
	}else
	   OptPort = P;
       }
       catch(Exception e){
          Reporter.writeError("optimizer-port is not a valid integer");
       }
   }else{
      Reporter.writeError("OPTIMIZER_PORT not defined, use default: "+OptPort);
   }
   ComPanel.setOptimizer(OptHost,OptPort);
   String OptEnable = Config.getProperty("ENABLE_OPTIMIZER");
   if(OptEnable==null){
      Reporter.writeError("ENABLE_OPTIMIZER not defined in configuration file");
   }
   else  {
      OptEnable=OptEnable.trim().toLowerCase();
      if(OptEnable.equals("true"))
          if(!ComPanel.enableOptimizer()){
             Reporter.writeError("error in enabling optimizer");
          } else {
             Reporter.writeInfo("optimizer enabled");
          }
   }

   String ShowLicence = Config.getProperty("SHOW_LICENCE");
   if(ShowLicence==null || !ShowLicence.toLowerCase().equals("false")){
       showLicence();
   } 

  } // config -file readed




  ComPanel.setConnection(UserName,PassWd,ServerName,ServerPort);
  if (StartConnection){
      if (!ComPanel.connect()){
         Reporter.showWarning("I can't find a Secondo-server");
      }
      else
         getServerInfos();
  }

  OList.setMaxStringLength(maxStringLength);
  ListExpr.setMaxStringLength(maxStringLength);
  Environment.MAX_STRING_LENGTH = maxStringLength;

  if(StartScript!=null ){
      StartScript = StartScript.trim();
      Reporter.writeInfo("execute "+StartScript);
      if (StartScript.endsWith("-i") ){
         StartScript = StartScript.substring(0,StartScript.length()-2).trim();
         executeFile(StartScript,true);
      }
      else{ // ignore errors in StartScript in testmode
         executeFile(StartScript,Environment.TESTMODE);
      }
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

    // load the last history
    loadHistory(new File(AUTO_HISTORY_FILE),false,false);


    // we create a global keylistener for making screenshots
    AWTEventListener SnapshotKL = new AWTEventListener(){
      public void eventDispatched(AWTEvent e){
          if(! (e instanceof KeyEvent))
             return;
          KeyEvent evt = (KeyEvent) e;
          if(e.getID()!=KeyEvent.KEY_PRESSED)
             return;
          if(!evt.isAltDown())
              return;
          int c = evt.getKeyCode();
          if(c==KeyEvent.VK_S){
             if(saveSnapshot(true)){
                Reporter.showInfo("Snapshot written");
             }
             evt.consume();
          }
          if(c==KeyEvent.VK_C){
             if(saveSnapshot(false)){
                Reporter.showInfo("Snapshot written");
             }
             evt.consume();
          }
      } 
    };
    Toolkit.getDefaultToolkit().addAWTEventListener(SnapshotKL,AWTEvent.KEY_EVENT_MASK);
}



public void addViewerChangeListener(ViewerChangeListener VCL){
  if(VCLs.indexOf(VCL)<0)
     VCLs.add(VCL);
}

public void removeViewerChangeListener(ViewerChangeListener VCL){
  VCLs.remove(VCL);
}


/** returns the name of the main type of the given ListExpr;
  * this means for a list (rel(...))  "rel is returned
  */
private String getMainType(ListExpr Type){
  while(!Type.isAtom() && Type.listLength()>0)
     Type = Type.first();
  int atomType = Type.atomType();
  if(atomType!=ListExpr.SYMBOL_ATOM)
     return "unknow type";
  return Type.symbolValue();
}


/** updates the list of objects in the database */
private void updateObjectList(){
   if(!ComPanel.isConnected()){
      ListOfObjects = null;
      return;
   }
   ListExpr LE = ComPanel.getCommandResult("list objects");
   if(LE==null){
      ListOfObjects = null;
      return;
   }
   LE = LE.second().second().rest();
   ListOfObjects = null;
   ListExpr CurrentObject = null;
   if(LE.isEmpty()){
      return;
   }

   ListOfObjects = new Vector(LE.listLength()+1);
   while(!LE.isEmpty()){
      ListOfObjects.add(ListExpr.threeElemList(LE.first().second(),
                                               ListExpr.symbolAtom(getMainType(LE.first().fourth())),
					       LE.first().fourth()));
      LE = LE.rest();
   }

}

/** Creates a snapshot from the current Javagui. 
  * If the snapshot can't be created, null is returned.
  **/
private BufferedImage makeSnapshot(){
   Rectangle2D R = getBounds();
   BufferedImage bi = new BufferedImage((int)R.getWidth(),(int)R.getHeight(),
                                        BufferedImage.TYPE_INT_RGB);
   Graphics2D g = bi.createGraphics();
   this.printAll(g);
   g.dispose();
   return bi;
}

/** Created a snapshot of the whole screen.
  * If an error occurs, the result will be null.
  **/
private BufferedImage makeScreenSnapshot(){
    try{
       return (new Robot()).createScreenCapture( new Rectangle(Toolkit.getDefaultToolkit().getScreenSize()));
    }catch(Exception e){
        return null;
    }
}

/** Saves a snapshot of the current state of Javagui */
private boolean saveSnapshot(boolean completeScreen){
   BufferedImage snapshot;
   if(completeScreen)
      snapshot=makeScreenSnapshot();
   else
      snapshot = makeSnapshot();
   if(snapshot==null){
      Reporter.showError("Error in creating snapshot");
      return false;
   }
   if(FC_Snapshot.showSaveDialog(this)==JFileChooser.APPROVE_OPTION){
       try{
          return javax.imageio.ImageIO.write(snapshot,"png",FC_Snapshot.getSelectedFile());
       } catch(Exception e){
           Reporter.debug(e);
           Reporter.showError("Error in saving snapshot");
           return false;
       } 

   }else
      return false;
}

/** Function enabling or disabling the entropy function of the 
    Optimizer. **/
private void enableEntropy(boolean on){
    String command = on?"use_entropy":"dont_use_entropy";
    ComPanel.appendText("\noptimizer "+command+"   ");
    if(ComPanel.sendToOptimizer(command)==null){
        ComPanel.appendText(" ... failed");
    }else{
        ComPanel.appendText(" ... successful");
    }
    ComPanel.showPrompt();
}


/**  reconstructed the menu updateRelationMenu
  *  the values are given through the current ListOfObjects-Vector
  */
private void updateRelationList(){
  // first remove all entries without the first two one
  UpdateRelationsMenu.removeAll();
  UpdateRelationsMenu.add(MI_UpdateRelationList);
  UpdateIndexMenu.removeAll();
  UpdateIndexMenu.add(MI_UpdateIndexList);
  boolean first = true;
  if(ListOfObjects==null)
     return;
  ListExpr CurrentObject;
  for(int i=0;i<ListOfObjects.size();i++){
      CurrentObject = (ListExpr) ListOfObjects.get(i);
      if(CurrentObject.second().symbolValue().equals("rel") ||
         CurrentObject.second().symbolValue().equals("mrel")){ // found a relation
           if(first){
	       UpdateRelationsMenu.addSeparator();
	       UpdateIndexMenu.addSeparator();
               first = false;
	   }
           // add the MenuEntry and a actionListener for this one
	   String RelName = CurrentObject.first().symbolValue();
	   // samples should not be updated
           if(!RelName.endsWith("_sample")){
              JMenuItem Rel = UpdateRelationsMenu.add(RelName);
	      Rel.addActionListener(new ActionListener(){
	         public void actionPerformed(ActionEvent evt){
	            String RelName = ((JMenuItem)evt.getSource()).getText().toLowerCase();
		    String res = ComPanel.sendToOptimizer("updateRel "+RelName);
		    ComPanel.appendText("\n optimizer updateRel "+RelName+"   ");
		    if(res ==null)
		       ComPanel.appendText("...failed");
		    else
   		       ComPanel.appendText("...successful");
		    ComPanel.showPrompt();
	         }
	      });
	      JMenu RelMenu = new JMenu(CurrentObject.first().symbolValue());
	      UpdateIndexMenu.add(RelMenu);
	      // insert the attributes into the RelMenu
	      ListExpr FullType = CurrentObject.third().first();
	      ListExpr TupleList = FullType.second().second();
	      JMenuItem MI_Attr;
	      while(!TupleList.isEmpty()){
	         String AttrName = TupleList.first().first().symbolValue();
	         MI_Attr = RelMenu.add(AttrName);
	         MI_Attr.setActionCommand("updateIndex "+RelName.toLowerCase()+" "+AttrName.toLowerCase());
	         MI_Attr.addActionListener(new ActionListener(){
	              public void actionPerformed(ActionEvent evt){
                         JMenuItem Btn = (JMenuItem) evt.getSource();
 	  	         String command = Btn.getActionCommand();
		         String res = ComPanel.sendToOptimizer(command);
		         ComPanel.appendText("\noptimizer "+command);
		         if(res ==null)
		             ComPanel.appendText("...failed");
		         else
		             ComPanel.appendText("...successful");
		         ComPanel.showPrompt();
		      }});
	          TupleList=TupleList.rest();
               }
	} // not a _sample


      }
  }


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
   SecondoViewer[] tmpViewers = new SecondoViewer[AllViewers.size()];
   for(int i=0;i<AllViewers.size();i++)
      tmpViewers[i] = (SecondoViewer) AllViewers.get(i);
   return tmpViewers;
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
     ComPanel.disableOptimizer();
     saveHistory(new File(AUTO_HISTORY_FILE),false,AUTO_HISTORY_LENGTH);
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
        Reporter.debug(e);
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
         ComPanel.appendText("unknown parameter\n");
     ComPanel.showPrompt();
  } else if(command.startsWith("clearAll")){
     clearAll();
     //ComPanel.showPrompt();
  } else if(command.startsWith("status")){
      if(ComPanel.isConnected()){
        ComPanel.appendText("connected to Secondo\n");
        if(ComPanel.getOpenedDatabase().equals(""))
           ComPanel.appendText("no database open");
         else
            ComPanel.appendText("opened database: "+ComPanel.getOpenedDatabase());
       }else{
         ComPanel.appendText("not connected");
       }
       ComPanel.showPrompt();
  }
  else {
    ComPanel.appendText("unknown gui command \n show help to get a list of available commands");
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
    Reporter.debug("io error occured",e);
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


/** Converts a list into a command.
  * The list can be a string atom, a text atom or a 
  * list consisting of a sequence of text atoms and string atoms.
  * If the list has an invalid format, the reuslt will be null.
  **/
private String listToCommand(ListExpr list){
  int at = list.atomType();
  String command="";
  switch(at){
     case ListExpr.STRING_ATOM: command = list.stringValue();
                                break;
     case ListExpr.TEXT_ATOM:   command = list.textValue();
                                break;
     case ListExpr.NO_ATOM:
          while(!list.isEmpty()){
              ListExpr part = list.first();
              list = list.rest();
              int atp = part.atomType();
              switch(atp){
                   case ListExpr.STRING_ATOM: command += part.stringValue();
                                                break;
                    case ListExpr.TEXT_ATOM: command += part.textValue();
                                             break;
                    default: return null;
                   }
                 }
                 break;
        default: return null;
  }  
  return command;  
}


/** Executes the test describen in the testlist.
  * The listformat has to be:
  *  ( COMMAND RESULT )
  * The COMMAND format is described in listToCommand.
  * The RESULT must be a list formatted as
  * ( SUCCESS EPSILON RESULTLIST )
  * where SUCCESS is a boolean atom (if false, the remaining parts
  * are ignored. The value of EPSILON described the precision used for
  * checking numeric values for equality. And the resultlist describes
  * expected result list. If an error occurs while executing a command,
  * the command is appended to theCommand).
  * @return: the succcess of the test 
  **/
private boolean makeTest(ListExpr test,StringBuffer failedCommands){
   if(test.listLength()!=2){
      Reporter.writeError("invalid listlength for test");
      test.writeListExpr();
      return false;
   }
   String command = listToCommand(test.first());
   if(command==null){
       Reporter.writeError("invalid format of command in test");
       test.writeListExpr();
       return false;
   }
   
   ListExpr resultSpec = test.second();
   if(resultSpec.listLength()!=2){
       Reporter.writeError("Error in resultspecification detected, \n"+      
                          "list containing two elements expected");
       return false;
   }
   ListExpr resPlace=resultSpec.first();
   int at = resPlace.atomType();
   if(at!=ListExpr.SYMBOL_ATOM){
      Reporter.writeError("first element of result must be an symbol ");
      return false;
   }


   String rp = resPlace.symbolValue();
   boolean fromFile=false;
   if(rp.equals("file")){
       fromFile=true;
   } else if(rp.equals("list")){
       fromFile=false;
   } else{
       Reporter.writeError("first element of result has to be 'list' or 'file'");
       return false;
   }
   resultSpec=resultSpec.second();
 
   at = resultSpec.atomType();
   if(at!=ListExpr.BOOL_ATOM && at!=ListExpr.NO_ATOM){
       Reporter.writeError("wrong result format in test");
       test.writeListExpr();
       return false;
   } 
   boolean expSuccess=false;
   ListExpr expResult = null;
   double epsilon = 0.0;
   boolean isTest=true;
   
   if(at==ListExpr.BOOL_ATOM){
      expSuccess=resultSpec.boolValue(); 
   } else{
      int len = resultSpec.listLength();
      if(len==0){
        // only a command, not a proper test
        isTest = false; 
      } else {  
        // read the value of success
        ListExpr f = resultSpec.first();
        if(f.atomType()!=ListExpr.BOOL_ATOM){
            Reporter.writeError("invalid result specification in test");
            resultSpec.writeListExpr();
            return false;
        } else{
             expSuccess = f.boolValue();
        }
        if(len>=2){
           if(!fromFile){
              expResult = resultSpec.second();
            } else {
               int at2 = resultSpec.second().atomType();
               String FileName="";
               switch(at2){
                  case ListExpr.SYMBOL_ATOM: 
                        FileName = resultSpec.second().symbolValue();
                        break;
                  case ListExpr.STRING_ATOM:
                         FileName = resultSpec.second().stringValue();
                         break;
                  case ListExpr.TEXT_ATOM:
                         FileName = resultSpec.second().textValue();
                         break;
                  default:
                         Reporter.writeError("invalid filename specification in result");

               }
               expResult = ListExpr.getListExprFromFile(FileName);
               if(expResult==null){
                  Reporter.writeError("Error in loading '" + FileName+"'");
                  return false;
               }
            }
        }
        if(len==3){ // full format (<success><result><epsilon>
           ListExpr t = resultSpec.third();
           int tat = t.atomType();
           switch(tat){
             case ListExpr.INT_ATOM : epsilon = t.intValue();
                                      break;
             case ListExpr.REAL_ATOM : epsilon = t.realValue();
                                       break;
             default: Reporter.writeError("invalid format for epsilon in test");
                      t.writeListExpr();
                      return false;
           }
        }
      }  
   }  
   Reporter.writeInfo("test command " + command); 
   boolean res = ComPanel.execUserCommand(command,isTest,expSuccess,
                                          epsilon,expResult);
   if(!res){
      failedCommands.append(command+"\n");  
   }
   return res;
}

/** executes a testfile 
  * @param haltOnErrors: if set to true, the test section will be finished if an
                         error occurs.
  * @return: numbe of errors found
  */
private int executeTestFile(String fileName, boolean haltOnErrors, 
                            StringBuffer failedCommands){
  Reporter.writeInfo("test file "+ fileName);
  int errors = 0;
  try{
			// Test specifications within Javagui are given as NestedList 
			ListExpr testList = ListExpr.getListExprFromFile(fileName);
			if(testList==null){
				 Reporter.writeError("Error in loading of test specification");
				 return 1;
			} 
			int len = testList.listLength();
			if(len!=3){
				 // format has to be ( <setup> <tests> <teardown> )
				 Reporter.writeError("Invalid listlength for testfile specification");
				 return 1;
			}
			ListExpr setup = testList.first();
			// the setup file is a list in form ( <com_1> <com_2> ... <com_n> )
			if(setup.atomType()!=ListExpr.NO_ATOM){
				 Reporter.writeError("Invalid Setup part in TestList found ");
				 return 1;
			}
			// execute all commands in setup part ignoring any errors in execution
      Reporter.writeInfo("--------------------------------\n"+
                         "      enter setup section \n"+
                         "--------------------------------n");
			while(!setup.isEmpty()){
				String command = listToCommand(setup.first());
				setup = setup.rest();
				if(command==null){ 
					 Reporter.writeError("Invalid format for a command in setup section detected");
					 errors++;
				 }else{
					 ComPanel.execUserCommand(command); 
				 }
			}

			
		ListExpr tests =  testList.second();
		// tests has to be in format ( <test_1> <test_2> ... <test_n> )
		if(tests.atomType() != ListExpr.NO_ATOM){
				 Reporter.writeError("Invalid format of tests found in file specification "); 
				 Reporter.writeInfo(" switch to teardown ");
		} else{
       Reporter.writeInfo("--------------------------------------------\n"+
                          "        enter test section \n"+
                          "---------------------------------------------");
                      
			 boolean stop = false;
       int testno = 1;
			 while(!tests.isEmpty() && !stop){
            Reporter.writeInfo (" -------> test number " + testno + " <------");
						if(!makeTest(tests.first(),failedCommands)){
							 errors++;
							 stop = haltOnErrors;
						}
            testno++;
						tests = tests.rest();
		   }
			}
			// teardown: this section has the same format as the setup section
			// all errors except wrong formatted lists are ignored
      ListExpr teardown = testList.third();
			if(teardown.atomType()!=ListExpr.NO_ATOM){
				 Reporter.writeError("Invalid format of teardown section in testfile ");
				 return errors + 1;
			}
      Reporter.writeInfo("--------------------------------------------\n"+
                          "        enter teardown section \n"+
                          "---------------------------------------------");
			while(!teardown.isEmpty()){
				String command = listToCommand(teardown.first());
				if(command==null){
					 Reporter.writeError("Invalid command format in teardown section found");
					 errors++;
				}else{
					ComPanel.execUserCommand(command);
				}
        teardown = teardown.rest();
			} 
  } catch(Exception e){
     Reporter.writeError("Error in testing file ");
     Reporter.debug(e);
     errors++;
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
        Reporter.debug(e);
        Reporter.showError("error when update the menu");
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

/** closes all connections and exits the program */
public void shutdown(int errorCode){
    ComPanel.disconnect();
    ComPanel.disableOptimizer();
    setVisible(false);
    System.exit(errorCode);
}

/* the main function to start program */
public static void main(String[] args){
  if(args.length<1){
     Reporter.writeInfo("start Javagui without any argument");
  }else{
     String allArgs=args[0];
     for(int i=1;i<args.length;i++){
       allArgs += " " + args[i];
     }
     Reporter.writeInfo("start Javagui with \""+allArgs+"\"");  
  }

  Environment.TESTMODE = args.length>0 && args[0].equals("--testmode");
  Environment.EXTENDED_TESTMODE = args.length>0 && args[0].equals("--testmode2");
  if(Environment.EXTENDED_TESTMODE){
     Environment.TESTMODE=true;
  }
  File testfile=null;
  if(Environment.TESTMODE){
     if(args.length>1){
        testfile = new File(args[1]);
        if(!testfile.exists()){
            Reporter.writeError("testfile " + testfile+" not found");
            System.exit(1);
        }
     }
  }
  MainWindow SecGui = new MainWindow("Secondo-GUI");
  SecGui.setVisible(true);
  if(Environment.TESTMODE && !Environment.EXTENDED_TESTMODE && testfile!=null){
     Reporter.writeInfo("Run Testfile");
     SecGui.executeFile(testfile.getAbsolutePath(),true);
  }
  if(Environment.EXTENDED_TESTMODE){
     Environment.DEBUG_MODE=true;
     if(testfile==null){
        Reporter.writeError("the extended testmode requires an testfile ");
        SecGui.shutdown(1);
     }
     StringBuffer failedCommands = new StringBuffer(); 
     int errors = SecGui.executeTestFile(testfile.getAbsolutePath(),true,
                                         failedCommands);
     if(errors>0){
        Reporter.writeError("They are "+errors+" erros occured during the test");
        Reporter.writeError("failed commands :"+failedCommands);
     } else{
        Reporter.writeInfo("All tests was performed successful");
     }
     try{
        Thread.sleep(10000);
     }catch(Exception e){}
     SecGui.shutdown(errors);
     
  }
  MainWindow.ComPanel.requestFocus();
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
              if(SV==null){
                Reporter.showError("no viewer found to display the result");
              }
              else{
                  if(SV!=CurrentViewer)
                     setViewer(SV);
                  CurrentViewer.addObject(o);
                  OList.updateMarks();
               }
  
            } catch(Exception e){
               Reporter.debug(e);
               Reporter.showError("an error is occurred (in current viewer)");
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
       Reporter.debug("error in method canDisplay in the current Viewer",e);
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
        Reporter.debug("error in current Viewer "+CurrentViewer+" method: isDisplayed",e);
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
             Reporter.showError("no Viewer found to display this object");
             return false;
          }
          else{
             setViewer(TheBest);
             return CurrentViewer.addObject(SO);
          }  
        }  
     }      
     catch(Exception e){
        Reporter.debug("error in Viewer :"+CurrentViewer+" method addObject", e);
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
           Reporter.debug("an Exception is occured in removeObject-Method of a Viewer", e);
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
          Reporter.debug(e);
          Reporter.showError("Exception in current viewer (method selectObject)");
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
   
   MI_Snapshot = ProgramMenu.add("Snapshot");
   MI_Snapshot.setAccelerator(KeyStroke.getKeyStroke("alt C"));
   MI_Snapshot.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           if(saveSnapshot(false)){
              Reporter.showInfo("Snapshot written");
           }
       }
   });
   JMenuItem MI_ScreenSnapshot = ProgramMenu.add("ScreenSnapshot");
   MI_ScreenSnapshot.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           if(saveSnapshot(true)){
              Reporter.showInfo("Snapshot written");
           }
       }
   });
   MI_ScreenSnapshot.setAccelerator(KeyStroke.getKeyStroke("alt S"));

   MI_Close = ProgramMenu.add("Exit");
   MI_Close.addActionListener( new ActionListener(){
     public void actionPerformed(ActionEvent E){
        ComPanel.disconnect();
	ComPanel.disableOptimizer();
	saveHistory(new File(AUTO_HISTORY_FILE),false,AUTO_HISTORY_LENGTH);
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



   OptimizerMenu = new JMenu("Optimizer");
   MI_OptimizerEnable = OptimizerMenu.add("Enable");
   MI_OptimizerDisable = OptimizerMenu.add("Disable");
   OptimizerCommandMenu = new JMenu("Command");
   OptimizerMenu.add(OptimizerCommandMenu);
   UpdateRelationsMenu = new JMenu("Update Relation");
   OptimizerCommandMenu.add(UpdateRelationsMenu);
   UpdateIndexMenu = new JMenu("Update Index");
   OptimizerCommandMenu.add(UpdateIndexMenu);

   MI_UpdateRelationList = UpdateRelationsMenu.add("Update List");
   ActionListener A = new ActionListener(){
          public void actionPerformed(ActionEvent evt){
           updateObjectList();
	   updateRelationList();
          }
       };
   MI_UpdateIndexList = UpdateIndexMenu.add("Update List");
   MI_UpdateRelationList.addActionListener(A);
   MI_UpdateIndexList.addActionListener(A);

   JMenu Entropy = new JMenu("Entropy");
   MI_EnableEntropy = Entropy.add("enable");
   MI_DisableEntropy = Entropy.add("disable");
   ActionListener EntropyListener = new ActionListener(){
       public void actionPerformed(ActionEvent evt){
              if(MI_EnableEntropy.equals(evt.getSource())){
                  enableEntropy(true);
              }else{
                  enableEntropy(false);
              }
       }
   }; 
   MI_EnableEntropy.addActionListener(EntropyListener);
   MI_DisableEntropy.addActionListener(EntropyListener); 

   if(useEntropy) 
      OptimizerCommandMenu.add(Entropy);

   MI_OptimizerSettings = OptimizerMenu.add("Settings");
   OptimizerMenu.addMenuListener(new MenuListener(){
     public void menuSelected(MenuEvent evt){
        if(ComPanel.useOptimizer()){
           MI_OptimizerEnable.setEnabled(false);
           MI_OptimizerDisable.setEnabled(true);
	   OptimizerCommandMenu.setEnabled(true);
        }
        else{
           MI_OptimizerEnable.setEnabled(true);
           MI_OptimizerDisable.setEnabled(false);
	   OptimizerCommandMenu.setEnabled(false);
	}
     }
     public void menuDeselected(MenuEvent evt){}
     public void menuCanceled(MenuEvent evt){}
   } );


   ActionListener OptimizerListener= new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          Object source = evt.getSource();
	  if(source.equals(MI_OptimizerEnable)){
	    if(!ComPanel.enableOptimizer()){
                ComPanel.appendText("enabling optimizer failed");
		ComPanel.showPrompt();
	    } else{
                ComPanel.appendText("optimizer enabled");
		ComPanel.showPrompt();
	    }
	  }
	  if(source.equals(MI_OptimizerDisable)){
	        ComPanel.appendText("optimizer disabled");
		ComPanel.showPrompt();
	        ComPanel.disableOptimizer();
	  }
	  if(source.equals(MI_OptimizerSettings))
	    ComPanel.showOptimizerSettings();
       }};

    MI_OptimizerEnable.addActionListener(OptimizerListener);
    MI_OptimizerSettings.addActionListener(OptimizerListener);
    MI_OptimizerDisable.addActionListener(OptimizerListener);

    MainMenu.add(OptimizerMenu);


   Menu_ServerCommand = new JMenu("Command");
   MainMenu.add(Menu_ServerCommand);
   // commands are only possible if connected
   Menu_ServerCommand.setEnabled(false);
   Menu_BasicCommands = new JMenu("Basic Commands");
   Menu_BasicCommands.setEnabled(false);
   Menu_Inquiries = new JMenu("Inquiries");
   Menu_Inquiries.setEnabled(false);
   Menu_Databases = new JMenu("Databases");
   Menu_Databases.setEnabled(false);
   Menu_Transactions = new JMenu("Transactions");
   Menu_Transactions.setEnabled(false);
   Menu_ImExport = new JMenu("Import / Export");
   Menu_ImExport.setEnabled(false);

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
   Menu_RestoreDatabase = new JMenu("~restore database~");
   Menu_ImExport.add(Menu_RestoreDatabase);
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
               MyHelp.setMode(HelpScreen.GUI_COMMANDS);
               MyHelp.setVisible(true);
           }
        });

   MI_ShowSecondoCommands.addActionListener(new ActionListener(){
           public void actionPerformed(ActionEvent evt){
               MyHelp.setMode(HelpScreen.SECONDO_COMMANDS);
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
              Reporter.showWarning("a viewer must be in the viewer directory");
           }
           else{
                Name = Name.substring(pos);
                char FileSep = File.separatorChar;
                Name = Name.replace(FileSep,'.');
                if (!Name.endsWith(".class")){
                   Reporter.showError("this is no class file");
                }
                else{
                   Name= Name.substring(0,Name.length()-6);  // remove ".class" extension
                   try{
                        Class NViewerClass = Class.forName(Name);
                        Object O = NViewerClass.newInstance();
                        if (!(O instanceof SecondoViewer)){
                          Reporter.showError("the selected class is not an SecondoViewer");
                        }
                        else{  // is allright
                           addViewer((SecondoViewer)O);
                        }
                   }
                   catch(Exception e){
                        ComPanel.appendText(""+e);
                        ComPanel.showPrompt();
                        Reporter.showError("cannot load the given Viewer\n"+
                                           " see commandPanel for details");
                        Reporter.debug(e);
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
  long usedMemory=0;
  if(Environment.MEASURE_MEMORY){
      usedMemory=Environment.usedMemory();
  } 
  ComPanel.clear();
  OList.clearList();
  for(int i=0;i<AllViewers.size();i++)
     ((SecondoViewer) AllViewers.get(i)).removeAll();
  System.gc();
  if(Environment.MEASURE_MEMORY){
     Reporter.writeInfo("Memory difference by clear: "+
                         Environment.formatMemory(Environment.usedMemory()-usedMemory));
  }
}



/** load a new History,
  * if replace is true, the old History is make clear before
  * load the new History
  */
public void loadHistory(boolean replace){

  if(FC_History.showOpenDialog(this)==JFileChooser.APPROVE_OPTION){
      // first try to load the file content
      loadHistory( FC_History.getSelectedFile(),replace,true);
   }
}


/** loads the history from the specified file,
  * if the replace flag is true, the current history is
  * replaced by the lines from the file, otherwise the
  * file-content is appended to the current history
  * if showMessage is false, no error messages are printed out,
  * this is needed for an automatically loading of the history
  */
private void loadHistory(File F,boolean replace,boolean showMessage){
      boolean ok = true;
      Vector TMP=new Vector();
      BufferedReader BR=null;
      try{
        BR = new BufferedReader(new FileReader(F));
        String Line = BR.readLine();
        if(!HIST_VERSION20_LINE.equals(Line)){
            // import version 1.0 style history
            while(Line!=null){
               TMP.add(Line);
               Line = BR.readLine();
            }
        }else if(Line!=null){ // import version 2.0 history supporting multiline-inputs
          String CurrentCommand=null;
          Line = BR.readLine();
          while(Line!=null){
            if(Line.startsWith("#")){ // end of command found
               if(CurrentCommand!=null){
                 CurrentCommand=CurrentCommand.trim();
                 if(!CurrentCommand.equals("")){
                     TMP.add(CurrentCommand);
                 }
                 CurrentCommand=null;
               }
            }else{
               // remove space automatically added while storing
               // the test of a whitespace is maked for "hand-tuned" history files
               if(Line.startsWith(" ")){
                  if(CurrentCommand==null)
                     CurrentCommand = Line.substring(1);
                  else
                     CurrentCommand += "\n" + Line.substring(1);
               }
            }
            Line = BR.readLine();
          }
          // insert the last command if available
          if(CurrentCommand!=null){
             CurrentCommand = CurrentCommand.trim();
             if(!CurrentCommand.equals(""))
                TMP.add(CurrentCommand);
          }
        } // end loading of version 2.0
      } catch(Exception e){
        if(showMessage){
           ComPanel.appendText("load history failed \n");
           ComPanel.showPrompt();
           Reporter.debug(e);
        }
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
     if(CurrentViewer==null){
       Reporter.showError("there is no viewer to show");
     }
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
             Reporter.showError("I can't find a SecondoServer ");
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
       saveHistory(F,true,-1);
    }
}


/** save the history in the specified file 
  * if the showMessage-flag is true error messages are printed out
  * if the length less the zero, all entries in the history are saved
  * the last length ones otherwise
  */
private void saveHistory(File F,boolean showMessage,int length){
      FileWriter FW =null;
       try{
          FW = new FileWriter(F);
          int start = length<0? 0 : Math.max(ComPanel.getHistorySize()-length,0);
          FW.write(HIST_VERSION20_LINE+"\n");
	  for(int i=start;i<ComPanel.getHistorySize();i++){
              MyStringTokenizer ST = new MyStringTokenizer(ComPanel.getHistoryEntryAt(i),'\n');
              while(ST.hasMoreTokens()){
                  FW.write(" "+ST.nextToken()+"\n");
              }
              // write command delimiter
              FW.write("#\n");
          }
       }
       catch(Exception e){
         if(showMessage){
            Reporter.debug(e);
            ComPanel.appendText("IO error");
	 }
       }
       finally{
         try{
          if(FW!=null)
             FW.close();
         }
         catch(Exception e2){}
       }
}

/* cleans the MenuBar (MenuBar without Viewer-Extension */
private void cleanMenu(){
   MainMenu.removeAll();
   MainMenu.add(ProgramMenu);
   MainMenu.add(ServerMenu);
   MainMenu.add(OptimizerMenu);
   MainMenu.add(Menu_ServerCommand);
   MainMenu.add(HelpMenu);
   MainMenu.add(Viewers);
}


/* get infos from server
   - known algebras, databases
 */
private void getServerInfos(){

  ListExpr Algebras = ComPanel.getCommandResult("list algebras");
  if(Algebras==null){
     Reporter.writeError("Error in reading algebras from server");
     return;
  }
  Algebras = Algebras.second().second();
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
			 MainWindow.ComPanel.appendText(cmd);
			 MainWindow.ComPanel.addToHistory(cmd);
		   MainWindow.ComPanel.execUserCommand(cmd);
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
  if(Databases==null)
     return false;
  Databases = Databases.second().second();
  OpenDatabaseMenu.removeAll();
  DeleteDatabaseMenu.removeAll();
  Menu_RestoreDatabase.removeAll();
  JMenuItem[] MI_OpenDatabases = new JMenuItem[Databases.listLength()];
  JMenuItem[] MI_DeleteDatabases = new JMenuItem[Databases.listLength()];
  JMenuItem[] MI_RestoreDatabases = new JMenuItem[Databases.listLength()];
  int index = 0;
  while(!Databases.isEmpty()){
    String Name = Databases.first().symbolValue();
    MI_OpenDatabases[index] = new JMenuItem(Name);
    MI_OpenDatabases[index].addActionListener(new ActionListener(){
                           public void actionPerformed(ActionEvent evt){
			      String cmd = "open database "+((JMenuItem)evt.getSource()).getText();
			      MainWindow.ComPanel.appendText(cmd);
			      MainWindow.ComPanel.addToHistory(cmd);
			      MainWindow.ComPanel.execUserCommand(cmd);
			   }});
    MI_RestoreDatabases[index] = new JMenuItem(Name);
    MI_RestoreDatabases[index].addActionListener(new ActionListener(){
                           public void actionPerformed(ActionEvent evt){
			      String cmd_part1 = "restore database "+((JMenuItem)evt.getSource()).getText() + " from ";
			      FC_Database.setDialogTitle(cmd_part1);
			      if(FC_Database.showOpenDialog(MainWindow.this)==JFileChooser.APPROVE_OPTION){
			         String FName = FC_Database.getSelectedFile().getAbsolutePath();
				 String cmd = cmd_part1+" '"+FName+"'";
				 MainWindow.ComPanel.addToHistory(cmd);
				 if(MainWindow.ComPanel.execUserCommand(cmd)){
				    Reporter.showInfo("restoring database successful");
         }
				 else{
            Reporter.showError("restoring database failed");
         }
			         MainWindow.ComPanel.showPrompt();
			      }
			   }});

    MI_DeleteDatabases[index] = new JMenuItem(Name);
    MI_DeleteDatabases[index].addActionListener(new ActionListener(){
                           public void actionPerformed(ActionEvent evt){
			      String db = ((JMenuItem)evt.getSource()).getText();
			      String cmd = "delete database "+db;
			      int c = JOptionPane.showConfirmDialog(null,"really delete the database "+db+"?",
			                                           "Confirm",JOptionPane.YES_NO_OPTION);
			      if(c==JOptionPane.YES_OPTION){
			         MainWindow.ComPanel.appendText(cmd);
			         MainWindow.ComPanel.addToHistory(cmd);
			         MainWindow.ComPanel.execUserCommand(cmd);
			      }
			   }});
    Databases = Databases.rest();
    index++;
   }
   for(int i=0;i<MI_OpenDatabases.length;i++){
      OpenDatabaseMenu.add(MI_OpenDatabases[i]);
      DeleteDatabaseMenu.add(MI_DeleteDatabases[i]);
      Menu_RestoreDatabase.add(MI_RestoreDatabases[i]);
   }


  return true;

}


/* the following methods implements the interface SecondoChangeListener */
public void databasesChanged(){
  updateDatabases();
  updateObjectList();
  updateRelationList();
}

public void objectsChanged(){
   updateObjectList();
   updateRelationList();
}

public void typesChanged(){}

public void databaseOpened(String DBName){
  MI_ListTypes.setEnabled(true);
  MI_ListObjects.setEnabled(true);
  MI_CloseDatabase.setEnabled(true);
  Menu_BasicCommands.setEnabled(true);
  MI_SaveObject.setEnabled(true);
  MI_RestoreObject.setEnabled(true);
  MI_SaveDatabase.setEnabled(true);

  OpenDatabaseMenu.setEnabled(false);
  MI_CreateDatabase.setEnabled(false);
  DeleteDatabaseMenu.setEnabled(false);
  Menu_RestoreDatabase.setEnabled(false);
  OList.enableStoring(true);
  updateObjectList();
  updateRelationList();
}

public void databaseClosed(){
  MI_ListTypes.setEnabled(false);
  MI_ListObjects.setEnabled(false);
  MI_CloseDatabase.setEnabled(false);
  Menu_BasicCommands.setEnabled(false);
  MI_SaveObject.setEnabled(false);
  MI_RestoreObject.setEnabled(false);
  MI_SaveDatabase.setEnabled(false);

  OpenDatabaseMenu.setEnabled(true);
  MI_CreateDatabase.setEnabled(true);
  DeleteDatabaseMenu.setEnabled(true);
  Menu_RestoreDatabase.setEnabled(true);
  OList.enableStoring(false);
  ListOfObjects=null;
  updateRelationList();
}

public void connectionOpened(){
  Menu_Databases.setEnabled(true);
  Menu_Inquiries.setEnabled(true);
  Menu_Transactions.setEnabled(true);
  Menu_ImExport.setEnabled(true);
  Menu_BasicCommands.setEnabled(true);
  Menu_ServerCommand.setEnabled(true);
  databaseClosed();
}

public void connectionClosed(){
  Menu_Databases.setEnabled(false);
  Menu_Inquiries.setEnabled(false);
  Menu_Transactions.setEnabled(false);
  Menu_ImExport.setEnabled(false);
  Menu_BasicCommands.setEnabled(false);
  Menu_ServerCommand.setEnabled(false);
  ListOfObjects = null;
  updateRelationList();
}

/** the Listener for the SecondoCommands in MainMenu */
class Command_Listener implements ActionListener{
        public void actionPerformed(ActionEvent evt){
             if (evt.getSource() instanceof JMenuItem){
                JMenuItem Source = (JMenuItem) evt.getSource();
                boolean ok=false;
                String cmd = "";
                if(Source.equals(MainWindow.this.MI_CreateDatabase)){
		   ok = false;
       MainWindow.ComPanel.showPrompt();
		   MainWindow.ComPanel.appendText("create database <dbname>");
		   return;
		}
                if(Source.equals(MainWindow.this.MI_SaveDatabase)){
                   ok = false;
                   MainWindow.ComPanel.showPrompt();
             		   MainWindow.ComPanel.appendText("save database to <filename>");
		   return;
		}
                if(Source.equals(MainWindow.this.MI_SaveObject)){
                   ok = false;
                   MainWindow.ComPanel.showPrompt();
		               MainWindow.ComPanel.appendText("save <objname> to <filename>");
		   return;
		}
                if(Source.equals(MainWindow.this.MI_RestoreObject)){
                   ok = false;
                   MainWindow.ComPanel.showPrompt();
                   MainWindow.ComPanel.appendText("restore <objname> from <filename>");
		   return;
		}
		if(Source.equals(MainWindow.this.MI_CreateType)){
 		   ok = false;
       MainWindow.ComPanel.showPrompt();
		   MainWindow.ComPanel.appendText("type <name> = <type expr>");
		   return;
   	}
		if(Source.equals(MainWindow.this.MI_DeleteType)){
  		   ok = false;
         MainWindow.ComPanel.showPrompt();
         MainWindow.ComPanel.appendText("delete type <name>");
         return;
   	}
		if(Source.equals(MainWindow.this.MI_CreateObject)){
  		   ok = false;
         MainWindow.ComPanel.showPrompt();
         MainWindow.ComPanel.appendText("create <objname> : <type expr>");
	       return;
   	}
		if(Source.equals(MainWindow.this.MI_UpdateObject)){
  		   ok = false;
         MainWindow.ComPanel.showPrompt();
         MainWindow.ComPanel.appendText("update <objname> := <value expr>");
         return;
   	}
		if(Source.equals(MainWindow.this.MI_DeleteObject)){
  		   ok = false;
         MainWindow.ComPanel.showPrompt();
	       MainWindow.ComPanel.appendText("delete <objname>");
         return;
   	}
		if(Source.equals(MainWindow.this.MI_Let)){
  		   ok = false;
         MainWindow.ComPanel.showPrompt();
         MainWindow.ComPanel.appendText("let <objname> = <value expr>");
         return;
   	}
		if(Source.equals(MainWindow.this.MI_Query)){
  		   ok = false;
         MainWindow.ComPanel.showPrompt();
         MainWindow.ComPanel.appendText("query <value expr>");
         return;
   	}
		if(Source.equals(MainWindow.this.MI_UpdateDatabases)){
         ok = false; // execute no Secondo command
		     if(!MainWindow.this.updateDatabases())
		          Reporter.showError("error in reading databases");
              return;
         }
         if(Source.equals(MainWindow.this.MI_ListDatabases)){
               ok = true;
               cmd = "list databases";
         }else if (Source.equals(MainWindow.this.MI_ListTypes)){
               ok = true;
               cmd ="list types";
         } else if (Source.equals(MainWindow.this.MI_ListTypeConstructors)){
               ok = true;
               cmd ="list type constructors";
         } else if (Source.equals(MainWindow.this.MI_ListObjects)){
               ok = true;
               cmd ="list objects";
         } else if(Source.equals(MainWindow.this.MI_ListOperators)){
               ok = true;
               cmd="list operators";
         } else if(Source.equals(MainWindow.this.MI_ListAlgebras)){
               ok = true;
               cmd = "list algebras";
         }else if(Source.equals(MainWindow.this.MI_CloseDatabase)){
               ok = true;
               cmd = "close database";
         } else if(Source.equals(MainWindow.this.MI_BeginTransaction)){
               ok = true;
               cmd = "begin transaction";
         } else if(Source.equals(MainWindow.this.MI_AbortTransaction)){
                ok = true;
                cmd = "abort transaction";
         }else if(Source.equals(MainWindow.this.MI_CommitTransaction)){
                ok = true;
                cmd = "commit transaction";
         }
         if (ok) {
		            MainWindow.ComPanel.showPrompt();
                MainWindow.ComPanel.appendText(cmd);
                MainWindow.ComPanel.addToHistory(cmd);
                MainWindow.ComPanel.execUserCommand(cmd);
         }
      }
  }
}

public int execCommand(String cmd){
   return ComPanel.internCommand(cmd);
}

public ListExpr getCommandResult(String cmd){
   return ComPanel.getCommandResult(cmd);
}

public boolean execUserCommand(String cmd){
   return ComPanel.execUserCommand(cmd);
}

public boolean execCommand(String cmd, IntByReference errorCode,ListExpr resultList, StringBuffer errorMessage){
     return ComPanel.execCommand(cmd,errorCode,resultList,errorMessage);
}


/** returns a small interface to secondo needed by the 'UpdateViewer'*/
public static UpdateInterface getUpdateInterface(){
	return ComPanel.getUpdateInterface();
}

private static void showLicence(){
    Reporter.showInfo(Licence);
}

private static final String Licence = 
    " Copyright (C) 2004, University in Hagen, \n" +
    " Department of Computer Science,  \n"+
    " Database Systems for New Applications. \n\n"+
    " This is free software; see the source for copying conditions.\n" +
    " There is NO warranty; not even for MERCHANTABILITY or FITNESS \n" +
    " FOR A PARTICULAR PURPOSE.";


}

