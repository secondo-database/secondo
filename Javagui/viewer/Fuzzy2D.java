package viewer;

import viewer.viewer3d.graphic3d.*;
import java.awt.event.*;
import javax.swing.*;
import gui.*;
import java.awt.*;
import sj.lang.ListExpr;
import viewer.viewer3d.objects.*;
import java.beans.*;
import gui.idmanager.*;
import viewer.fuzzy2d.*;
import javax.swing.event.*;

/** this class provioded a viewer for fuzzy spatial objects
  * the objects are displayed with 2 dimensions
  * to show the membership value one can for each object the
  * color for the values 0 and 1. For membershipvalues in(0,1)
  * is a linear approximatiopn between the Colors used.
  * To display the 3the dimension is also used a ZBuffer.
  */
public class Fuzzy2D extends SecondoViewer implements ApplyListener{

/** creates the new Viewer */
public Fuzzy2D(){

   Img = new FuzzyImage(pw,ph); 

   ImgPanel = new JPanel(){
     public void paint(Graphics G){
       super.paint(G);
       G.drawImage(Img,0,0,null);
     }
   
   };

   ImgPanel.setSize(pw,ph);
   Dimension D = new Dimension(pw,ph);
   ImgPanel.setMinimumSize(D);
   ImgPanel.setMaximumSize(D);
   ImgPanel.setPreferredSize(D);
   ScrollPane = new JScrollPane(ImgPanel);
   ComboBox = new JComboBox();
   setLayout(new BorderLayout());
   JPanel P= new JPanel();
   P.add(ComboBox);
   P.add(CoordLabel);
   P.setLayout(new GridLayout(2,1));
   add(P,BorderLayout.NORTH);
   add(ScrollPane,BorderLayout.CENTER);
   add(BBLabel,BorderLayout.SOUTH);

   ((Graphics2D)Img.getGraphics()).setBackground(new Color(255,255,255));
   ImgPanel.repaint();

   // build the MenuExtension
   JMenuItem MI_ShowSettings = ObjectMenu.add("settings");
   MI_ShowSettings.addActionListener(new ActionListener(){
    public void actionPerformed(ActionEvent evt){
      showObjectSettings();
    }
   });

   JMenuItem MI_HideSelectedObject=ObjectMenu.add("hide");
   MI_HideSelectedObject.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         hideSelectedObject();
      }
   });

   JMenuItem MI_ShowViewConfig = ViewMenu.add("settings");
   MI_ShowViewConfig.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        showViewSettings();
      }
   });



   JMenuItem MI_AutoBB = ViewMenu.add("automatic bounding box");
   MI_AutoBB.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
      if(!AutoBoundingBox){
         AutoBoundingBox=true;
         update();

      }
   }});

   MenuExtension.addMenu(ObjectMenu);
   MenuExtension.addMenu(ViewMenu);

   MouseInputListener MIL=new MouseInputAdapter(){
       public void mouseMoved(MouseEvent evt){
          double[] wc = Img.getWorld(evt.getX(),evt.getY());
          // round to 3 positions after comma
          wc[0] = ((int)(wc[0]*1000))/1000.0;
          wc[1] = ((int)(wc[1]*1000))/1000.0;
          CoordLabel.setText("("+wc[0]+"  ,  "+wc[1]+")");
 
       }
       
       public void mousePressed(MouseEvent evt){
          startX = evt.getX();
          startY = evt.getY();
       }
       
       public void mouseDragged(MouseEvent evt){
          double[] wc = Img.getWorld(evt.getX(),evt.getY());
          // round to 3 positions after comma
          wc[0] = ((int)(wc[0]*1000))/1000.0;
          wc[1] = ((int)(wc[1]*1000))/1000.0;
          CoordLabel.setText("("+wc[0]+"  ,  "+wc[1]+")");
          if(isPainting)
            drawRectangle(startX,startY,oldX,oldY);
          oldX = evt.getX();
          oldY = evt.getY();
          drawRectangle(startX,startY,oldX,oldY);
          isPainting=true;
       }
       
       public void mouseReleased(MouseEvent evt){
          if(isPainting)
             drawRectangle(startX,startY,oldX,oldY);
          isPainting=false;   
          // set the new BoundingBox
          AutoBoundingBox = false;
          double[] P1 = Img.getWorld(startX,startY);
          double[] P2 = Img.getWorld(oldX,oldY);
          BB2.setTo(P1[0],P1[1],P2[0],P2[1]);
          Img.setBoundingBox(BB2);
          BB2.equalize(Img.getBoundingBox());
          update();
       }
       
       
       public void mouseClicked(MouseEvent evt){
          selectNextAt(evt.getX(),evt.getY());     
       }
      
      
       public void drawRectangle(int x1,int y1,int x2,int y2){
         Graphics2D G = (Graphics2D) ImgPanel.getGraphics();
         G.setColor(Color.green);
         G.setXORMode(Color.white);
         int x = Math.min(x1,x2);
         int w = Math.abs(x1-x2);
         int y = Math.min(y1,y2);
         int h= Math.abs(y1-y2);
         G.drawRect(x,y,w,h);
       }
       
       private int startX;
       private int startY;
       private int oldX;
       private int oldY;
       private boolean isPainting = false;
   };
   
   ImgPanel.addMouseListener(MIL);
   ImgPanel.addMouseMotionListener(MIL);
      
  update();
}


