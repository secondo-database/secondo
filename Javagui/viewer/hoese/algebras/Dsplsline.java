//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package  viewer.hoese.algebras;



import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  viewer.hoese.*;


/**
 * The displayclass of the sline datatype.
 */
public class Dsplsline extends Dsplline {

private static Area basicArrow;
boolean isArrow = true;
Point2D.Double p1;
Point2D.Double p2;

public int numberOfShapes(){
  return 2;
}

public boolean isLineType(int num){
   if (num==0){
      return super.isLineType(num);
   }
   return false;  // draw arrow as a region
}


public Shape getRenderObject(int num, AffineTransform at){
   if(num==0){
      return super.getRenderObject(num,at);
   }
   if(!isArrow){
      return null;
   }
   if(!err && defined && p1!=null && p1!=null){
     return getArrow(at,p1,p2);
   }
   return null;
}

public void ScanValue(ListExpr value){

   p1 = new Point2D.Double();
   p2 = new Point2D.Double();
   
   boolean ok = super.ScanValue(value,p1,p2);
   if(!ok){
      p1=null;
      p2=null;
   }
}


  public Shape getArrow(AffineTransform af, Point2D.Double point1, Point2D.Double point2){

   createBasicArrow(Cat);
   // transform the basicArrow to be in the correct angle at the end of the connection
   double x1 = point1.getX();
   double x2 = point2.getX();
   double y1 = point1.getY();
   double y2 = point2.getY();

   AffineTransform aat = new AffineTransform();

   double sx = af.getScaleX();
   double sy = af.getScaleY();

   AffineTransform trans = AffineTransform.getTranslateInstance(x2,y2);

   aat.concatenate(trans);
   // normalize
   double dx =  x1-x2;
   double dy = y1-y2;
   double len = Math.sqrt(dx*dx+dy*dy); // the length
   dx = dx / len;
   dy = dy / len;

   AffineTransform Rot = new AffineTransform(dx,dy,-dy,dx,0,0);
   aat.concatenate(Rot);
   

   AffineTransform scale = AffineTransform.getScaleInstance(5/sx,5/sy);

   aat.concatenate(scale);
   
   Shape S = aat.createTransformedShape(basicArrow);
   return S;
  }


  void createBasicArrow(Category cat){
     GeneralPath gparrow = new GeneralPath();
     gparrow.moveTo(0,0);
     gparrow.lineTo(5,-1);
     gparrow.lineTo(3,0);
     gparrow.lineTo(5,1);
     gparrow.lineTo(0,0);
     Shape s = gparrow;
     if(cat!=null){
        double x = cat.getPointSize(renderAttribute, CurrentState.ActualTime);
        if(x>0){
           AffineTransform at = AffineTransform.getScaleInstance(x/5,x/5);
           s =  at.createTransformedShape(gparrow);
        } else {
           basicArrow=null;
           return;
        }

          
     }
     
     basicArrow = new Area(s);
  }



}


