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


package  viewer.hoese.algebras.fileviewers;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import java.io.*;
import java.awt.image.*;
import tools.Reporter;



public class JPGViewer extends FileViewer{

/** Creates a new Frame displaying a single image.
*/
public JPGViewer(){

  setLayout(new BorderLayout());
  SP = new JScrollPane(picture); 
  add(SP,BorderLayout.CENTER);
  JPanel ControlPanel = new JPanel();
  FitToFrame = new JCheckBox("Fit to frame");
  FitToFrame.addChangeListener(new ChangeListener(){
       public void stateChanged(javax.swing.event.ChangeEvent evt){
            if(FitToFrame.isSelected()){
                JPGViewer.this.remove(SP);
                JPGViewer.this.add(FitPanel,BorderLayout.CENTER);
                DontGrowCB.setEnabled(true);
            } else{
                JPGViewer.this.remove(FitPanel);
                JPGViewer.this.add(SP,BorderLayout.CENTER);
                DontGrowCB.setEnabled(false);
            }
            JPGViewer.this.invalidate();
            JPGViewer.this.validate();
            JPGViewer.this.repaint();
       }});
  DontGrowCB = new JCheckBox("only shrink");
  DontGrowCB.setEnabled(false);
  DontGrowCB.addChangeListener(new ChangeListener(){
       public void stateChanged(ChangeEvent e){
           FitPanel.repaint();
       }
  });  
  FitPanel = new JPanel(){
     public void paint(Graphics g){
          super.paint(g);
          Dimension D = getSize();
          if((image!=null) && (image.getHeight(null)>0) && (image.getWidth(null)>0)){
              int w = image.getWidth(null); 
              int h = image.getHeight(null);
              double sx = ((double) D.width)/w;
              double sy = ((double) D.height)/h;
              double scale = Math.min(sx,sy);
              if(DontGrowCB.isSelected()){
                 scale = Math.min(scale,1.0);
              }
              int dx = (D.width-(int)( scale*w))/2;
              int dy = (D.height-(int)(scale*h))/2; 
              g.drawImage(image,dx,dy,(int)(w*scale),(int)(h*scale),null);
          }
          
     }
  };
  ControlPanel.add(FitToFrame);
  ControlPanel.add(DontGrowCB);
  add(ControlPanel,BorderLayout.SOUTH); 
}

public boolean canDisplay(File f){
 if(f==null) {
   return false;
 } if(! f.exists()){ 
   return false;
 } else {
   
   String n = f.getName().toLowerCase();
   return n.endsWith("jpg") || n.endsWith("jpeg");
 }
}

public boolean display(File f){
  try{
       image = javax.imageio.ImageIO.read(f);
       picture.setImage(image);
       picture.checkImage(image,iob);
       repaint(); 
       return true;
  } catch(Exception e){
     Reporter.debug(e);
     return false;
  }
}


private static Image image;
private ScrollablePicture picture=new ScrollablePicture();
private JScrollPane SP; // scrooling in a image
private JPanel FitPanel; // fit image to window
private JCheckBox FitToFrame;
private JCheckBox DontGrowCB;
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
             JPGViewer.this.invalidate();
             JPGViewer.this.validate();
             SP.setViewportView(picture);
             JPGViewer.this.repaint();
         }
     
     }

     private Image image=null;
     private Dimension MyDimension=new Dimension(300,200);
  }


} // Class JPGViewer 




