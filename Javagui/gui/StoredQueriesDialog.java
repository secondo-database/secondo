//This file is part of SECONDO.

//Copyright (C) 2009, University in Hagen, 
// Faculty of Mathematics and Computer Science, 
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
import java.util.*;
import java.awt.event.*;
import javax.swing.event.*;

public class StoredQueriesDialog extends JDialog{
 
  public StoredQueriesDialog(Frame parent){
    super(parent,true);
    model = new StoredQueriesModel();
    combobox = new JComboBox(model); 
    textarea = new JTextArea();
    textarea.setEditable(false);
    getContentPane().setLayout(new BorderLayout());
    getContentPane().add(combobox, BorderLayout.NORTH);
    getContentPane().add(textarea, BorderLayout.CENTER);
    JPanel buttom = new JPanel();
    JButton okBtn = new JButton("ok");
    JButton cancelBtn = new JButton("cancel");
    buttom.add(cancelBtn);
    buttom.add(okBtn);
    okBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
         StoredQueriesDialog.this.finish(true);
       }
    });

    cancelBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
         StoredQueriesDialog.this.finish(false);
       }
    });
    
    combobox.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
         int index = combobox.getSelectedIndex();
         if(index>=0){
            Pair p =  model.get(index);
            textarea.setText(p.query);
         }
       }
    }); 
    
    getContentPane().add(buttom,BorderLayout.SOUTH);
    setSize(640,480);

  }

  public void finish(boolean ok){
    if(ok){
      selectedQuery = textarea.getText();
    }
    setVisible(false);
  }

  public int readFromFile(String fileName){
    int res = model.readFromFile(fileName);
    if(model.getSize()>0){
      model.setSelectedItem(model.getElementAt(0));
    }
    return res;
  } 
  
  public boolean add(String name, String query){
    boolean res = model.add(name,query);
    if(res){
       model.setSelectedItem(name);
    }
    return res;
  }

  public boolean saveToFile(String fileName){
    return model.saveToFile(fileName);
  }

  public boolean remove(String name){
    return model.removeName(name);
  }

  public boolean contains(String name){
    return model.containsName(name);
  }

  public String showQueries(){
     selectedQuery=null;
     setVisible(true);
     return selectedQuery;
  }

  private JComboBox combobox;
  private JTextArea textarea; 
  private StoredQueriesModel model;
  private String selectedQuery;

  private class StoredQueriesModel implements ComboBoxModel{
    public StoredQueriesModel(){
       data = new StoredQueries();
       listener = new Vector();
    }   

    public Object getSelectedItem(){
      return selected;
    }

    public void setSelectedItem(Object anItem){
       if(!(anItem instanceof String)){
         return;
       }
       if(data.containsName((String)anItem)){
         selected = (String)anItem;
         informListener();
       }
    }

    public int getSize(){
       return data.getSize();
    }

    public Object getElementAt(int index){
       return ((Pair)data.getElementAt(index)).name;
    }

    public Pair get(int index){
       return (Pair)data.getElementAt(index);
    }

    public void addListDataListener( ListDataListener l){
       if(!listener.contains(l)){
         listener.add(l);
       }
    }
    
   public void removeListDataListener(ListDataListener l){
     listener.remove(l);
   }

   public int readFromFile(String fileName){
     int res =  data.readFromFile(fileName);
     informListener();
     return res;
   }

   public boolean saveToFile(String fileName){
     return data.saveToFile(fileName);
   }

  public boolean add(String Name, String Query){
    boolean res = data.putEntry(Name,Query);
    informListener();
    return res;
  }

  public String getQuery(String name){
    return data.getQuery(name);
  }

  public boolean removeName(String name){
     boolean res =  data.removeName(name);
     if(res){
        informListener();
     }
     return res;
  }

  public boolean containsName(String name){
     return data.containsName(name);
  }


   private void informListener(){
     for(int i=0;i<listener.size();i++){
        ListDataListener l = (ListDataListener) listener.get(i);
        l.contentsChanged(new ListDataEvent(this, ListDataEvent.CONTENTS_CHANGED, 0,0));
     }
   }
    private Vector listener;
    private String selected=null;
    private StoredQueries data; 
  } // class StoredQueriesModel

}

