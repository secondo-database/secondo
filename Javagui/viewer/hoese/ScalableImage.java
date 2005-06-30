package viewer.hoese;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.geom.*;
import java.awt.image.BufferedImage;



/** This class provides an component which displays an image
  * in the size of the bounds of this component
  */


public class ScalableImage extends Component {

   /** Creates a new instance of type ScalableImage without any content. 
     **/
    public ScalableImage () {
       this.image=null;
       this.scaledImage = null;
       this.clippedScaledImage=null;
    }

    /** Draws this component 
      * If an image is available, this image is painted otherwise nothing.
      */
    public void paint(Graphics g) {
      super.paint(g);
      if(scaledImage!=null){
          g.drawImage(scaledImage,0,0,this);
      }else if(clippedScaledImage!=null){
          Rectangle2D b = getBounds();
          int x = (int) (ClipRect.getX()-b.getX());
          int y = (int) (ClipRect.getY()-b.getY());
          g.drawImage(clippedScaledImage,x,y,this); 
      }
    }

   /** Sets a new image for this component 
    */
    public void setImage(BufferedImage img){
        this.image = img;
        if(img!=null){
            Rectangle2D bounds = getBounds();
            double w = bounds.getWidth();
            double h = bounds.getHeight();
            if((long)h*w>MAXPIXELS){
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

    /* Sets the bounds for this components.
     * The contained image is resized to fit into the new 
     * box
     **/
    public void setBounds(int x, int y, int  w, int  h){
        Rectangle2D oldBounds = getBounds();
        if(oldBounds.getWidth()!=w || oldBounds.getHeight()!=h){
            if((long)h*w>MAXPIXELS){
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
        ClipRect=null;    
    } 
    
    /** Returns the managed image */
    public BufferedImage getImage(){ return image;}

    /** Function for implementing the imageobserver interface */
    public boolean imageUpdate(Image img, int infoflags, int x, int y, int width, int height) {
       if((infoflags & ALLBITS) >0){
           repaint();
           return  false;
       }
       return true;
    }

   /** Sets the cliprect */
   public void setClipRect(Rectangle2D ClipRect){
       if(scaledImage!=null || image==null || ClipRect==null){
           this.ClipRect=ClipRect; 
           clippedScaledImage = null;
           changeClipRect();
           return;
       }
       if(clippedScaledImage!=null){ // an image is already created
          if(ClipRect.equals(this.ClipRect)){
              return; // the same cliprect => no changes
          } else{
           clippedScaledImage=null;
         }
       }
       this.ClipRect=ClipRect;
       changeClipRect();
       createClippedScaledImage();      
   }

  /** This method ensure that the cliprect in part of the bounds of this
    * component.
    */
   private void changeClipRect(){
       if(ClipRect==null) return;
       Rectangle2D r = getBounds(); // the cliprectangle cannot be larger than the bounds
       double x1 = Math.max(r.getX(),ClipRect.getX());
       double y1 = Math.max((int)r.getY(),ClipRect.getY());
       double x2 = Math.min( r.getX()+r.getWidth(),ClipRect.getX()+ClipRect.getWidth());
       double y2 = Math.min( r.getY()+r.getHeight(),ClipRect.getY()+ClipRect.getHeight());
       double w = Math.max(1,x2-x1);
       double h = Math.max(1,y2-y1);
       ClipRect.setRect(x1,y1,w,h);
   }


   /**  This function create a clip of the complete image to save memory;
     */ 
   private void createClippedScaledImage(){
      if(image==null){
        clippedScaledImage=null;
        return;
      }
      Rectangle2D r = getBounds();
       // first, we convert the cliprectangle into coordinated in the image
      int iw = image.getWidth();
      int ih = image.getHeight();
      // copmpute the scale factors
      double sfx = ((double)iw)/r.getWidth();
      double sfy = ((double)ih)/r.getHeight();
      int x = (int)((ClipRect.getX()-r.getX())*sfx);
      int y = (int)((ClipRect.getY()-r.getY())*sfy);
      int w = Math.max(1,(int)(ClipRect.getWidth()*sfx));
      int h = Math.max(1,(int)(ClipRect.getHeight()*sfy));
      if(x>iw || y > ih){
        clippedScaledImage=null;
        return;
      }
      if(x+h<0 || y+h<0){
        clippedScaledImage=null;
        return;
      }
      x = Math.max(0,x);
      y = Math.max(0,y);
      if(y+h>ih){
         h = ih - y;
      }
      if(x+w>iw){
         w = iw -x;  
      }
      clippedScaledImage=image.getSubimage(x,y,w,h);
      int cw = ((int)ClipRect.getWidth());
      int ch = (int)ClipRect.getHeight();
      if(cw>0 && ch>0)
         clippedScaledImage=clippedScaledImage.getScaledInstance(cw,ch,Image.SCALE_FAST);
      else
        clippedScaledImage = null;
   } 

   public static long getMaxPixels(){
      return MAXPIXELS;
   }   
   public static void setMaxPixels(long maxpixels){
       // we require a minumum maxpixels of 1 million pixels
       MAXPIXELS = Math.max(1000000,maxpixels);
   }
 
  /* the managed image */
    private BufferedImage image;

   /** A scaled version of the managed image with at most MAXPIXELS
     * pixels. 
     * Using this picture avoids frequently computations on pictures 
     */
    private Image scaledImage;

    /* The maximum number of pixels  for the scaled image */

    private static long MAXPIXELS = 10000000L;

    /* The clip when the numbe rof pixels is to large */
    private Rectangle2D ClipRect;
    /* the viewable clip */
    private Image clippedScaledImage;
  }

