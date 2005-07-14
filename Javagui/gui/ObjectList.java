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
import java.awt.*;
import java.awt.event.*;
import java.util.Vector;
import javax.swing.plaf.basic.*;
import java.io.*;
import sj.lang.ListExpr;
import sj.lang.ServerErrorCodes;
import gui.idmanager.*;
import java.io.File;
import javax.swing.event.*;
import extern.*;
import extern.binarylist.*;

public class ObjectList extends JPanel{

public final static int NO_ERROR=0;
public final static int ERROR_NAME_EXISTS=1;
public final static int ERROR_OBJECT_NOT_FOUND=2;

private JPanel ControlPanel;
private JList Content;
private JScrollPane ScrollPane;

private Vector Objects;
private ResultProcessor RP;
private ViewerControl VC;
private JButton ShowBtn;     // show the selected Object if possible
private JButton HideBtn;     // hide selected Object
private JButton SaveBtn;     // save Selected Object to file
private JButton LoadBtn;     // load a new Object from File
private JButton RemoveBtn;   // remove selected Object from List and invoke hide
private JButton ClearBtn;
private JButton StoreBtn;    // save the selected Object into database
private JButton RenameBtn;   // set a new name for this object
private boolean isRenameMode;
private boolean StoringEnabled;

private JFileChooser FileChooser;
private RenamePanel aRenamePanel;

private ObjectListModel myListModel;
private ImportManager importmanager = new ImportManager();

private final static String DisplayMark ="** ";    // ensure thas Displaymark and NoDisplayMark have the same length
private final static String NoDisplayMark ="   ";

private void showMessage(String text){
  JOptionPane.showMessageDialog(this,text);
}


public String getErrorText(int ErrorCode){
  if(ErrorCode==NO_ERROR) return "OK";
  if(ErrorCode==ERROR_OBJECT_NOT_FOUND) return "object not found";
  if(ErrorCode==ERROR_NAME_EXISTS) return "name already exists";
  return"unknow error";

}


public ObjectList(ResultProcessor aRP,ViewerControl aVC){
  setLayout(new BorderLayout());
  ControlPanel = new JPanel();
  add(ControlPanel,BorderLayout.NORTH);
  Content=new JList();
  myListModel = new ObjectListModel();
  Content.setModel(myListModel);
  aRenamePanel = new RenamePanel();
  isRenameMode = false;
  ScrollPane = new JScrollPane(Content);
  add(ScrollPane,BorderLayout.CENTER);
  Content.setFont(new Font("MonoSpaced",Font.PLAIN,12));
  Objects = new Vector();
  RP = aRP;
  VC = aVC;
  ShowBtn = new JButton("show");
  HideBtn = new JButton("hide");
  RemoveBtn = new JButton("remove");
  ClearBtn = new JButton("clear");
  SaveBtn = new JButton("save");
  LoadBtn = new JButton("load");
  StoreBtn = new JButton("store");
  RenameBtn = new JButton("rename");
  ShowBtn.setEnabled(false);
  HideBtn.setEnabled(false);
  RemoveBtn.setEnabled(false);
  SaveBtn.setEnabled(false);
  RenameBtn.setEnabled(false);
  StoreBtn.setEnabled(false);
  StoringEnabled=false;
  Content.addListSelectionListener(new ListSelectionListener(){
        public void valueChanged(ListSelectionEvent evt){
           ObjectList.this.listChanged();
	}});
   Content.getModel().addListDataListener(new ListDataListener(){
      public void contentsChanged(ListDataEvent evt){
          ObjectList.this.listChanged();
      }
      public void intervalAdded(ListDataEvent evt){}
      public void intervalRemoved(ListDataEvent evt){}
    });

  ControlPanel.setLayout(new GridLayout(2,4));
  ControlPanel.add(ShowBtn);
  ControlPanel.add(HideBtn);
  ControlPanel.add(RemoveBtn);
  ControlPanel.add(ClearBtn);
  ControlPanel.add(SaveBtn);
  ControlPanel.add(LoadBtn);
  ControlPanel.add(StoreBtn);
  ControlPanel.add(RenameBtn);
  FileChooser = new JFileChooser();
  addAllListeners();
}



/** set the FontSize for this list
  * if size is not in [6,50] then size is changed
  * to the next value in this interval */
public void setFontSize(int size){
   if(size<6) size=6;
   if(size>50) size=50;
   Content.setFont(new Font("MonoSpaced",Font.PLAIN,size));
}


/** get the actual used fontsize */
public int getFontSize(){
  return Content.getFont().getSize();
}


/** set the directory to load and save objects */
public void setObjectDirectory(File dir){
  if(dir!=null)
     FileChooser.setCurrentDirectory(dir);
}

/** enables / disables the store button */
public void enableStoring(boolean enabled){
   StoringEnabled=enabled;
   if(!enabled)
     StoreBtn.setEnabled(false);
   else{
     int index =Content.getSelectedIndex();
     int max = Content.getModel().getSize();
     if (index>=0 & index < max)
         StoreBtn.setEnabled(true);
   }

}


/** returns the index of object with Name Objectname */
private int getIndexOf(String ObjectName){
   ObjectName = ObjectName.trim();
   int pos = -1;
   for(int i=0;i<Objects.size();i++)
      if (((SecondoObject)Objects.get(i)).getName().trim().equals(ObjectName))
          pos = i;
   return pos;
}


public int renameObject(String oldName,String newName){
  int index = getIndexOf(oldName);
  if(index<0)
    return ERROR_OBJECT_NOT_FOUND;
  else{
      int newindex = getIndexOf(newName);
      if(newindex>=0)
         return ERROR_NAME_EXISTS;
      else{
         ((SecondoObject)Objects.get(index)).setName(newName);
         updateList();
         return NO_ERROR;
      }
  }
}


public boolean showObject(String Name){
   int index = getIndexOf(Name);
   if (index<0)
      return false;
   else{
     Content.setSelectedIndex(index);
     Content.ensureIndexIsVisible(index);
     showSelectedObject();
     return true;
   }
}


public boolean hideObject(String Name){
  int index = getIndexOf(Name);
  if(index <0)
     return false;
  else{
     Content.setSelectedIndex(index);
     hideSelectedObject();
     System.gc();
     return true;
  }
}

public boolean removeObject(String Name){
 int index = getIndexOf(Name);
  if(index <0)
     return false;
  else{
     Content.setSelectedIndex(index);
     removeSelectedObject();
     System.gc();
     return true;
  }
}

public boolean saveObject(String Name){
 int index = getIndexOf(Name);
  if(index <0)
     return false;
  else{
     Content.setSelectedIndex(index);
     saveSelectedObject();
     return true;
  }
}




public boolean storeObject(String Name){
 int index = getIndexOf(Name);
  if(index <0)
     return false;
  else{
     Content.setSelectedIndex(index);
     storeSelectedObject();
     return true;
  }
}




/** returns the minimumSize of this Component */
public Dimension getMinimumSize(){
   if (RP==null)
      return new Dimension(320,400);
   else
      return new Dimension(320,RP.getSize().height/4);
}

/** return the preferredSize **/
public Dimension getPreferredSize(){
   return getMinimumSize();
}

public Dimension getMaximumSize(){
   return getMinimumSize();
}

/** enabled or disables buttons, when list is changed */
private void listChanged(){
   int index = Content.getSelectedIndex();
   boolean on = index>=0;
   if(index> Content.getModel().getSize()-1)
      on=false;
   ShowBtn.setEnabled(on);
   HideBtn.setEnabled(on);
   RemoveBtn.setEnabled(on);
   SaveBtn.setEnabled(on);
   RenameBtn.setEnabled(on);
   
   StoreBtn.setEnabled( on & StoringEnabled);
}


private void addAllListeners(){
   Content.addMouseListener(new MouseAdapter(){
      public void mouseClicked(MouseEvent evt){
          if(evt.getClickCount()==2)
             showSelectedObject();
      }});

   ShowBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
          showSelectedObject();
      }});

   HideBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          hideSelectedObject();
       }});

   SaveBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         saveSelectedObject();
     }});

   LoadBtn.addActionListener(new ActionListener(){
     public void actionPerformed(ActionEvent evt){
        loadObject();
     }});   
   
   RemoveBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
          removeSelectedObject();
      }}); 
   RenameBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           setRenameMode(true);
       }}); 
   ClearBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           clearList();
       }});

   StoreBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           storeSelectedObject();
       }});  
}

