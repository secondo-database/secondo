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
import java.security.spec.EllipticCurve;


import gui.SecondoObject;
import sj.lang.*;

/* this viewer shows nested relations and attribute relations */
public class NestedRelationViewer extends StandardViewer {

  private static final int INDENT_BY = 4;

  private JScrollPane ScrollPane = new JScrollPane();
  private JTextArea TextArea = new JTextArea();
  private JComboBox ComboBox = new JComboBox();
  private Vector ItemObjects = new Vector(10, 5);
  private MenuVector menuVector = new MenuVector();
  private SecondoObject CurrentObject = null;

  /* create a new StandardViewer */
  public NestedRelationViewer() {
    add(BorderLayout.NORTH, ComboBox);
    add(BorderLayout.CENTER, ScrollPane);
    ScrollPane.setViewportView(TextArea);
    Font font = TextArea.getFont(); // Keep text size at default value
    TextArea.setFont(new Font("monospaced", Font.PLAIN, font.getSize()));
    JMenu StdMenu = new JMenu("NestedRelation-Viewer");
    JMenuItem MI_Remove = StdMenu.add("Discard current view");
    JMenuItem MI_RemoveAll = StdMenu.add("Discard all views");
    menuVector.addMenu(StdMenu);

    MI_Remove.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (CurrentObject != null) {
          SecondoObject selectedObject = CurrentObject;
          if (ItemObjects.remove(CurrentObject)) {
            ComboBox.removeItem(CurrentObject.getName());
            CurrentObject = null;
            int index = ComboBox.getSelectedIndex(); // the new current object
            if (index >= 0) {
              CurrentObject = (SecondoObject) ItemObjects.get(index);
              showObject();
            }
          }
          if (VC != null)
            VC.removeObject(selectedObject); // inform the ViewerControl
        }
      }
    });

    MI_RemoveAll.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        ItemObjects.removeAllElements();
        ComboBox.removeAllItems();
        CurrentObject = null;
        if (VC != null)
          VC.removeObject(null);
        showObject();
      }
    });

    ComboBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent evt) {
        showObject();
        if (VC != null) {
          int index = ComboBox.getSelectedIndex();
          if (index >= 0) {
            try {
              CurrentObject = (SecondoObject) ItemObjects.get(index);
              VC.selectObject(NestedRelationViewer.this, CurrentObject);
            } catch (Exception e) {
            }
          }
        }
      }
    });
  }

  /* adds a new Object to this Viewer and display it */
  public boolean addObject(SecondoObject o) {
    if (isDisplayed(o))
      selectObject(o);
    else {
      ItemObjects.add(o);
      ComboBox.addItem(o.getName());
      try {
        ComboBox.setSelectedIndex(ComboBox.getItemCount() - 1); // make the new object to active object
        showObject();
      } catch (Exception e) {
      }
    }
    return true;
  }

  /* returns true if o a SecondoObject in this viewer */
  public boolean isDisplayed(SecondoObject o) {
    return ItemObjects.indexOf(o) >= 0;

  }

  /** remove o from this Viewer */
  public void removeObject(SecondoObject o) {
    if (ItemObjects.remove(o))
      ComboBox.removeItem(o.getName());
  }

  /** remove all containing objects */
  public void removeAll() {
    ItemObjects.removeAllElements();
    ComboBox.removeAllItems();
    CurrentObject = null;
    if (VC != null)
      VC.removeObject(null);
    showObject();
  }

  /* returns allways true (this viewer can display all SecondoObjects) */
  public boolean canDisplay(SecondoObject o) {
    boolean result = false;
    ListExpr type = o.toListExpr().first();
    try {
    String shortType = getShortType(type);
    result = (shortType.equals("arel2") || shortType.equals("nrel2"));
    }
    catch(Exception e)
    {
      JOptionPane.showMessageDialog(this, e.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
    }
    return result;
  }

  /* returns the Menuextension of this viewer */
  public MenuVector getMenuVector() {
    return menuVector;
  }

  /* returns Standard */
  public String getName() {
    return "NestedRelation";
  }

  /* select O */
  public boolean selectObject(SecondoObject O) {
    int i = ItemObjects.indexOf(O);
    if (i >= 0) {
      ComboBox.setSelectedIndex(i);
      showObject();
      return true;
    } else //object not found
      return false;
  }

  private void showObject() {
    TextArea.setText("");
    int index = ComboBox.getSelectedIndex();
    if (index >= 0) {
      try {
        CurrentObject = (SecondoObject) ItemObjects.get(index);
        ListExpr type = CurrentObject.toListExpr().first();
        ListExpr value = CurrentObject.toListExpr().second();
        String relation = DisplayRelation(type, value);
        TextArea.setText(relation);
      } catch (Exception e) {
        TextArea.setText(e.getMessage());
      }
    }
  }

  private static final int getBaseIndentation(ListExpr type) {
    int result = 0;
    String attrName;
    String shortType;

    while (!type.isEmpty()) {
      ListExpr current = type.first();
      attrName = current.first().symbolValue();
      shortType = getShortType(current.second());
      result = (attrName.length() > result) ? attrName.length() : result;
      if ((shortType == "arel2") || (shortType == "nrel2")) {
        int len = getBaseIndentation(current.second().second().second()) - 4;
        result = (len > result) ? len : result;
      }
      type = type.rest();
    }
    return result;
  }

  private static final String getShortType(ListExpr attrType) {
    String result = "";
    if (!attrType.isEmpty()) {
      if (attrType.isAtom()) {
        if (attrType.atomType() == ListExpr.SYMBOL_ATOM) {
          result = attrType.symbolValue();
        }
      } else {
        attrType = attrType.first();
        if (attrType.isAtom()) {
          if (attrType.atomType() == ListExpr.SYMBOL_ATOM) {
            result = attrType.symbolValue();
          }
        }
      }
    }
    return result;
  }

  private static String DisplayRelation(ListExpr type, ListExpr value) {
    StringBuilder output = new StringBuilder(type.writeListExprToString());
    output.append("\n\n  =======  \n\n");
    DisplayRelation(type, value, 0, output);
    return output.toString();
  }

  private static void DisplayRelation(ListExpr type, ListExpr value, int indentation, StringBuilder output) {
    ListExpr attrDescs = type.second().second();
    if (indentation == 0) {
      indentation = getBaseIndentation(attrDescs);
    }
    if (!value.isAtom()) {
      output.append('\n');
      while (!value.isEmpty()) {
        ListExpr currentTuple = value.first();
        try {
          DisplayTuple(attrDescs, currentTuple, indentation, output);
        } catch(Exception e)
        {
          output.append("<<< Broken tuple >>>\n");
        }
        value = value.rest();
      }
    } else {
      value.writeListExprToString();
    }
  }

    private static void DisplayTuple(ListExpr attrDescs, ListExpr value,
          int indentation, StringBuilder output)   {
        while (!attrDescs.isEmpty())
        {
          ListExpr currentAttrDesc = attrDescs.first();
          ListExpr currentValue = value.first();
          String attrName = currentAttrDesc.first().symbolValue();
          ListExpr currentAttrType = currentAttrDesc.second();
          String attrShortType = getShortType(currentAttrType);
          for(int i = 0; i < indentation - attrName.length(); i++) {
            output.append(' ');
          }
          output.append(attrName);
          output.append(" : ");    
          if (attrShortType.equals("arel2") || attrShortType.equals("nrel2"))
          {
            DisplayRelation(currentAttrType, currentValue,
                indentation + INDENT_BY, output);
          }
          else
          {
            try
            {
	            String attrValue = currentValue.writeListExprToString();
	            if (attrValue.length()>=1 && attrValue.startsWith("\n"))
	            {
	              attrValue = attrValue.substring(1);
	            }
	            output.append(attrValue);
            }
            catch(Exception e)
            {
              output.append("<<< Broken attribute value >>>");
            }
            
          }
          output.append("\n");
          attrDescs = attrDescs.rest();
          value = value.rest();
        }
        //Print an empty line after each tuple to group attributes of one tuple
        //together visually
        output.append("\n");
      }
}
