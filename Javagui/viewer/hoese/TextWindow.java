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

import  java.awt.*;
import  java.awt.event.*;
import  javax.swing.*;
import  sj.lang.ListExpr;
import  java.util.Properties;
import  java.util.*;
import  javax.swing.event.*;
import  viewer.HoeseViewer;


/**
 * This class displays the textual results of a query
 */
public class TextWindow extends JPanel {
/** Allows scrolling over the query result */
  private JScrollPane QueryScrollPane;
/** A ComboBox of all query or import results */ 
  private JComboBox QueryCombo;
/** The main app. */
  private HoeseViewer parent;
/** The Code for no error */
  private static final int NOT_ERROR_CODE = ServerErrorCodes.NOT_ERROR_CODE;

/** a dummy for empty display */
  private JPanel   dummy = new JPanel();

 
/** Components for a search Panel */
  private JTextField SearchText;
  private JButton SearchBtn;

 /**
   * Construktor 
   * @param   MainWindow aparent
   * @see <a href="TextWindowsrc.html#TextWindow">Source</a> 
   */
  public TextWindow (HoeseViewer aparent) {
    super();
    setLayout(new BorderLayout());

    QueryCombo = new JComboBox(new DefaultComboBoxModel());
    QueryCombo.setMaximumSize(new Dimension(200, 300));
    setMinimumSize(new Dimension(100, 100));
    QueryScrollPane = new JScrollPane();
    add(QueryCombo, BorderLayout.NORTH);
    add(QueryScrollPane, BorderLayout.CENTER);
    QueryCombo.addActionListener(new ActionListener() {
      public void actionPerformed (ActionEvent evt) {
        QueryResult qr = null;
        qr = (QueryResult)QueryCombo.getSelectedItem();
        if (qr != null){
          qr.clearSelection();
          QueryScrollPane.setViewportView(qr);
        }
        else
          QueryScrollPane.setViewportView(dummy);
      }
    }); 

    parent = aparent;

    //construct a search panel
    JPanel SearchPanel = new JPanel();
    SearchBtn = new JButton("go");
    JLabel SearchLabel = new JLabel("search");
    SearchText= new JTextField(6);
    SearchPanel.add(SearchLabel);
    SearchPanel.add(SearchText);
    SearchPanel.add(SearchBtn);
    add(SearchPanel,BorderLayout.SOUTH);
    SearchBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
           search();
        }
    });
    SearchText.addKeyListener(new KeyAdapter(){
        public void keyPressed(KeyEvent evt) {
           if(evt.getKeyCode()==KeyEvent.VK_ENTER)
             search();
        }
    });
 }
 
 
 public void ensureSelectedIndexIsVisible(){
   QueryResult qr = (QueryResult) QueryCombo.getSelectedItem();
   if(qr==null)
      return;
   int Pos = qr.getSelectedIndex();
   if(Pos>=0){
      int h = QueryScrollPane.getSize().height;
      int fh = qr.getFont().getSize();
      int rows = h/(fh+4); // ca. number of visible rows (+4 = gap between rows)
      rows = rows/2 -1;  // the rows under and above from Pos;
      int Count = qr.getModel().getSize();
      if(Pos<rows)
         qr.ensureIndexIsVisible(Pos);
      else if(Pos+rows>Count)
         qr.ensureIndexIsVisible(Count);
      else{
         qr.ensureIndexIsVisible(Pos-rows);
         qr.ensureIndexIsVisible(Pos+rows);
      }
   }   
 }
 


 private void search(){
   QueryResult QR = (QueryResult) QueryCombo.getSelectedItem();
   if(QR==null){
     JOptionPane.showMessageDialog(this,"no query result selected");
     return;
   }
   String Text=SearchText.getText().trim();
   if(Text.equals("")){
     JOptionPane.showMessageDialog(this,"no text to search entered");
     return;
   }

   int SelectedIndex = QR.getSelectedIndex();
   int Offset= (SelectedIndex<0)?0:SelectedIndex+1;
   int Pos = QR.find(Text,false,Offset);
   if (Pos<0)
       Pos=QR.find(Text,false,1);
   if(Pos<0){
     JOptionPane.showMessageDialog(this,"text not found");
     return;
   }   
   QR.setSelectedIndex(Pos);
   int h = QueryScrollPane.getSize().height;
   int fh = QR.getFont().getSize();
   int rows = h/(fh+4); // ca. number of visible rows (+4 = gap between rows)
   rows = rows/2 -1;  // the rows under and above from Pos;
   int Count = QR.getModel().getSize();
   if(Pos<rows)
      QR.ensureIndexIsVisible(Pos);
   else if(Pos+rows>Count)
      QR.ensureIndexIsVisible(Count);
   else{
      QR.ensureIndexIsVisible(Pos-rows);
      QR.ensureIndexIsVisible(Pos+rows);
   }

 }


 /* set a new ComboBox()  */
  public void clearComboBox(){
    remove(QueryCombo);
    QueryCombo = new JComboBox();
    QueryCombo.addActionListener(new ActionListener() {
      public void actionPerformed (ActionEvent evt) {
        QueryResult qr = null;
        qr = (QueryResult)QueryCombo.getSelectedItem();
        if (qr != null){
          qr.clearSelection();
          QueryScrollPane.setViewportView(qr);
        }
        else
          QueryScrollPane.setViewportView(dummy);
      }
    }); 
    add(QueryCombo, BorderLayout.NORTH);
  }  



  /**
   * Converts a QueryResult to a listexpr. Used in session-saving
   * @param qr The queryresult to convert
   * @return The result as a ListExpr
   * @see <a href="TextWindowsrc.html#convertQueryResulttoLE">Source</a> 
   */
  private ListExpr convertQueryResulttoLE (QueryResult qr) {
    ListExpr catl = ListExpr.theEmptyList();
    ListExpr left = catl;
    for (int i = 0; i < qr.getGraphObjects().size(); i++) {
      DsplGraph dg = (DsplGraph)qr.getGraphObjects().elementAt(i);
      int catnr = parent.Cats.indexOf(dg.getCategory());
      if (catl.isEmpty()) {
        left = ListExpr.cons(ListExpr.intAtom(catnr), catl);
        catl = left;
        String lab = dg.getLabelText(CurrentState.ActualTime);
        if(lab==null){
            lab ="";
        }
        left = ListExpr.append(left, ListExpr.stringAtom(lab));
        left = ListExpr.append(left, ListExpr.realAtom(dg.getLabPosOffset().getX()));
        left = ListExpr.append(left, ListExpr.realAtom(dg.getLabPosOffset().getY()));
        left = ListExpr.append(left, ListExpr.boolAtom(dg.getVisible()));
      } 
      else {
        left = ListExpr.append(left, ListExpr.intAtom(catnr));
        String lab = dg.getLabelText(CurrentState.ActualTime);
        if(lab==null){
             lab="";
        }
        left = ListExpr.append(left, ListExpr.stringAtom(lab));
        left = ListExpr.append(left, ListExpr.realAtom(dg.getLabPosOffset().getX()));
        left = ListExpr.append(left, ListExpr.realAtom(dg.getLabPosOffset().getY()));
        left = ListExpr.append(left, ListExpr.boolAtom(dg.getVisible()));
      }
    }
    ListExpr layerl = ListExpr.theEmptyList();
    left = layerl;
    for (int i = 0; i < qr.getGraphObjects().size(); i++) {
      DsplGraph dg = (DsplGraph)qr.getGraphObjects().elementAt(i);
      //int layernr = parent.GraphDisplay.getLayer(dg.getLayer());
      int layernr = JLayeredPane.getLayer(dg.getLayer());
      int layerpos = dg.getLayer().getGeoObjects().indexOf(dg);
      if (layerl.isEmpty()) {
        left = ListExpr.cons(ListExpr.intAtom(layernr), layerl);
        layerl = left;
        left = ListExpr.append(left, ListExpr.intAtom(layerpos));
      } 
      else {
        left = ListExpr.append(left, ListExpr.intAtom(layernr));
        left = ListExpr.append(left, ListExpr.intAtom(layerpos));
      }
    }
    return  ListExpr.fourElemList(ListExpr.stringAtom(qr.command), qr.LEResult, 
        catl, layerl);
  }

  /**
   * Converts all QueryResults to a listexpr. Used in session-saving
   * @return The result as a ListExpr
   * @see <a href="TextWindowsrc.html#convertAllQueryResults">Source</a> 
   */
  public ListExpr convertAllQueryResults () {
    QueryResult qr = (QueryResult)QueryCombo.getSelectedItem();
    if (qr != null)
      qr.clearSelection();
    ListExpr le = ListExpr.theEmptyList();
    ListExpr left = le;
    for (int i = 0; i < QueryCombo.getItemCount(); i++)
      if (le.isEmpty()) {
        left = ListExpr.cons(convertQueryResulttoLE((QueryResult)QueryCombo.getItemAt(i)), 
            le);
        le = left;
      } 
      else 
        left = ListExpr.append(left, convertQueryResulttoLE((QueryResult)QueryCombo.getItemAt(i)));
    return  ListExpr.twoElemList(ListExpr.symbolAtom("QueryResults"), le);
  }

  /**
   * Reads the saved QueryResult from a ListExpr.Used in session-loading.
   * @param le
   * @return True if no error has ocured
   */
  public boolean readAllQueryResults (ListExpr le) {

    if(le.listLength()!=2)
      return false;

    Vector Layers = new Vector(10, 10);
    if (le.first().atomType() != ListExpr.SYMBOL_ATOM)
      return  false;
    if (!le.first().symbolValue().equals("QueryResults"))
      return  false;
    le = le.second();
    ListExpr Current;
    while (!le.isEmpty()) {
      //Query lesen
      Current = le.first();
      QueryResult qr = new QueryResult(Current.first().stringValue(), Current.second());
      // ensure to take the background from the textarea for this new object
      qr.setOpaque(this.isOpaque());
      qr.setBackground(this.getBackground());
      if (parent.addQueryResult(qr)) {
        ListExpr CatList = Current.third();
        ListExpr LayerList = Current.fourth();
        ListIterator li = qr.getGraphObjects().listIterator();

        while (li.hasNext()) {
          DsplGraph dg = (DsplGraph)li.next();
          setGOtoLayerPos(Layers, dg, CatList.first().intValue(), LayerList.first().intValue(), 
              LayerList.second().intValue());

          CatList = CatList.rest();
          dg.setLabelAttribute(new
DefaultLabelAttribute(CatList.first().stringValue()));

          CatList = CatList.rest();
          dg.getLabPosOffset().setLocation(CatList.first().realValue(), CatList.second().realValue());
          CatList = CatList.rest().rest();
          dg.setVisible(CatList.first().boolValue());
          CatList = CatList.rest();
          LayerList = LayerList.rest().rest();
        }
      }
      le = le.rest();
    }
    ListIterator lil = Layers.listIterator();
    while (lil.hasNext()) {
      Layer lay = (Layer)lil.next();
      if (lay.getGeoObjects().size() > 0)
        parent.addSwitch(parent.GraphDisplay.addLayer(lay), -1);
    }
    return  true;
  }

  /**GOs will be placed on its original position before saving.Used in Session-saving 
   * @see <a href="TextWindowsrc.html#setGOtoLayerPos">Source</a> 
   */
  private void setGOtoLayerPos (Vector l, DsplGraph dg, int catnr, int laynr, 
      int laypos) {
    while (l.size() <= laynr)
      l.add(new Layer());
    Layer lay = (Layer)l.elementAt(laynr);
    while (lay.getGeoObjects().size() <= laypos)
      lay.getGeoObjects().add(null);
    dg.setCategory((Category)parent.Cats.elementAt(catnr));
    lay.getGeoObjects().setElementAt(dg, laypos);
  }

  /**
   * Gets the ComboBox where all query-results are listed 
   * @return QueryCombo
   * @see <a href="TextWindowsrc.html#getQueryCombo">Source</a> 
   */
  public JComboBox getQueryCombo () {
    return  QueryCombo;
  }

  /**
   * Process the query qr an adds it to the QueryComboBox
   * @param qr
   * @return A list with no error.
   * @see <a href="TextWindowsrc.html#newQueryResult ">Source</a> 
   */
  public ListExpr newQueryResult (QueryResult qr) {
    ListExpr numericType;
    ListExpr answerList;
    if (qr.LEResult.isEmpty()) {
      answerList = ListExpr.twoElemList(ListExpr.intAtom(NOT_ERROR_CODE), ListExpr.theEmptyList());
      return  answerList;
    }
    if (qr.LEResult.listLength() != 2) {
      // If the queryResult is not a two elements list.
      System.out.println("laenge nicht 2");
      qr.addEntry(new String(qr.LEResult.writeListExprToString()));
      answerList = ListExpr.twoElemList(ListExpr.intAtom(NOT_ERROR_CODE), ListExpr.theEmptyList());
      return  answerList;
    }
    processQuery(qr);
    addQueryResult(qr);
    QueryCombo.setSelectedItem(qr);
    //   QueryScrollPane.setViewportView(qr);	
    answerList = ListExpr.twoElemList(ListExpr.intAtom(NOT_ERROR_CODE), ListExpr.theEmptyList());
    return  answerList;
  }

  /**
   * Adds qr to the Combo-Box of queries
   * @param qr
   */
  public void addQueryResult (QueryResult qr) {
    QueryResult q = (QueryResult)QueryCombo.getSelectedItem();
    if (q != null)
          q.clearSelection();
    qr.setToolTipText(qr.toString());
    QueryCombo.addItem(qr);
    // ensure that the querycombo has the same background like 
    // its environment
    QueryCombo.setOpaque(this.isOpaque());
    QueryCombo.setBackground(this.getBackground());
  }

/** Starts scanning of the query result qr for datatypes
  */
  private void processQuery (QueryResult qr) {
    LEUtils.analyse(qr.LEResult.first(), qr.LEResult.second(), qr);  
  }


}




