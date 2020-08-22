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
import java.io.*;
import tools.Reporter; 
import java.awt.*;
import java.awt.event.*;


import org.apache.batik.anim.dom.SAXSVGDocumentFactory;
import org.apache.batik.swing.JSVGCanvas;
import org.apache.batik.swing.svg.SVGUserAgentAdapter;
import org.apache.batik.transcoder.TranscoderInput;
import org.apache.batik.transcoder.TranscoderOutput;
import org.apache.batik.util.XMLResourceDescriptor;

import org.apache.fop.render.ps.*;
import org.apache.batik.transcoder.*;
import org.w3c.dom.Document;
import java.net.URI;
import org.apache.batik.swing.svg.SVGUserAgentAdapter;


/* this viewer shows all SecondoObjects as Lists */
public class SvgViewer extends SecondoViewer{

 private JScrollPane scrollPane = new JScrollPane(); 
 private JPanel emptyPanel = new JPanel();
 private JComponent currentPanel;
 private JComboBox comboBox = new JComboBox();
 private Vector itemObjects = new Vector(10,5);
 private SecondoObject currentObject=null;
 private MenuVector mV = new MenuVector();
 private JSVGCanvas svgCanvas;

 private JButton loadBtn;
 private JButton saveBtn;
 private JButton saveEPSBtn;
 private JFileChooser fc;
 private SAXSVGDocumentFactory factory;
 private String lastUri;


/* create a new SvgViewer */
 public SvgViewer(){
   String parser = XMLResourceDescriptor.getXMLParserClassName();
   factory = new SAXSVGDocumentFactory(parser); 

   currentPanel = emptyPanel;
   setLayout(new BorderLayout());
   add(BorderLayout.NORTH,comboBox);
   add(BorderLayout.CENTER,scrollPane);
   scrollPane.setViewportView(currentPanel); // replace by the svg component !!
   
  JMenu StdMenu = new JMenu("SVG-Viewer");
  JMenuItem MI_Remove = StdMenu.add("remove");
  JMenuItem MI_RemoveAll = StdMenu.add("remove all");
  mV.addMenu(StdMenu);

  SVGUserAgentAdapter ua = new SVGUserAgentAdapter();
  svgCanvas = new JSVGCanvas(ua, true, true);

  loadBtn = new JButton("load");
  saveBtn = new JButton("save");
  saveEPSBtn = new JButton("save as eps");
  loadBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         loadGraphic();
      }
   }); 

  saveBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        saveSvg();
     }
  });

  saveEPSBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        saveEPS();
     }
  });

   lastUri= "" + (new File(".")).toURI();;

   fc = new JFileChooser(".");
   fc.setAcceptAllFileFilterUsed(false);
   javax.swing.filechooser.FileFilter filter = new javax.swing.filechooser.FileFilter(){
     public boolean accept(File f){
       return f.isDirectory() || f.getName().endsWith(".svg");
     }
     public String getDescription(){
        return "svg images";
     }
   };
   fc.setFileFilter(filter);


   JPanel control = new JPanel();
   control.add(loadBtn);
   control.add(saveBtn);
   control.add(saveEPSBtn);
   add(BorderLayout.SOUTH, control);


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
                       VC.selectObject(SvgViewer.this,currentObject);
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
    return type.atomType()==ListExpr.SYMBOL_ATOM && type.symbolValue().equals("svg");
 }


 /* returns the Menuextension of this viewer */
 public MenuVector getMenuVector(){
    return mV;
 }

 /* returns SVG */
 public String getName(){
    return "SVG";
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



 private void loadGraphic(){
    int choice = fc.showOpenDialog(null);
    if (choice != JFileChooser.APPROVE_OPTION) {
       return;
    }
    currentPanel = emptyPanel;
    scrollPane.setViewportView(currentPanel);
    File f = fc.getSelectedFile();
    try {
      lastUri = f.toURI().toString();
      svgCanvas.setURI(lastUri);
      
      // create a Secondo Object from file
      BufferedReader r = new BufferedReader(new FileReader(f));
      StringBuffer sb = new StringBuffer();
      while(r.ready()){
        sb.append(r.readLine()+"\n");
      } 
      r.close();
      ListExpr t =  ListExpr.textAtom(sb.toString());
      SecondoObject so = new SecondoObject( f.getName() ,
            ListExpr.twoElemList( ListExpr.symbolAtom("svg"), t));
      addObject(so);
      if(VC!=null){ // build a secondo object form the file
         VC.addObject(so); 
       }
    } catch(Exception e){
       Reporter.showError("Error in loading svg graphic"); 
       Reporter.debug(e);
    }
 }

 private void saveSvg(){
    int index = comboBox.getSelectedIndex();
    if (index<0){
       tools.Reporter.showError("no svg available"); 
    } else {
       int choice = fc.showSaveDialog(null);
       if (choice != JFileChooser.APPROVE_OPTION) {
          return;
       }
       SecondoObject obj = (SecondoObject) itemObjects.get(index);
       
       try{
          File f = fc.getSelectedFile();
           
          String svg = obj.toListExpr().second().textValue();
          System.out.println(obj.toListExpr().second());
          System.out.println(svg);
          BufferedWriter bf = new BufferedWriter(new FileWriter(f));
          bf.write(svg,0,svg.length());
          bf.close();
      }catch(Exception e){
        tools.Reporter.debug(e);
        tools.Reporter.showError("problem in storing file");
      }
    }
 }

 private void saveEPS(){
    int index = comboBox.getSelectedIndex();
    if (index<0){
       tools.Reporter.showError("no svg available");
       return;
    }
    try{
      SecondoObject obj  = (SecondoObject) itemObjects.get(index);
      String text = obj.toListExpr().second().textValue();
      InputStream in = new ByteArrayInputStream(text.getBytes());
      String fn = lastUri;
      Document doc = factory.createSVGDocument(fn,in);

      EPSTranscoder t = new EPSTranscoder();
      int choice = fc.showSaveDialog(null);
      if (choice != JFileChooser.APPROVE_OPTION) {
          return;
      }
      TranscoderInput input = new TranscoderInput(doc);
      OutputStream o = new FileOutputStream(fc.getSelectedFile());
      TranscoderOutput output = new TranscoderOutput(o);
      t.transcode(input, output);
      o.flush();
      o.close();
   } catch(Exception e){
      tools.Reporter.debug(e);
      tools.Reporter.showError("problem in writing file");
   }
 }



 private void showObject(){
   // delete old stuff
   currentPanel = emptyPanel;
   scrollPane.setViewportView(currentPanel);
   // get the current object
   int index = comboBox.getSelectedIndex();
   if (index>=0){
       try{
          SecondoObject obj = currentObject = (SecondoObject) itemObjects.get(index);
          String text = obj.toListExpr().second().textValue(); 
          InputStream in = new ByteArrayInputStream(text.getBytes());
          String fn = lastUri;
          Document doc = factory.createSVGDocument(fn,in);
          svgCanvas.setDocument(doc);
          currentPanel = svgCanvas;
          scrollPane.setViewportView(currentPanel);
       } catch(Exception e){
          Reporter.showError("problem in creating svg graphic");
          Reporter.debug(e);
       }
   }
 }


}

