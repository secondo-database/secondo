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


package  viewer.spatial3D; 

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import sj.lang.ListExpr;
import sj.lang.ServerErrorCodes;
import java.util.Properties;
import java.util.*;
import javax.swing.event.*;
import viewer.Spatial3DViewer;
import tools.Reporter;
import gui.SecondoObject;

/**
 * This class displays the textual results of a query
 */
public class TextWindow extends JPanel {
  
  private JList queryItems;
  private JScrollPane queryScrollPane;
  private DefaultListModel listModel = new DefaultListModel();
  private Spatial3DViewer parent;
  private JButton objectProperties;
  private ObjectPropertiesDialog dialog;
  

  
  
  /**
   * Construktor 
   */
  public TextWindow (Spatial3DViewer aparent) {
    super();
    setLayout(new BorderLayout());
    parent = aparent;
    queryScrollPane = new JScrollPane();
    add(queryScrollPane, BorderLayout.CENTER);
    
    dialog = new ObjectPropertiesDialog(parent, this);
    
    queryItems = new JList();
    queryItems.setModel(listModel);
    queryItems.setFont(new Font("Monospaced",Font.PLAIN,12));
    queryItems.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    queryItems.setBackground(Color.lightGray);
    
    queryItems.addMouseListener(new MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        int index = queryItems.getSelectedIndex();
        parent.showSelectedObject(index);
      }
      public void mouseClicked(MouseEvent e) {
        if (e.getClickCount() == 2) {
          showPropertiesDialog();
        }
      }
    });
    
    objectProperties = new JButton("Object Properties");
    add(objectProperties,  BorderLayout.SOUTH);
    objectProperties.setEnabled(false);
    
    objectProperties.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showPropertiesDialog();
      }
    });
    
    queryScrollPane.setViewportView(queryItems);
  }
    
  /**
   * setView
   * set the items in list
   */
  public void setView(Vector<String> items) {
    if (items != null) {
      clearView();
      Iterator<String> it = items.iterator();
      while (it.hasNext()) {
        listModel.addElement(it.next());
      }
      objectProperties.setEnabled(true);
    }
    else
      clearView();
  }
  
  /**
   * clearView
   * remove all Items from List
   */
  public void clearView() {
    listModel.removeAllElements();
    objectProperties.setEnabled(false);
  }
  
  /**
   * setSelection
   * select item on positon "index"
   */
  public void setSelection(int index) {
    clearSelection();    
    queryItems.setSelectedIndex(index);
    queryItems.ensureIndexIsVisible(index);
  }
  
  /**
   * clearSelection
   * no item is selected
   */
  public void clearSelection() {
    queryItems.clearSelection();
  }
  
  /**
   * showPropertiesDialog
   * opens the proporties dialog
   */
  public void showPropertiesDialog() {
    int index = queryItems.getSelectedIndex();
    if ((index<0) || (parent.getCurrentShape()== null))
    Reporter.showInfo("There is no Item selected! Please select one!");
    else
    {
      dialog.setIndex(index);
      dialog.setVisible(true);
    }
  }
  
}