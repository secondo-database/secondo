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
    int labno=0;
    int rendno=0;
    for (int i = 0; i < qr.getGraphObjects().size(); i++) {
      DsplGraph dg = (DsplGraph)qr.getGraphObjects().elementAt(i);
      int catnr = parent.Cats.indexOf(dg.getCategory());
      LabelAttribute la = dg.getLabelAttribute();
      RenderAttribute ra = dg.getRenderAttribute();
      // find the objects within the query result
      labno=-1; 
      rendno=-1;
      boolean doneLab  = (la==null) || (la instanceof DefaultLabelAttribute);
      boolean doneRend = (ra==null) || (ra instanceof DefaultRenderAttribute);
      if(ra instanceof DefaultRenderAttribute){
        rendno = -2;
      }
      ListModel lm = qr.getModel();
      int p = 0;
      int size = lm.getSize();
      while(p<size && !(doneLab || doneRend)){
        Object o = lm.getElementAt(p);
        if(!doneLab){
          if(la.equals(o)){
             labno=p;
             doneLab=true;
          }   
        }
        if(!doneRend){
          if(ra.equals(o)){
             rendno = p; 
             doneRend=true;
          }
        }
        p++;
      }
     String lab = dg.getLabelText(CurrentState.ActualTime);
     if(lab==null){
        lab ="";
     }
     ListExpr currentCat = ListExpr.cons(
                              ListExpr.intAtom(catnr),
                              ListExpr.sixElemList(
                                 ListExpr.stringAtom(lab),
                                 ListExpr.realAtom(dg.getLabPosOffset().getX()),
                                 ListExpr.realAtom(dg.getLabPosOffset().getY()),
                                 ListExpr.boolAtom(dg.getVisible()),
                                 ListExpr.intAtom(labno),
                                 ListExpr.intAtom(rendno)
                              )
                             );
     if(catl.isEmpty()){
       catl=ListExpr.oneElemList(currentCat);
       left = catl;
     }else{
       left = ListExpr.append(left,currentCat);
     }
        
    }

    // create the list of layer-assignments
    ListExpr layerl = ListExpr.theEmptyList();
    left = layerl;
    for (int i = 0; i < qr.getGraphObjects().size(); i++) {
      DsplGraph dg = (DsplGraph)qr.getGraphObjects().elementAt(i);
      //int layernr = parent.GraphDisplay.getLayer(dg.getLayer());
      int layernr = JLayeredPane.getLayer(dg.getLayer());
      int layerpos = dg.getLayer().getGeoObjects().indexOf(dg);
      ListExpr SingleLayerList = ListExpr.twoElemList(
                      ListExpr.intAtom(layernr),
                      ListExpr.intAtom(layerpos));

      if (layerl.isEmpty()) {
         layerl = ListExpr.oneElemList(SingleLayerList);
         left = layerl;
      } 
      else {
        left = ListExpr.append(left, SingleLayerList);
      }
    }
    return  ListExpr.fourElemList(ListExpr.textAtom(qr.command), qr.LEResult, 
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

  /** Method supporting the readAllQueryResults method **/
  private void assignCatsAndLayersOldVersion(Vector Layers, QueryResult qr, ListExpr catList, ListExpr layerList){
     Iterator li = qr.getGraphObjects().iterator();
     while (li.hasNext()) {
        DsplGraph dg = (DsplGraph)li.next();
        setGOtoLayerPos(Layers, 
                        dg, 
                        catList.first().intValue(), 
                        layerList.first().intValue(), 
                        layerList.second().intValue());
        catList = catList.rest();
        String label = catList.first().stringValue();
        if(label.equals("")){
          dg.setLabelAttribute(null);
        }
        dg.setLabelAttribute(new DefaultLabelAttribute(label));
				catList = catList.rest();
	      dg.getLabPosOffset().setLocation(catList.first().realValue(), catList.second().realValue());
        catList = catList.rest().rest();
        dg.setVisible(catList.first().boolValue());
        catList = catList.rest();
        layerList = layerList.rest().rest();
		}
  }


  /** Method supporting the readAllQueryResult method.
    **/
  private void assignCatsAndLayers(Vector Layers, QueryResult qr,
                                   ListExpr catList, ListExpr layerList){

     Vector GraphObjects = qr.getGraphObjects();
     if(catList.atomType()!=ListExpr.NO_ATOM){ // wromg format
        return;
     }
     if(layerList.atomType()!=ListExpr.NO_ATOM){ // wrong format
        return;
     }
     int size = GraphObjects.size();
     // method stops if an error occurs
     ListModel lm = qr.getModel();
     int catno;
     String label;
     double xOffset;
     double yOffset;
     boolean visible;
     int la_no;
     int ra_no;
     int layerno;
     int layerpos;
     for(int i=0;i<size ;i++){
        if(catList.isEmpty() || layerList.isEmpty()){
          System.err.println("empty lists found ");
          return;
        } 
        ListExpr cat = catList.first();
        ListExpr aLayer = layerList.first();
        catList=catList.rest();
        layerList = layerList.rest();
        if(cat.listLength()!=7 || aLayer.listLength()!=2){
           System.err.println("invalid listlength found ");
           return;
        }
        if(cat.first().atomType() != ListExpr.INT_ATOM ||
           cat.second().atomType() != ListExpr.STRING_ATOM ||
           cat.third().atomType() != ListExpr.REAL_ATOM ||
           cat.fourth().atomType() !=ListExpr.REAL_ATOM || 
           cat.fifth().atomType() !=ListExpr.BOOL_ATOM ||
           cat.sixth().atomType() != ListExpr.INT_ATOM ||
           cat.rest().sixth().atomType() != ListExpr.INT_ATOM){
           System.err.println("invalid list structure for cat");
           return;
        } 
        if(aLayer.first().atomType()!=ListExpr.INT_ATOM ||
           aLayer.second().atomType()!=ListExpr.INT_ATOM){
           System.err.println("invalid list structure for layer");
           return;
        }       
        catno = cat.first().intValue();
        label = cat.second().stringValue();
        xOffset = cat.third().realValue();
        yOffset = cat.fourth().realValue();
        visible = cat.fifth().boolValue();
        la_no = cat.sixth().intValue();
        ra_no = cat.rest().sixth().intValue();
        layerno = aLayer.first().intValue();
        layerpos = aLayer.second().intValue();
        DsplGraph dg = (DsplGraph) GraphObjects.get(i);
        setGOtoLayerPos(Layers, dg, catno, layerno, layerpos);
        dg.setVisible(visible);
        dg.getLabPosOffset().setLocation(xOffset,yOffset);
        // compute the label attribute
        if(la_no<0){
          if(label.equals("")){
             dg.setLabelAttribute(null);
          }else{
             dg.setLabelAttribute(new DefaultLabelAttribute(label));
          }
        } else{ // get the attribute
           Object o = lm.getElementAt(la_no);
           if(o==null ||  !(o instanceof LabelAttribute)) {
              dg.setLabelAttribute(null);
           } else{
              dg.setLabelAttribute((LabelAttribute)o);
           }
        }
        // compute the rendering attribute
        if(ra_no<0){
          if(ra_no==-2){
            dg.setRenderAttribute(new DefaultRenderAttribute(i));
          } else{
            dg.setRenderAttribute(null);
          }
        }else{
            Object o = lm.getElementAt(ra_no);
            if(o==null || !(o instanceof RenderAttribute)){
               dg.setRenderAttribute(null);
            } else{
               dg.setRenderAttribute((RenderAttribute) o);
            }
        }
     } 
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


    le = le.second(); // switch to single query results
    ListExpr Current;
    while (!le.isEmpty()) {
      //Query lesen
      Current = le.first();
      if(Current.listLength()!=4){ // wrong format for query result
         return false;
      }
      String qrName=null;
      ListExpr nameList = Current.first();
      int at = nameList.atomType();
      switch(at){
        case ListExpr.STRING_ATOM: qrName = nameList.stringValue(); 
                                   break;
        case ListExpr.TEXT_ATOM: qrName = nameList.textValue();
                                 break;
        default: return false; // wrong format for name
      }
      // create the corresponding queryresult

      QueryResult qr = new QueryResult(qrName, Current.second());



      // ensure to take the background from the textarea for this new object
      qr.setOpaque(this.isOpaque());
      qr.setBackground(this.getBackground());

      if (parent.addQueryResult(qr)) { // successful adding this result
        ListExpr CatList = Current.third();
        ListExpr LayerList = Current.fourth();
        if(CatList.atomType()==ListExpr.NO_ATOM && !CatList.isEmpty()){
            at = CatList.first().atomType();
            if(at==ListExpr.NO_ATOM){
               assignCatsAndLayers(Layers, qr, CatList,LayerList);
            } else{
               assignCatsAndLayersOldVersion(Layers, qr,CatList,LayerList);
            }
        } // non empty catlist
      } else{ // query result successful added 
        System.out.println("can't add the queryresult");
      }
      le = le.rest(); // switch to the next qr in the list
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




