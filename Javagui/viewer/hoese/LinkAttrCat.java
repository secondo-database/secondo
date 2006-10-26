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

import viewer.*;

import sj.lang.ListExpr;
import javax.swing.*;
import java.util.Vector;
import java.awt.*;
import javax.swing.event.*;
import java.awt.event.*;
import javax.swing.table.*;
import tools.Reporter;

public class LinkAttrCat extends JDialog{

/** Construct a new Dialog for assigning
  * categories to given values 
  **/
public LinkAttrCat(HoeseViewer HV){
   super(HV.getMainFrame(),true);
   this.HV = HV;
   setTitle("link attributes to categorys");
   setSize(300,300);
   getContentPane().setLayout(new BorderLayout());

   Tab = new JTable(TM);
   JScrollPane SP = new JScrollPane(Tab);

   // Control Panel <Cat-ComboBox> <Set-Button> <CatEdit-Button>
   JPanel P2 = new JPanel();
   P2.add(ComboBox);
   SetBtn = new JButton("set");
   P2.add(SetBtn);

   CatEditBtn = new JButton("category editor");
   CatEditBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        new CategoryEditor(LinkAttrCat.this.HV, true).show();
        LinkAttrCat.this.HV.GraphDisplay.repaint();
        if(LinkAttrCat.this.HV.Cats.size()!= ComboBox.getItemCount())
          setCategories(LinkAttrCat.this.HV.Cats);
       }});

   P2.add(CatEditBtn);

   // Close Btn
   JPanel P3 = new JPanel();
   CloseBtn = new JButton("OK");
   CancelBtn = new JButton("Cancel");
   P3.add(CloseBtn);
   P3.add(CancelBtn);

   // Name Btn
   JPanel P4 = new JPanel(new GridLayout(1,2));
   P4.add(new JLabel("Name :"));
   NameText = new JTextField(12);
   P4.add(NameText);

   JPanel P1 = new JPanel(new BorderLayout()); // combines P2 and SP
   P1.add(P2,BorderLayout.NORTH);
   P1.add(SP,BorderLayout.CENTER);

   getContentPane().add(P4,BorderLayout.NORTH);
   getContentPane().add(P1,BorderLayout.CENTER);
   getContentPane().add(P3,BorderLayout.SOUTH);

   CloseBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent e){
          if(NameText.getText().equals("")){
	           Reporter.showError("Please enter a Name ");
	     return;
	  }
	  if(!UpdateMode){
	     if(ManualLinkPool.exists(NameText.getText())){
          Reporter.showError("the name allready exists \n please chose another one ");
	        return;
	     }
	     TM.getLinks().setName(NameText.getText());
	     RetValue = OK;
             LinkAttrCat.this.setVisible(false);
	  } else{ // begin updateMode
	    if(OldName.equals(NameText.getText())){ // no change
	       RetValue=OK;
               LinkAttrCat.this.setVisible(false);
	    }else{ // Name is changed
                if(ManualLinkPool.exists(NameText.getText())){
                    Reporter.showError("the name allready exists \n please chose another one ");
	            return;
	         }
	         TM.getLinks().setName(NameText.getText());
		 RetValue = OK;
                 LinkAttrCat.this.setVisible(false);
	     }
	  }
      }});

   CancelBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        LinkAttrCat.this.setVisible(false);
     }});

   SetBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         setCat();
       }});

}



public void setName(String N){
   NameText.setText(N);
   TM.getLinks().setName(N);
}


public void setUpdateMode(boolean on,String OldName){
  UpdateMode = on;
  this.OldName = OldName;
}


public String getName(){
  return TM.getLinks().getName();
}

/* set the available categories.
   all Categories in v are inserted other
   objects are ignored
 */
public void setCategories(Vector v){
   ComboBox.removeAllItems();
   if(v==null)
      return;
   Object o;
   for(int i=0;i<v.size();i++){
      o=v.get(i);
      if(o instanceof Category){
         ComboBox.addItem(o);
      }
   }
}


