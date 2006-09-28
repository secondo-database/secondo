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

package viewer.hoese.algebras;

import javax.swing.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.*;
import viewer.hoese.DateTime;
import java.io.*;
import tools.Reporter;




public class FunctionFrame extends JFrame{
   public FunctionFrame(){
      super();
      setSize(640,480);
      getContentPane().setLayout(new BorderLayout());
      functionpanel.setOpaque(true);
      functionpanel.setBackground(Color.WHITE);
      functionSP = new JScrollPane(functionpanel);
      getContentPane().add(functionSP,BorderLayout.CENTER);
      JPanel P1 = new JPanel(new GridLayout(1,3));
      P1.add(TimeLabel);
      P1.add(ValueLabel);
      getContentPane().add(P1,BorderLayout.NORTH);
      functionpanel.addMouseMotionListener(new MouseMotionAdapter(){
           public void mouseMoved(MouseEvent e){
               if(functionpanel.getOrig(e.getX(),e.getY(),P,MP)){
                   TimeLabel.setText("x= "+DateTime.getString(P.x));
                   ValueLabel.setText("y= "+P.y);
               } else{
                  TimeLabel.setText(" ");
                  ValueLabel.setText(" ");
               }
           }
      });
      functionpanel.showCross(true);
      functionpanel.showY(true);
      closeBtn.addActionListener(new ActionListener() {
             public void actionPerformed(ActionEvent evt){
                setVisible(false);
             }
      });

      JPanel controlPanel = new JPanel();
      controlPanel.add(saveBtn);
      controlPanel.add(closeBtn);
      getContentPane().add(controlPanel,BorderLayout.SOUTH);
      addMagnifier();
      // initialize filechooser
      javax.swing.filechooser.FileFilter ff = new javax.swing.filechooser.FileFilter(){
            public String getDescription(){
              return ("eps graphics");
            }
            public boolean accept(File f){
              if(f==null) return false;
              return f.getName().endsWith("eps");
            }
      };
      fc.setFileFilter(ff);
      saveBtn.addActionListener(new ActionListener(){
          public void actionPerformed(ActionEvent evt){
             exportToPS();
          }
      });

   }
    private void addMagnifier(){
      functionpanel.addMouseListener(new MouseAdapter(){
         public void mouseClicked(MouseEvent evt){
            double zf = 1.0;
            if(evt.getButton()==MouseEvent.BUTTON1){
                zf = 1.25;
            }
            if(evt.getButton()==MouseEvent.BUTTON3){
                zf=0.75;
            }
            if(evt.getButton()==MouseEvent.BUTTON2){
               zf=0.0;
            }
            Dimension oldDim = functionpanel.getSize();
            functionpanel.setVirtualSize(oldDim.getWidth()*zf,oldDim.getHeight()*zf);
            double x = evt.getX()*zf;
            double y = evt.getY()*zf;
            double nw = functionSP.getViewport().getWidth();
            double nh = functionSP.getViewport().getHeight();
            double px = Math.max(0,(int)(x-nw/2));
            double py = Math.max(0,(int)(y-nh/2));
            functionSP.getViewport().setViewPosition(new Point( (int)px,(int)py));
         }
      });
  }
  void setSource(Function function){
        this.function=function;
        functionpanel.setFunction(function);
        functionpanel.setInterval(function.getInterval().getStart(),function.getInterval().getEnd());
   }

   Function getSource(){
      return function;
   }

   private void exportToPS(){
     if(fc.showSaveDialog(this)==JFileChooser.APPROVE_OPTION){
        try{
          File F = fc.getSelectedFile();
          if(F.exists()){
            if(Reporter.showQuestion("File already exits\n overwrite it?")!=Reporter.YES){
              return;
            }
          }
          functionpanel.showCross(false);
          functionpanel.showY(false);
          boolean res = extern.psexport.PSCreator.export(functionpanel,F);
          if(!res){
             Reporter.showError("Error in Exporting image");
          }
        } catch(Exception e){
           Reporter.showError("Error in exporting graphic");
           Reporter.debug(e);
        } finally {
             functionpanel.showCross(true);
             functionpanel.showY(true);
        }
           
     }
   }

   FunctionPanel functionpanel = new FunctionPanel();
   Function function;
   JLabel TimeLabel = new JLabel(" ");
   JLabel ValueLabel = new JLabel(" ");
   Point2D.Double P = new Point2D.Double();
   Point2D.Double MP = new Point2D.Double();
   JButton closeBtn = new JButton("close");
   JButton saveBtn = new JButton("export to ps");
   JScrollPane functionSP;

   static JFileChooser fc = new JFileChooser();

}