/* find the next Object3D which lays under (x,y) and selected it */
private void selectNextAt(int x, int y){
  int count = ComboBox.getItemCount();
  if(count<=0)
     return;
  
  double[] Pos =   Img.getWorld(x,y);
  double exactness = Img.getWorldAccuracy(Math.max(Img.getPointSize()/2,2));
  int index = ComboBox.getSelectedIndex();
  if(index<0)
     index=0;
  else
     index++;  // begin after selected object

  boolean found = false;   
  int next=index;
  for(int i=0;i<count&&!found;i++){
     next = (index+i)%count;
     Object3D O3D = (Object3D) ComboBox.getItemAt(next);
     if(O3D.nearByXY(Pos[0],Pos[1],exactness))
       found = true;
  }
     
  if(found)
    ComboBox.setSelectedIndex(next);
  else
    ComboBox.setSelectedIndex(-1);
}



/** remove the selected object*/
private void hideSelectedObject(){
    Object3D O3D = (Object3D) ComboBox.getSelectedItem();
    if(O3D==null)
       return;
    ID id = O3D.getID();
    removeGraphicalObjects(id);
    ComboBox.removeItem(O3D);
    Img.paint();
    ImgPanel.repaint();
    if(VC!=null)
       VC.removeObject(null);
}



/** return the name "Fuzzy2D"*/
public String getName(){
     return "Fuzzy2D";
}


/** take the setting of the current ViewConfig */
public void apply(Object source){
  if(source.equals(View)) {
     AutoBoundingBox = View.getAutoBoundingBox();
     BorderSize = View.getBorderSize();
     Proportional = View.getProportional();
     if(!AutoBoundingBox){
        BoundingBox2D B = View.getBoundingBox();
        BB2.equalize(B);
        Img.setBoundingBox(BB2);
        BB2.equalize(Img.getBoundingBox());
        View.setBoundingBox(BB2);
     }
     Img.setProportional(Proportional);
     Img.paintBorders(View.getBorderPaint());
     Img.setPointSize(View.getPointSize());
     update();
  }
}


