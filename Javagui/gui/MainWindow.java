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

public class MainWindow extends JFrame implements ResultProcessor,ViewerControl{

public final String CONFIGURATION_FILE="gui.cfg";

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

private JMenu ServerCommand;
private JMenu ServerCommand_List;
private JMenuItem MI_ListDatabases;
private JMenuItem MI_ListTypes;
private JMenuItem MI_ListTypeConstructors;
private JMenuItem MI_ListObjects;
private JMenuItem MI_ListOperators;

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


/* display a message in a new frame */
public void showMessage(String Text){
  OptionPane.showMessageDialog(this,Text);
}

/* return this */
public Frame getMainFrame(){ return this; }


/* creates a new MainWindow */
public MainWindow(String Title){
  super(Title);
  setSize(800,600);
  OptionPane = new JOptionPane();
  ServerDlg = new ServerDialog(this); 
  MyHelp = new HelpScreen(this);
  this.getContentPane().setLayout(new BorderLayout());
  PanelTop = new JPanel(new BorderLayout(),true);
  ComPanel = new CommandPanel(this);
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
  try{
    FileInputStream CFG = new FileInputStream(CONFIGURATION_FILE);
    Config.load(CFG);
    CFG.close();

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
           System.out.println("cannot load viewer:"+ViewerName+"\n");
           }
        }
   } 
   
   ObjectDirectory= Config.getProperty("OBJECT_DIRECTORY");
   if(ObjectDirectory!=null){
      OList.setObjectDirectory(new  File(ObjectDirectory));
   }
   else
      ObjectDirectory=".";
   
   String HistoryDirectory= Config.getProperty("HISTORY_DIRECTORY");
   if(HistoryDirectory!=null)      
      FC_History.setCurrentDirectory(new File(HistoryDirectory));
      
   
 
  } catch(Exception e){
    System.out.println("I can't read the configuration-file: "+CONFIGURATION_FILE);
  }

  ComPanel.setConnection(UserName,PassWd,ServerName,ServerPort);
  if (StartConnection){
      if (!ComPanel.connect()) 
         showMessage("I can't find a Secondo-server");
  }
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
      MI_Viewer.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent e){
            int index = ViewerMenuItems.indexOf(e.getSource());
            if (index>=0)
                MainWindow.this.setViewerindex(index);
         }}); 
      viewersChanged();
   }
   setViewer(NewViewer); 
}


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
  * addViewer <ViewerName>
  * selectViewer <ViewerName>
  * clearHistory
  * saveHistory
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
       if(ComPanel.connect())
          ComPanel.appendText("you are connected to a secondo server");
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
  }
  else {
    ComPanel.appendText("unknow gui command \n input \"gui listCommands\" to get a list of available commands");
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
    System.out.println(e);
    e.printStackTrace();
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
     catch(Exception e) {showMessage("error when update the menu");}
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
}


/* show the result of a command */ 
public void processResult(String command,ListExpr ResultList,IntByReference ErrorCode,
                        IntByReference ErrorPos,StringBuffer ErrorMessage){
  if (ErrorCode.value!=0){
    ComPanel.appendText("Error: " + ErrorMessage+"\n");
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
               if (CurrentViewer.canDisplay(o)){
                   CurrentViewer.addObject(o);
                   OList.updateMarks();
               }
               else {   // search a Viewer to display the result
                  SecondoViewer Cand=null;
                  boolean found = false;
                  for(int i=0;i<AllViewers.size()&!found;i++){
                     Cand = (SecondoViewer) AllViewers.get(i);
                     if (Cand.canDisplay(o))
                         found = true;
                  }
                  if(found){
                     setViewer(Cand);
                     CurrentViewer.addObject(o);
                     OList.updateMarks();
                  }
                  else
                     showMessage("no Viewer loaded to display this result");
               }
            } catch(Exception e){showMessage("an error is occurred (in current viewer)");}
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
        System.out.println("error in current Viewer "+CurrentViewer+" method: isDisplayed");
        return false; 
     }
} 

/** shows SO in current Viewer if possible **/
public boolean showObject(SecondoObject SO){
  if (CurrentViewer==null) 
      return false;
  else
     try{return CurrentViewer.addObject(SO);}
     catch(Exception e){
        System.out.println("error in Viewer :"+CurrentViewer+" method addObject");
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

   JMenu MI_ExecuteFile = new JMenu("execute file");
   ProgramMenu.add(MI_ExecuteFile);
   
   MI_ExecuteFile_HaltOnError = new JMenuItem("halt on error");
   MI_ExecuteFile_IgnoreErrors = new JMenuItem("ignore errors");
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
   

   JMenu HistoryMenu = new JMenu("history");
   ProgramMenu.add(HistoryMenu);
   MI_SaveHistory=HistoryMenu.add("save history");
   MI_ClearHistory=HistoryMenu.add("clear history");
   JMenu LoadHistoryMenu = new JMenu("load");
   HistoryMenu.add(LoadHistoryMenu);
   MI_ExtendHistory = LoadHistoryMenu.add("append");
   MI_ReplaceHistory = LoadHistoryMenu.add("replace");
   

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
          ComPanel.connect();
       }});
   MI_Disconnect.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          ComPanel.disconnect();
       }});

   MI_Settings.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){ 
       showServerSettings();
    }}); 

   ServerCommand = new JMenu("Command");
   ServerMenu.add(ServerCommand);
   ServerCommand_List = new JMenu("list");
   ServerCommand.add(ServerCommand_List);
   MI_ListDatabases=ServerCommand_List.add("databases");
   MI_ListTypes = ServerCommand_List.add("types");
   MI_ListTypeConstructors = ServerCommand_List.add("type constructors");
   MI_ListObjects = ServerCommand_List.add("objects");
   MI_ListOperators = ServerCommand_List.add("operators");
   Command_Listener Com_Listener = new Command_Listener();
   MI_ListDatabases.addActionListener(Com_Listener);
   MI_ListTypes.addActionListener(Com_Listener);
   MI_ListTypeConstructors.addActionListener(Com_Listener);
   MI_ListObjects.addActionListener(Com_Listener);
   MI_ListOperators.addActionListener(Com_Listener);

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




/** load a new History,
  * if replace is true, the old History is make clear before
  * load the new History
  */
public void loadHistory(boolean replace){

  if(FC_History.showSaveDialog(this)==JFileChooser.APPROVE_OPTION){
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



/** the Listener for the SecondoCommands in MainMenu */
class Command_Listener implements ActionListener{
        public void actionPerformed(ActionEvent evt){
             if (evt.getSource() instanceof JMenuItem){
                JMenuItem Source = (JMenuItem) evt.getSource();
                boolean ok=false;
                String cmd = "";
                if (Source.equals(MainWindow.this.MI_ListDatabases)){
                    ok = true;
                    cmd = "list databases";
                } 
                if (Source.equals(MainWindow.this.MI_ListTypes)){
                    ok = true;
                    cmd ="list types";
                }

                if (Source.equals(MainWindow.this.MI_ListTypeConstructors)){
                    ok = true;
                    cmd ="list type constructors";
                }
                if (Source.equals(MainWindow.this.MI_ListObjects)){
                    ok = true;
			  cmd ="list objects";
                } 
                if(Source.equals(MainWindow.this.MI_ListOperators)){
                    ok = true;
			  cmd="list operators";
                }
     
                if (ok) {
                    //MainWindow.this.ComPanel.appendText(cmd+"\n");
                    MainWindow.this.ComPanel.execUserCommand(cmd);
                }
             }
        } 
       }  


}




