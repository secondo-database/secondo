package viewer;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import sj.lang.*;         // ListExpr and IntByReference is needed
import gui.SecondoObject;
import java.util.Vector;
import gui.idmanager.*;
import viewer.relsplit.*;
import javax.swing.event.*;

public class RelSplit extends SecondoViewer{

public String getName(){return "RelSplitter";}

public boolean addObject(SecondoObject o){
  if (Relation.isRelation(o))
     return addRelation(o);
  else{
    SecondoObject Rel = makeRelation(o);
    if(Rel!=null){
       return addRelation(Rel);
    } 
    else 
       return false;
  }
}

public void removeObject(SecondoObject o){
  if(o==null)
    return;
  int index = getIndexOf(o.getID());
  if(index>=0){
    AllRelations.removeItemAt(index);
    if(AllRelations.getItemCount()==0){
      CurrentRelation.setModel(dummy);
    }
  }
}

public void removeAll(){
     AllRelations.removeAllItems();
     CurrentRelation.setModel(dummy);
}



public boolean canDisplay(SecondoObject o){
  return Relation.isRelation(o) || (makeRelation(o)!=null);
}

public boolean isDisplayed(SecondoObject o){
  return getIndexOf(o.getID())>=0;
}

public boolean selectObject(SecondoObject O){
   int index= getIndexOf(O.getID());
   if(index >=0){
      AllRelations.setSelectedIndex(index);
      return true;
   } 
   IntByReference RI = new IntByReference();
   IntByReference LI = new IntByReference();
   if(searchID(O.getID(),RI,LI)){
      AllRelations.setSelectedIndex(RI.value);  
      CurrentRelation.setSelectedIndex(LI.value);
      return true;
   }
   else
     return false;
}


public MenuVector getMenuVector(){ return null;}

/** returns the index of Relation with ID */
private int getIndexOf(ID aID){
  int c = AllRelations.getItemCount();
  int pos = -1;
  boolean found = false;
  for(int i=0; i<c && !found;i++){
    RelationListModel RLM = (RelationListModel) AllRelations.getItemAt(i);
    if(aID.equals(RLM.getID())){
       found = true;
       pos = i;
    }
  }
  return pos;
}


/** store a single SecondoObject in a Relation ,
  * if SO is not a queryResult, this mean the
  * corresponding ListExpr = (<type> <value>) the null is returned
  */
private SecondoObject makeRelation(SecondoObject SO){
  ListExpr SOLE = SO.toListExpr();
  if(SOLE.listLength()!=2)
     return null;
  ListExpr Type = SOLE.first();
  ListExpr Value = SOLE.second();
  if(!(Type.isAtom() && Type.atomType()==ListExpr.SYMBOL_ATOM))
     return null;

  ListExpr TupleEntry = ListExpr.twoElemList(Type,Type); // we have not a name given
  ListExpr Tuple = ListExpr.twoElemList( ListExpr.symbolAtom("tuple"),
                                         ListExpr.oneElemList(TupleEntry));
  ListExpr RelType = ListExpr.twoElemList( ListExpr.symbolAtom("rel"),
                                           (Tuple));
  ListExpr Rel = ListExpr.twoElemList( RelType,ListExpr.oneElemList(ListExpr.oneElemList(Value)));  
  SecondoObject R = new SecondoObject(SO.getID());
  R.fromList(Rel);
  R.setName(SO.getName());
  return R;
}


/** to Capture selection in the list */
public void addListSelectionListener(ListSelectionListener LSL){
  CurrentRelation.addListSelectionListener(LSL);
}

/** returns the SecondoObject in the list
  * if no secondoobject is selected null is returned
  */
public SecondoObject getSelectedObject(){
  int index = CurrentRelation.getSelectedIndex();
  if (index<0)
     return null;
  else
    return ((RelationListModel) CurrentRelation.getModel()).getSecondoObjectAt(index);
}

/** returns the selected attribute */
public SecondoObject[] getSelectedAttribute(){
   int index = CurrentRelation.getSelectedIndex();
   if(index<0)
      return null;
  else
    return ((RelationListModel) CurrentRelation.getModel()).getAttributeAt(index);
}

/** returns the selected Tuple as SecondoObject array*/
public SecondoObject getSelectedTuple(){
  int index = CurrentRelation.getSelectedIndex();
   if(index<0)
      return null;
  else
    return ((RelationListModel) CurrentRelation.getModel()).getTupleOnPos(index);
}


/** returns all objects in this relation */
public SecondoObject[] getAllObjects(){
  return ((RelationListModel) CurrentRelation.getModel()).getAllObjects();
}


/** returns all tuples in this relation */
public SecondoObject[] getAllTuples(){
  return ((RelationListModel) CurrentRelation.getModel()).getAllTuples();
}

/** return the relation */
public SecondoObject getRelation(){
   return ((RelationListModel) CurrentRelation.getModel()).getRelation();
}


/** search in all lists for aID */
private boolean searchID(ID aID,IntByReference RelationIndex,IntByReference ListIndex){
  int c = AllRelations.getItemCount();
  boolean found = false;
  int tmpPos;
  for(int i=0; i<c && !found;i++){
    RelationListModel RLM = (RelationListModel) AllRelations.getItemAt(i);
    tmpPos = RLM.getIndexOf(aID);
    if(tmpPos>=0){
       found = true;
       RelationIndex.value = i;
       ListIndex.value = tmpPos;
    }
  }
  return found;
}



public RelSplit(){
   setLayout(new BorderLayout());
   AllRelations    = new JComboBox();
   CurrentRelation = new JList();
   CurrentRelation.setFont(new Font("MonoSpaced",Font.PLAIN,12));
   ScrollPane = new JScrollPane();
   ScrollPane.setViewportView(CurrentRelation);
   SecondoObjectVector = new Vector();
   add(AllRelations,BorderLayout.NORTH);
   add(ScrollPane,BorderLayout.CENTER);
   AllRelations.addItemListener(new ItemListener(){
      public void itemStateChanged(ItemEvent evt){
          if (evt.getStateChange()==ItemEvent.SELECTED){
             ListModel LM = (ListModel) evt.getItem();
             CurrentRelation.setModel(LM);
          }
      }
   });

   // add a Panel to Search Strings in the ListExpr
   JPanel SearchPanel = new JPanel(); //new GridLayout(1,3));
   GoBtn = new JButton("go");
   SearchText = new JTextField(6);
   JLabel SearchLabel = new JLabel("search:");
   SearchPanel.add(SearchLabel);
   SearchPanel.add(SearchText);
   SearchPanel.add(GoBtn);    
   add(SearchPanel,BorderLayout.SOUTH);
   GoBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           search();
       }
   });
}


