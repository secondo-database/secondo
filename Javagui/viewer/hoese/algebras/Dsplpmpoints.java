package viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import viewer.*;
import viewer.hoese.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.hoese.algebras.periodic.*;
import javax.swing.JPanel;

public class Dsplpmpoints extends DisplayTimeGraph{

Point2D.Double point;
Rectangle2D.Double bounds;
Class linearClass = (new PMPsLinear()).getClass();
Time T = new Time();
TotalMove Move=null;
AffineTransform at;

public Shape getRenderObject(AffineTransform at){
   this.at=at;
   return null;
}

public void draw(Graphics g){
 if(Move==null){
     return;
  }
  double t = RefLayer.getActualTime();
  T.readFrom(t);
  Point2D.Double[] Pts =  (Point2D.Double[]) Move.getObjectAt(T);
  if(Pts==null){
     return;
  }
  Shape RenderObject;
  Graphics2D G = (Graphics2D)g;
  for(int i=0;i<Pts.length;i++){
      Point2D.Double Pos = Pts[i];
      double pixy = Math.abs(Cat.getPointSize()/at.getScaleY());
      double pixx = Math.abs(Cat.getPointSize()/at.getScaleX());
      if(Cat.getPointasRect()){
	RenderObject = new Rectangle2D.Double(Pos.getX()-pixx/2,Pos.getY()-pixy/2,pixx,pixy);
      }else{
	RenderObject = new Ellipse2D.Double(Pos.getX()-pixx/2,Pos.getY()-pixy/2,pixx,pixy);
      }
      G.draw(RenderObject);
  }
  drawLabel(g,bounds);
 }

public void init(ListExpr type,ListExpr value,QueryResult qr){
  AttrName = type.symbolValue();
  ispointType = true;
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