/** turn off the listdisplay (problems with  overlapping Menus) **/
public void showNothing(){
  //remove(Content);
}

/** turn on the listdisplay (problems with overlapping Menus) **/
public void showList(){
  //add(Content,BorderLayout.CENTER);
}


public void selectObject(SecondoObject SO){
   int index = Objects.indexOf(SO);
   if (index>=0){
       Content.setSelectedIndex(index);
   }
}

public void clearList(){
  long usedMemory = Environment.MEASURE_MEMORY?Environment.usedMemory():0L;

  int len=Objects.size();
  for(int i=0;i<len;i++){
    SecondoObject SO = (SecondoObject) Objects.get(0);
    if (VC!=null)
        VC.hideObject(this,SO);
    Objects.remove(0);
    myListModel.remove(0);
    SO.toListExpr().destroy();
  }
  System.gc();
  updateList();
  if(Environment.MEASURE_MEMORY){
     System.out.println("Memory difference by clearing object list: "+
                         Environment.formatMemory(Environment.usedMemory()-usedMemory));
     Environment.printMemoryUsage();
  }
}


public boolean hideSelectedObject(){
  boolean hidden=false;
  int index = Content.getSelectedIndex();
  if (index>=0){
      SecondoObject SO = (SecondoObject) Objects.get(index);
      if (VC!=null)
          VC.hideObject(this,SO);
      markAsNoDisplayed(SO);
      hidden=true;
  } 
  return hidden;
}