/** adds a SecondoObject to this viewer */
public boolean addObject(SecondoObject o){
   int index = getIndexOf(o);
   if(index>=0){ // object allready displayed
     ComboBox.setSelectedIndex(index);
     return true;
   }

   ListExpr LE = o.toListExpr();
   if(LE.listLength()!=2)
     return false;
   ListExpr type = LE.first();
   ListExpr value = LE.second();
   if(! (type.isAtom() && type.atomType()==ListExpr.SYMBOL_ATOM))
      return false;
   String TypeName = type.symbolValue();
   if (TypeName.equals("fpoint")){
       FPoint3D P3D = new FPoint3D();
       if(!P3D.readFromSecondoObject(o))
          return false;
       IDPoint3DVector PV = P3D.getPoints();
       if(PV!=null)
         for(int i=0;i<PV.getSize();i++)
            myPoints.append(PV.get(i));
       ComboBox.addItem(P3D);
       update();
       ComboBox.setSelectedIndex(ComboBox.getItemCount()-1);
       return true;
   }else if(TypeName.equals("fline")){
       FLine3D L3D = new FLine3D();
       if(!L3D.readFromSecondoObject(o))
          return false;
       Line3DVector LV = L3D.getLines();
       ComboBox.addItem(L3D);
       if(LV!=null)
          for(int i=0;i<LV.getSize();i++)
             myLines.append(LV.get(i));
       update();
       ComboBox.setSelectedIndex(ComboBox.getItemCount()-1);
       return true;
   } else if(TypeName.equals("fregion")){
       FRegion3D R3D = new FRegion3D();
       if(!R3D.readFromSecondoObject(o))
          return false;
       Triangle3DVector TV = R3D.getTriangles();
       ComboBox.addItem(R3D);
       if(TV!=null)
          for(int i=0;i<TV.getSize();i++)
             myTriangles.append(TV.get(i));
       update();
       ComboBox.setSelectedIndex(ComboBox.getItemCount()-1);
       return true;
   } else
      return false;
 }



/** shows a ViewConfig and changed the View */
private void showViewSettings(){
  if(View==null){
      View = new ViewConfig(VC.getMainFrame());
      View.addApplyListener(this);
  }

  if(AutoBoundingBox){
     BB2.readFrom(BB3);
     View.setBoundingBox(BB2);
  }
  View.setBorderPaint(Img.isPaintingBorders());
  View.setProportional(Proportional);
  View.setBorderSize(BorderSize);
  View.setAutoBoundingBox(AutoBoundingBox);
  View.setPointSize(Img.getPointSize());
  View.setVisible(true);
}


/** inserts all points, lines and triangles from O */
private void addGraphicalObjects(Object3D O){
    Triangle3DVector Trs = O.getTriangles();
    if(Trs!=null)
       for(int i=0;i<Trs.getSize();i++)
          myTriangles.append(Trs.get(i));

    Line3DVector Lns = O.getLines();
    if(Lns!=null)
       for(int i=0;i<Lns.getSize();i++)
           myLines.append(Lns.get(i));

    IDPoint3DVector Pts = O.getPoints();
    if(Pts!=null)
       for(int i=0;i<Pts.getSize();i++)
           myPoints.append(Pts.get(i));
}


/** computes the current bounding box in tzhe world */
private void computeBoundingBox(){
   BB3.set(0,0,0,0,0,0);
   for(int i=0;i<ComboBox.getItemCount();i++){
      Object3D O3D = (Object3D) ComboBox.getItemAt(i);
      if(i==0)
          BB3.equalize(O3D.getBoundingBox());
      else
         BB3.extend(O3D.getBoundingBox());
   }
}


/** remove the given Object from this viewer*/
public void removeObject(SecondoObject o){
   removeAll();
   /* ID id = o.getID();
    removeGraphicalObjects(id);
    int index = getIndexOf(o);
    if(index>=0)
       ComboBox.removeItemAt(index);
    Img.paint();
    ImgPanel.repaint();
    */
 }

/** remove all objects from this viewer */
public void removeAll(){
  ComboBox.removeAllItems();
  Img.removeAll();
  myPoints.empty();
  myLines.empty();
  myTriangles.empty();
  Img.paint();
  ImgPanel.repaint();
}

 
/** remove all graphical object with given ID */
private void removeGraphicalObjects(ID id){
    int i=0;
    while (i<myPoints.getSize())
       if(myPoints.get(i).getID().equals(id))
          myPoints.remove(i);
       else
          i++;
          
    i=0;
    while (i<myLines.getSize())
       if(myLines.get(i).getID().equals(id))
          myLines.remove(i);
       else
          i++;
          
    i=0;
    while (i<myTriangles.getSize())
       if(myTriangles.get(i).getID().equals(id))
          myTriangles.remove(i);
       else
          i++;
         
    Img.remove(id);
}


