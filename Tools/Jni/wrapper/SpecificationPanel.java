package wrapper;
import java.util.Vector;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.event.*;


/** This class provides a panel for editing OperatorSpecifications. **/
public class SpecificationPanel extends JPanel{

   /**  Creates a new instance of this panel. **/
   public SpecificationPanel(String[] OperatorNames){
       ST = new SpecificationTable(OperatorNames);
       Table = new JTable(ST);
       JScrollPane SP = new JScrollPane(Table);
       add(SP);
   }

   /** Sets all OperatorNames. */ 
   public void set(String[] OperatorNames){
       ST.set(OperatorNames);
   }

   /** Returns all specifications. **/
   public  OperatorSpecification[] getSpecs(){
        return ST.getSpecifications();
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
   SpecificationTable ST;

/** A Table for editing specifications */
private class SpecificationTable implements TableModel{

   /** Returns the content of this table as array of OperatorSpecificxations. */
   OperatorSpecification[] getSpecifications(){
      return Specifications;
   }

   /**
    Creates a new Instance.
   */
   public SpecificationTable(String[] OperatorNames){
      TME = new TableModelEvent(this);
      Specifications = new OperatorSpecification[OperatorNames.length];
      for(int i=0;i<Specifications.length;i++)
          Specifications[i] = new OperatorSpecification(OperatorNames[i]);
   }

   /** Sets the specifications of operators with given names to be empty. **/
   public void set(String[] OperatorNames){
      Specifications = new OperatorSpecification[OperatorNames.length];
      for(int i=0;i<Specifications.length;i++)
          Specifications[i] = new OperatorSpecification(OperatorNames[i]);
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

   /** Returns the name of the specified column.
   */
   public  String getColumnName(int columnIndex) {
       if(columnIndex==0) return "";
       if(columnIndex==1) return "Value";
       return null;
   }

    /** Return the number of entries */
    public int getRowCount(){return Specifications.length*5-1;}

    /** Returns the entry at the specidied position */
    public  Object getValueAt(int rowIndex, int columnIndex) {
         if(rowIndex<0 | rowIndex>Specifications.length*5-1)
            return null;
         int pos = rowIndex%5;
         int which = rowIndex/5;
         if(columnIndex==0){
             if(pos==0)
                 return "Name";
             if(pos==1)
                 return "Signature";
            if(pos==2)
                 return "Syntax";
            if(pos==3)
                 return "Example";
            return "";
         }
         if(columnIndex==1){
            if(pos==0)
                 return Specifications[which].getOperatorName();
            if(pos==1)
                 return Specifications[which].getSignature();
            if(pos==2)
                 return Specifications[which].getSyntax();
            if(pos==3)
                 return Specifications[which].getExample();
            return "";
         }
         return null;
    }

     /** Returns false for all arguments */
     public   boolean isCellEditable(int rowIndex, int columnIndex) {
         if(columnIndex!=1) return false;
         if(rowIndex%5==0) return false; // name
         if(rowIndex%5==4) return false; // empty line
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
         if(rowIndex%5==0) return; // name
         if(rowIndex%5==4) return; // empty line 
         if(aValue instanceof String){
             String S = (String) aValue;
             int pos=rowIndex%5;
             int which = rowIndex/5;
             if(pos==1) Specifications[which].setSignature(S);
             if(pos==2) Specifications[which].setSyntax(S);
             if(pos==3) Specifications[which].setExample(S);
             informListeners();
         }

     }


     /** Informs all registered Listeners about changes in the table.
     */
     private void informListeners(){
         for(int i=0;i<Listener.size();i++)
            ((TableModelListener)Listener.get(i)).tableChanged(TME);
     }

     OperatorSpecification[] Specifications;

     Vector classes = new Vector();
     Vector Listener = new Vector();
     TableModelEvent TME;
}

}

