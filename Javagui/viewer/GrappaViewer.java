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

package viewer;

import javax.swing.*;
import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import gui.SecondoObject;
import sj.lang.ListExpr;
import att.grappa.*;
import java.io.*;
import tools.Reporter; 

/* this viewer shows all SecondoObjects as Lists */
public class GrappaViewer extends SecondoViewer{

 private JScrollPane scrollPane = new JScrollPane(); 
 private JPanel emptyPanel = new JPanel();
 private JPanel currentPanel;
 private JComboBox comboBox = new JComboBox();
 private Vector itemObjects = new Vector(10,5);
 private SecondoObject currentObject=null;
 private MenuVector mV = new MenuVector();
 

/* create a new GrappaViewer */
 public GrappaViewer(){
   currentPanel = emptyPanel;
   setLayout(new BorderLayout());
   add(BorderLayout.NORTH,comboBox);
   add(BorderLayout.CENTER,scrollPane);
   scrollPane.setViewportView(currentPanel); // replace by the grappa component !!
   
  JMenu StdMenu = new JMenu("Grappa-Viewer");
  JMenuItem MI_Remove = StdMenu.add("remove");
  JMenuItem MI_RemoveAll = StdMenu.add("remove all");
  mV.addMenu(StdMenu);

  MI_Remove.addActionListener( new ActionListener(){
       public void actionPerformed(ActionEvent e){
          if (currentObject!=null){
             SecondoObject Victim = currentObject;
             if ( itemObjects.remove(currentObject)){
                comboBox.removeItem(currentObject.getName());
                currentObject = null;
                int index = comboBox.getSelectedIndex();          // the new current object
                if (index>=0){
                   currentObject = (SecondoObject) itemObjects.get(index);
                   showObject();
                }
             }
             if (VC!=null)
                VC.removeObject(Victim);  // inform the ViewerControl
          }
      }});

   MI_RemoveAll.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent e){
        itemObjects.removeAllElements();
        comboBox.removeAllItems();
        currentObject= null;
        if(VC!=null)
           VC.removeObject(null);
        showObject();
      }
   });

   comboBox.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
           showObject();
           if(VC !=null){
                int index = comboBox.getSelectedIndex();
                if (index>=0){
                   try{
                       currentObject = (SecondoObject) itemObjects.get(index);
                       VC.selectObject(GrappaViewer.this,currentObject);
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
      itemObjects.add(o);
      comboBox.addItem(o.getName());
      try{
         comboBox.setSelectedIndex(comboBox.getItemCount()-1);  // make the new object to active object
         showObject();
      }
      catch(Exception e){}
   } 
   return true;
 }

 /* returns true if o a SecondoObject in this viewer */
 public boolean isDisplayed(SecondoObject o){
   return itemObjects.indexOf(o)>=0;

 } 

 /** remove o from this Viewer */
 public void removeObject(SecondoObject o){
    if (itemObjects.remove(o))
        comboBox.removeItem(o.getName());
 }


 /** remove all containing objects */
 public void removeAll(){
     itemObjects.removeAllElements();
     comboBox.removeAllItems();
     currentObject= null;
     if(VC!=null)
        VC.removeObject(null);
     showObject();
 }    


 /* returns allways true (this viewer can display all SecondoObjects) */
 public boolean canDisplay(SecondoObject o){
    ListExpr type = o.toListExpr();
    if(type.listLength()!=2){
      return false;
    }
    type = type.first();
    return type.atomType()==ListExpr.SYMBOL_ATOM && type.symbolValue().equals("graphviz");
 }


 /* returns the Menuextension of this viewer */
 public MenuVector getMenuVector(){
    return mV;
 }

 /* returns Grappa */
 public String getName(){
    return "Grappa";
 }
   

 /* select O */
 public boolean selectObject(SecondoObject O){
    int i=itemObjects.indexOf(O);
    if (i>=0) {
       comboBox.setSelectedIndex(i);
       showObject();
       return true;
    }else //object not found
      return false;
 }


 private void showObject(){
    currentPanel = emptyPanel;
    scrollPane.setViewportView(currentPanel);
    
    int index = comboBox.getSelectedIndex();
    if (index>=0){
    try{
       currentObject = (SecondoObject) itemObjects.get(index);
       String text = currentObject.toListExpr().second().textValue();
       InputStream in = new ByteArrayInputStream(text.getBytes());
       Parser parser = new Parser(in,System.err); 
       parser.parse();
       Graph graph = parser.getGraph();
       GrappaPanel gp = new GrappaPanel(graph);
       gp.setScaleToFit(false);
       currentPanel = gp;
       scrollPane.setViewportView(currentPanel);
    }
    catch(Exception e){
      Reporter.debug(e);
    }
    }
 }


}