/** returns true if o is a fuzzy object */
public boolean canDisplay(SecondoObject o){
    ListExpr LE = o.toListExpr();
    if(LE.listLength()!=2) return false;
    ListExpr type = LE.first();
    if (!(type.isAtom() && type.atomType()==ListExpr.SYMBOL_ATOM))
       return false;
    String TypeName = type.symbolValue();
    return (TypeName.equals("fpoint") || TypeName.equals("fregion") || TypeName.equals("fline"));  
 }


/** returns true if o already displayes */
 public boolean isDisplayed(SecondoObject o){
   return getIndexOf(o)>=0;
}

/** select O in the ComboBox if displayed */
public  boolean selectObject(SecondoObject O){
    int index = getIndexOf(O);
    if (index<0)
       return false;
    else{
      ComboBox.setSelectedIndex(index);
      return true;
    }
 }

/** returns the menuextension for this viewer */
 public MenuVector getMenuVector(){
    return MenuExtension;
 }


/** returns the index of SO in the ComboBox indicated by the ID
  * if the combobox not contains SO -1 is returned 
  */
private int getIndexOf(SecondoObject SO){
   int pos = -1;
   int count= ComboBox.getItemCount();
   boolean found =false;
   ID id = SO.getID();
   for(int i=0;i<count && !found;i++){
      Object3D O3D = (Object3D)ComboBox.getItemAt(i);
      if(id.equals(O3D.getID())){
       pos=i;
       found = true;
      }
   }
   return pos;
 } 


/* computes the image and paint it */
private void update(){
  computeBoundingBox();
  DisplayBB.readFrom(BB3);
  BBLabel.setText("BoundingBox ="+DisplayBB);
  Img.removeAll();
  if (AutoBoundingBox){ 
     AutoBB2.setTo(BB3.getMinX()-BorderSize,BB3.getMinY()-BorderSize,
             BB3.getMaxX()+BorderSize,BB3.getMaxY()+BorderSize);
     Img.setBoundingBox(AutoBB2);        
  }
  Img.add(myPoints);
  Img.add(myLines);
  Img.add(myTriangles);
  Img.paint();
  ImgPanel.repaint();
}


/** shows the settings dialog for the selected object */
private void showObjectSettings(){
  Object3D O3D = (Object3D) ComboBox.getSelectedItem();
  if(O3D==null){
     MessageBox.showMessage("no Object selected");
     return;
  }
  O3D.showSettings(VC.getMainFrame());
  removeGraphicalObjects(O3D.getID());
  addGraphicalObjects(O3D);
  update();
}

private IDPoint3DVector myPoints=new IDPoint3DVector();
private Line3DVector myLines= new Line3DVector();
private Triangle3DVector myTriangles=new Triangle3DVector();
private BoundingBox3D BB3=new BoundingBox3D();
private JScrollPane ScrollPane;
private FuzzyImage Img; 
private JComboBox ComboBox;
private JPanel ImgPanel; 
private JLabel BBLabel = new JLabel();
private int pw=1280;
private int ph=1024;
private MenuVector MenuExtension = new MenuVector();
private JMenu ObjectMenu = new JMenu("Object");
private JMenu ViewMenu = new JMenu("View");
private JLabel CoordLabel = new JLabel("(0,0)");
private BoundingBox2D DisplayBB = new BoundingBox2D();

private ViewConfig View;
private boolean AutoBoundingBox = true;
private double BorderSize = 0.0;
private BoundingBox2D BB2 = new BoundingBox2D();
private BoundingBox2D AutoBB2 = new BoundingBox2D();
private boolean Proportional = true;

}














