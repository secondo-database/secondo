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

package  viewer.hoese;

import java.util.Vector;
import sj.lang.ListExpr;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import viewer.HoeseViewer;
import tools.Reporter;


/**
 * This class enhances JList. A list is generated dependant to the types given back and
 * formatted by the query.
 */
public class QueryResult extends JList {
/** The query command */
  public String command;
/** The result given back by the query command as a Listexpr */
  public ListExpr LEResult;
/** A list of the ggraphic objects of this query */
  private Vector GraphObjects;
/** No. of tuples, no. of attributes of a tuple   */
  private int TupelCount, AttrCount;
  



/** the QueryRepresentations for this QueryResult */
  // private ViewConfig myViewConfig = null;
  private Vector ViewConfigs= new Vector(2);


/** stores the interval where Time dependent objects are defined
  **/
  private Interval interval;


  /**
   * Creates a QueryResult with a command and a result of a query
   * @param   String acommand
   * @param   ListExpr aLEResult
   */
  public QueryResult (String acommand, ListExpr aLEResult) {
    super();
    interval = null;
    setFont(new Font("Monospaced",Font.PLAIN,12));

    // processing double clicks
    addMouseListener(new MouseAdapter() {
      public void mouseClicked (MouseEvent e) {
        if (e.getClickCount() != 2){
          return;
        }
        Object o = QueryResult.this.getSelectedValue();
        if ((o instanceof DsplBase) && (((DsplBase)o).getFrame() != null)) {
            ((DsplBase)o).getFrame().select(o);
            ((DsplBase)o).getFrame().show(true);
        }
        if((o instanceof ExternDisplay)){
            ExternDisplay BG = (ExternDisplay) o;
            if(!BG.isExternDisplayed()){
                BG.displayExtern(); 
            }
        }
      }
    });


    setModel(new DefaultListModel());
    setCellRenderer(new QueryRenderer());
    setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    setBackground(Color.lightGray);
    command = acommand;
    LEResult = aLEResult;
    TupelCount = LEResult.second().listLength();
    if (LEResult.first().isAtom()){
      AttrCount = 0;
    }
    else {
     try{
       AttrCount = LEResult.first().second().second().listLength();
     }catch(Exception e){
       AttrCount = 0;
     }
    }
    GraphObjects = new Vector(50);
  }



  /** get the ViewConfigs for this query */
  public ViewConfig[] getViewConfigs(){
     ViewConfig[] Cfgs = new ViewConfig[ViewConfigs.size()];
     for(int i=0;i<Cfgs.length;i++)
         Cfgs[i] = (ViewConfig) ViewConfigs.get(i);
     return Cfgs;
  }


  /** get the ViewConfig with specific index */
  public ViewConfig getViewConfigAt(int index){
    if(index<0 || index>=ViewConfigs.size())
       return null;
    else
       return (ViewConfig) ViewConfigs.get(index);
  } 

  /** set the ViewConfig for this query */
  public void addViewConfig(ViewConfig VCfg){
     ViewConfigs.add(VCfg);
  }

  /** get the Pos of the ViewConfig with specific AttrName 
    * if not exists a ViewConfig with AttrName -1 is returned
    */
  public int getViewConfigIndex(String AttrName){
    int pos = -1;
    boolean found = false;
    for(int i=0;i<ViewConfigs.size()&&!found;i++)
         if( ((ViewConfig)ViewConfigs.get(i)).AttrName.equals(AttrName)){
               pos=i;
               found = true;
         }
    return pos; 
  }


  /**
   * 
   * @return The ListExpr of the picked (selected) list-entry
   */
  public ListExpr getPick () {
    if (AttrCount == 0){
      return  LEResult;
    }
    int selind = getSelectedIndex();
    int TupelNr = (selind/(AttrCount + 1));
    int AttrNr = (selind%(AttrCount + 1));
    if (AttrNr == AttrCount)
      return  null;             // Separator
    ListExpr TupelList = LEResult.second();
    for (int i = 0; i < TupelNr; i++)
      TupelList = TupelList.rest();
    ListExpr AttrList = TupelList.first();
    for (int i = 0; i < AttrNr; i++)
      AttrList = AttrList.rest();
    ListExpr TypeList = LEResult.first().second().second();
    for (int i = 0; i < AttrNr; i++)
      TypeList = TypeList.rest();
    return  ListExpr.twoElemList(TypeList.first().second(), AttrList.first());
  }

