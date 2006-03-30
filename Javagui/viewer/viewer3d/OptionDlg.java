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

package viewer.viewer3d;

import viewer.Viewer3D;
import viewer.viewer3d.graphic3d.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import tools.Reporter;

public class OptionDlg extends JDialog{

public OptionDlg(Frame F,World3D W3D){
   super(F,"paint options",true);
   this.W3D = W3D;
   reset();  //take the values from W3D
   getContentPane().setLayout(new BorderLayout());
   JPanel POptions = new JPanel(new GridLayout(1,4));
   POptions.add(CB_Gradient);
   POptions.add(CB_Border);
   POptions.add(CB_Filled);
   POptions.add(CB_Proportional);
   getContentPane().add(POptions,BorderLayout.CENTER);

   JPanel PButtons = new JPanel();
   PButtons.add(OkBtn);
   PButtons.add(CancelBtn);
   PButtons.add(ResetBtn);
   PButtons.add(ApplyBtn);
   ActionListener AL = new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          Object Source = evt.getSource();
          if (Source.equals(OkBtn))
              if (OptionDlg.this.apply())
                 setVisible(false);
          if (Source.equals(CancelBtn))
              OptionDlg.this.cancel();
          if (Source.equals(ResetBtn))
              OptionDlg.this.reset();
          if(Source.equals(ApplyBtn))
            OptionDlg.this.apply();
       }
   }; 
   OkBtn.addActionListener(AL);
   CancelBtn.addActionListener(AL);
   ResetBtn.addActionListener(AL);
   ApplyBtn.addActionListener(AL);
   setSize(400,100);
   getContentPane().add(PButtons,BorderLayout.SOUTH);
}


public void reset(){
   if(W3D!=null){
     CB_Border.setSelected(W3D.isBorder());
     CB_Filled.setSelected(W3D.isFill());
     CB_Gradient.setSelected(W3D.isGradient());
     CB_Proportional.setSelected(W3D.isProportional());
   }
}


public boolean apply(){
  if(W3D==null)
     return true;
  else{
     if(! (CB_Border.isSelected() | CB_Filled.isSelected())){
        Reporter.writeError("please select border or filled");
        return false;
     }
     else{
       W3D.setBorder(CB_Border.isSelected());
       W3D.setFill(CB_Filled.isSelected());
	 W3D.setGradient(CB_Gradient.isSelected());
	 W3D.setProportion(CB_Proportional.isSelected());
       W3D.update();
	 W3D.repaint();
       return true; 
     }
  }
}

public void cancel(){
  reset();
  setVisible(false); 
}

private World3D W3D;
private JCheckBox CB_Gradient = new JCheckBox("gradient");
private JCheckBox CB_Border = new JCheckBox("paint border");
private JCheckBox CB_Filled = new JCheckBox("filled");
private JCheckBox CB_Proportional = new JCheckBox("proportional");
private JButton OkBtn = new JButton("ok");
private JButton CancelBtn = new JButton("cancel");
private JButton ResetBtn = new JButton("reset");; 
private JButton ApplyBtn = new JButton("apply");
}
