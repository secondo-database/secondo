package viewer;

import javax.swing.*;
import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import gui.SecondoObject;

/* this viewer shows all SecondoObjects as Lists */
public class StandardViewer extends SecondoViewer{

 private JScrollPane ScrollPane = new JScrollPane(); 
 private JTextArea TextArea = new JTextArea();
 private JComboBox ComboBox = new JComboBox();
 private Vector ItemObjects = new Vector(10,5);
 private MenuVector MV = new MenuVector();
 private SecondoObject CurrentObject=null;

/* create a new StandardViewer */
 public StandardViewer(){
   setLayout(new BorderLayout());
   add(BorderLayout.NORTH,ComboBox);
   add(BorderLayout.CENTER,ScrollPane);
   ScrollPane.setViewportView(TextArea);
 //  ScrollPane.add(TextArea);
   JMenu StdMenu = new JMenu("Standard-Viewer");
   JMenuItem MI_Remove = StdMenu.add("remove");
   JMenuItem MI_RemoveAll = StdMenu.add("remove all");
   MV.addMenu(StdMenu);

   MI_Remove.addActionListener( new ActionListener(){
       public void actionPerformed(ActionEvent e){
          if (CurrentObject!=null){
             SecondoObject Victim = CurrentObject;
             if ( ItemObjects.remove(CurrentObject)){
                ComboBox.removeItem(CurrentObject.getName());
                CurrentObject = null;
                int index = ComboBox.getSelectedIndex();          // the new current object
                if (index>=0){
                   CurrentObject = (SecondoObject) ItemObjects.get(index);
                   showObject();
                }
             }
             if (VC!=null)
                VC.removeObject(Victim);  // inform the ViewerControl
          }
      }});

   MI_RemoveAll.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent e){
        ItemObjects.removeAllElements();
        ComboBox.removeAllItems();
        CurrentObject= null;
        if(VC!=null)
           VC.removeObject(null);
        showObject();
      }
   });

   ComboBox.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
           showObject();
           if(VC !=null){
                int index = ComboBox.getSelectedIndex();
                if (index>=0){
                   try{
                       CurrentObject = (SecondoObject) ItemObjects.get(index);
                       VC.selectObject(StandardViewer.this,CurrentObject);
                   }
                   catch(Exception e){}
                }
           }
     }});

 }


 /* adds a new Object to this Viewer and display it */
 public boolean addObject(SecondoObject o){
   if (isDisplayed(o))
       selectObject(o);
   else{
      ItemObjects.add(o);
      ComboBox.addItem(o.getName());
      try{
         ComboBox.setSelectedIndex(ComboBox.getItemCount()-1);  // make the new object to active object
         showObject();
      }
      catch(Exception e){}
   } 
   return true;
 }

 /* returns true if o a SecondoObject in this viewer */
 public boolean isDisplayed(SecondoObject o){
   return ItemObjects.indexOf(o)>=0;

 } 

 /** remove o from this Viewer */
 public void removeObject(SecondoObject o){
    if (ItemObjects.remove(o))
        ComboBox.removeItem(o.getName());
 }


 /** remove all containing objects */
 public void removeAll(){
     ItemObjects.removeAllElements();
     ComboBox.removeAllItems();
     CurrentObject= null;
     if(VC!=null)
        VC.removeObject(null);
     showObject();
 }    


 /* returns allways true (this viewer can display all SecondoObjects) */
 public boolean canDisplay(SecondoObject o){
    return true;
 }


 /* returns the Menuextension of this viewer */
 public MenuVector getMenuVector(){
    return MV;
 }

 /* returns Standard */
 public String getName(){
    return "Standard";
 }
   

 /* select O */
 public boolean selectObject(SecondoObject O){
    int i=ItemObjects.indexOf(O);
    if (i>=0) {
       ComboBox.setSelectedIndex(i);
       showObject();
       return true;
    }else //object not found
      return false;
 }


 private void showObject(){
    TextArea.setText("");
    int index = ComboBox.getSelectedIndex();
    if (index>=0){
    try{
       CurrentObject = (SecondoObject) ItemObjects.get(index);
       TextArea.setText(CurrentObject.toListExpr().writeListExprToString());
    }
    catch(Exception e){}
    }
 }


}

