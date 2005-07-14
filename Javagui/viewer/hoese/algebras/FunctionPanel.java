package viewer.hoese.algebras;

import javax.swing.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;

public class FunctionPanel extends JPanel{

public FunctionPanel(){

  MML = new MouseMotionAdapter(){
     public void mouseMoved(MouseEvent e){
        lastX = e.getX();
        lastY = e.getY();
        repaint();
     }
     public void mouseDragged(MouseEvent evt){
        mouseMoved(evt);
     }
  };
  addComponentListener(new ComponentAdapter(){
      public void componentResized(ComponentEvent e){
         GPcomputed=false;
         repaint();
      }
  });

}


/** If enabled is set to true, around the cursur, a cross will be displayed.
  * The cross takes the whole height of this panel and a width several pixels.
  * This makes it possible to view the y value at the current position.
  **/
public void showCross(boolean enabled){
    if(enabled){
        removeMouseMotionListener(MML); 
        addMouseMotionListener(MML);
        setCursor(crossCursor);
    } 
    else{
        removeMouseMotionListener(MML);
        setCursor(defaultCursor);
    }
    crossEnabled=enabled; 
}


/** Enables displaying of the y coordinate at the current mouse position.
  * Actually, the coordinate is only displayed if also the cross is activated.
  **/
public void showY(boolean enabled){
    yEnabled=enabled;
}


public boolean isCrossEnabled(){
      return crossEnabled;
}

public boolean isYEnabled(){
    return yEnabled;
}



public void paint(Graphics g){
    super.paint(g);
    if(function==null)
       return;

    width=getWidth()-2*borderSize;
    int height=getHeight()-2*borderSize;
    if(!GPcomputed){
       if(!y_computed)
           compute_y(width,height);
       double dx = xmax-xmin;
       boolean first = true;
       GP = new GeneralPath();
       Double y;
       for(int ix=0;ix<width;ix++){
         double x = xmin+dx*ix/width;
         if((y=function.getValueAt(x))!=null){
             if(first){
                GP.moveTo((float)ix,(float)(y.doubleValue()));
                first=false; 
             } else{
                GP.lineTo((float)ix,(float)(y.doubleValue()));
             }    
         } else{ // unfefined state
            first = true;
         }
       }
       GP.transform(at);
       GPcomputed=true;
    }
    Graphics2D g2 = (Graphics2D) g;
    if(GP!=null)
      g2.draw(GP);
    if(crossEnabled){
        g.drawLine(lastX,0,lastX,height+2*borderSize);
     //   g.drawLine(lastX-20,lastY,lastX+20,lastY);
    }
    if(yEnabled){
        double my = atinv[1]*lastX+atinv[3]*lastY+atinv[5]; // position of the mouse
        String Label = ""+my;
        // Rectangle2D R  = g.getFont().getStringBounds(Label,g2.getFontRenderContext());
        // g.drawString(""+my,lastX+2,lastY+(int)R.getHeight());
         g.drawString(Label,lastX+2,lastY-2);
    }
}


public boolean getOrig(int mouseX,int mouseY,java.awt.geom.Point2D.Double result,Point2D.Double coords){
   if(atinv==null)
        return false;
   if(function==null) 
        return false;
   if(width<=0)
        return false; 
   double pixSize = (xmax-xmin)/width;
   double x = (mouseX-borderSize)*pixSize+xmin;
   double my = atinv[1]*mouseX+atinv[3]*mouseY+atinv[5]; // position of the mouse
   Double y = function.getValueAt(x);
   if(y==null) 
        return false;
   result.x=x;
   result.y=y.doubleValue();
   coords.x = x;
   coords.y = my;
   
   return true;
}


public void setFunction(Function function){
    this.function=function;
    GPcomputed=false;
}

public boolean setInterval(double xmin,double xmax){
    if(xmin==xmax)
      return false;
     GPcomputed=false;
    if(xmin>xmax){
       this.xmax=xmin;
       this.xmin=xmax;
    } else{
       this.xmin=xmin;
       this.xmax=xmax;
    }
    return true;
}


public void setVirtualSize(double x, double  y){
   GPcomputed=false;
   setPreferredSize(new Dimension((int)x,(int)y));
   revalidate();
}


/** computes the minumum and maximum y value
  */
private void compute_y(int width,int height){
   boolean first=true;
   double dx = xmax-xmin;
   ymin=ymax=0;
   Double y;
   for(int i=0;i<width;i++){
       double x = xmin + dx*i/width;
       if((y=function.getValueAt(x))!=null){
          double y1 = y.doubleValue();
          if(first){
               ymin=ymax=y1;
               first=false;
          }else{
               if(y1<ymin) ymin=y1;
               if(y1>ymax) ymax=y1;
          }
       } 
   }
   double dy = ymax-ymin;
   // now, the bounding box of the function is determined
   // we can compute an affine transformation for bringing the
   // function to screen
   double scaleX =  1.0; //width/dx;  
   double scaleY =  height/dy;
   double tx = 0; // -xmin;
   double ty = -ymin;
   AffineTransform Flip = new AffineTransform();
   at.setTransform(scaleX,0,0,scaleY,scaleX*tx,scaleY*ty); 
   at.preConcatenate(AffineTransform.getTranslateInstance(0,-height));
   at.preConcatenate(AffineTransform.getScaleInstance(1,-1));
   AffineTransform trans = (AffineTransform.getTranslateInstance(borderSize,borderSize));
   at.preConcatenate(trans);
   try{
      at.createInverse().getMatrix(atinv);
   }catch(Exception e){
       atinv=null;
   }
}


private Function function=null;
private double xmin=0;
private double xmax=1;
private boolean y_computed=false;
private double ymin;
private double ymax;
private AffineTransform at=new AffineTransform();
private double[] atinv=new double[6];
private GeneralPath GP = null;
private boolean crossEnabled=false;
private int lastX;
private int lastY;
private boolean yEnabled=false;
private MouseMotionListener MML;
private Cursor defaultCursor = new Cursor(Cursor.DEFAULT_CURSOR);
private Cursor crossCursor = new Cursor(Cursor.CROSSHAIR_CURSOR);
private static final int borderSize = 35;
private boolean GPcomputed=false;
private int width; // width for function = whole width - 2*bordersize


}
