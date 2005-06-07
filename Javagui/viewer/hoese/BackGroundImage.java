package viewer.hoese;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.geom.*;
import java.io.File;
import java.awt.image.BufferedImage;
import sj.lang.ListExpr;
import tools.Base64Encoder;
import tools.Base64Decoder;
import java.io.*;

/**
  * This class handles a image object together with an 
  * rectangle describing, where the position of this 
  * image is in worldcoordinates.
  **/

public class  BackGroundImage extends JDialog{


/** Construct this 
  */
  public BackGroundImage(Frame owner){
     // create the dialog
      super(owner,true);
      JPanel P = new JPanel(new GridLayout(6,2));
      P.add(new JLabel("x",JLabel.RIGHT));
      P.add(XTF = new JTextField(6));
      P.add(new JLabel("y",JLabel.RIGHT));
      P.add(YTF = new JTextField(6));
      P.add(new JLabel("width",JLabel.RIGHT));
      P.add(WTF = new JTextField(6));
      P.add(new JLabel("height",JLabel.RIGHT));
      P.add(HTF = new JTextField(6));
      P.add(SelectImageBtn = new JButton("select Image"));
      P.add(RemoveImageBtn = new JButton("remove Image"));
      P.add(ImageName = new JLabel(""));
      getContentPane().setLayout(new BorderLayout());
      JPanel P2 = new JPanel(new GridLayout(2,1));
      JPanel P4 = new JPanel();
      P4.add(P); 
      P2.add(P4);
      imageDisplay = new ImageDisplay();
      P2.add(imageDisplay);
      getContentPane().add(P2,BorderLayout.CENTER);
      JPanel CP = new JPanel(new GridLayout(2,2));
      CP.add(ApplyBtn = new JButton("Apply"));
      CP.add(ResetBtn = new JButton("Reset"));
      CP.add(OkBtn = new JButton("Ok"));
      CP.add(CancelBtn = new JButton("Cancel"));
      getContentPane().add(CP,BorderLayout.SOUTH);
      // assign methods to the buttons
      ActionListener AL = new ActionListener(){
          public void actionPerformed(ActionEvent evt){
               Object s = evt.getSource();
               if(s==null) return;
               if(s.equals(SelectImageBtn))
                  selectImage();
               else if(s.equals(RemoveImageBtn))
                  removeImage();
               else if(s.equals(ApplyBtn))
                  apply();
               else if(s.equals(ResetBtn))
                  reset();
               else if(s.equals(OkBtn)){
                  if(apply())
                     setVisible(false); 
               } else if(s.equals(CancelBtn)){
                   reset();
                   setVisible(false);
               }
          }
      };
     ApplyBtn.addActionListener(AL);
     CancelBtn.addActionListener(AL);
     ResetBtn.addActionListener(AL);
     SelectImageBtn.addActionListener(AL);
     RemoveImageBtn.addActionListener(AL);
     OkBtn.addActionListener(AL);
     setSize(640,480);
     reset();// put the initail values into the textfields
  }

private void selectImage(){
  if(FC==null)
     FC = new JFileChooser();
  if(FC.showOpenDialog(this)==FC.APPROVE_OPTION){
     File f = FC.getSelectedFile();
     try{
        BufferedImage img = javax.imageio.ImageIO.read(f);
        if(img==null){
            viewer.MessageBox.showMessage("Error in creating image from file");
            return;
        }
        theImage = img;
        imageDisplay.setImage(theImage);
        ImageName.setText(f.getName());
     } catch(Exception e){
        viewer.MessageBox.showMessage("Error in creating image from file");
     }
  }
}

private void removeImage(){
     theImage = null;
     imageDisplay.setImage(theImage);
     ImageName.setText("");
}

private boolean  apply(){
    double tmpx,tmpy,tmpw,tmph;
    try{
      tmpx = Double.parseDouble(XTF.getText());
      tmpy = Double.parseDouble(YTF.getText());
      tmpw = Double.parseDouble(WTF.getText());
      tmph = Double.parseDouble(HTF.getText());
    }catch(Exception e){
        viewer.MessageBox.showMessage("All entries have to be real numbers. ");
        return false;
    }
    if(tmpw<=0 || tmph <=0){  
        viewer.MessageBox.showMessage("width and height have to be greater than zero. ");
        return false;
    }
    bounds.setRect(tmpx,tmpy,tmpw,tmph);
    return true;
}

private void reset(){
    XTF.setText(""+bounds.getX());
    YTF.setText(""+bounds.getY());
    WTF.setText(""+bounds.getWidth());
    HTF.setText(""+bounds.getHeight()); 
}

public void setImage(BufferedImage img){
   theImage = img;
   imageDisplay.setImage(theImage);
   if(img==null)
      ImageName.setText("");
   else
      ImageName.setText("internal");
}


public boolean setBBox(double x, double y, double w, double h){
    if(w<=0 || h<=0) return false;
    bounds.setRect(x,y,w,h);
    reset(); // put the values into the textfields
    return true; 
}

/** 
  * Returns the image of this BackgroundImage 
  * If no image is available, the result will be null.
  */
public BufferedImage getImage(){
    return theImage;
}

/** 
  * returns the bounding box of this Image
  */
public Rectangle2D.Double getBBox(){
   return bounds;
}

/** Returns the nested list representation of this BackgroundImage */
public ListExpr toListExpr(String BackgroundImagePath){
   ListExpr ImageList;
   if(theImage==null){
       ImageList = ListExpr.theEmptyList();
   }else{
      try{
         // select a non-existent fileName for the image
         File F = new File(BackgroundImagePath);
         if(!F.exists()){
            viewer.MessageBox.showMessage("Error in creating background image \n"+
                                          " directory "+BackgroundImagePath +
                                          "don't exists");
            ImageList = ListExpr.theEmptyList();
         } else if(!F.isDirectory()){
            viewer.MessageBox.showMessage("Error in creating background image \n"+
                                          BackgroundImagePath +
                                          "is not a directory");
            ImageList = ListExpr.theEmptyList();
         } else{
            if(!BackgroundImagePath.endsWith(File.separator))
                BackgroundImagePath+=File.separator;
             String FileName =  ".background";
             int BGNumber = -1;
             do{
                BGNumber++;
                F = new File(BackgroundImagePath+FileName+BGNumber+".png");
             }while(F.exists());
            javax.imageio.ImageIO.write(theImage,"png",F);
            ImageList = ListExpr.textAtom(FileName+BGNumber+".png"); 
         }
      }catch(Exception e){
         viewer.MessageBox.showMessage("cannot create the ListExpr for Background Image ");
         e.printStackTrace();
         ImageList=ListExpr.theEmptyList();
      }
   }
   return ListExpr.fiveElemList(
                ListExpr.realAtom(bounds.getX()),
                ListExpr.realAtom(bounds.getY()),
                ListExpr.realAtom(bounds.getWidth()),
                ListExpr.realAtom(bounds.getHeight()),
                ImageList);  

}

/** Reads the values from this component from a ListExpr **/
public boolean readFromListExpr(ListExpr le,String BackgroundImagePath){
   if(le.listLength()!=5)
      return false;
   if(le.first().atomType()!=le.REAL_ATOM    ||
      le.second().atomType()!=le.REAL_ATOM   ||
      le.third().atomType()!=le.REAL_ATOM    ||
      le.fourth().atomType()!=le.REAL_ATOM)  
      return false;
   
   ListExpr ImageList = le.fifth();
   BufferedImage bi=null;
   if(ImageList.atomType()==le.NO_ATOM){
       if(!ImageList.isEmpty())
           return false;
   }else{
     if(ImageList.atomType()!=le.TEXT_ATOM)
         return false;
     // try to restore the background from given file
     if(!BackgroundImagePath.endsWith(File.separator))
         BackgroundImagePath+=File.separator;
     File F = new File(BackgroundImagePath+le.fifth().textValue());
     if(!F.exists()){
        viewer.MessageBox.showMessage("Background File not found");
        bi = null;;
     } else{ // ok file exists-> read the image
        try{
           bi = javax.imageio.ImageIO.read(F);
        }catch(Exception e){
             viewer.MessageBox.showMessage("Error in loading background-Image");
             bi=null;   
        }
     }
  }
  
  if(!setBBox(le.first().realValue(),le.second().realValue(),
      le.third().realValue(),le.fourth().realValue())){
      return false;
   }
   setImage(bi);
   reset();
   return true;
}




private BufferedImage theImage=null;
private ImageDisplay imageDisplay;
private Rectangle2D.Double bounds = new Rectangle2D.Double(0,0,10,10);

private JTextField XTF;
private JTextField YTF;
private JTextField WTF;
private JTextField HTF;
private JButton SelectImageBtn;
private JButton RemoveImageBtn;
private JLabel ImageName;
private JButton ResetBtn;
private JButton ApplyBtn;
private JButton OkBtn;
private JButton CancelBtn;
private JFileChooser FC = null;      

private class ImageDisplay extends JLabel implements java.awt.image.ImageObserver{

  public ImageDisplay(){}
  public ImageDisplay(String str){super(str);}
  
  public void paint(Graphics g){
    super.paint(g);
    if(theImage!=null){
       Rectangle2D bounds = getBounds();
       int w = theImage.getWidth(this);
       int h= theImage.getHeight(this);
       double sf = Math.min(bounds.getWidth()/w,bounds.getHeight()/h);
       int w1 = (int) (sf*w);
       int h1 = (int) (sf*h);
       int x = (int) ((bounds.getWidth()-w1) /2);
       int y = (int) ((bounds.getHeight()-h1) /2);
       g.drawImage(theImage,x,y,w1,h1,this);
    }  
  }
 /** Function for implementing the imageobserver interface */
    public boolean imageUpdate(Image img, int infoflags, int x, int y, int width, int height) {
       if((infoflags & ALLBITS) >0){
           repaint();
           return  false;
       }
       return true;
    }
 
  public void setImage(Image img){
        theImage = img;
        repaint();
  }
  private Image theImage;

}


}

