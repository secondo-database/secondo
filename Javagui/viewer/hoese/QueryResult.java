package  viewer.hoese;

import  java.util.Vector;
import  sj.lang.ListExpr;
import  javax.swing.*;
import  java.awt.*;
import  java.awt.event.*;
import  viewer.HoeseViewer;


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
  // for each DsplGraph we need a separate ViewConfig
  private Vector ViewConfigs= new Vector(2);

  /**
   * Creates a QueryResult with a command and a result of a query 
   * @param   String acommand
   * @param   ListExpr aLEResult
   */
  public QueryResult (String acommand, ListExpr aLEResult) {
    super();
    setFont(new Font("Monospaced",Font.PLAIN,12));
    addMouseListener(new MouseAdapter() {
      public void mouseClicked (MouseEvent e) {
        if (e.getClickCount() != 2)
          return;
        Object o = QueryResult.this.getSelectedValue();
        if ((o instanceof DsplBase) && (((DsplBase)o).getFrame() != null)) {
          ((DsplBase)o).getFrame().select(o);
          ((DsplBase)o).getFrame().show(true);
        }
        //			((DsplBase)o).getFrame().select(o);	
      }
    });
    setModel(new DefaultListModel());
    setCellRenderer(new QueryRenderer());
    setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    setBackground(Color.lightGray);
    command = acommand;
    LEResult = aLEResult;
    TupelCount = LEResult.second().listLength();
    if (LEResult.first().isAtom())
      AttrCount = 0; 
    else 
      AttrCount = LEResult.first().second().second().listLength();
    GraphObjects = new Vector(50, 50);
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
    if (entry instanceof DsplBase) {
      if (entry instanceof DsplGraph)
        GraphObjects.add(entry);
      if (((DsplBase)entry).getFrame() != null)
        ((DsplBase)entry).getFrame().addObject(entry);
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
      return true;
    else{
      QueryResult qr = (QueryResult)o;
      return command.equals(qr.command) & LEResult.equals(qr.LEResult);
    }
  }

  /** 
    * return the command 
    */
  public String getCommand(){return command;}

  /** return the ListExpr */
  public  ListExpr getListExpr(){return LEResult;}

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
      return  this;
    }
  }
}



