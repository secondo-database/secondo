package viewer.hoese;

import viewer.*;

import sj.lang.ListExpr;
import javax.swing.*;
import java.util.Vector;
import java.awt.*;
import javax.swing.event.*;
import java.awt.event.*;

public class LinkAttrCat extends JDialog{


public LinkAttrCat(HoeseViewer HV){
   super(HV.getMainFrame(),true);
   this.HV = HV;
   setTitle("link attributes to categorys");
   setSize(300,300);
   getContentPane().setLayout(new BorderLayout());
   JPanel P = new JPanel(new GridLayout(1,2));
   AListModel = new AttrListModel();
   AttrList =  new JList(AListModel);
   CListModel = new CatListModel();
   CatList = new JList(CListModel);
   P.add(AttrList);   
   P.add(CatList);
   JScrollPane SP = new JScrollPane(P);
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

   
   getContentPane().add(P2,BorderLayout.NORTH);
   getContentPane().add(SP,BorderLayout.CENTER);
   
   ListSelectionListener LSL = new ListSelectionListener(){
     public void valueChanged(ListSelectionEvent evt){
        int AttrIndex = AttrList.getSelectedIndex();
        int CatIndex = CatList.getSelectedIndex();
        if(AttrIndex!=CatIndex){
          Object source = evt.getSource();
          if(source.equals(AttrList))
             CatList.setSelectedIndex(AttrIndex);
          if(source.equals(CatList))
             AttrList.setSelectedIndex(CatIndex);
        }
     
     }
   
   };
   CatList.addListSelectionListener(LSL);
   AttrList.addListSelectionListener(LSL);
   
   SetBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         setCat();
       }});  
   
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
      if(o instanceof Category)
         ComboBox.addItem(o);
   }      
   if(v.size()>0){
     DefaultCategory = (Category) ComboBox.getItemAt(0);
     if(!CListModel.isDefined())
        CListModel.setStandard(DefaultCategory,AListModel.getSize());
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
   AListModel.setDatas(LE);
   CListModel.setStandard(DefaultCategory,AListModel.getSize());
}


/* set the Default Cat */
public void setDefaultCat(Category Cat){
   if(Cat==null) return;
   if(getIndexOf(Cat)>=0){
      DefaultCategory = Cat;
      CListModel.setStandard(Cat,AListModel.getSize());  
   }   
}


/* set the selected Cat for the selected attribute value */
private void setCat(){
  Category TheCat = (Category) ComboBox.getSelectedItem();
  if(TheCat==null){
     MessageBox.showMessage("no category selected");
     return;
  }
  int index = AttrList.getSelectedIndex();
  if(index <0){
     MessageBox.showMessage("no attribute value selected");
     return;
  }
  CListModel.set(TheCat,index);  
}


public Category getCategory(ListExpr LE){
  int index = AListModel.getIndexOf(LE);
  if(index<0)
     return DefaultCategory;
  else{
    String CatName = (String)CListModel.getElementAt(index);
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
}


public void setRefName(String Name){
   RefName = Name;
}   
public String getRefName(){
   return  RefName;
}


private JComboBox ComboBox = new JComboBox();
private JList AttrList;
private JList CatList;
private Category DefaultCategory;
private AttrListModel AListModel;
private CatListModel CListModel;
private String RefName=""; // name of attribute 
private JButton SetBtn;
private HoeseViewer HV; // needed to show a CategoryEditor
private JButton CatEditBtn;




/*      private classes for ListModels */


private class AttrListModel implements ListModel{
  
  public void addListDataListener(ListDataListener LDL){
     if(LDL!=null && !LDLs.contains(LDL))
        LDLs.add(LDL);
  }
  
  public void removeListDataListener(ListDataListener LDL){
    if(LDL!=null)
       LDLs.remove(LDL);
  }

  
  
  public Object getElementAt(int index){
     if(index<0 || index  > Datas.size())
        return null;
     else return Datas.get(index);   
  }
  
  /* transform a ListExpr to a simple String */
  private String getListString(ListExpr LE){
     if(LE==null) return "";
     if(LE.isEmpty()) return "";
     int AtomType = LE.atomType();
     switch(AtomType){
       case ListExpr.INT_ATOM : return ""+LE.intValue();
       case ListExpr.REAL_ATOM : return ""+LE.realValue();
       case ListExpr.BOOL_ATOM : return ""+LE.boolValue();
       case ListExpr.STRING_ATOM : return LE.stringValue();
       case ListExpr.SYMBOL_ATOM : return LE.symbolValue();
       case ListExpr.TEXT_ATOM : return "komplex";
       case ListExpr.NO_ATOM : return "komplex";
     }
     System.out.println("LinkAttrCat.AttrListModel.getListString : unknow atomtype ");
     return "";
  }


  public int getIndexOf(ListExpr LE){
    String SLE = getListString(LE);
    return Datas.indexOf(SLE);
  }
  
  
  /* reads all entry for the List from given ListExpr */
  public void setDatas(ListExpr LE){
     Datas.clear();
     if(LE==null) return;
     boolean komplex = false;
     ListExpr Rest=LE;
     String entry;
     if(Rest.atomType()!=ListExpr.NO_ATOM){
          entry = getListString(Rest);
          if(!entry.equals(""))
             Datas.add(entry);
     }
     
     
     while(!Rest.isEmpty()){
        entry =  getListString(Rest.first());
        if(!entry.equals("") && !Datas.contains(entry))
           Datas.add(entry);
        Rest = Rest.rest();   
     }
     informLDLs();
  }
  
  public int getSize(){
     return  Datas.size();
  }

  private void informLDLs(){
     for(int i=0;i<LDLs.size();i++){
        Object o = LDLs.get(i);
        ListDataListener LDL = (ListDataListener)o;
        LDL.contentsChanged(new ListDataEvent(this,ListDataEvent.CONTENTS_CHANGED,0,0));
    }    
  }
  
  
  private Vector LDLs = new Vector();
  private Vector Datas = new Vector();

}


private class CatListModel implements ListModel{
  
  public void addListDataListener(ListDataListener LDL){
     if(LDL!=null && !LDLs.contains(LDL))
        LDLs.add(LDL);
  }
  
  public void removeListDataListener(ListDataListener LDL){
    if(LDL!=null)
       LDLs.remove(LDL);
  }

  
  
  public Object getElementAt(int index){
     if(index<0 || index  > Datas.size())
        return null;
     else return Datas.get(index);   
  }
  
  
  public void setStandard(Category Cat,int Size ){
      Datas.clear();
      if(Cat!=null){
        for(int i=0;i<Size;i++)
           Datas.add(Cat.toString());
        defined=true;   
      }     
      else  {   
        for(int i=0;i<Size;i++)
           Datas.add("undefined");
        defined=false;   
      }
      informLDLs();   
  }

  public void set(Category Cat,int index){
    if(index<0 || index>Datas.size()) 
       return;
    Datas.setElementAt(Cat.toString(),index);
    informLDLs();
  }

  public boolean isDefined(){
     return defined;
  }
  
 
 
  public int getSize(){
     return  Datas.size();
  }
  

  private void informLDLs(){
     for(int i=0;i<LDLs.size();i++){
        Object o = LDLs.get(i);
        ListDataListener LDL = (ListDataListener)o;
        LDL.contentsChanged(new ListDataEvent(this,ListDataEvent.CONTENTS_CHANGED,0,0));
    }    
  }
  
  
  private Vector LDLs = new Vector();
  private Vector Datas = new Vector();
  private boolean defined = false;

}





}



