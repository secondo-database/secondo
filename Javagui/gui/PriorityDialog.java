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

package gui;

import javax.swing.*;
import java.awt.event.*;
import javax.swing.event.*;
import viewer.SecondoViewer;
import java.awt.*;
import java.util.Vector;


public class PriorityDialog extends JDialog{

public PriorityDialog(Frame F){
  super(F,true);
  setTitle("Viewer Priorities");
  Model = new ViewerListModel();
  ViewerList = new JList(Model);
  ScrollPane = new JScrollPane(ViewerList);
  ObjectDepending = new JCheckBox("depending from object");
  KeepCurrentViewer = new JCheckBox(" try to keep the current Viewer");
  JPanel P = new JPanel();
  P.add(ObjectDepending);
  P.add(KeepCurrentViewer);
  P.setLayout(new GridLayout(2,1));
  KeepCurrentViewer.setSelected(false);
  ObjectDepending.setSelected(true);
  UpBtn = new JButton("up");
  DownBtn = new JButton("down");
  ReadyBtn = new JButton("ok");
  JPanel P2 = new JPanel();
  P2.add(UpBtn);
  P2.add(DownBtn);
  P2.add(ReadyBtn);
  getContentPane().setLayout(new BorderLayout());
  getContentPane().add(P,BorderLayout.NORTH);
  getContentPane().add(ScrollPane,BorderLayout.CENTER);
  getContentPane().add(P2,BorderLayout.SOUTH);
  ActionListener UpDownOkListener = new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        Object Source = evt.getSource();
        if(Source.equals(UpBtn))
           up();
        else if(Source.equals(DownBtn))
           down();
        else if(Source.equals(ReadyBtn))
            setVisible(false);
     }
   };
   UpBtn.addActionListener(UpDownOkListener);
   DownBtn.addActionListener(UpDownOkListener);
   ReadyBtn.addActionListener(UpDownOkListener);
   setSize(400,500);
}

/** set a higher priority for the selected Viewer */
private void up(){
  int index = ViewerList.getSelectedIndex();
  if(Model.up(index))
     ViewerList.setSelectedIndex(index-1);
}

/** set a lower priority for the selected Viewer*/
private void down(){
  int index = ViewerList.getSelectedIndex();
  if(Model.down(index))
     ViewerList.setSelectedIndex(index+1);

}


/** adds a Viewer to this List with highest Priority */
public void addViewer(SecondoViewer SV){
  if(SV!=null)
     Model.addHPViewer(SV);
}

/** if CurrentViewer.canDisplay SO and KeepCurrentViewer is
  * selected then the CurrentViewer is returned;
  * else the best viewer for SO is returned
  */
public SecondoViewer getBestViewer(SecondoViewer CurrentViewer,SecondoObject SO){
  if(SO==null) 
     return CurrentViewer;
  if(KeepCurrentViewer.isSelected())
     if(CurrentViewer!=null && CurrentViewer.canDisplay(SO))
        return CurrentViewer;
  // search the best Viewer for SO
  int count = Model.getSize();
  SecondoViewer TheBestViewer=null;
  boolean found = false;

  if(!ObjectDepending.isSelected()){
     for(int i=0;i<count&&!found;i++)
        if(Model.getViewerAt(i).canDisplay(SO)){
           found=true;
           TheBestViewer = Model.getViewerAt(i);
        }
   }     
   else{
     double quality=-1; // a Viewer get a Value between 0 and 1
     for(int i=0;i<count;i++){
        SecondoViewer Next = Model.getViewerAt(i);
        if(Next.canDisplay(SO)){
           double currentQuality = Next.getDisplayQuality(SO);
           // ensure valid values
           currentQuality = Math.max(Math.min(1,currentQuality),0);
           if(currentQuality>quality){
              quality=currentQuality;
              TheBestViewer = Next;
           }
        }
     
     }
   }
   return TheBestViewer;

}


private JScrollPane ScrollPane;
private ViewerListModel Model;
private JList ViewerList;
private Vector TheViewers;
private JCheckBox ObjectDepending;
private JCheckBox KeepCurrentViewer;
private JButton UpBtn;
private JButton DownBtn;
private JButton ReadyBtn;

private class ViewerListModel implements ListModel{

  public void addListDataListener(ListDataListener LDL){
    if(!MyDataListeners.contains(LDL))
      MyDataListeners.add(LDL);
  }
  public void removeListDataListener(ListDataListener LDL){
     MyDataListeners.remove(LDL); 
  }

  public int getSize(){
    return Viewers.size();
  }
  
  public Object getElementAt(int index){
    if(index<0 || index>=Viewers.size())
       return null;
    else 
       return ((SecondoViewer) Viewers.get(index)).getName();
  }
  
  public SecondoViewer getViewerAt(int index){
    if(index<0 || index>=Viewers.size())
       return null;
    else
       return (SecondoViewer) Viewers.get(index); 
  }

  /** adds a high priority viewer */
  public void addHPViewer(SecondoViewer SV){
    if(!Viewers.contains(SV)){
       Viewers.add(0,SV);
       informListeners();   
    }   
  }

  /** remove a Viewer */
  public void removeViewer(SecondoViewer SV){
    if(Viewers.remove(SV))
        informListeners();
  }
  
 /** set the viewer at index to a higher priority */ 
 public boolean up(int index){
     if(index<1)  // invalid or highest priority
        return false;
     else{
       Object TMP = Viewers.get(index);
       Viewers.setElementAt(Viewers.get(index-1),index);
       Viewers.setElementAt(TMP,index-1);
       informListeners();
       return true;
     }
  }
  
  /** decrement the priority of the viewer at index */
  public boolean down(int index){
     if(index>= Viewers.size()-1 || index<0) // invalid or lowest priority
        return false;
     else{
         Object TMP = Viewers.get(index);    
         Viewers.setElementAt(Viewers.get(index+1),index);
         Viewers.setElementAt(TMP,index+1);
         informListeners();
         return true;
     }
  }
  


  private void informListeners(){
    ListDataEvent LDE = new ListDataEvent(this,ListDataEvent.CONTENTS_CHANGED,0,0);
    for(int i=0;i<MyDataListeners.size();i++)
       ((ListDataListener) MyDataListeners.get(i)).contentsChanged(LDE);
  }

  private Vector MyDataListeners = new Vector();
  private Vector Viewers = new Vector();
}

}


