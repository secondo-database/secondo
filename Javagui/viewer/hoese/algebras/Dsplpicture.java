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
import javax.swing.event.*;
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
  
/* returns the String representation of this */
public String toString(){
   return Entry;
}

/** reads the data from the value list.
  */
  
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

/* Opens a Frame displaying the picture.
 */
public void displayExtern(){
    Display.setSource(this);
    Display.setVisible(true);    
}

/** returns true, if this picture is displayed external.
  */
public boolean isExternDisplayed(){
   return Display.isVisible() && this.equals(Display.getSource());
}


/** In this frame all pictures are displayed. This must be static
  * to ensure to hold only the data of a single image in the main memory.
  */
private static PictureViewerFrame Display=null; 
private String Entry;
private ListExpr theList;


private static class PictureViewerFrame extends JFrame{

/** Creates a new Frame displaying a single image .
*/
public PictureViewerFrame(){

  getContentPane().setLayout(new BorderLayout());
  SP = new JScrollPane(picture); 
  getContentPane().add(SP,BorderLayout.CENTER);
  CloseBtn = new JButton("Close");
  CloseBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            picture.setImage(null);
            PictureViewerFrame.this.setVisible(false);
       }
  } );
  JPanel ControlPanel = new JPanel();
  FitToFrame = new JCheckBox("Fit to frame");
  FitToFrame.addChangeListener(new ChangeListener(){
       public void stateChanged(javax.swing.event.ChangeEvent evt){
            if(FitToFrame.isSelected()){
                PictureViewerFrame.this.getContentPane().remove(SP);
                PictureViewerFrame.this.getContentPane().add(FitPanel,BorderLayout.CENTER);
            } else{
                PictureViewerFrame.this.getContentPane().remove(FitPanel);
                PictureViewerFrame.this.getContentPane().add(SP,BorderLayout.CENTER);
            }
            PictureViewerFrame.this.invalidate();
            PictureViewerFrame.this.validate();
            PictureViewerFrame.this.repaint();
       }}); 
  FitPanel = new JPanel(){
     public void paint(Graphics g){
          Dimension D = getSize();
          if((image!=null) && (image.getHeight(null)>0) && (image.getWidth(null)>0)){
              int w = image.getWidth(null); 
              int h = image.getHeight(null);
              double sx = ((double) D.width)/w;
              double sy = ((double) D.height)/h;
              double scale = Math.min(sx,sy);
              int dx = (D.width-(int)( scale*w))/2;
              int dy = (D.height-(int)(scale*h))/2; 
              g.drawImage(image,dx,dy,(int)(w*scale),(int)(h*scale),null);
          }
          
     }
  };
  ControlPanel.add(FitToFrame);
  ControlPanel.add(CloseBtn);
  getContentPane().add(ControlPanel,BorderLayout.SOUTH); 
  setSize(640,480); 
}

/** Sets ths Dsplpicture from which the image data comes. **/
public void setSource(Dsplpicture S){
    Source = S;
    try{
       byte[] imageData = Base64Decoder.decode(S.theList.textValue()) ; 
       image = TK.createImage(imageData);
       picture.setImage(image);
       picture.checkImage(image,iob); 
    }catch(Exception e){
       System.err.println("Error in reading picture data");
       e.printStackTrace();
       picture.setImage(null);
    }
    repaint();
}

/** Returns the source of the image data. */
public Dsplpicture getSource(){
     return Source;
}

private JButton CloseBtn;
private Dsplpicture Source;
private static Image image;
private ScrollablePicture picture=new ScrollablePicture();
private JScrollPane SP; // scrooling in a image
private JPanel FitPanel; // fit image to window
private JCheckBox FitToFrame;
private static Toolkit TK = Toolkit.getDefaultToolkit();

/** This ImageObserver repaints the image if it full loaded.
  *
  **/
private ImageObserver iob = new ImageObserver(){
        public boolean imageUpdate(Image img,int infoflags,int x, int y, int width, int height){
            if( (infoflags &  ALLBITS)>0){
                 picture.setImage(img);
                 picture.repaint();
                 FitPanel.repaint();	
            }
            return true;
        }
   };



/** This class provides a image which can be used in a ScrollPane */
private class ScrollablePicture extends Component implements Scrollable{

 /** paints the contained image */
     public void paint(Graphics g){
         super.paint(g);
         if(image!=null)
            g.drawImage(image,0,0,null);
     }

/** returns the size of the contained image */
     public Dimension getPreferredSize(){
        return MyDimension; 
     }
  
     public Dimension getPreferredScrollableViewportSize(){
         return getPreferredSize();
     }

     public int getScrollableUnitIncrement(Rectangle visibleRect, 
                                           int orientation,
                                           int direction){
          return 1;
     }
    
     public int getScrollableBlockIncrement(Rectangle visibleRect,
                                            int orientation,
                                            int direction){
         return 10;
     }

     public boolean getScrollableTracksViewportWidth(){
         return false;
     }
     
     public boolean getScrollableTracksViewportHeight(){
         return false;
     }

   /** Sets the image to manage. */
     public  void setImage(Image image){
         this.image=image;
         if(image==null || image.getHeight(null)<0){ // no picture or not fully loaded
             MyDimension.width=300;
             MyDimension.height = 200;
         } else{
             MyDimension.width = image.getWidth(null);
             MyDimension.height = image.getHeight(null);
             PictureViewerFrame.this.invalidate();
             PictureViewerFrame.this.validate();
             SP.setViewportView(picture);
             PictureViewerFrame.this.repaint();
         }
     
     }

     private Image image=null;
     private Dimension MyDimension=new Dimension(300,200);
  }


} // Class PictureViewerFrame

}



