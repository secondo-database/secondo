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

package viewer.hoese;

import java.awt.*;
import java.util.*;
import javax.swing.*;
import viewer.HoeseViewer;
/** 
 * This class lists the layers and allow moving up or down of layers, based upun swing's JDialog
 * @author  Höse
 * @version 
 */
public class LayerMgmt extends javax.swing.JDialog {
  /** Application's main window */
  HoeseViewer mw;
  /** An  array of all layers in GraphWindow */
  Component [] layers;
  /** A list of layers of type Layer ( no imagelayer) */
  Vector entry=new Vector(15,5);
  /** A description of each layer in entry */
  Vector LayerEntry=new Vector(15,5);
  /** Creates new JDialog LayerMgmt with a parent-frame and a list of layers
   * @see <a href="LayerMgmtsrc.html#LayerMgmt">Source</a>
   */
  public LayerMgmt(HoeseViewer parent,Component [] l) {
    super (new JFrame(), true);
    mw= parent;
    layers=l;
    QueryResult qr = (QueryResult)mw.TextDisplay.getQueryCombo().getSelectedItem();
    qr.clearSelection();
    setTitle("Layermanagement");
    initComponents ();
    pack ();
  }

  /** This method is called from within the constructor to
   * initialize the dialog.
   * @see <a href="LayerMgmtsrc.html#initComponents">Source</a> 
   */
  private void initComponents () {//GEN-BEGIN:initComponents
    LayerList = new javax.swing.JList ();
    Button_P = new javax.swing.JPanel ();
    Up_B = new javax.swing.JButton ();
    Down_B = new javax.swing.JButton ();
    Close_B = new javax.swing.JButton ();
    addWindowListener (new java.awt.event.WindowAdapter () {
      public void windowClosing (java.awt.event.WindowEvent evt) {
        closeDialog (evt);
      }
    }
    );

    LayerList.setVisibleRowCount (10);
    for (int i=layers.length-1;i>=0;i--)
      if (layers[i] instanceof Layer) {
      	Layer l=(Layer) layers[i];
      	LayerEntry.add(l);
      	entry.add(new String(l.LayerNo +":"+l.getGeoObjects().size()+" elements"));
      }
      LayerList.setListData(entry);
    	

    getContentPane ().add (LayerList, java.awt.BorderLayout.CENTER);

    Button_P.setLayout (new javax.swing.BoxLayout (Button_P, 1));

      Up_B.setText ("Up");
    Up_B.addActionListener(new java.awt.event.ActionListener() {

      /**
       * Moves the selected entry one position up, if not at the beginning
       * @param evt
       * @see <a href="LayerMgmtsrc.html#actionPerformed1">Source</a> 
       */
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        int ind=LayerList.getSelectedIndex();
        if (ind<=0) return;
        Object o=entry.elementAt(ind);
        entry.removeElementAt(ind);
        entry.insertElementAt(o,ind-1);
        o=LayerEntry.elementAt(ind);
        LayerEntry.removeElementAt(ind);
        LayerEntry.insertElementAt(o,ind-1);
        
        LayerList.setListData(entry);
      }
    });
  
      Button_P.add (Up_B);
  
    Down_B.addActionListener(new java.awt.event.ActionListener() {

      /**
       * Moves the selected entry one position down, if not at the end
       * @param evt
       * @see <a href="LayerMgmtsrc.html#actionPerformed2">Source</a> 
   */
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        int ind=LayerList.getSelectedIndex();
        if (ind>=entry.size()-1) return;
        Object o=entry.elementAt(ind);
        entry.removeElementAt(ind);
        entry.insertElementAt(o,ind+1);
        o=LayerEntry.elementAt(ind);
        LayerEntry.removeElementAt(ind);
        LayerEntry.insertElementAt(o,ind+1);
        
        LayerList.setListData(entry);
      }
    });
      Down_B.setText ("Down");
  
      Button_P.add (Down_B);
  
      Close_B.setText ("Apply");
    Close_B.addActionListener(new java.awt.event.ActionListener() {

      /**
       * Applies the order to the app.
       * @param evt
       * @see <a href="LayerMgmtsrc.html#actionPerformed3">Source</a> 
   */
      public void actionPerformed (java.awt.event.ActionEvent evt) {
	  mw.GraphDisplay.removeAll();
	  mw.LayerSwitchBar.removeAll();
	  for (int i=0;i<LayerEntry.size();i++){
	      Layer lay= (Layer)LayerEntry.elementAt(i);
	      lay.LayerNo=i+1;    	
    	  mw.GraphDisplay.add(lay, new Integer(i+1));
    	  mw.LayerSwitchBar.add(
    	  	lay.CreateLayerButton(mw.GraphDisplay.LayerButtonListener, i+1));
      	}
      closeDialog(null);
      }
    });
  
      Button_P.add (Close_B);
  

    getContentPane ().add (Button_P, java.awt.BorderLayout.EAST);

  }//GEN-END:initComponents

  /** Closes the dialog * @see <a href="LayerMgmtsrc.html#closeDialog">Source</a> 
   */
  private void closeDialog(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_closeDialog
    setVisible (false);
    dispose ();
  }//GEN-LAST:event_closeDialog



  // Variables declaration - do not modify//GEN-BEGIN:variables
  private javax.swing.JList LayerList;
  private javax.swing.JPanel Button_P;
  private javax.swing.JButton Up_B;
  private javax.swing.JButton Down_B;
  private javax.swing.JButton Close_B;
  // End of variables declaration//GEN-END:variables

}
