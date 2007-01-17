package viewer.hoese.algebras;

import javax.swing.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.util.Vector;
import java.util.Iterator;
import tools.Reporter;

/*
 This class provides a panel able to display several functions.
*/

public class MultiFunctionPanel extends JPanel{

public MultiFunctionPanel(){

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
         // all functions must be computed new
         setToNull();
         y_computed=false;
         repaint();
      }
  });
  //segmentsComputed = false;
  setToNull();
  y_computed = false;

}

/** sets all segment vector to be null. 
  *  So all segments must be computed newly;
  */
private void setToNull(){
  for(int i=0;i<allSegments.size();i++){
      allSegments.set(i,null);
  }
  
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

/** Checks whether a crosshair is shown. **/
public boolean isCrossEnabled(){
      return crossEnabled;
}

/** Checks whether the y value of the cursor position is shown. **/
public boolean isYEnabled(){
    return yEnabled;
}

/** Computes the segment Vector for the given function **/
private Vector getSegments(Function f){
   
   Vector result = new Vector(width);
   double[] matrix = new double[6];
   at.getMatrix(matrix);
   double dx = xmax-xmin;
   Double y;
   java.awt.geom.Point2D.Double lastpoint = null;
   java.awt.geom.Point2D.Double thepoint = null;
   for(int ix=0;ix<width;ix++){
      double x = xmin + (dx*ix)/width;
      boolean inRange = (f.getInterval().getStart()<=x) &&   
                         (x<= f.getInterval().getEnd());
      if(inRange && (y=f.getValueAt(x))!=null){
          double xd1 = (double) ix; 
          double yd1 = y.doubleValue();
          double xd = matrix[0]*xd1 + matrix[2]*yd1 + matrix[4];
          double yd = matrix[1]*xd1 + matrix[3]*yd1 + matrix[5];
          thepoint = new Point2D.Double(xd,yd);
          if(lastpoint!=null){
              Line2D.Double line =  new Line2D.Double(lastpoint,thepoint);
              result.add(line);
           }
           lastpoint = thepoint;
      } else { // undefined state
           lastpoint=null;
      }
   }
   return result;
}


/** Overwrites the paint function. 
  * Paints all Functions. 
  **/
public void paint(Graphics g){
    super.paint(g); // clear the panel
    if(allFunctions.size()<1){
       return;
    }

    width=getWidth()-2*borderSize;
    g.setColor(Color.BLACK);
    int height=getHeight()-2*borderSize;

    Graphics2D g2 = (Graphics2D) g;

    if(!y_computed){ // set of function changed
       compute_y(width,height);
       y_computed = true;
       setToNull(); // changed y-value => compute functions new
    }
    for(int i=0;i<allFunctions.size();i++){
        if(allSegments.get(i)==null){
            allSegments.set(i,getSegments((Function)allFunctions.get(i)));
        }
    }

    // draw the functions
    for(int i=0;i< allSegments.size();i++){
         Vector lines = (Vector) allSegments.get(i);
         Color C = (Color) allColors.get(i);
         Iterator it = lines.iterator();
         g2.setColor(C);
         while(it.hasNext()){
             Shape s = (Shape) it.next();
             g2.draw(s);
         }
    }
    g2.setColor(Color.BLACK);

    // draw cross if enabled
    if(crossEnabled){
        g.drawLine(lastX,0,lastX,height+2*borderSize);
    }
    // draw y if enabled
    if(yEnabled && (atinv!=null)){
        double my = atinv[1]*lastX+atinv[3]*lastY+atinv[5]; // position of the mouse
        String Label = ""+my;
        g.drawString(Label,lastX+2,lastY-2);
    } 
}

/** Stores the position of the mouse as world-coordinates in coords.
  * If their are problems, the reuslt will be false.
  **/
public boolean getOrig(int mouseX,int mouseY,Point2D.Double coords){
   if(atinv==null){
        return false;
   }
   if(width<=0) {
        return false; 
   }
   double pixSize = (xmax-xmin)/width;
   double x = (mouseX-borderSize)*pixSize+xmin;
   double my = atinv[1]*mouseX+atinv[3]*mouseY+atinv[5]; // position of the mouse
   coords.x = x;
   coords.y = my;
   return true;
}

/** inserts a function to the set of functions to display. **/
public void addFunction(Function function){
    if(allFunctions.contains(function)){
       return;
    }
    allFunctions.add(function);
    addInterval(function.getInterval().getStart(), function.getInterval().getEnd());
    addColor();
    //segmentsComputed=false;
    allSegments.add(null);
    y_computed=false; // may be a new y value
}

private void addInterval(double min, double max){
  // swap values if needed
  if(min>max){
     double tmp = min;
     min = max;
     max = tmp;
  }
  if(allFunctions.size()==1){ // the first function
      xmin = min;
      xmax = max;
  } else {
      if(min<xmin){
        xmin = min;
      }
      if(max > xmax){
        xmax = max;
      }
  }
}

/** computes a new color from the last one and stores it 
  * in the corresponding vector.
  **/
private void addColor(){
  if(removedColors.size()>0){
      allColors.add(removedColors.remove(0));
      return;
  }

  int r = 0;
  int g = 0;
  int b = 0;

  int number = allFunctions.size()-1;


  int pos = 1;
  while(number>0){
     int next = number % 8; // extract the last 3 bits
     number = number / 8; 
     int ext = 255 / pos;
     if( (next & 1) != 0) {
        r += ext % 256;
     }
     if( (next & 2) != 0 ){
        g += ext % 256;
     }
     if( (next & 4 ) !=  0 ){
        b += ext % 256;
     }
     pos++;
  }
  allColors.add(new Color(r,g,b));
}


/** Checks whether the given function is displayed. **/
public boolean contains(Function f){
    return allFunctions.contains(f);
}



public void setVirtualSize(double x, double  y){
   //segmentsComputed=false;
   setToNull();
   y_computed = false;
   setPreferredSize(new Dimension((int)x,(int)y));
   revalidate();
}

public boolean remove(Function f){
  int index = allFunctions.indexOf(f);
  if(index < 0){
     return false;
  }
  allFunctions.remove(index);
  allSegments.remove(index);
  removedColors.add(allColors.remove(index));
  //segmentsComputed = false;
  updateInterval();
  setToNull(); // may be y value is changed, compute all new
  y_computed = false;
  return true;
}

public void removeAllFunctions(){
   allFunctions.clear();
   allSegments.clear();
   allColors.clear();
   removedColors.clear();
   y_computed = false;
   updateInterval();
}

private void updateInterval(){

   xmin = 0;
   xmax = 1;
   for(int i=0;i< allFunctions.size();i++){
       boolean first = i==0;
       Function f = (Function) allFunctions.get(i);
       double min = f.getInterval().getStart();
       double max = f.getInterval().getEnd();
       if(first || min<xmin){
          xmin = min;
       }    
       if(first || max>xmax){
          xmax = max;
       }
   }

}



/* computes the minimum and the maximum y value for the given function **/
private void compute_y(Function f, int witdh, int height, java.awt.geom.Point2D.Double result){
   boolean first=true;
   double dx = xmax-xmin;
   double y_min= 0;
   double y_max=0;
   Double y;
   for(int i=0;i<width;i++){
       double x = xmin + dx*i/width;
       boolean inRange = (f.getInterval().getStart() <= x) && 
                         (x <= f.getInterval().getEnd()); 
       if((y=f.getValueAt(x))!=null){
          double y1 = y.doubleValue();
          if(first){
               y_min=y_max=y1;
               first=false;
          }else{
               if(y1<y_min) y_min=y1;
               if(y1>y_max) y_max=y1;
          }
       } 
   }
   result.x = y_min;
   result.y = y_max;
}


/** Computes the minumum and maximum y value of all functions.
  */
private void compute_y(int width,int height){
   java.awt.geom.Point2D.Double p = new java.awt.geom.Point2D.Double();
   for(int i=0;i<allFunctions.size();i++){
      compute_y( (Function) allFunctions.get(i), width,height,p);
      if(i==0){ // first function
          ymin = p.x;
          ymax = p.y;
      } else {
          if(p.x<ymin){
             ymin = p.x;
          }
          if(p.y>ymax){
             ymax = p.y;
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


private Vector allFunctions = new Vector(10);
private Vector allSegments = new Vector(10);
private Vector allColors = new Vector(10);
private Vector allComputed = new Vector(10);
private Vector removedColors = new Vector(10);
private double xmin=0;
private double xmax=1;
private boolean y_computed=false;
private double ymin;
private double ymax;
private AffineTransform at=new AffineTransform();
private double[] atinv=new double[6];
private boolean crossEnabled=false;
private int lastX;
private int lastY;
private boolean yEnabled=false;
private MouseMotionListener MML;
private Cursor defaultCursor = new Cursor(Cursor.DEFAULT_CURSOR);
private Cursor crossCursor = new Cursor(Cursor.CROSSHAIR_CURSOR);
private static final int borderSize = 35;
//private boolean segmentsComputed=false;
private int width; // width for function = whole width - 2*bordersize


}
