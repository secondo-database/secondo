package gui;


import javax.swing.*;
import java.util.Vector;
import java.awt.*;
import java.awt.event.*;

public class HelpScreen extends JDialog{

public final int GUI_COMMANDS = 0;
public final int SECONDO_COMMANDS = 1;


private JScrollPane ScrollPane= new JScrollPane();
private JList ServerCommands;
private JList GuiCommands;
private JButton OkBtn = new JButton("close");


public HelpScreen(Frame F){
  super(F,true);
  init();
  getContentPane().setLayout(new BorderLayout());
  JPanel P = new JPanel();
  P.add(OkBtn);
  getContentPane().add(P,BorderLayout.SOUTH);
  getContentPane().add(ScrollPane,BorderLayout.CENTER);
  setSize(400,400);
  OkBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        HelpScreen.this.setVisible(false);
     }
  }); 

}

public void setMode(int mode){
  if (mode==SECONDO_COMMANDS)
      ScrollPane.setViewportView(ServerCommands);
  if (mode==GUI_COMMANDS)
     ScrollPane.setViewportView(GuiCommands); 
}


private void init(){
 Vector SC = new Vector(20);
 SC.add("list databases");
 SC.add("list types");
 SC.add("list type constructors");
 SC.add("list objects");
 SC.add("list operators");
 SC.add("--------------");
 SC.add("create database <identifier> ");
 SC.add("delete database <identifier> ");
 SC.add("open database <identifier>");
 SC.add("close database");
 SC.add("save database to <filename>");
 SC.add("restore database <identifier> from <filename>");
 SC.add("extend database <identifier> from <filename>");
 SC.add("extend and update database <identifier> from <filename>");
 SC.add("save object <identifier> to <filename>");
 SC.add("add object <identifier> from <filename>");
 SC.add("export object <identifier> to <filename>");
 SC.add("import object <identifier> from <filename>");
 SC.add("---------------");
 SC.add("type <identifier> = <type expression>");
 SC.add("delete type <identifier>");
 SC.add("create <identifier> : <type expression>");
 SC.add("update <identifier> := <value expression>");
 SC.add("delete <identifier>");
 SC.add("let <identifier> = <value expression>");
 SC.add("query <value expression>");
 SC.add("----------------");
 SC.add("begin transaction");
 SC.add("commit transaction");
 SC.add("abort transaction");
 ServerCommands= new JList(SC);
 
 Vector GC = new Vector(20);
 GC.add("gui exit");
 GC.add("gui addViewer <ViewerName>");
 GC.add("gui selectViewer <ViewerName> ");
 GC.add("gui clearHistory");
 GC.add("gui showObject <ObjectName>");
 GC.add("gui showAll");
 GC.add("gui hideObject <ObjectName>");
 GC.add("gui hideAll");
 GC.add("gui removeObject <ObjectName>");
 GC.add("gui clearObjectList ");
 GC.add("gui saveObject <ObjectName> ");
 GC.add("gui loadObject ");
 GC.add("gui storeObject <ObjectName>");
 GC.add("gui connect ");
 GC.add("gui disconnect ");
 GC.add("gui serverSettings ");
 GC.add("gui renameObject <oldName> -> <newName> ");
 GC.add("gui onlyViewer ");
 GC.add("gui executeFile [-i] filename");
 GC.add("    -i = ignore errors");
 GC.add("gui listCommands ");
 GuiCommands = new JList(GC);
}


}