/** search the Text in the TextField and select the next line*/
private void search(){
  String What = SearchText.getText();
  if (What.trim().equals(""))
      showMessage("no text to search");
  else
      search(What);
}


/** select the next line containing S in the list */

private void search(String S){
  if(CurrentRelation==null){
    showMessage("no relation");
    return;
  }
  ListModel LM = CurrentRelation.getModel();
  if(LM==null || !(LM instanceof RelationListModel)){
    showMessage("no relation");
    return;
  }
  
  RelationListModel RLM= (RelationListModel) CurrentRelation.getModel();
  int SelIndex = CurrentRelation.getSelectedIndex();
  int Start = SelIndex<0?1:SelIndex+1;
  int next = RLM.find(S,false,Start);
  if(next<0) 
     next = RLM.find(S,false,1);
  if(next<0)
      showMessage("text not found");
  else{
      int H = ScrollPane.getSize().height;
      int FH = CurrentRelation.getFont().getSize();
      int rows= H/((FH+4)*2);        // the half of all visible rows
      if(next<rows)
        CurrentRelation.ensureIndexIsVisible(0);
      else if((next+rows)>RLM.getSize())
        CurrentRelation.ensureIndexIsVisible(RLM.getSize());
      else{
        CurrentRelation.ensureIndexIsVisible(next-rows);
        CurrentRelation.ensureIndexIsVisible(next+rows);
      }
      CurrentRelation.setSelectedIndex(next);
  }
}


/** return the MinimumSize of this Component */
public Dimension getMinimumSize(){
  return new Dimension(200,300);
}


public Dimension getPreferredSize(){
   return getMinimumSize();
}

public boolean addRelation(SecondoObject SO){
  Relation R=new Relation();
  if (!R.readFromSecondoObject(SO))
    return false;
  else{
    try{
       RelationListModel RLM = new RelationListModel(R);
       AllRelations.addItem(RLM);
       AllRelations.setSelectedIndex(AllRelations.getItemCount()-1);
       return true;
    }
    catch(Exception e){
       return false;
    }    
  }
}


private void showMessage(String S){
   OptionPane.showMessageDialog(this,S);
}

private JComboBox AllRelations;
private JList CurrentRelation;
private JScrollPane ScrollPane;
private Vector SecondoObjectVector;
private DefaultComboBoxModel dummy= new DefaultComboBoxModel(); // to show nothing
private JTextField SearchText;
private JButton    GoBtn;
private JOptionPane OptionPane = new JOptionPane();


}




