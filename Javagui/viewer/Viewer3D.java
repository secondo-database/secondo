package viewer;

import viewer.viewer3d.*;
import viewer.viewer3d.graphic3d.*;
import java.awt.event.*;
import javax.swing.*;
import gui.*;
import java.awt.*;
import sj.lang.ListExpr;
import viewer.viewer3d.objects.*;
import java.beans.*;
import gui.idmanager.*;

public class Viewer3D extends SecondoViewer{

 public Viewer3D(){
   W3D = new World3D();
   W3D.setWindow(200,200);
   W3D.setView(200,-200,500,100,600,150,0,0,1);
 
   BoundingBoxLabel = new JLabel(BBText+BoundingBox.toString());

   MenuExtension = new MenuVector();
   JMenu V3DMenu = new JMenu("Fuzzy-Viewer");
   OptionsView=new Options3D(null,W3D);
   OptionsPaint = new OptionDlg(null,W3D); 
   JMenuItem MI_showOptions = V3DMenu.add("3D-Options");
   
   MI_showOptions.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt) {
         showOptions3D();
      }
   });

   JMenuItem MI_showPaintOptions = V3DMenu.add("paint options");
   MI_showPaintOptions.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         showPaintOptions();
     }
   });

   JMenu ObjectMenu = new JMenu("Objects");
   JMenuItem MI_Clear = ObjectMenu.add("remove all");
   MI_Clear.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           clear();
       }});  
   JMenuItem MI_RemoveSelected = ObjectMenu.add("remove selected object");
   MI_RemoveSelected.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          removeSelectedObject();
       }});

    JMenuItem MI_ShowObjectSettings = ObjectMenu.add("settings");
    MI_ShowObjectSettings.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          showObjectSettings();
       }});
  

  MenuExtension.addMenu(V3DMenu); 
  MenuExtension.addMenu(ObjectMenu);
  ComboBox = new JComboBox();
  setLayout(new BorderLayout());
  add(ComboBox,BorderLayout.NORTH);
  add(W3D,BorderLayout.CENTER);
  add(BoundingBoxLabel,BorderLayout.SOUTH);
 }

private void showObjectSettings(){
  Object o = ComboBox.getSelectedItem();
  if(o==null) return;
  Object3D O3D = (Object3D) o;
  ID id = O3D.getID();
  W3D.removeID(id);      // remove the old representation
  O3D.showSettings(VC.getMainFrame());

  IDPoint3DVector PV = O3D.getPoints();  
  Line3DVector LV = O3D.getLines();
  Triangle3DVector TV = O3D.getTriangles();
  if(PV!=null) W3D.add(PV);
  if(LV!=null) W3D.add(LV);
  if(TV!=null) W3D.add(TV); 
  W3D.update();
  W3D.repaint();
}


/** shows the dialog for view settings */
private void showOptions3D(){
    OptionsView.reset();
    OptionsView.setBoundingBox(BoundingBox);
    OptionsView.show();
}

/** remove all Aobjects from Viewer */
private void clear(){
   ComboBox.removeAllItems();
   W3D.removeAll();
   W3D.repaint();
}

public void removeAll(){
  clear();

}


private void showPaintOptions(){
    OptionsPaint.reset();
    OptionsPaint.show();    
}


 public String getName(){
     return "Fuzzy_Viewer";
 }

 
 public boolean addObject(SecondoObject o){
   if(isDisplayed(o))
     return true;

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
       ComboBox.addItem(P3D);
       BoundingBox.extend(P3D.getBoundingBox());
       BoundingBoxLabel.setText(BBText+BoundingBox);
       W3D.add(PV);
       W3D.repaint();
       return true;
   }else if(TypeName.equals("fline")){
       FLine3D L3D = new FLine3D();
       if(!L3D.readFromSecondoObject(o))
          return false;
       Line3DVector LV = L3D.getLines();
       ComboBox.addItem(L3D);
       BoundingBox.extend(L3D.getBoundingBox());
       BoundingBoxLabel.setText(BBText+BoundingBox);
       W3D.add(LV);
       W3D.repaint();
       return true;

   } else if(TypeName.equals("fregion")){
       FRegion3D R3D = new FRegion3D();
       if(!R3D.readFromSecondoObject(o))
          return false;
       BoundingBox.extend(R3D.getBoundingBox());
       BoundingBoxLabel.setText(BBText+BoundingBox);
       Triangle3DVector TV = R3D.getTriangles();        
       ComboBox.addItem(R3D);
       W3D.add(TV);
       W3D.repaint();
       return true;
   } else
      return false;
 }

 private void removeSelectedObject(){
   Object o = ComboBox.getSelectedItem();
   if(o==null)
      return;
   Object3D O3D = (Object3D) o;
   ID id = O3D.getID();
   W3D.removeID(id);
   ComboBox.removeItem(o);
   computeBoundingBox();
   W3D.repaint();
 }


 private void computeBoundingBox(){
   BoundingBox.set(0,0,0,0,0,0);
   for(int i=0;i<ComboBox.getItemCount();i++){
      Object3D O3D = (Object3D) ComboBox.getItemAt(i);
      if(i==0)
          BoundingBox.equalize(O3D.getBoundingBox());
      else
         BoundingBox.extend(O3D.getBoundingBox());         
   }
   BoundingBoxLabel.setText(BBText+BoundingBox); 
 }


 public void removeObject(SecondoObject o){
   int index = getIndexOf(o);
   if(index>=0){
      Object3D O3D = (Object3D) ComboBox.getItemAt(index);
      ID id = O3D.getID();
      W3D.removeID(id);
      ComboBox.removeItemAt(index);
      computeBoundingBox();
      W3D.repaint();
   }
 }

 public boolean canDisplay(SecondoObject o){
    ListExpr LE = o.toListExpr();
    if(LE.listLength()!=2) return false;
    ListExpr type = LE.first();
    if (!(type.isAtom() && type.atomType()==ListExpr.SYMBOL_ATOM))
       return false;
    String TypeName = type.symbolValue();
    return (TypeName.equals("fpoint") || TypeName.equals("fregion") || TypeName.equals("fline"));  
 }


 public boolean isDisplayed(SecondoObject o){
   return getIndexOf(o)>=0;
 }

 public  boolean selectObject(SecondoObject O){
     int index = getIndexOf(O);
     if(index<0)
        return false;
     else{
        ComboBox.setSelectedIndex(index);
        return true;
     }
       
 }

 public MenuVector getMenuVector(){
    return MenuExtension;
 }


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

 public double getDisplayQuality(SecondoObject o){
   if(canDisplay(o))
     return 0.95;
   else
     return 0;
 
 }
 private MenuVector MenuExtension;
 private Options3D OptionsView; 
 private OptionDlg OptionsPaint; 
 private World3D W3D;
 private JComboBox ComboBox;
 private JLabel BoundingBoxLabel;
 private BoundingBox3D BoundingBox = new BoundingBox3D();
 private final String BBText = "BoundingBox =";
}



