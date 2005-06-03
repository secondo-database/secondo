package viewer.hoese;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.geom.*;


/** This class provides an component which displays an image
  * in the size of the bounds of this component
  */


public class ScalableImage extends Component {
    public ScalableImage (Image image) {
      //setBorder( new BevelBorder(BevelBorder.RAISED) );
      this.image = image;
    }

    public void paint(Graphics g) {
      super.paint(g);
      if(scaledImage!=null){
          g.drawImage(scaledImage,0,0,this);
      }
    }

    public void setImage(Image img){
        this.image = img;
        if(img!=null){
            Rectangle2D bounds = getBounds();
            double w = bounds.getWidth();
            double h = bounds.getHeight();
            if((long)h*w>MAXPIXELS){
                System.out.println("image too big, disable background");
                this.scaledImage=null;
                System.gc();
                return;
            } 
            if(h>0 && w>0) 
                  this.scaledImage = img.getScaledInstance((int)w,(int)h,0);
            else
                this.scaledImage = null;
        } else
             scaledImage = null;
        System.gc();
    }

    public void setBounds(int x, int y, int  w, int  h){
        Rectangle2D oldBounds = getBounds();
        if(oldBounds.getWidth()!=w || oldBounds.getHeight()!=h){
            if((long)h*w>MAXPIXELS){
                System.out.println("image too big, disable background");
                this.scaledImage=null;
            } else{ 
               if(w>0 && h>0 && image!=null)
                   this.scaledImage = image.getScaledInstance(w,h,0);
               else
                   this.scaledImage = null; 
            }
            System.gc();
            System.runFinalization();
        }
        super.setBounds(x,y,w,h);
    } 
    
    public Image getImage(){ return image;}

    /** Function for implementing the imageobserver interface */
    public boolean imageUpdate(Image img, int infoflags, int x, int y, int width, int height) {
       if((infoflags & ALLBITS) >0){
           repaint();
           return  false;
       }
       return true;
    }

   public static long getMaxPixels(){
      return MAXPIXELS;
   }   
   public static void setMaxPixels(long maxpixels){
       // we require a minumum maxpixels of 1 million pixels
       MAXPIXELS = Math.max(1000000,maxpixels);
       System.out.println("Set MaxPixels to " + MAXPIXELS);
   }
 
    private Image image;
    private Image scaledImage;
    private static long MAXPIXELS = 10000000L;
    

  }

