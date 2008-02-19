

package viewer.viewer3d.graphic2d;

import java.util.Vector;
import java.awt.Graphics;
import gui.idmanager.*;

/** A two dimensional triangle is a figure which can be created 
  * from a three dimenional triangle. Because of clipping operations
  * on the original triangle, it consists of a set of triangles forming the 
  * interior and a set of segments (forming the visible boundary of the
  * original triangle.
  **/

public class Triangle2D extends Figure2D{
 
 public Triangle2D(ID aID){
   triangles = new Vector();
   lines = new Vector();
   myID = new ID();
   myID.equalize(aID);
 }


 public boolean equals(Figure2D f){
    if(!( f instanceof Triangle2D)){
       return false;
    }
    Triangle2D t = (Triangle2D) f;
    if(triangles.size() != t.triangles.size() ||
       lines.size() != t.lines.size()){
       return false;
    }
    for(int i=0; i< triangles.size();i++){
       Triangle2DSimple t1 = (Triangle2DSimple) triangles.get(i);
       Triangle2DSimple t2 = (Triangle2DSimple) t.triangles.get(i);
       if(! t1.equals(t2)){
         return false;
       }  
    }    
    for(int i=0; i< lines.size();i++){
       Line2D s1 = (Line2D) lines.get(i);
       Line2D s2 = (Line2D) lines.get(i);
       if(! s1.equals(s2)){
         return false;
       }  
    }   
    return true; 
 }

 public Figure2D duplicate(){
    Triangle2D t = new Triangle2D(myID);
    t.equalize(this);
    return t;
 } 

 public void equalize(Triangle2D t){
    triangles.clear();
    lines.clear();
    for(int i=0;i<t.triangles.size();i++){
       triangles.add(t.triangles.get(i));
    } 
    for(int i=0;i<t.lines.size();i++){
       lines.add(t.lines.get(i));
    }
    myID.equalize(t.myID);
 }

 public void paint(Graphics g, boolean filled, boolean gradient){
    for(int i=0;i<triangles.size();i++){
        ((Triangle2DSimple)triangles.get(i)).paint(g,filled,gradient);
    }  
    for(int i=0;i<lines.size();i++){
        ((Line2D)lines.get(i)).paint(g,false,false);
    }
 } 

 public void insert(Triangle2DSimple t){
     triangles.add(t);
 }

 public void insert(Line2D l){
    lines.add(l);
 }


 public String toString(){
   String res ="Trs(";
   for(int i=0;i<triangles.size();i++){
      res += triangles.get(i);
   }
   return res;
 }


 private Vector triangles;
 private Vector lines;

}