/* sends the remove message for all objects to the ViewerControl*/
public void hideAll(){
   SecondoObject SO;
   for(int i=0;i<Objects.size();i++){
      SO = (SecondoObject) Objects.get(i);
      if (VC!=null)
          VC.hideObject(this,SO);
   }
   updateMarks();
}

/* sends the showObject message for the selected object to the ViewerControl */
public boolean showSelectedObject(){
    boolean ok=false; 
    int index = Content.getSelectedIndex();
    if (index>=0){
        SecondoObject SO = (SecondoObject) Objects.get(index); 
        if (VC!=null){
            if (VC.showObject(SO)){
                 markAsDisplayed(SO);
                 ok = true;
            }
       }
    }
    return ok;
} 

/* sends the showObject message for all Objects to the ViewerControl */
public void showAll(){
   SecondoObject SO;
   for(int i=0;i<Objects.size();i++){
      SO =(SecondoObject)Objects.get(i);
      if(VC!=null && VC.canActualDisplay(SO))
          VC.showObject(SO);
   }
   updateMarks();
}




/** load a new Object from a file into Objectlist
  * @return number of loaded Objects
  */
public int loadObject(){
       int number = 0;
       File CurrentDir = FileChooser.getCurrentDirectory();
       FileChooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
       FileChooser.setSelectedFile(CurrentDir);
       FileChooser.setCurrentDirectory(CurrentDir);
       FileChooser.setMultiSelectionEnabled(true);
       FileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
       if (FileChooser.showOpenDialog(this)==JFileChooser.APPROVE_OPTION){
           File[] Fs = FileChooser.getSelectedFiles();
           for(int i=0;i<Fs.length;i++){
              File F = Fs[i];
              if(loadObject(F))
                 number++;
              else
                 showMessage("cannot load the file:"+F.getName());
           }
       }
       return number;
}


/** reads a Secondo object from file
  * returns true if successful
  */
public boolean loadObject(File ObjectFile){
  ListExpr LE = importmanager.importFile(ObjectFile.getPath());
  if(LE==null)
     return false;
  else{
     SecondoObject SO = new SecondoObject(IDManager.getNextID());
     SO.setName("File :"+ObjectFile.getName());
     SO.fromList(LE);
     addEntry(SO);
     return true;
  }
}

/** set the maximal String length for import files */
public void setMaxStringLength(int len){
  importmanager.setMaxStringLength(len);
}



