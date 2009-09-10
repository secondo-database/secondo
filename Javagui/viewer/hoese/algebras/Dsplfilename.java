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

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import javax.swing.*;
import tools.Reporter;
import java.io.*;
import viewer.hoese.algebras.fileviewers.*;
import java.util.Vector;


/**
 * A displayclass for the html formatted code 
 */
public class Dsplfilename extends DsplGeneric implements ExternDisplay{

private boolean defined;

 /** Creates a new Instance of this.
   */ 
public  Dsplfilename(){
   if(Display==null){
      Display = new FileViewerFrame();
   }
}



public String toString(){
   return Entry;
}

public void init (String name, int nameWidth, int indent,
                  ListExpr type, ListExpr value, QueryResult qr)
  {
     String T = name;
     String V;

     if (value.listLength()==1){
         value = value.first();
     }
     defined = !isUndefined(value);
     if(!defined){
        V = "undefined";
     }else{
				 if((value.atomType()!=ListExpr.TEXT_ATOM) && 
            (value.atomType()!=ListExpr.STRING_ATOM)){
						V =  "error in value ";
            defined = false;
				 }
				 else{
            if(value.atomType()==ListExpr.TEXT_ATOM){
             	V =  value.textValue();
            } else {
              V = value.stringValue();
            }
				 }
     }
     T=extendString(T, nameWidth, indent);
     text = V;
     File f = new File(V);
     Entry = f.getName();
     if(Entry.length()>MAX_DIRECT_DISPLAY_LENGTH){
        Entry = Entry.substring(0,MAX_DIRECT_DISPLAY_LENGTH-3)+"...";
     } 
     // look for an possible Viewe
     if(qr!=null){
        qr.addEntry(this);
     }
     return;
}

public void displayExtern(){
    Display.setSource(this);
    Display.setVisible(true);    
}

public boolean isExternDisplayed(){
   return Display.isVisible() && this.equals(Display.getSource());
}


private static FileViewerFrame Display=null; 
private String Entry; // shortened filename
private String text;  // full filename

private static final int MAX_DIRECT_DISPLAY_LENGTH = 15;

private static final String WHITESPACES = " \t\n\r";


private static class FileViewerFrame extends JFrame{

private static Vector fileViewers;
private static NoFileViewer noFileViewer;
private static JFileChooser baseDirSelect;
private String baseDir = "./";

private JLabel fullName;

public FileViewerFrame(){
 if(fileViewers==null){
    fileViewers = new Vector();
    fileViewers.add(new JPGViewer());
    // ensure that NoFileViewer is the last one in the vector
  }
  noFileViewer = new NoFileViewer(); 
  getContentPane().setLayout(new BorderLayout());

  Display = noFileViewer;

  JPanel controlPanel = new JPanel();
  JButton selectDir = new JButton("Base Directory");

  JButton closeBtn = new JButton("Close");
  closeBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            FileViewerFrame.this.setVisible(false);
       }
  } );
  JButton repaintBtn = new JButton("repaint");
  repaintBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            FileViewerFrame.this.setSource(FileViewerFrame.this.Source);
       }
  } );
  

  controlPanel.add(selectDir);
  controlPanel.add(repaintBtn);
  controlPanel.add(closeBtn);
  
  fullName = new JLabel("");
  JPanel p = new JPanel();
  p.add(fullName);
  getContentPane().add(p,BorderLayout.NORTH);
  getContentPane().add(Display,BorderLayout.CENTER);
  getContentPane().add(controlPanel,BorderLayout.SOUTH);

  baseDirSelect = new JFileChooser(".");
  baseDirSelect.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
  baseDirSelect.setAcceptAllFileFilterUsed(false);


  selectDir.addActionListener(new ActionListener(){
   public void actionPerformed(ActionEvent evt){
     if(baseDirSelect.showOpenDialog(FileViewerFrame.this)==JFileChooser.APPROVE_OPTION){
          baseDir = baseDirSelect.getSelectedFile().getAbsolutePath();
          if(!baseDir.endsWith(File.separator)){
            baseDir += File.separator;
          }
     }
   }
  });
  setSize(640,480); 
}


public void setSource(Dsplfilename S){
  Source = S;
  // add processing of the base directory here
  fullName.setText(S.text);
  File f = new File(S.text);
  if(!f.isAbsolute()){
     f = new File(baseDir,S.text);
  }

  if(!f.exists()){
     noFileViewer.setMessage("File not found");
     selectViewer(noFileViewer);
  } else {
     for(int i=0;i<fileViewers.size();i++){
         FileViewer fV = (FileViewer) fileViewers.get(i);
         if(fV.canDisplay(f)){
            selectViewer(fV);
            fV.display(f);
            return;
         }   
     }
     // no appropriate viewer found
     noFileViewer.setMessage("no viewer found");
     selectViewer(noFileViewer);
  }
}

public Dsplfilename getSource(){
     return Source;
}

private void selectViewer(FileViewer fV){
   if(currentViewer!=null) {
      getContentPane().remove(currentViewer);
   }
   currentViewer = fV;
   getContentPane().add(fV,BorderLayout.CENTER);
   invalidate();
   validate();
   repaint(); 
}


private FileViewer Display;
private JButton CloseBtn;
private Dsplfilename Source;
private FileViewer currentViewer=null;

// class for error messages
private static class NoFileViewer extends FileViewer{
  private String message ="no message found";

  public boolean canDisplay(File f){
     return true;
  }
  
  public boolean display(File f){
     repaint();
     return true;
  }

  public void setMessage(String m){
    message = m;
  }
  
  public void paint(Graphics g){
    int h = getHeight();
    int w = getWidth();
    Graphics2D g2 = (Graphics2D)g;
    g2.scale(2,2);
    Rectangle2D r = g2.getFont().getStringBounds(message,g2.getFontRenderContext());    
    int sw = (int) r.getWidth();
    g2.drawString(message,h/4,(w-sw)/4); 
  }
}
}

}



