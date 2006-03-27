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


package  viewer.hoese;

import  java.util.*;
import  javax.swing.*;
import  java.awt.*;
import viewer.HoeseViewer;


/** 
 * A Dialog based on Swing JDialog, to set labeltext and label-position of a graphic-object
 * @author  hoese
 * @version 
 */
public class LabelAttrDlg extends javax.swing.JDialog {
  HoeseViewer mw;
  JPanel InfoPanel;
  DsplGraph AktGO;

  /** Constructor:Creates new dialog LabelAttrDlg
   * @param parent The applications HoeseViewer  
   * @param dg The graphic-object with the label parameters stored in AktGO
   * @see <a href="LabelAttrDlgsrc.html#LabelAttrDlg">Source</a> 
   */
  public LabelAttrDlg (HoeseViewer parent, DsplGraph dg) {
    super(parent.getMainFrame(), true);
    setTitle("Label Attributes");
    mw = parent;
    AktGO = dg;
    initComponents();
    pack();
    setResizable(false);
  }

  /**
   * Method to init. the widgets of the dialog
   * @see <a href="LabelAttrDlgsrc.html#initComponents">Source</a> 
   */
  private void initComponents () {              //GEN-BEGIN:initComponents
    OKB = new javax.swing.JButton();
    CancelB = new javax.swing.JButton();
    LTLabel = new javax.swing.JLabel();
    LabelText = new JTextField();
    LabXOffText = new JTextField(6);
    LabYOffText = new JTextField(6);
    InfoPanel = new JPanel();
    setSize(300,150);
    InfoPanel.setPreferredSize(new Dimension(250, 100));
    LTLabel.setText("Label Text:");
    InfoPanel.add(LTLabel);
    LabelText.setColumns(10);
    LabelText.setText(AktGO.getLabelText(CurrentState.ActualTime));
    LabXOffText.setText(Double.toString(AktGO.getLabPosOffset().getX()));
    LabYOffText.setText(Double.toString(AktGO.getLabPosOffset().getY()));
    InfoPanel.add(LabelText);
    InfoPanel.add(new JLabel("Label Offset X Y"));
    InfoPanel.add(LabXOffText);
    InfoPanel.add(LabYOffText);
    OKB.setText("OK");
    OKB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        OKBActionPerformed(evt);
      }
    });
    InfoPanel.add(OKB);
    CancelB.setText("Cancel");
    CancelB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        CancelBActionPerformed(evt);
      }
    });
    InfoPanel.add(CancelB);
    setContentPane(InfoPanel);
  }             //GEN-END:initComponents

  /**
   * This method is called after "cancel", closes the dialog
   * @param evt The ActionEvent
   * @see <a href="LabelAttrDlgsrc.html#CancelbActionPerformed">Source</a> 
   */
  private void CancelBActionPerformed (java.awt.event.ActionEvent evt) {        //GEN-FIRST:event_CancelBActionPerformed
    // Add your handling code here:
    setVisible(false);
    dispose();
  }             //GEN-LAST:event_CancelBActionPerformed

  /**
   * This method is called after "ok" and sets the label-parameter to AktGO
   * @param evt The ActionEvent
   * @see <a href="LabelAttrDlgsrc.html#OKBActionPerformed">Source</a> 
   */
  private void OKBActionPerformed (java.awt.event.ActionEvent evt) {            //GEN-FIRST:event_OKBActionPerformed
    if ((LabXOffText.getText().equals("") || LabYOffText.getText().equals("")))
      ; 
    else 
      AktGO.getLabPosOffset().setLocation(Double.parseDouble(LabXOffText.getText()), 
          Double.parseDouble(LabYOffText.getText()));
    AktGO.setLabelAttribute(new DefaultLabelAttribute(LabelText.getText()));
    CancelBActionPerformed(null);
  }
  // Variables declaration - do not modify//GEN-BEGIN:variables
  private javax.swing.JTextField LabelText;
  private javax.swing.JTextField LabXOffText;
  private javax.swing.JTextField LabYOffText;
  private javax.swing.JButton OKB;
  private javax.swing.JButton CancelB;
  private javax.swing.JLabel LTLabel;
  // End of variables declaration//GEN-END:variables
}