/** save the selected Object into a File */
public boolean saveSelectedObject(){
  boolean saved = false;
 int index =Content.getSelectedIndex();
  if (index<0){
      showMessage("no item selected");
  }
  else{
       File CurrentDir = FileChooser.getCurrentDirectory();
       FileChooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
       FileChooser.setSelectedFile(CurrentDir);
       FileChooser.setCurrentDirectory(CurrentDir);
       FileChooser.setMultiSelectionEnabled(false);
       FileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);

      if (FileChooser.showSaveDialog(ObjectList.this)==JFileChooser.APPROVE_OPTION){
          File F = FileChooser.getSelectedFile();
          String FullFileName=F.getPath();
          SecondoObject SO = (SecondoObject) Objects.get(index);
	  ListExpr LE = SO.toListExpr();
	  // convert to SecondoObject ready for a restore
	  if(( FullFileName.endsWith("obj") |
	       FullFileName.endsWith("obj.bnl")) && LE.listLength()==2){
	      String name = SO.getName().trim();
	      name = name.replace(' ','_');
	      if(name.length()>48)
	         name = name.substring(0,48);
	      LE = ListExpr.sixElemList( ListExpr.symbolAtom("OBJECT"),
	                                 ListExpr.symbolAtom(name),
	                                 ListExpr.theEmptyList(),
					 LE.first(),
					 LE.second(),
           				 ListExpr.theEmptyList());
	  }
        if(FullFileName.endsWith(".bnl")){
	   BufferedOutputStream FOS =null;
           try{
              FOS = new BufferedOutputStream(new FileOutputStream(FullFileName));
              saved = LE.writeBinaryTo(FOS);
	      }
              catch(Exception e){
                saved = false;
              }finally{
	         try{FOS.close();} catch(Exception e){}
	      }

        }
        else
          saved = LE.writeToFile(FullFileName)==0;
       }
  }
  return saved;
 }



 // delete the selected object from list **/
 public boolean removeSelectedObject(){
   boolean removed = false;
   int index = Content.getSelectedIndex();
   if (index<0 || index>=Objects.size())
      showMessage("no item selected");
   else {
     SecondoObject SO = (SecondoObject) Objects.get(index);
     VC.hideObject(this,SO);
     myListModel.remove(index);
     Objects.remove(index);
     SO.toListExpr().destroy();
     removed = true;
   }
   return removed;
 }

 // remove the given Object
 public void removeObject(SecondoObject SO){
   int index = Objects.indexOf(SO);
   if(index>=0){
      myListModel.remove(index);
      Objects.remove(index);
   }
 }



 // store selected object into DB
public boolean storeSelectedObject(){
  boolean stored = false;
  int index = Content.getSelectedIndex();
  if (index<0){
     showMessage("no item selected");
  } else{
       SecondoObject SO = (SecondoObject) Objects.get(index);
       String SOName = SO.getName().trim();
       if (SOName.indexOf(" ")>=0)  // a space cannot be in objectname
           showMessage("objectname contains spaces\n please rename");
       else{
          String cmd;
          ListExpr SOList = SO.toListExpr();
          ListExpr type = SOList.first();
          StringBuffer SB = new StringBuffer();
          if (type.writeToString(SB)!=0)
              showMessage("type analyse failed");
          else{
              cmd = "(create "+SOName+" : "+SB+")";
              int ErrorCode = RP.internCommand(cmd);
              if (ErrorCode!=0){
                 showMessage("create object error:\n"+ServerErrorCodes.getErrorMessageText(ErrorCode));
              }else{
                  if (SOList.writeToString(SB)!=0){
                     showMessage("cannot update object (error in objectList)");
                     // delete the object
                     cmd ="delete "+SOName;
                     ErrorCode = RP.internCommand(cmd);
                     if (ErrorCode!=0)
                         showMessage("cannot delete object\n"+ServerErrorCodes.getErrorMessageText(ErrorCode));
                  }  // list not ok
  
                  cmd = "(update "+SOName+" := "+SB+")";
                  ErrorCode = RP.internCommand(cmd);
                  if (ErrorCode!=0){
                     showMessage("cannot update object\n"+ServerErrorCodes.getErrorMessageText(ErrorCode));
                     // delete the object
                     cmd ="delete "+SOName;
                     ErrorCode = RP.internCommand(cmd);
                     if (ErrorCode!=0)
                         showMessage("cannot delete object\n"+ServerErrorCodes.getErrorMessageText(ErrorCode));
                  } 
                  else { // success
                     showMessage("object "+SOName+" stored into database");
                     stored = true;
                  }
              }
          }
       }
  }
  return stored;
}