private int getIndexOf(Category Cat){
  int pos=-1;
  boolean found = false;
  for(int i=0;i<ComboBox.getItemCount()&&!found;i++)
     if(Cat.equals(ComboBox.getItemAt(i))){
        pos = i;
        found = true;
     }
  return pos;
}


/* set the Attribute */
public void setAttributes(ListExpr LE){
   TM.setAttributes(LE,DefaultCategory);
}

/* add Attributes in LE, if an attribute allready exists
 * then is this value ignored, thgis means the linked
 * category name is not changed */
public void addAttributes(ListExpr LE,Category DefaultCategory){
  TM.addAttributes(LE,DefaultCategory);
}

/* set the Default Cat */
public void setDefaultCat(Category Cat){
   if(Cat==null) return;
   DefaultCategory = Cat;
   TM.setDefaultCat(Cat);
}


/* set the selected Cat for the selected attribute value */
private void setCat(){
  Category TheCat = (Category) ComboBox.getSelectedItem();
  if(TheCat==null){
     Reporter.showError("no category selected");
     return;
  }
  String CatName = TheCat.getName();
  int index = Tab.getSelectedRow();
  if(index<0) {
      Reporter.showError("no attribute selected");
      return;
  }
  TM.setCatNameAt(index,CatName);
}


public Category getCategory(ListExpr LE){
    String CatName = TM.getCatName(LE);
    // search the category for this name
    boolean found =false;
    Category result= DefaultCategory;
    for(int i=0;i<ComboBox.getItemCount()&&!found;i++)
       if(((Category)ComboBox.getItemAt(i)).toString().equals(CatName)){
          found = true;
          result = (Category)ComboBox.getItemAt(i);
       }
    return result;
}


public void setRefName(String Name){
   RefName = Name;
}

public String getRefName(){
   return  RefName;
}


public AttrCatList getLinks(){
   return TM.getLinks();
}

public void setLinks(AttrCatList ACL){
   TM.setLinks(ACL);
   NameText.setText(ACL.getName());
}

public void setVisible(boolean on){
  if(on)
     RetValue=CANCELED;

  super.setVisible(on);
}

public int getRetValue(){
  return RetValue;
}



private JComboBox ComboBox = new JComboBox();
private Category DefaultCategory;
private String RefName=""; // name of attribute
private JButton SetBtn;
private HoeseViewer HV; // needed to show a CategoryEditor
private JButton CatEditBtn;
private JButton CloseBtn;
private JButton CancelBtn;
private AttrCatTableModel TM = new AttrCatTableModel();
private JTable Tab;
private JTextField NameText;
private boolean UpdateMode = false;
private String OldName = "";
private int RetValue=OK;
public static final int OK = 0;
public static final int CANCELED = 1;



/**  This class represents the internal model of the table 
  * which is shown.
  */ 

