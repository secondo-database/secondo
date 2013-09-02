




package viewer.hoese.algebras;


import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import java.text.DecimalFormat;


public class Dspllabel extends DisplayGraph implements DisplayComplex, LabelAttribute{

   private String text;
   private double x;
   private double y;
   Point2D orig;
   Point2D trans;
   private double d;
   private Rectangle2D.Double rect;

   public String getLabel( double time){
     return text;
   }


   public void draw(Graphics g1, double time, AffineTransform at){
     Graphics2D g = ( Graphics2D) g1;
     try{
        at.transform(orig,trans);

        g.rotate(d,trans.getX(), trans.getY());
        //g.setPaint(Cat.getFillStyle(null,0));
        g.drawString(text, (int)trans.getX(), (int) trans.getY());
     } catch(Exception e){
         e.printStackTrace();
     }
   }

   public boolean isPointType(){
     return false;
   }

   public int numberOfShapes(){
     return 0;
   }

   public Shape getRenderObject(int num, AffineTransform at){
     return null;
   }

   public String toString(){
      return ""+text + " ("+x+", " + y+", " + d + ")";
   }

   private boolean scanValue(ListExpr v){
     if(v.listLength()!=4){
       return false;
     }
     text = v.first().textValue();
     Double X = LEUtils.readNumeric(v.second());
     Double Y = LEUtils.readNumeric(v.third());
     Double D = LEUtils.readNumeric(v.fourth());
     if(X==null || Y == null || D == null){
       return false;
     }  
     x = X.doubleValue();
     y = Y.doubleValue();
     d = D.doubleValue()*Math.PI / 180.0;
     if(!ProjectionManager.project(x,y,aPoint)){
         return false;
     }
     x = aPoint.getX();
     y = aPoint.getY();
     orig = new Point2D.Double(x,y);
     trans = new Point2D.Double(x,y);
     rect = new Rectangle2D.Double(x,y,0,0);
     return true;
 
   }
   public void init(String name, int nameWidth, int indent,
                    ListExpr type, ListExpr value, QueryResult qr){

       AttrName = extendString(name,nameWidth, indent);
       if(isUndefined(value)){
         qr.addEntry(new String(""+AttrName+ ": undefined"));
         return;
       }
       if(!scanValue(value)){
         qr.addEntry(AttrName + " : invalid" );
         return;
       }
       qr.addEntry(this);
   }

   public Rectangle2D.Double getBounds(){
       return rect;
   }


}