public void addEntry(SecondoObject SO){
  if(SO!=null){
     if (Objects.indexOf(SO)<0){   // object not in list
          Objects.add(SO); 
         // check if name used
         String Name = SO.getName().trim();
         Vector UsedNames = new Vector(myListModel.getSize());
         for(int i=0;i<myListModel.getSize();i++)
           UsedNames.add(((String)myListModel.getElementAt(i)).substring(DisplayMark.length()).trim());
         if (UsedNames.contains(Name) | Name.equals("")){  
            int no = 1;
            String NName=Name+"_"+no;
            while (UsedNames.contains(NName)){
               no++;
               NName = Name+"_"+no;
            }
            Name = NName;
         }

         SO.setName(Name);  // save the change 
         myListModel.add(Name);
         if ((VC!=null) && (VC.isActualDisplayed(SO)))
            markAsDisplayed(SO);
         else
            markAsNoDisplayed(SO);
     }
  }   
}


public void updateObject(SecondoObject SO){
  if(SO !=null){  
      String Name = SO.getName().trim();
      // store all used names
      Vector UsedNames = new Vector(myListModel.getSize());
      for(int i=0;i<myListModel.getSize();i++){
        UsedNames.add(((String)myListModel.getElementAt(i)).substring(DisplayMark.length()).trim());
      }
      int index = UsedNames.indexOf(Name);
      if(index<0)
        addEntry(SO);
      else
        Objects.set(index,SO);
      updateList();  
      selectObject(SO);
  }
}


public void updateMarks(){
  SecondoObject SO; 
  for(int i=0;i<Objects.size();i++){
     SO = (SecondoObject) Objects.get(i);
     myListModel.remove(i);
     if (VC!=null && VC.isActualDisplayed(SO))
        myListModel.add(DisplayMark+SO.getName(),i);
     else
        myListModel.add(NoDisplayMark+SO.getName(),i);
  }
}


/** set a "isDisplayed" mark for SO **/
private void markAsDisplayed(SecondoObject SO){
   int index = Objects.indexOf(SO);
   if (index>=0){
      myListModel.remove(index);
      myListModel.add(DisplayMark+SO.getName(),index);
   }
}

/** mark an Entry as NO Displayed **/
private void markAsNoDisplayed(SecondoObject SO){
   int index = Objects.indexOf(SO);
   if (index>=0){
      myListModel.remove(index);
      myListModel.add(NoDisplayMark+SO.getName(),index);
   }
}



private void setRenameMode(boolean mode){
  boolean on;
  if(mode && !isRenameMode){
     int index = Content.getSelectedIndex();
     if (index>=0){
         SecondoObject SO = (SecondoObject) Objects.get(index);
         remove(ScrollPane);
         aRenamePanel.setObject(SO);
         add(aRenamePanel,BorderLayout.CENTER);
         isRenameMode=true;
         aRenamePanel.revalidate();
	 revalidate();
	 repaint();
	 // disable all Buttons
         on = false;
         ShowBtn.setEnabled(on);
         HideBtn.setEnabled(on);
         RemoveBtn.setEnabled(on);
         SaveBtn.setEnabled(on);
         LoadBtn.setEnabled(on);
         StoreBtn.setEnabled(on);
         RenameBtn.setEnabled(on);
     }
     else
       showMessage("no item selected");
  }
  if(!mode && isRenameMode){
    remove(aRenamePanel);
    add(ScrollPane,BorderLayout.CENTER);
    isRenameMode=false;
    on = true;
    ShowBtn.setEnabled(on);
    HideBtn.setEnabled(on);
    RemoveBtn.setEnabled(on);
    SaveBtn.setEnabled(on);
    LoadBtn.setEnabled(on);
    StoreBtn.setEnabled(on);
    RenameBtn.setEnabled(on);
    ScrollPane.revalidate();
  }
}

/** set the content of Content to the content of Objects **/
private void updateList(){
  myListModel.removeAll();
  SecondoObject SO;
  for(int i=0;i<Objects.size();i++){
    SO= (SecondoObject) Objects.get(i);
    myListModel.add(SO.getName());
  }
  updateMarks();
}



