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

package wrapper;
import java.util.Vector;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.event.*;


/** A class providing a panel for editing type descriptions. */
public class TypeDescriptionPanel extends JPanel{

   /** Creates a new instance of this with given set of typenames. */
   public TypeDescriptionPanel(String[] TypeNames){
       ST = new DescriptionTable(TypeNames);
       Table = new JTable(ST);
       Table.setColumnSelectionAllowed(false);
       Table.setRowSelectionAllowed(false);
       JScrollPane SP = new JScrollPane(Table);
       add(SP);
   }

   /** Sets the typenames. */
   public void set(String[] TypeNames){
       ST.set(TypeNames);
   }

   /** Returns all contained TypeDescriptions */
   public  TypeDescription[] getDescs(){
        return ST.getDescriptions();
   }

   /** finish a started editing of a cell */
   public void finish(){ 
      int row = Table.getEditingRow();
      int col = Table.getEditingColumn();
      if(row<0) return;
      if(col<0) return;
      TableCellEditor E = Table.getCellEditor(row,col);
      E.stopCellEditing(); 
      Object o = E.getCellEditorValue();
      Table.setValueAt(o,row,col);
   }

   JTable Table;
   DescriptionTable ST;

/** A Table for editing type constructor descriptions. */
private class DescriptionTable implements TableModel{

   /** Returns all contained descriptions. */
   TypeDescription[] getDescriptions(){
      return Descriptions;
   }

   /**
    Creates a new Instance.
   */
   public DescriptionTable(String[] TypeNames){
      TME = new TableModelEvent(this);
      set(TypeNames);
   }

   /** Sets all typenames.
     * The signatures are set to ->Data.
     */
   public void set(String[] TypeNames){
      Descriptions = new TypeDescription[TypeNames.length];
      for(int i=0;i<Descriptions.length;i++){
          Descriptions[i] = new TypeDescription(TypeNames[i]);
          Descriptions[i].setSignature("->Data");
          Descriptions[i].setTypeList(TypeNames[i]);
      }
   }

   /** Adds a TableModelListener */
   public void addTableModelListener(TableModelListener l){
      Listener.add(l);
   }

   public Class getColumnClass(int index){
      return "".getClass();
   }

   /**
    * Returns 2;
   **/
   public int getColumnCount() {return 2;}

   /** Returns the name of the column "Class"
   */
   public  String getColumnName(int columnIndex) {
       if(columnIndex==0) return "";
       if(columnIndex==1) return "Value";
       return null;
   }

    /** Return the number of entries */
    public int getRowCount(){return Descriptions.length*6-1;}

    /** Returns the entry at the specidied position */
    public  Object getValueAt(int rowIndex, int columnIndex) {
         if(rowIndex<0 | rowIndex>Descriptions.length*6-1)
            return null;
         int pos = rowIndex%6;
         int DescPos = rowIndex/6;
         if(columnIndex==0){
            if(pos==0)
                 return "name";
            if(pos==1)
                 return "signature";
            if(pos==2)
                 return "type list example";
            if(pos==3)
                 return "value list";
            if(pos==4)
                 return "value list example";
            return "";
         }
         if(columnIndex==1){
            if(pos==0)
                 return Descriptions[DescPos].getName();
            if(pos==1)
                 return Descriptions[DescPos].getSignature();
            if(pos==2)
                 return Descriptions[DescPos].getTypeList();
            if(pos==3)
                 return Descriptions[DescPos].getValueList();
            if(pos==4)
                 return Descriptions[DescPos].getValueListExample(); 
            return "";
         }
         return null;
    }

     /** Returns false for all arguments */
     public   boolean isCellEditable(int rowIndex, int columnIndex) {
         if(columnIndex!=1) return false;
         if(rowIndex%6==5) return false; // empty line
         if(rowIndex%6==0) return false; // name
         return true;
     }

     /** Removes the TableModelListener l.
     */
     public  void removeTableModelListener(TableModelListener l) {
       Listener.remove(l);
     }

     /** Makes nothing because we only allow to change the table via
       * add and remove.
     */
     public void setValueAt(Object aValue, int rowIndex, int columnIndex) {
         if(columnIndex!=1) return;
         if(rowIndex%6==0) return; // name
         if(rowIndex%6==5) return; // empty line 
         if(aValue instanceof String){
             String S = (String) aValue;
             int pos=rowIndex%6;
             int which = rowIndex/6;
             if(pos==1) Descriptions[which].setSignature(S);
             if(pos==2) Descriptions[which].setTypeList(S);
             if(pos==3) Descriptions[which].setValueList(S);
             if(pos==4) Descriptions[which].setValueListExample(S);
             informListeners();
         }

     }


     /** Informs all registered Listeners about changes in the table.
     */
     private void informListeners(){
         for(int i=0;i<Listener.size();i++)
            ((TableModelListener)Listener.get(i)).tableChanged(TME);
     }

     TypeDescription[] Descriptions;

     Vector classes = new Vector();
     Vector Listener = new Vector();
     TableModelEvent TME;
}

}
