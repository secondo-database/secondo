package viewer.viewer3d.graphic3d;

import gui.idmanager.*;
import viewer.viewer3d.graphic2d.*;

/****************************
*
* Autor   : Thomas Behr
* Version : 1.0
* Datum   : 16.5.2000
*
******************************/

import javax.swing.*;
import java.awt.*;
import java.awt.image.*;

public class World3D extends JComponent {

/** creates a new World3D */
public World3D() {

   Fct3D = new FM3DGraphic();
   W2D   = new World2D();
   FV    = new Triangle3DVector();
   PV    = new IDPoint3DVector();
   LV    = new Line3DVector();
   border = false;

   Point3D Eye    = Fct3D.getEye();
   Point3D VRP    = Fct3D.getVRP();
   Point3D ViewUp = Fct3D.getViewUp();
   Visitor        = new Visitorclass();

   Visitor.XEye = Eye.getX();
   Visitor.YEye = Eye.getY();
   Visitor.ZEye = Eye.getZ();
   Visitor.XVRP = VRP.getX();
   Visitor.YVRP = VRP.getY();
   Visitor.ZVRP = VRP.getZ();
   Visitor.XViewUp =  ViewUp.getX();
   Visitor.YViewUp  =  ViewUp.getY();
   Visitor.ZViewUp  =  ViewUp.getZ();
   imageChanged=true;
}

/** get the width of the window */
public double getWindowX(){ return Fct3D.getWindowX();}
/** get the height of the window */
public double getWindowY(){return Fct3D.getWindowY(); }
/** get the x-coordinate of the view reference point */
public double getVRPX(){ return Visitor.XVRP; }
/** get the y-coordinate of the view reference point */
public double getVRPY(){ return Visitor.YVRP;}
/** get the z-coordinate of the view reference point */
public double getVRPZ(){ return Visitor.ZVRP;}
/** get the x-coordinate of the position of the eye */
public double getEyeX(){ return Visitor.XEye;}
/** get the y-coordinate of the position of the eye */
public double getEyeY(){ return Visitor.YEye;}
/** get the z-coordinate of the position of the eye */
public double getEyeZ(){ return Visitor.ZEye;}
/** get the x-coordinate of ViewUp-Point */
public double getViewUpX(){ return Visitor.XViewUp;}
/** get the y-coordinate of ViewUp-Point */
public double getViewUpY(){ return Visitor.YViewUp;}
/** get the z-coordinate of ViewUp-Point */
public double getViewUpZ(){ return Visitor.ZViewUp;}
     

/** paint border of triangles ? */
public void setBorder(boolean M) {
  // paint a border by the triangles ?
  if (M!=border) {
     border = M;
     imageChanged=true;
   }
}

/** is border painting enabled ? */
public boolean isBorder(){
  return border;
}

/** fill triangles ? */
public void setFill(boolean f) {
   if (f!=fill) {
      fill = f;
      imageChanged=true;
   }
}

/** is filling enabled ?*/
public boolean isFill(){
   return fill;
}


/** single color for figure or gradient-paint */
public void setGradient(boolean V) {
   if (V != this.Gradient) {
       this.Gradient = V;
       imageChanged=true;
   }
}

/** is gradient painting enabled ? */
public boolean isGradient(){
   return Gradient;
}


/** viewport proportional to the window ? */
public void setProportion(boolean P){
 if(P!=proportion){
    proportion = P;
    if(P)
      setViewport(10,10,width-20,height-20);
    else{
      int min = Math.min(width,height);
      setViewport(10,10,min-20,min-20);
    }  
    imageChanged=true;
 }
}

/** is proportional painting enabled ? */
public boolean isProportional(){
  return proportion;
}


/** set the size of this world */
public void setSize(int w, int h) {
    width=w;
    height=h;
    if(proportion)
       setViewport(10,10,w-20,h-20);
    else{
      int min = Math.min(w,h);
      setViewport(10,10,min-20,min-20);
    }
    img = null;
    super.setSize(w,h);
  }

/** set bounds of this world */
public void setBounds(int x, int y, int w, int h){
    width=w;
    height=h;
    if(proportion)
       setViewport(10,10,w-20,h-20);
    else{
      int min = Math.min(w,h);
      setViewport(10,10,min-20,min-20);
    }
    img = null;
    super.setBounds(x,y,w,h);
 }


/** set the view */
public boolean setView(double EyeX, double EyeY, double EyeZ,
                    double VRPX, double VRPY, double VRPZ,
                    double ViewUpX, double ViewUpY, double ViewUpZ){

  boolean result = true;
  try{
    Fct3D.setView(EyeX,EyeY,EyeZ,
                  VRPX,VRPY,VRPZ,
                  ViewUpX,ViewUpY,ViewUpZ);
    Visitor.XEye = EyeX;
    Visitor.YEye = EyeY;
    Visitor.ZEye = EyeZ;
    Visitor.XVRP = VRPX;
    Visitor.YVRP = VRPY;
    Visitor.ZVRP = VRPZ;
    Visitor.XViewUp = ViewUpX;
    Visitor.YViewUp = ViewUpY;
    Visitor.ZViewUp = ViewUpZ;
    imageChanged=true;
  }
  catch(Exception e) {
    result = false;
  }
  return result;
}


/** set the window */
public void setWindow(double x , double y) {
  Fct3D.setWindow(x,y);
  imageChanged=true;
}

/** update the world */
public void update(){
  allNew();
}

/** set the viewport of this world */
public void setViewport(double x0,double y0,double width,double height) {
  Fct3D.setViewport(x0,y0,width,height);
  imageChanged=true;
  allNew();
}


/** insert a new Triangle to the world */
public void insertTriangle(Triangle3D F) {
  FV.append(F);
  insertFigure(F.getFigure3D());
  imageChanged=true;
}

/** inserts all Triangles in TV **/
public void add(Triangle3DVector TV){
  for(int i=0;i<TV.getSize();i++)
      insertTriangle(TV.get(i));
}


/** insert a new point to the world */
public void insertPoint(IDPoint3D P) {
   PV.append(P);
   Figure3D Fig = P.getFigure3D();
   insertFigure(Fig);
   imageChanged=true;
}

/** inserts all points in PV */
public void add(IDPoint3DVector PV){
  for(int i=0;i<PV.getSize();i++)
      insertPoint(PV.get(i));
}


/** insert a new line to the world */
public void insertLine(Line3D L){
   LV.append(L);
   insertFigure(L.getFigure3D());
   imageChanged=true;
}

/** inserts all lines in LV */
public void add(Line3DVector LV){
  for(int i=0;i<LV.getSize();i++)
      insertLine(LV.get(i));
}



/** removes all figures from the world */
public void removeAll() {
   W2D.deleteAll();
   FV.empty();
   LV.empty();
   PV.empty();
   imageChanged=true;
}


/** removes all figures with given ID */
public void removeID(ID RID) {
  W2D.deleteFiguresWithID(RID);
  int i=0;
  int max = FV.getSize();
  while (i<max) {
    if ( FV.getTriangle3DAt(i).getID().equals(RID)  ) {
       FV.remove(i);
       max--;
    }else
      i++;
  }
 
  i=0;
  max = LV.getSize();
  while (i<max) {
    if ( LV.getLine3DAt(i).getID().equals(RID)  ) {
       LV.remove(i);
       max--;
    }else
      i++;
  }
 
  i=0;
  max = PV.getSize();
  while (i<max) {
    if ( PV.getIDPoint3DAt(i).getID().equals(RID)  ) {
       PV.remove(i);
       max--;
    }else
      i++;
  }

  imageChanged=true;
}



/** paint this world */
public void paint(Graphics G) {
  if(img==null){
     img =(BufferedImage) createImage(getSize().width,getSize().height);
  }
  if (imageChanged) {
       W2D.paintWorld(img,border,fill,Gradient);
       imageChanged=false;
  }
  G.clearRect(0,0,img.getHeight(),img.getWidth());
  G.drawImage(img,0,0,null);
}


//##########  private methods


/** insert a figure */
private void insertFigure(Figure3D Fig) {

 Figure2D Fig2D = Fct3D.figureTransformation(Fig);

 if (! Fig2D.isEmpty()) {
    W2D.insertFigure(Fig2D);
 }

}


/** update the world */
private void allNew() {

  W2D.deleteAll();
  for(int i=0; i<FV.getSize(); i++) {
     insertFigure(FV.getTriangle3DAt(i).getFigure3D());
  }
  for(int i=0; i<PV.getSize();i++){
    insertFigure(PV.getIDPoint3DAt(i).getFigure3D());
  }
  for(int i=0; i<LV.getSize();i++){
    insertFigure(LV.getLine3DAt(i).getFigure3D());
  }

}

	
/** paint a border ? */
private boolean       border;                   // paint a border ?
/** fill the interior of a triangle */
private boolean       fill=true;                // only a gridpaint ?
/** gradient-paint or single color per figure */
private boolean       Gradient = false;

/** describe the visitor */
private class Visitorclass {
      double  XEye,YEye,ZEye,XVRP,YVRP,ZVRP,
                      XViewUp,YViewUp,ZViewUp;
   }

/** define the view */
private Visitorclass Visitor;

/** convert 3dim-Objects to 2-dim objects */
private FM3DGraphic   Fct3D;

/** here are the 2D-figures */
private World2D   W2D;

/** the set of triangles */
private Triangle3DVector   FV;
/** the set of lines */
private Line3DVector       LV;
/** the set of points */
private IDPoint3DVector      PV;

/** is the image cahnged ? */
private boolean  imageChanged;
/** here is paint the world */
private BufferedImage img;

/** viewport proportinal to window ? */
private boolean  proportion = false;

private int width;
private int height;

}

