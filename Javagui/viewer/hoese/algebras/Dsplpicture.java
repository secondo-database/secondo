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
import java.io.*;
import tools.Base64Decoder;
import java.awt.image.*;


/**
 * A displayclass for the html formatted code 
 */
public class Dsplpicture extends DsplGeneric implements ExternDisplay{

 /** Creates a new Instance of this.
   */ 
public  Dsplpicture(){
   if(Display==null){
      Display = new PictureViewerFrame();
   }
}


/** Adds a button to the query which on a Click would be 
  * pop up a window
  **/
public void init (ListExpr type, ListExpr value, QueryResult qr) {
     init(type,0,value,0,qr);
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
     if(value.atomType()!=ListExpr.TEXT_ATOM){
        V =  "error in value ";
        theList = ListExpr.textAtom(V);
     }
     else{
        V =  value.textValue();
        theList = value;
     }
     T=extendString(T,typewidth);
     Entry = T+" : <picture>"; 
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


private static PictureViewerFrame Display=null; 
private String Entry;
private ListExpr theList;


private static class PictureViewerFrame extends JFrame{

public PictureViewerFrame(){

  getContentPane().setLayout(new BorderLayout());
  Display = new JPanel(){
       public void paint(Graphics g){
          super.paint(g);
          if(image!=null){
             g.drawImage(image,0,0,null);
          }
       }
  };

  getContentPane().add(Display,BorderLayout.CENTER);
  CloseBtn = new JButton("Close");
  CloseBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            image=null;
            PictureViewerFrame.this.setVisible(false);
       }
  } );
  
  setSize(640,480); 
}


public void setSource(Dsplpicture S){
    Source = S;
    try{
       byte[] imageData = Base64Decoder.decode(S.theList.textValue()) ; 
       image = TK.createImage(imageData);
       Display.checkImage(image,iob); 
    }catch(Exception e){
       System.err.println("Error in reading picture data");
       e.printStackTrace();
       image = null;
    }
    repaint();
}

public Dsplpicture getSource(){
     return Source;
}

private static JPanel Display;
private JButton CloseBtn;
private Dsplpicture Source;
private Image image;
private static Toolkit TK = Toolkit.getDefaultToolkit();
private static ImageObserver iob = new ImageObserver(){
        public boolean imageUpdate(Image img,int infoflags,int x, int y, int width, int height){
            if( (infoflags &  ALLBITS)>0)
                 Display.repaint();
            return true;
        }
   };

}

}



