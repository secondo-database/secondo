package viewer.hoese.algebras;

import javax.swing.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.util.Vector;
import java.util.Iterator;
import tools.Reporter;
import java.io.*;

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


/** Write a value table of this function.
  * Note that the output will depend on the current size.
  **/ 
public boolean writeToFile(File f){
    // compute the segments building the function
    width=getWidth()-2*borderSize;
    int height=getHeight()-2*borderSize;
    Vector segments = new Vector();
    if(height <= 0 ){
        return false; // no space to paint anything
    } else { // compute the lines
       if(!y_computed){
          compute_y(width,height);
       }
       double dx = xmax-xmin;
       segments = new  Vector(width);
       Double y;
       java.awt.geom.Point2D.Double thepoint = null;
       Vector pointList = new Vector(width);
       double epsilon = Math.min( (xmax-xmin)/width, (ymax-ymin)/width);
       for(int ix=0;ix<width;ix++){
         double x = xmin + (dx*ix)/width;
         if((y=function.getValueAt(x))!=null){
             thepoint = new Point2D.Double(x,y);
             pointList.add(thepoint);
         } else { // undefined state
           tools.LineSimplification.addSegments(pointList,segments,epsilon);
           pointList.clear();
         }
      }
      tools.LineSimplification.addSegments(pointList,segments,epsilon);
    }
    if(segments==null){ // nothing to output
       return false; 
    }
    try{
       PrintStream out = new PrintStream(new FileOutputStream(f));
       Point2D  lastPoint =null;
       for(int i=0;i<segments.size();i++){
          Line2D.Double line = (Line2D.Double) segments.get(i);
          Point2D p1 = line.getP1();
          Point2D p2 = line.getP2();
          if(lastPoint==null ||  !lastPoint.equals(p1)){
             if(lastPoint!=null){ // hole in definition
                out.println(""); 
             }
             out.println(p1.getX()+ "  " + p1.getY());
          } 
          out.println(p2.getX()+ " " + p2.getY());
          lastPoint = p2; 
       }
       out.close();
       return true;
    }catch(Exception e){
        Reporter.debug(e);
        return false;
    }
}


public void paint(Graphics g){
    super.paint(g);
    if(function==null)
       return;

    width=getWidth()-2*borderSize;
    g.setColor(Color.BLACK);
    int height=getHeight()-2*borderSize;

    Graphics2D g2 = (Graphics2D) g;
    if(height <= 0 ){
        segments = null; // no space to paint anything
    } else if((!GPcomputed || (segments == null) )){
       double[] matrix = new double[6];
       if(!y_computed){
          compute_y(width,height);
       }
       at.getMatrix(matrix);
       double dx = xmax-xmin;
       segments = new  Vector(width);
       Double y;
       java.awt.geom.Point2D.Double thepoint = null;
       Vector pointList = new Vector(width);
       double epsilon = Math.min( (xmax-xmin)/width, (ymax-ymin)/width);
       for(int ix=0;ix<width;ix++){
         double x = xmin + (dx*ix)/width;
         if((y=function.getValueAt(x))!=null){
             double xd1 = (double) ix; 
             double yd1 = y.doubleValue();
             
             double xd = matrix[0]*xd1 + matrix[2]*yd1 + matrix[4];
             double yd = matrix[1]*xd1 + matrix[3]*yd1 + matrix[5];
             thepoint = new Point2D.Double(xd,yd);
             pointList.add(thepoint);
         } else { // undefined state
           tools.LineSimplification.addSegments(pointList,segments,epsilon);
           pointList.clear();
         }
      }
      tools.LineSimplification.addSegments(pointList,segments,epsilon);
      GPcomputed=true;
    }
    if(segments!=null){
       Iterator it = segments.iterator();
       while(it.hasNext()){
         Shape s = (Shape) it.next();
         g2.draw(s);
       }
    }


    if(crossEnabled){
        g.drawLine(lastX,0,lastX,height+2*borderSize);
     //   g.drawLine(lastX-20,lastY,lastX+20,lastY);
    }
    if(yEnabled && (atinv!=null)){
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
    repaint();
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
   double scaleY;
   if(dy!=0){
      scaleY =  height/dy;
   }else{
      scaleY = 1; 
   }
   double tx = 0; // -xmin;
   double ty = -ymin;
   AffineTransform Flip = new AffineTransform();
   at.setTransform(scaleX,0,0,scaleY,scaleX*tx,scaleY*ty); 
   at.preConcatenate(AffineTransform.getTranslateInstance(0,-height));
   at.preConcatenate(AffineTransform.getScaleInstance(1,-1));
   AffineTransform trans = (AffineTransform.getTranslateInstance(borderSize,borderSize));
   at.preConcatenate(trans);
   try{
      AffineTransform ati = at.createInverse();
      if(atinv==null){ // create a new matrix if required
         atinv = new double[6]; 
      }
     ati.getMatrix(atinv);
   }catch(Exception e){
       Reporter.debug("could not create the inverse matrix");
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
//private GeneralPath GP = null;
private Vector segments = null;
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
