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


/**
 * A displayclass for the display texts 
 */
public class Dspltext extends DsplGeneric implements ExternDisplay{

 /** Creates a new Instance of this.
   */ 
public  Dspltext(){
   Text = "";
   if(Display==null){
      Display = new TextViewerFrame();
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
        if(Text.length()<MAX_DIRECT_DISPLAY_LENGTH){
          Entry = type.symbolValue()+" : "+Text;
          qr.addEntry(Entry); // because we add an string the extern display fucntions are
                         // not called
        } else{ // big text atom -> enable external view
           Entry = type.symbolValue()+" : double click to display";
           qr.addEntry(this);
        }
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
     if(Text.length()<MAX_DIRECT_DISPLAY_LENGTH){
        Entry = T + " : "+ V ;
        qr.addEntry(Entry);
     } else{
         Entry = T+" : double click to display";
         qr.addEntry(this);
     }
     return;

  }


public void displayExtern(){
    Display.setSource(this);
    Display.setVisible(true);    
}

public boolean isExternDisplayed(){
   return Display.isVisible() && this.equals(Display.getSource());
}


private static TextViewerFrame Display=null; 
private String Text;
private String Entry;


private static class TextViewerFrame extends JFrame{

public TextViewerFrame(){
  getContentPane().setLayout(new BorderLayout());
  Display = new JEditorPane();
  JScrollPane ScrollPane = new JScrollPane(Display);
  getContentPane().add(ScrollPane,BorderLayout.CENTER);
  CloseBtn = new JButton("Close");
  CloseBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            TextViewerFrame.this.setVisible(false);
       }
  } );
  getContentPane().add(CloseBtn,BorderLayout.SOUTH);
  setSize(640,480); 
}


public void setSource(Dspltext S){
    Source = S;
    Display.setEditable(false); 
    Display.setText(S.Text);
}

public Dspltext getSource(){
     return Source;
}

private JEditorPane Display;
private JButton CloseBtn;
private Dspltext Source;

}

private final int MAX_DIRECT_DISPLAY_LENGTH=20;

}