private class RenamePanel extends JPanel{
  private JTextField OldName;
  private JTextField NewName;
  private SecondoObject SO;
  private JButton OkBtn;
  private JButton CancelBtn;
  private JPanel innerPanel;
  RenamePanel(){
     super();
     innerPanel = new JPanel(new GridLayout(4,1));
     OldName = new JTextField();
     OldName.setEditable(false);
     NewName = new JTextField();
     SO=null;
     innerPanel.add(new JLabel("old name"));
     innerPanel.add(OldName);
     innerPanel.add(new JLabel("new name"));
     innerPanel.add(NewName);
     OkBtn = new JButton("OK");
     CancelBtn = new JButton("cancel");
     innerPanel.add(OkBtn);
     innerPanel.add(CancelBtn);
     add(innerPanel);

     OkBtn.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent evt){
	    accept();
         }
     });

     CancelBtn.addActionListener(new ActionListener(){
          public void actionPerformed(ActionEvent evt){
             cancel();
	  }
       });


     NewName.addKeyListener(new KeyAdapter(){
      public void keyPressed(KeyEvent evt){
         int k = evt.getKeyCode();
         if (k==KeyEvent.VK_ENTER)
             accept();
         if(k==KeyEvent.VK_ESCAPE)
            cancel();
       }
     });

  } // constructor

   void setObject(SecondoObject SO){
     OldName.setText("");
     NewName.setText("");
     this.SO = SO;
     if (SO!=null){
	   OldName.setText(SO.getName());
         NewName.setText(SO.getName());
     }
   } // setObject

  private void cancel(){
      ObjectList.this.setRenameMode(false);
  }

  private void accept(){
       Vector UsedNames = new Vector(myListModel.getSize());
       for(int i=0;i<myListModel.getSize();i++)
           UsedNames.add(((String)myListModel.getElementAt(i)).substring(DisplayMark.length()).trim());

        String Name = NewName.getText();

	if (UsedNames.contains(Name)) {
            ObjectList.this.showMessage("Name allready used\n please choose another one");
        }
        else{
           if(!Name.equals("")){
               ObjectList.this.VC.hideObject(ObjectList.this,SO);
               SO.setName(Name);
               ObjectList.this.updateList();
               ObjectList.this.setRenameMode(false);
	       ObjectList.this.revalidate();
	       ObjectList.this.repaint();
           } else
	      ObjectList.this.showMessage("not a valid object name");
       }
 } // accept

} // renamePanel


private class ObjectListModel implements ListModel{

public void addListDataListener(ListDataListener LDL){
  myListDataListeners.add(LDL);
}

public void removeListDataListener(ListDataListener LDL){
  int index = myListDataListeners.indexOf(LDL);
  myListDataListeners.remove(index);
}


public int getSize(){
  return Content.size();
}

public Object getElementAt(int index){
  if(index<0 || index>myListModel.getSize())
    return null;
  else
    return Content.get(index);
}


public boolean contains(String S){
  return Content.contains(S);
}


public int getIndexOf(String S){
  return Content.indexOf(S);
}

public boolean removeIndex(int index){
  boolean ok = index>=0 & index<Content.size();
  Content.remove(index);
  if(ok)
     informListeners(index);
  return ok;
}

public boolean remove(int index){
  return removeIndex(index);
}

public void removeAll(){
  Content.clear();
  informListeners(0);

}


public boolean add(String S){
  if(Content.indexOf(S)<0){
     Content.add(S);
     informListeners(Content.size());
     return true;
  }
  else
    return false;
}

public void add(String S,int index){
   if(index<0) index=0;
   if(index>Content.size()) index=Content.size();
   Content.add(index,S);
   informListeners(index);
}


private void informListeners(int index){
  ListDataEvent evt = new ListDataEvent(this,ListDataEvent.CONTENTS_CHANGED,index,index);
  for(int i=0;i<myListDataListeners.size();i++)
       ((ListDataListener)myListDataListeners.get(i)).contentsChanged(evt);
}

private Vector myListDataListeners = new Vector();
private Vector Content = new Vector(20);
}


}