  /**
   * 
   * @return graphical objects of this query-result
   */
  public Vector getGraphObjects () {
    return  GraphObjects;
  }

  /**
   * Adds an object to the end of the result list
   * @param entry The entry object
   */
  public void addEntry (Object entry) {
    if(entry!=null){
      if (entry instanceof DsplBase) {
        if (entry instanceof DsplGraph){
          GraphObjects.add(entry);
        }
        if (((DsplBase)entry).getFrame() != null){
          ((DsplBase)entry).getFrame().addObject(entry);
        }
      }
    }

    ((DefaultListModel)getModel()).addElement(entry);
  }



  /** search the given String in this list and returns 
    * the index, the search is started with offset 
    * and go to the end of the list. if the given string
    * is not containing between offset and end -1 is
    * returned
    */

 public int find(String S,boolean CaseSensitiv,int Offset){
   ListModel LM = getModel();
   if(LM==null)
     return -1;
   String UCS = S.toUpperCase();   
   boolean found = false;
   int pos = -1;
   for(int i=Offset;i<LM.getSize() && !found;i++){
      if(CaseSensitiv && LM.getElementAt(i).toString().indexOf(S)>=0){
         pos=i;
         found = true;
      } 
      if(!CaseSensitiv && LM.getElementAt(i).toString().toUpperCase().indexOf(UCS)>=0){
         pos=i;
         found = true;
      }
   }
   return pos;
 }


  public boolean equals(Object o){
    if(!(o instanceof QueryResult))
      return false;
    else{
      QueryResult qr = (QueryResult)o;
      return command.equals(qr.command) && (LEResult==(qr.LEResult));
    }
  }

  /**
    * return the command
    */
  public String getCommand(){return command;}

  /** return the ListExpr */
  public  ListExpr getListExpr(){return LEResult;}


  /** computes the TimeBounds from the contained objects.
   **/
  public void computeTimeBounds(){
     ListModel listModel = getModel();
     int size = listModel.getSize();
     this.interval=null;
     for(int i=0;i<size;i++){
        Object o = listModel.getElementAt(i);
        if(o instanceof Timed){
           Interval oInterval = ((Timed)o).getBoundingInterval();
           if(oInterval!=null){
              if(this.interval==null){
                 this.interval = oInterval.copy();
              } else{
                 this.interval.unionInternal(oInterval);
              }
           }
        }
     } 
  } 



  /** Returns the interval containing all definition times of
    * object instances of Timed. If no such time exist, the result
    * is null.
    **/
  public Interval getBoundingInterval(){
    return interval;
  }
   


  /**
   * 
   * @return The command of the query as string representation for the query combobox  
   */
  public String toString () {
    return  command;
  }
  /** A class for special rendering of datatypes in the list
   */
  private class QueryRenderer extends DefaultListCellRenderer {
    public Component getListCellRendererComponent (JList list, Object value, 
        int index, boolean isSelected, boolean cellHasFocus) {
      super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
      //if (value instanceof DsplGraph)
      //setIcon(new ImageIcon("images/labdlg.gif"));
      setForeground(Color.BLACK);
      if ((value instanceof DsplGraph) && (value instanceof Timed))
        setForeground(Color.magenta); 
      else if (value instanceof DsplGraph)
        setForeground(Color.red); 
      else if (value instanceof Timed)
        setForeground(Color.blue); 
      else if (value instanceof DsplBase)
        setForeground(new Color(0, 100, 0));
      if (value instanceof DsplBase)
        if (!((DsplBase)value).getVisible())
          setForeground(Color.gray);
      if( value instanceof ExternDisplay){
         setForeground(Color.GREEN);
      } 
      return  this;
    }
  }
}



