package viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import viewer.*;
import viewer.hoese.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.hoese.algebras.periodic.*;
import javax.swing.JPanel;

public class Dsplpmrect extends DisplayTimeGraph{

Rectangle2D.Double rect;
Rectangle2D.Double bounds;
Class linearClass = (new PMRectLinear()).getClass();
Time T = new Time();
TotalMove Move=null;

public Shape getRenderObject(AffineTransform at){
  if(Move==null){
     RenderObject = null;
     return null;
  }
  double t = RefLayer.getActualTime();
  T.readFrom(t);
  Rectangle2D.Double Rect =  (Rectangle2D.Double) Move.getObjectAt(T);
  RenderObject= Rect;
  return RenderObject;
}

public void init(ListExpr type,ListExpr value,QueryResult qr){
  AttrName = type.symbolValue();
  ispointType = false;
  Move = new TotalMove();
  if(!Move.readFrom(value,linearClass)){
     qr.addEntry("("+AttrName +"WrongListFormat )");
     return;
  }
  qr.addEntry(this);
  if(Move.getBoundingBox()==null){
     System.err.println("Bounding Box can't be created");
  }
  bounds = Move.getBoundingBox().toRectangle2D();
  double StartTime = Move.getStartTime().getDouble();
  RelInterval D = Move.getInterval();
  if(D.isLeftInfinite())
     StartTime -= MaxToLeft;
  double EndTime = StartTime;
  if(D.isRightInfinite())
    EndTime += MaxToRight;
  else
    EndTime += D.getLength().getDouble();
  TimeBounds = new Interval(StartTime,EndTime,D.isLeftClosed(),D.isRightClosed());
}

public JPanel getTimeRenderer(double PixelTime){
   return new JPanel();
}

/* returns the minimum bounding box of this moving point */
public Rectangle2D.Double getBounds(){
   return bounds;
}

// we need this beacuse the Hoese viewer can't handle infinite time intervals
private static final double MaxToLeft = 3000;
private static final double MaxToRight = 3000;

}
