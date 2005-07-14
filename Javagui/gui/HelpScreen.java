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


import javax.swing.*;
import java.util.Vector;
import java.awt.*;
import java.awt.event.*;

public class HelpScreen extends JDialog{

public final static int GUI_COMMANDS = 0;
public final static int SECONDO_COMMANDS = 1;


private JScrollPane ScrollPane= new JScrollPane();
private JList ServerCommands;
private JList GuiCommands;
private JButton OkBtn = new JButton("close");


public HelpScreen(Frame F){
  super(F,false);
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
  if (mode==SECONDO_COMMANDS){
      ScrollPane.setViewportView(ServerCommands);
      setTitle("SECONDO commands");
  }
  if (mode==GUI_COMMANDS){
     ScrollPane.setViewportView(GuiCommands);
     setTitle("Gui commands");
  }
}


private void init(){
 Vector SC = new Vector(30);
 SC.add("list databases");
 SC.add("list types");
 SC.add("list type constructors");
 SC.add("list objects");
 SC.add("list operators");
 SC.add("list algebras");
 SC.add("list algebra <identifier>");
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
 GC.add("gui clearAll");
 GC.add("gui addViewer <ViewerName>");
 GC.add("gui selectViewer <ViewerName> ");
 GC.add("gui clearHistory");
 GC.add("gui loadHistory [-r]");
 GC.add("    -r : replace the actual history");
 GC.add("gui saveHistory");
 GC.add("gui showObject <ObjectName>");
 GC.add("gui showAll");
 GC.add("gui hideObject <ObjectName>");
 GC.add("gui hideAll");
 GC.add("gui removeObject <ObjectName>");
 GC.add("gui clearObjectList ");
 GC.add("gui saveObject <ObjectName> ");
 GC.add("gui loadObject ");
 GC.add("gui setObjectDirectory <directory>");
 GC.add("gui loadObjectFrom <FileName>");
 GC.add("    the file should be inthe ObjectDirectory");
 GC.add("gui storeObject <ObjectName>");
 GC.add("gui connect ");
 GC.add("gui disconnect ");
 GC.add("gui status");
 GC.add("gui serverSettings ");
 GC.add("gui renameObject <oldName> -> <newName> ");
 GC.add("gui onlyViewer ");
 GC.add("gui executeFile [-i] filename");
 GC.add("    -i = ignore errors");
 GuiCommands = new JList(GC);
}


}



