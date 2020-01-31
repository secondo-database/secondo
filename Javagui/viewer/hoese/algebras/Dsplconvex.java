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


   private Path2D.Double conv;
           
    
       
   public boolean ScanValue (ListExpr value) {
      
  
    boolean firstval = true;
    ListExpr f, s, ss, fs, ff, sf;
    Double newx = 42.0;
    Double newy = 42.0;
    Double lastx = 42.0;
    Double lasty = 42.0;
    double tx1 ,ty1, tx2, ty2;    
    boolean firstvalsetdone = false;
   
   
   
   
   
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
        
    conv.moveTo(tx1, ty1);
    if(!ProjectionManager.project(tx1,ty1,aPoint)){
          err = true;
          }
      else{
      
       if(!ProjectionManager.project(tx2,ty2,aPoint)) {
             err = true;
             }
          else {
            conv.lineTo(tx2, ty2);
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
    tx2 = newx.doubleValue();
    ty2 = newy.doubleValue();
        
    
    
    if(!ProjectionManager.project(tx2,ty2,aPoint))
          err = true;
      else  conv.lineTo(tx2, ty2);
    
    
        
     
    value = value.rest();
      
     
      
    }
    
    
        
            
    conv.closePath();
    return true;
  }

    
    
    
      
    
    

    public int numberOfShapes(){
        return 1;
    }
    
    
    
    public Shape getRenderObject (int num,AffineTransform at) {
    
        
        if (num<1) {
        
            return conv;
        }
        
        return null;
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
    
    
   
   
   
    public boolean mayBeDefined() {
        return true;
    }

    public double getMinRenderValue() {
        return 3;
    }

    public double getMaxRenderValue() {
        return 1000;
    }
}
