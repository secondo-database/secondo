package viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import java.io.*;


public class Dsplconvex extends DisplayGraph  {


   private  Path2D.Double conv;
            Area areas;
   
    
       
   public boolean ScanValue (ListExpr value) {
      
  
    
    ListExpr f, s, ss, fs, ff, sf;
    Double newx = 42.0;
    Double newy = 42.0;
    Double lastx = 42.0;
    Double lasty = 42.0;
    double tx1 ,ty1, tx2, ty2, txx2, tyy2;
    boolean empty = true;
    float  tx1f,  ty1f, tx2f, ty2f, txx2f, tyy2f;
   
   
   
    if(isUndefined(value)){
       Reporter.writeError("Undefined polygon");
       err = true;
       conv=null;
       return false;
    }
    
    
    if (value.listLength() <= 2) {
            Reporter.writeError("No correct polygon expression: 3 Points needed at least ");
            err = true;
            return false;
        }
    
    conv = new Path2D.Double();
   
    f = value.first();
    ff = f.first();
    fs = f.second();
    lastx = LEUtils.readNumeric(ff);
    lasty = LEUtils.readNumeric(fs);
      
    s = value.second();
    sf = s.first();
    ss = s.second();
    newx = LEUtils.readNumeric(sf);
    newy = LEUtils.readNumeric(ss);
        
    
    tx1 = lastx.doubleValue();
    ty1 = lasty.doubleValue();
    tx2 = newx.doubleValue();
    ty2 = newy.doubleValue();
        
    
    if(!ProjectionManager.project(tx1,ty1,aPoint)){
          err = true;
          }
      else{
      
     tx1f = (float) aPoint.x;    
     ty1f = (float) aPoint.y;
     
     conv.moveTo(tx1f, ty1f);
     empty = false;
      
      
       if(!ProjectionManager.project(tx2,ty2,aPoint)) {
             err = true;
             }
          else {
          
            tx2f = (float) aPoint.x;
            ty2f = (float) aPoint.y;
            
            conv.lineTo(tx2f, ty2f);
            }
        }
      
    
    
    
    
    value = value.rest();
    value = value.rest();
   
       
    
    while (!value.isEmpty()) {
           
     f = value.first();
    ff = f.first();
    fs = f.second();
    newx = LEUtils.readNumeric(ff);
    newy = LEUtils.readNumeric(fs);
    txx2 = newx.doubleValue();
    tyy2 = newy.doubleValue();
        
    
    
    if(!ProjectionManager.project(txx2,tyy2, aPoint)) {
          err = true;
          }
      else {
      txx2f = (float) aPoint.x;
      tyy2f = (float) aPoint.y;
         
      conv.lineTo(txx2f, tyy2f);
     }
    
        
     
    value = value.rest();
      
     
      
    }
    
    
        
            
    conv.closePath();
    
     if(!empty) 
        areas= new Area(conv);
     else
        areas= null;
    
    return true;
  }

    
   public int numberOfShapes(){
        return 1;
    }
    
    
   public Shape getRenderObject(int num,AffineTransform at){
    if(num<1){
       return areas;
    } else{
       return null;
    }
  }
    
    
    
    public void init (String name, int nameWidth, int indent,
                      ListExpr type,
                      ListExpr value,
                      QueryResult qr) {
        AttrName = extendString(name,nameWidth, indent);
        ScanValue(value);
        if (err) {
            Reporter.writeError("Error in ListExpr: parsing aborted");
            qr.addEntry(new String("(" + AttrName + ": GA(Convex))"));
            return;
        }
        
        qr.addEntry(this);
        
    }
    
    
    
    
    // Interface Methods
    
    public boolean isDefined(double time ){
        return true;
    }
    
    
   
   
   
   
}
