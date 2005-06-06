package viewer.hoese;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.geom.*;
import java.io.File;
import java.awt.image.BufferedImage;


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
      JPanel P2 = new JPanel();
      P2.add(P);
      P2.add(new JLabel());
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
        ImageName.setText(f.getName()); 
     } catch(Exception e){
        viewer.MessageBox.showMessage("Error in creating image from file");
     }
  }
}

private void removeImage(){
     theImage = null;
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


private BufferedImage theImage=null;
private Rectangle2D.Double bounds = new Rectangle2D.Double(0,10,0,10);

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

}

