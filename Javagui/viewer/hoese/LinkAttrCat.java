package viewer.hoese;

import viewer.*;

import sj.lang.ListExpr;
import javax.swing.*;
import java.util.Vector;
import java.awt.*;
import javax.swing.event.*;
import java.awt.event.*;
import javax.swing.table.*;

public class LinkAttrCat extends JDialog{


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
	     MessageBox.showMessage("you must enter a Name ");
	     return;
	  }
	  if(!UpdateMode){
	     if(ManualLinkPool.exists(NameText.getText())){
                MessageBox.showMessage("the name allready exists \n please chose another one ");
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
                    MessageBox.showMessage("the name allready exists \n please chose another one ");
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
     MessageBox.showMessage("no category selected");
     return;
  }
  String CatName = TheCat.getName();
  int index = Tab.getSelectedRow();
  if(index<0) {
      MessageBox.showMessage("no attribute selected");
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


private class AttrCatTableModel implements TableModel{

public void addTableModelListener(TableModelListener l){
  if(!Listeners.contains(l))
     Listeners.add(l);
}

public Class getColumnClass(int columnIndex){
  return "".getClass();
}

public int getColumnCount(){
  return 2;
}

public String getColumnName(int columnIndex){
  if (columnIndex==0)
     return "Value";
  if (columnIndex==1)
     return "Category";
  return "";
}

public int getRowCount(){
  if(Links==null) return 0;
  return Links.getSize();
}

public Object getValueAt(int rowIndex,int ColumnIndex){
  if(Links==null) return "";
  if(ColumnIndex==0)
     return Links.getValueStringAt(rowIndex);
  if(ColumnIndex==1)
     return Links.getCatNameAt(rowIndex);
  return "";
}


public void setCatNameAt(int index,String Name){
   if(Links==null) return;
   Links.setCatNameAt(index,Name);
   informListeners();
}

public boolean isCellEditable(int rowIndex, int ColumnIndex){
  return false;
}

public void removeTableModelListener(TableModelListener l){
  Listeners.remove(l);
}

public void setValueAt(Object aValue,int rowIndex,int ColumnIndex){
  // we avoid extern changes
}

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

/* set the attributes from given list, link all value to defaultcategory */
public void setAttributes(ListExpr LE,Category DefaultCat){ // set all non komplex attribute
   if(Links==null) return;
   Links.clear(); // remove old attributes
   addAttributes(LE,DefaultCat);
}

/* set the attributes from given list, link all value to defaultcategory */
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



public void readFrom(AttrCatList ACL){
   Links = ACL;
   informListeners();
}

public AttrCatList getLinks(){
   return Links;
}

public void setLinks(AttrCatList ACL){
   Links = ACL;
   informListeners();
}

private void informListeners(){
   TableModelEvent TME = new TableModelEvent(this);
   for(int i=0;i<Listeners.size();i++)
       ((TableModelListener)Listeners.get(i)).tableChanged(TME);
}


public void setDefaultCat(Category Cat){
   if(Cat!=null && Links!=null )
     Links.setDefaultCatName(Cat.getName());
}

private Vector Listeners = new Vector();
private AttrCatList Links = new AttrCatList();
}

}



