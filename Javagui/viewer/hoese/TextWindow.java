

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
        String lab = (dg.getLabelText() == null) ? "" : dg.getLabelText();
        left = ListExpr.append(left, ListExpr.stringAtom(lab));
        left = ListExpr.append(left, ListExpr.realAtom((float)dg.getLabPosOffset().getX()));
        left = ListExpr.append(left, ListExpr.realAtom((float)dg.getLabPosOffset().getY()));
        left = ListExpr.append(left, ListExpr.boolAtom(dg.getVisible()));
      } 
      else {
        left = ListExpr.append(left, ListExpr.intAtom(catnr));
        String lab = (dg.getLabelText() == null) ? "" : dg.getLabelText();
        left = ListExpr.append(left, ListExpr.stringAtom(lab));
        left = ListExpr.append(left, ListExpr.realAtom((float)dg.getLabPosOffset().getX()));
        left = ListExpr.append(left, ListExpr.realAtom((float)dg.getLabPosOffset().getY()));
        left = ListExpr.append(left, ListExpr.boolAtom(dg.getVisible()));
      }
    }
    ListExpr layerl = ListExpr.theEmptyList();
    left = layerl;
    for (int i = 0; i < qr.getGraphObjects().size(); i++) {
      DsplGraph dg = (DsplGraph)qr.getGraphObjects().elementAt(i);
      int layernr = parent.GraphDisplay.getLayer(dg.getLayer());
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
      if (parent.addQueryResult(qr)) {
        ListExpr CatList = Current.third();
        ListExpr LayerList = Current.fourth();
        ListIterator li = qr.getGraphObjects().listIterator();

        while (li.hasNext()) {
          DsplGraph dg = (DsplGraph)li.next();
          setGOtoLayerPos(Layers, dg, CatList.first().intValue(), LayerList.first().intValue(), 
              LayerList.second().intValue());

          CatList = CatList.rest();
          dg.setLabelText(CatList.first().stringValue());
          if (dg.getLabelText().equals(""))
            dg.setLabelText(null);

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
  }

/** Starts scanning of the query result qr for datatypes
  */
  private void processQuery (QueryResult qr) {
    LEUtils.analyse(qr.LEResult.first(), qr.LEResult.second(), qr);  
  }


}