private class AttrCatTableModel implements TableModel{

/** Adds a new TableModelListener **/
public void addTableModelListener(TableModelListener l){
  if(!Listeners.contains(l))
     Listeners.add(l);
}

/* Returns the class of String **/
public Class getColumnClass(int columnIndex){
  return "".getClass();
}

/* Returns the constant 2.
 * The first row decsribes the attribute value and the second one the
 * assigned category.
 **/
public int getColumnCount(){
  return 2;
}

/** Returns the name of the requested column.
*/
public String getColumnName(int columnIndex){
  if (columnIndex==0)
     return "Value";
  if (columnIndex==1)
     return "Category";
  return "";
}

/** Returns the number of different values */
public int getRowCount(){
  if(Links==null) return 0;
  return Links.getSize();
}

/** Returns the string representing the value of an attribute
  * and the name of a category respectively.
  **/
public Object getValueAt(int rowIndex,int ColumnIndex){
  if(Links==null) return "";
  if(ColumnIndex==0)
     return Links.getValueStringAt(rowIndex);
  if(ColumnIndex==1)
     return Links.getCatNameAt(rowIndex);
  return "";
}

/** Changes the name of the Category located in the
  * given row.
  */
public void setCatNameAt(int index,String Name){
   if(Links==null) return;
   Links.setCatNameAt(index,Name);
   informListeners();
}

/** Returns allways false.
  * Changes are only allowed by the appropriate methods.
  **/
public boolean isCellEditable(int rowIndex, int ColumnIndex){
  return false;
}

/** Removes a listener object from this Model.
  **/
public void removeTableModelListener(TableModelListener l){
  Listeners.remove(l);
}

/** Does nothing.
  * We don't allow changes using this function
  */
public void setValueAt(Object aValue,int rowIndex,int ColumnIndex){
  // we avoid extern changes
}

/** Returns the assigned category name for a attribute value given
  * in the list LE.
  * We only allow list atoms of type int, real, bool, string, or symbol.
  **/
public String getCatName(ListExpr LE){
     if(Links==null) return null;
     if(LE==null) return null;
     if(LE.isEmpty()) return null;
     int AtomType = LE.atomType();
     switch(AtomType){
       case ListExpr.INT_ATOM    : return Links.getCatName(LE.intValue());
       case ListExpr.REAL_ATOM   : return Links.getCatName(LE.realValue());
       case ListExpr.BOOL_ATOM   : return Links.getCatName(LE.boolValue());
       case ListExpr.STRING_ATOM : return Links.getCatName(LE.stringValue());
       case ListExpr.SYMBOL_ATOM : return Links.getCatName(LE.symbolValue());
       case ListExpr.TEXT_ATOM : return null;
       case ListExpr.NO_ATOM : return null;
     }
     return null;
}

/** Sets the attributes from given list, link all value to defaultcategory 
  **/
public void setAttributes(ListExpr LE,Category DefaultCat){ // set all non komplex attribute
   if(Links==null) return;
   Links.clear(); // remove old attributes
   addAttributes(LE,DefaultCat);
}

/** Adds new values to the linklist.
  * All new values are assigned to the given category. 
  **/
public void addAttributes(ListExpr LE,Category DefaultCat){ // set all non komplex attribute
   if(Links==null) return;
   if(LE==null) return;
   String Name;
   if (DefaultCat!=null)
       Name = DefaultCat.getName();
   else
       Name ="unknow Category";
   while (!LE.isEmpty()){
     ListExpr Attr = LE.first();
     LE = LE.rest();
     int AtomType = Attr.atomType();
     switch(AtomType){
       case ListExpr.INT_ATOM    : Links.addLink(Attr.intValue(),Name);break;
       case ListExpr.REAL_ATOM   : Links.addLink(Attr.realValue(),Name);break;
       case ListExpr.BOOL_ATOM   : Links.addLink(Attr.boolValue(),Name);break;
       case ListExpr.STRING_ATOM : Links.addLink(Attr.stringValue(),Name);break;
       case ListExpr.SYMBOL_ATOM : Links.addLink(Attr.symbolValue(),Name);break;
       case ListExpr.TEXT_ATOM : break;
       case ListExpr.NO_ATOM : break;
     }
   }
   informListeners();
}

/** Reads the content of this model.
  **/
public void readFrom(AttrCatList ACL){
   Links = ACL;
   informListeners();
}

/** Returns the managed content. **/
public AttrCatList getLinks(){
   return Links;
}

/** sets the content **/
public void setLinks(AttrCatList ACL){
   Links = ACL;
   informListeners();
}

/** Informs all assigned listeners about changes within the content **/
private void informListeners(){
   TableModelEvent TME = new TableModelEvent(this);
   for(int i=0;i<Listeners.size();i++)
       ((TableModelListener)Listeners.get(i)).tableChanged(TME);
}

/** Sets the default category **/
public void setDefaultCat(Category Cat){
   if(Cat!=null && Links!=null )
     Links.setDefaultCatName(Cat.getName());
}

/** all listeners **/
private Vector Listeners = new Vector();
/** The links **/
private AttrCatList Links = new AttrCatList();
}

}



