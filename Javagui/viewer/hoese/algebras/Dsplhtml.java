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


package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.EditorKit;


/**
 * A displayclass for the html formatted code 
 */
public class Dsplhtml extends DsplGeneric implements ExternDisplay{

 /** Creates a new Instance of this.
   */ 
public  Dsplhtml(){
   Text = "";
   if(Display==null){
      Display = new HTMLViewerFrame();
   }
}


/** Adds a button to the query which on a Click would be 
  * pop up a window
  **/
 public void init (ListExpr type, ListExpr value, QueryResult qr) {
     if (value.listLength()==1)// Textatom within a list
         value = value.first();
     if(value.atomType()!=ListExpr.TEXT_ATOM)
        qr.addEntry(new String(type.symbolValue()) + ": error in value ");
     else{
        Text = value.textValue();
        Entry = type.symbolValue()+" : double click to display";
        qr.addEntry(this);
     }
     return;
  }
  

public String toString(){
   return Entry;
}

public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V;

     if (value.listLength()==1)
     value = value.first();
     if(value.atomType()!=ListExpr.TEXT_ATOM)
        V =  "error in value ";
     else
        V =  value.textValue();
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     Text = value.textValue();
     Entry = T+" : double click to display";
     qr.addEntry(this);
     return;

  }


public void displayExtern(){
    Display.setSource(this);
    Display.setVisible(true);    
}

public boolean isExternDisplayed(){
   return Display.isVisible() && this.equals(Display.getSource());
}


private static HTMLViewerFrame Display=null; 
private String Text;
private String Entry;


private static class HTMLViewerFrame extends JFrame{

public HTMLViewerFrame(){

  getContentPane().setLayout(new BorderLayout());
  Display = new JEditorPane();
  if(EKText==null){
      EKText = Display.createEditorKitForContentType("text/plain");
      EKHtml = Display.createEditorKitForContentType("text/html");
  }

  JScrollPane ScrollPane = new JScrollPane(Display);
  getContentPane().add(ScrollPane,BorderLayout.CENTER);
  CloseBtn = new JButton("Close");
  CloseBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            HTMLViewerFrame.this.setVisible(false);
       }
  } );
  
  ShowSourceBtn = new JButton(SHOWSOURCE);
  ShowSourceBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
            if(ShowSourceBtn.getText().equals(SHOWSOURCE)){ // switch to source
                 String Text = TheText;// Text without automatic changes from editor
                 Display.setEditorKit(EKText);
                 Display.setText(Text);
                 Display.setEditable(true);
                 ShowSourceBtn.setText(SHOWFORMATTED);
                 Display.setCaretPosition(0);
            } else{ // switch to html view
                 String Text = Display.getText();
                 TheText = Text;
                 Display.setEditorKit(EKHtml);
                 Display.setEditable(false);
                 Display.setText(Text);
                 ShowSourceBtn.setText(SHOWSOURCE);
                 Display.setCaretPosition(0); 
           }
        }      
  } ); 

  JPanel ControlPanel = new JPanel();
  ControlPanel.add(CloseBtn);
  ControlPanel.add(ShowSourceBtn);  
  getContentPane().add(ControlPanel,BorderLayout.SOUTH);
  setSize(640,480); 
}

public void setSource(Dsplhtml S){
    Source = S;
    Display.setEditorKit(EKHtml);
    Display.setEditable(false); 
    TheText = S.Text;
    Display.setText(TheText);
    Display.setCaretPosition(0);// go to top 
    ShowSourceBtn.setText(SHOWSOURCE);
}

public Dsplhtml getSource(){
     return Source;
}

private JEditorPane Display;
private JButton CloseBtn;
private Dsplhtml Source;
private JButton ShowSourceBtn;
private String TheText;
private final static String SHOWSOURCE = "Show Source";
private final static String SHOWFORMATTED = "Show Formatted";
private static EditorKit EKText=null;
private static EditorKit EKHtml=null;

}

}



