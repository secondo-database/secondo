package  viewer.hoese;

import  java.util.*;
import  javax.swing.*;
import  sj.lang.ListExpr;
import  java.awt.*;
import  javax.swing.border.*;
import  java.awt.geom.*;
import  java.io.*;
import  java.awt.image.*;
import  java.awt.event.*;
import  viewer.HoeseViewer;
import  viewer.MessageBox;
import gui.Environment;

/**
 * The objects in a query result had no category in the beginning. This dialog offers the
 * ability to associate single raphical objects or whole quries. A label attribute can be
 * selected. An attribute can be chosen for attribute dependant rendering.The top is made
 * up by the InfoPanel, followed by the Ref or NavPanel (reference depending mode or single
 * tupel mode.The buttonPanel is the bottommost region in this dialog.
 * @author  hoese
 * @version
 */
public class ViewConfig extends javax.swing.JDialog {
  /** Application's main window */
  HoeseViewer mw;
  /** Two lists storing possible reference-attr. and label-attr. */
  Vector RefAList, LabelAList;
  /** The actual query result  */
  QueryResult Query;
  /** The no. of attributes in a tupel */
  int AttrCount = 0;
  /** The no. of tupel in the query */
  int TupelCount = 1;
  /** The actual tupel in single tupel modus */
  int AktTupNr = 1;
  /** The four JPanels the dialog is devided in.*/
  JPanel InfoPanel, NavPanel, RefPanel, ButPanel;
  /** actual graph. attribute-name */
  String AttrName;
  /**  /** The actual graph. object in single tupel modus */
  DsplGraph AktGO;

  ListExpr AttrValues;
  static LinkAttrCat LAC=null;

  /** Creates new JDialog ViewConfig with the attribute-name an
   */
  public ViewConfig (HoeseViewer parent, String an) {
    super(parent.getMainFrame(), true);
    setTitle("Set Representation of GraphObjects");
    mw = parent;
    if(LAC==null)
        LAC = new LinkAttrCat(mw);
    AttrName = an;
    JComboBox cb = mw.TextDisplay.getQueryCombo();
    Query = (QueryResult)cb.getSelectedItem();
    RefAList = getRefAttrList();
    initComponents();
    pack();
    setSize(500,500);
    setResizable(true);
  }

  /**
   * Searches the attributes for possible reference attributes (int,real,string,bool)
   * @return A Vector with attributes been found
   */
  private Vector getRefAttrList () {
    Vector v = new Vector(5, 1);
    LabelAList = new Vector(10, 5);
    ListExpr TypeList = Query.LEResult;
    while(TypeList.atomType()==ListExpr.NO_ATOM && !TypeList.isEmpty())
       TypeList = TypeList.first();
    String MainType = TypeList.symbolValue();

    if(MainType.equals("rel") ){
	v.add("Tupel-No.");
	LabelAList.add("no Label");
	TupelCount = Query.LEResult.second().listLength();
	ListExpr attrlist = Query.LEResult.first().second().second();
	while (!attrlist.isEmpty()) {
	  String type = attrlist.first().second().symbolValue();
	  LabelAList.add(attrlist.first().first().symbolValue());
	  if ((type.equals("int")) || (type.equals("real")) || (type.equals("string"))
	      || (type.equals("bool")))
	     v.add(attrlist.first().first().symbolValue());
	  AttrCount++;
	  attrlist = attrlist.rest();
	}
    }

    if(MainType.equals("nmap")){
       //v.add("Tupel-No.");
       TupelCount = 0;
       LabelAList.add("no Label");
       /* 
         A nautical map consist of a name a scale and a maximum of three relations.
         The realations contains a name and a graphica object describing a point, a line
         or a region value. For this reason, the Reference-Attribute as well as the
         possible Label is 'name'.
       */
       LabelAList.add("name"); 
       v.add("name");
    }

    return  v;
  }

  /** This method is called from within the constructor to
   * initialize the dialog.
   */
  private void initComponents () {              //GEN-BEGIN:initComponents
    GALabel = new javax.swing.JLabel();
    VCLabel = new javax.swing.JLabel();
    CatCB = new javax.swing.JComboBox(mw.Cats);
    RefDepCBo = new javax.swing.JCheckBox();
    RALabel = new javax.swing.JLabel();
    RTLabel = new javax.swing.JLabel();
    RefAttrCB = new javax.swing.JComboBox(RefAList);
    RendTypeCB = new javax.swing.JComboBox();
    NGLabel = new javax.swing.JLabel();
    NoSlider = new javax.swing.JSlider();
    NoText = new javax.swing.JTextField();
    SingleTupelCBo = new javax.swing.JCheckBox();
    NeTuB = new javax.swing.JButton();
    PrTuB = new javax.swing.JButton();
    GATuB = new javax.swing.JButton();
    ApplyTuB = new javax.swing.JButton();
    OKB = new javax.swing.JButton();
    CancelB = new javax.swing.JButton();
    DefaultB = new javax.swing.JButton();
    LALabel = new javax.swing.JLabel();
    LTLabel = new javax.swing.JLabel();
    LabelText = new JTextField();
    LabXOffText = new JTextField(6);
    LabYOffText = new JTextField(6);
    LabelAttrCB = new javax.swing.JComboBox(LabelAList);
    LabelAttrCB.setSelectedIndex(-1);
//    RendPosCB = new javax.swing.JComboBox();
    getContentPane().setLayout(new BoxLayout(getContentPane(), BoxLayout.Y_AXIS));
    InfoPanel = new JPanel();
    InfoPanel.setBorder(LineBorder.createGrayLineBorder());
    InfoPanel.setPreferredSize(new Dimension(400, 130));
    NavPanel = new JPanel();
    NavPanel.setBorder(LineBorder.createGrayLineBorder());
    NavPanel.setPreferredSize(new Dimension(300, 100));
    RefPanel = new JPanel();
    //RefPanel.setBorder(LineBorder.createGrayLineBorder());
    RefPanel.setVisible(false);
    //RefPanel.setPreferredSize(new Dimension(300, 100));
    GALabel.setText("Attribute: ");
    InfoPanel.setLayout(new GridLayout(4,1));
    JPanel FirstRow = new JPanel();
    FirstRow.add(GALabel);
    GAAttrName = new JLabel(AttrName);
    FirstRow.add(GAAttrName);
    FirstRow.add(Box.createHorizontalStrut(30));
    VCLabel.setText("View Category:");
    FirstRow.add(VCLabel);
    JButton ShowCEB = new JButton("..");
    ShowCEB.setPreferredSize(new Dimension(15, 20));
    ShowCEB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        new CategoryEditor(mw, true).show();
        mw.GraphDisplay.repaint();
        CatCB.setSelectedIndex(CatCB.getItemCount() - 1);
        //			((DsplBase)o).getFrame().select(o);
      }
    });
    FirstRow.add(CatCB);
    FirstRow.add(ShowCEB);
    InfoPanel.add(FirstRow);

    JPanel SecondRow = new JPanel();
    LTLabel.setText("Label Text:");
    SecondRow.add(LTLabel);
    LabelText.setColumns(10);
    SecondRow.add(LabelText);
    LALabel.setText("Labelattribute");
    SecondRow.add(LALabel);
    SecondRow.add(LabelAttrCB);
    InfoPanel.add(SecondRow);

    JPanel ThirdRow = new JPanel();
    ThirdRow.add(new JLabel("Label Offset X Y"));
    ThirdRow.add(LabXOffText);
    ThirdRow.add(LabYOffText);
    InfoPanel.add(ThirdRow);

    JPanel FourthRow = new JPanel();
    RefDepCBo.setText("Reference dependent Rendering");
    RefDepCBo.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        RefDepCBoActionPerformed(evt);
      }
    });
    FourthRow.add(RefDepCBo);



    // we need a Panel to Select, change,add and remove manual links
    LinkComboBox = new JComboBox();

    JPanel MLPanel2 = new JPanel();
    MLChangeBtn = new JButton("Change");
    MLNewBtn = new JButton("New");
    MLRemoveBtn = new JButton("Remove");
    MLPanel2.add(MLChangeBtn);
    MLPanel2.add(MLNewBtn);
    MLPanel2.add(MLRemoveBtn);


    MLNewBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          // create a new AttrCatList from given values
	  Category SelectedCat = (Category) CatCB.getSelectedItem();
	  if (SelectedCat==null){
	     MessageBox.showMessage("no Category selected");
	     return;
	  }
	  String CatName = SelectedCat.getName();
	  String as = (String)RefAttrCB.getSelectedItem();
	  int index = LabelAList.indexOf(as);
	  AttrValues = getAttrValues(index);
	  if(AttrValues==null){
	     MessageBox.showMessage("no reference attribute selected");
	     return;
	  }
	  LAC.setLinks(new AttrCatList());
	  LAC.setDefaultCat(SelectedCat);
	  LAC.setAttributes(AttrValues);
	  LAC.setCategories(mw.Cats);
	  LAC.setName("");
          LAC.setVisible(true);
	  if(true) { // later depending from result from LAC
             String Name = LAC.getName();
             LinkComboBox.addItem(Name);
	     LinkComboBox.setSelectedItem(Name);
	     AttrCatList ACL = LAC.getLinks();
	     if(!ManualLinkPool.add(ACL)){
	       // should never be reached
	       MessageBox.showMessage("i can't insert this new references to reference pool");
	     }
	  }
       }});


    MLChangeBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          // create a new AttrCatList from given values
	  Category SelectedCat = (Category) CatCB.getSelectedItem();
	  if (SelectedCat==null){
	     MessageBox.showMessage("no Category selected");
	     return;
	  }
	  String CatName = SelectedCat.getName();
	  String as = (String)RefAttrCB.getSelectedItem();
	  int index = LabelAList.indexOf(as);
	  AttrValues = getAttrValues(index);
	  if(AttrValues==null){
	     MessageBox.showMessage("no reference attribute selected");
	     return;
	  }
	  String RefName = (String) LinkComboBox.getSelectedItem();
	  if(RefName==null){
	    MessageBox.showMessage("no Referenceclass selected");
	    return;
	  }

	  AttrCatList ACL = ManualLinkPool.getLinkWithName(RefName);
	  if(ACL==null){
	     MessageBox.showMessage("i can't find the selected Referenceclass");
	     return;
	  }

	  AttrCatList ACLClone = (AttrCatList) ACL.clone();
          LAC.setLinks(ACLClone);
	  LAC.addAttributes(AttrValues,SelectedCat);
	  LAC.setCategories(mw.Cats);
	  LAC.setUpdateMode(true,RefName);
          LAC.setVisible(true);
	  LAC.setUpdateMode(false,"");
	  if(LAC.getRetValue()==LAC.OK) {
             String Name = LAC.getName();
             LinkComboBox.removeItem(RefName);
	     LinkComboBox.addItem(Name);
	     LinkComboBox.setSelectedItem(Name);
	     ACL = LAC.getLinks();
	     if(!ManualLinkPool.update(RefName,ACL)){
	       // should never be reached
	       MessageBox.showMessage("i can't update the references in reference pool");
	     }
	  }
       }});


    MLRemoveBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent e){
         String Name = (String) LinkComboBox.getSelectedItem();
	 if(Name==null){
	   MessageBox.showMessage("no reference selected ");
	   return;
	 }
	 LinkComboBox.removeItem(Name);
	 ManualLinkPool.removeLinkWithName(Name);
       }});

    InfoPanel.add(FourthRow);


    SingleTupelCBo.setText("Single Tuple");
    SingleTupelCBo.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        NeTuB.setEnabled(SingleTupelCBo.isSelected());
        PrTuB.setEnabled(SingleTupelCBo.isSelected());
        ApplyTuB.setEnabled(SingleTupelCBo.isSelected());
        GATuB.setEnabled(SingleTupelCBo.isSelected());
        RefDepCBo.setEnabled(!SingleTupelCBo.isSelected());
        if (SingleTupelCBo.isSelected()) {
          updateFields();
        }
        else {
          TupNrLabel.setText("Tuples: " + TupelCount);
          LabelText.setText(null);
          LabXOffText.setText(null);
          LabYOffText.setText(null);
        }
      }
    });

    JPanel NavRow1 = new JPanel();
    NavRow1.add(SingleTupelCBo);
    NeTuB.setText("Next Tuple");
    NeTuB.setEnabled(false);
    NeTuB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        NeTuBActionPerformed(evt);
      }
    });
    PrTuB.setText("Prev Tuple");
    PrTuB.setEnabled(false);
    PrTuB.addActionListener(new java.awt.event.ActionListener() {

      public void actionPerformed (java.awt.event.ActionEvent evt) {
        PrTuBActionPerformed(evt);
      }
    });
    NavRow1.add(PrTuB);
    NavPanel.setLayout(new GridLayout(4,1));
    NavRow1.add(NeTuB);

    NavPanel.add(NavRow1);

    JPanel NavRow2 = new JPanel();
    NavRow2.add(Box.createHorizontalStrut(60));
    TupNrLabel = new JLabel("Tuples: " + TupelCount);
    NavRow2.add(TupNrLabel);
    ApplyTuB.setText("Apply Tuple");
    ApplyTuB.setEnabled(false);
    ApplyTuB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        Category cat = (Category)CatCB.getSelectedItem();
        int LabIndex = LabelAttrCB.getSelectedIndex();
        AktGO.setCategory(cat);
        if ((LabXOffText.getText().equals("")) || (LabYOffText.getText().equals("")))
          ;
        else
          AktGO.getLabPosOffset().setLocation(Double.parseDouble(LabXOffText.getText()),
              Double.parseDouble(LabYOffText.getText()));
        if (LabelText.getText().equals(""))
          AktGO.setLabelText(null);
        else
          AktGO.setLabelText(LabelText.getText());
      }
    });
    NavRow2.add(ApplyTuB);
    NavPanel.add(NavRow2);
    GATuB.setText("Get Attribute");
    GATuB.setEnabled(false);
    GATuB.addActionListener(new java.awt.event.ActionListener() {

      public void actionPerformed (java.awt.event.ActionEvent evt) {
        int LabIndex = LabelAttrCB.getSelectedIndex();
        if (LabIndex >= 0) {
          String text = Query.getModel().getElementAt((AktTupNr - 1)*(AttrCount
              + 1) + LabIndex).toString();
          LabelText.setText(text.substring(text.indexOf(":") + 1));
        }
      }
    });
    JPanel NavRow3 = new JPanel();
    NavRow3.add(GATuB);
    NavPanel.add(NavRow3);
    // RefDepCBo.setText ("jCheckBox1");
    RALabel.setText("Reference attribute");



    JPanel P = new JPanel();
    P.setLayout(new GridLayout(5,2));

    //P.add(Box.createHorizontalStrut(40));
    RTLabel.setText("Rendering Type");
    NoText.setColumns(4);
    NoText.setText("16");

    //P.add(Box.createHorizontalStrut(60));
    RendTypeCB.addItem(new String("Solid Gray Values"));
    RendTypeCB.addItem(new String("Solid Red Values"));
    RendTypeCB.addItem(new String("Solid Green Values"));
    RendTypeCB.addItem(new String("Solid Blue Values"));
    RendTypeCB.addItem(new String("Point Size <32Px"));
    RendTypeCB.addItem(new String("Standard Palette"));
    searchForCalcCats();
    searchForImageDirs();
    //P.add(Box.createHorizontalStrut(60));
    NGLabel.setText("No. of different groups");
    LinkCheckBox = new JCheckBox("manual link");
    LinkCheckBox.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          boolean s = LinkCheckBox.isSelected();
          if(s){
            LinkComboBox.setEnabled(true);
            MLChangeBtn.setEnabled(true);
	    MLNewBtn.setEnabled(true);
	    MLRemoveBtn.setEnabled(true);
            RendTypeCB.setEnabled(false);
            NoText.setEnabled(false);

          } else{
            LinkComboBox.setEnabled(false);
            MLChangeBtn.setEnabled(false);
	    MLNewBtn.setEnabled(false);
	    MLRemoveBtn.setEnabled(false);
            RendTypeCB.setEnabled(true);
            NoText.setEnabled(true);
          }
       }
    });

    LinkCheckBox.setSelected(false);
    LinkComboBox.setEnabled(false);
    MLChangeBtn.setEnabled(false);
    MLNewBtn.setEnabled(false);
    MLRemoveBtn.setEnabled(false);



    P.add(RALabel);
    P.add(RefAttrCB);

    P.add(RTLabel);
    P.add(RendTypeCB);

    P.add(NGLabel);
    P.add(NoText);

    P.add(LinkCheckBox);
    P.add(LinkComboBox);
    P.add(new JLabel("")); // a dummy
    P.add(MLPanel2);

    KeyListener RKL = new KeyAdapter(){
            public void keyPressed(KeyEvent evt){
                if(evt.getKeyCode()==evt.VK_ENTER){
                   if(evt.getSource().equals(CancelB)){
                      setVisible(false);
                      dispose();
		   }
		   else
                      OKBActionPerformed();
                }
                if(evt.getKeyCode()==evt.VK_ESCAPE){
                      setVisible(false);
                      dispose();
                }
            }
    };
    CatCB.addKeyListener(RKL);
    RefDepCBo.addKeyListener(RKL);
    RefAttrCB.addKeyListener(RKL);
    RendTypeCB.addKeyListener(RKL);
    LabelAttrCB.addKeyListener(RKL);
    NoSlider.addKeyListener(RKL);
    NoText.addKeyListener(RKL);
    SingleTupelCBo.addKeyListener(RKL);
    NeTuB.addKeyListener(RKL);
    PrTuB.addKeyListener(RKL);
    GATuB.addKeyListener(RKL);
    ApplyTuB.addKeyListener(RKL);
    OKB.addKeyListener(RKL);
    CancelB.addKeyListener(RKL);
    DefaultB.addKeyListener(RKL);
    LabelText.addKeyListener(RKL);
    LabXOffText.addKeyListener(RKL);
    LabYOffText.addKeyListener(RKL);


    OKB.setText("OK");
    OKB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        OKBActionPerformed();
      }
    });
    RefPanel.add(P);

    ButPanel = new JPanel();
    ButPanel.add(OKB);
    CancelB.setText("Cancel");
    CancelB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        setVisible(false);
        dispose();
      }
    });
    ButPanel.add(CancelB);
    getContentPane().add(InfoPanel);
    getContentPane().add(NavPanel);
    getContentPane().add(RefPanel);
    getContentPane().add(ButPanel);
  }             //GEN-END:initComponents

  /**
   * In single-tuple mode some fields need to be refreshed, when stepping through tuples
   */
  private void updateFields () {
    TupNrLabel.setText("Tuple " + AktTupNr + "/" + TupelCount);
    //int LabIndex=LabelAttrCB.getSelectedIndex();
    ListIterator li = Query.getGraphObjects().listIterator();
    int cnt = 0;
    DsplGraph dg = null;
    while (cnt < AktTupNr) {
      dg = (DsplGraph)li.next();
      if (dg.getAttrName().equals(AttrName))
        cnt++;
    }
    AktGO = dg;
    CatCB.setSelectedItem(AktGO.getCategory());
    LabXOffText.setText(Double.toString(AktGO.getLabPosOffset().getX()));
    LabYOffText.setText(Double.toString(AktGO.getLabPosOffset().getY()));
    LabelText.setText(AktGO.getLabelText());
  }


  /** read a AttrCatLinks from pool and insert it in
    * the appropriate ComboBox
    */
  public void readPool(){
     LinkComboBox.removeAllItems();
     for(int i=0;i<ManualLinkPool.numberOfLinks();i++)
           LinkComboBox.addItem(ManualLinkPool.get(i).getName());
  }


  /**
   * When 'OK' is pushed this method checks
   * 1. Not in single tuple mode:
   *    labeltext, labelpos, category is associated to all tupels of the graphic attribute
   * 2. Reference dependant rendering?
   *    calculates a category for each value of the ref. attr. ,based upon the selected
   *    category and rendering type.
   * @param evt
   */
  private void OKBActionPerformed () {          
    if (!SingleTupelCBo.isSelected()) {
      Category cat = (Category)CatCB.getSelectedItem();
      int LabIndex = LabelAttrCB.getSelectedIndex();
      ListIterator li = Query.getGraphObjects().listIterator();
      int cnt = 0;
      while (li.hasNext()) {
        DsplGraph dg = (DsplGraph)li.next();
        if (dg.getAttrName().equals(AttrName)) {
          dg.setCategory(cat);
          if ((LabXOffText.getText().equals("")) || (LabYOffText.getText().equals("")))
            ;
          else
            dg.getLabPosOffset().setLocation(Double.parseDouble(LabXOffText.getText()),
                Double.parseDouble(LabYOffText.getText()));
          if ((LabIndex >= 0) && (LabelText.getText().equals(""))) {
            String text;
            if(LabIndex==0)  // no Label
               text ="";
            else {
               text = Query.getModel().getElementAt(cnt*(AttrCount + 1)
                + LabIndex-1).toString();
               text = text.substring(text.indexOf(":") + 1);
            }
            dg.setLabelText(text);
          }
          else if (LabelText.getText().equals(""))
            dg.setLabelText(null);
          else
            dg.setLabelText(LabelText.getText());
          cnt++;
        }
      }
    }

    String as = (String)RefAttrCB.getSelectedItem();

    if (RefDepCBo.isSelected()) {
      if(!LinkCheckBox.isSelected()){
         int RefAttrIndex = LabelAList.indexOf(as)-1;
         //int RendTypeIndex=RendTypeCB.getSelectedIndex());
         Point2D.Double p = calcMinMax(RefAttrIndex);
         double min = p.getX();
         double max = p.getY();
         //create number of groups cats
         int nr = 8;
         try {
           nr = Integer.parseInt(NoText.getText());
         } catch (NumberFormatException n) {
	   MessageBox.showMessage("Invalid number of groups");
	   return;
	 }
         if (max != min) {         // no cat need to be created if <=
           Category GroupCats[] = new Category[nr];
           for (int i = 0; i < nr;i++)
	      GroupCats[i] = calcCategory(i, nr, (Category)CatCB.getSelectedItem());

           ListExpr   tl = extractRelation();

           ListIterator li = Query.getGraphObjects().listIterator();
           nr -= 1;
           int val = 0;
           double value;
           //create new Cat for every GO with Attribute AttrName according to its ref. Tupel-Value
           while (li.hasNext()) {
             DsplGraph dg = (DsplGraph)li.next();
             if (dg.getAttrName().equals(AttrName)) {              //create Cat. only for recent GAttribute
               ListExpr le = tl.first();
               for (int i = 1; i <= RefAttrIndex; i++)
                 le = le.rest();
               if (RefAttrIndex < 0 )
                 value = (double)val;
               else
                 value = convertLEtoDouble(le.first());

               int chg = (int)(nr*((value - min)/(max - min)));
	       if(mw.Cats.indexOf(GroupCats[chg])<0){
                  mw.Cats.add(GroupCats[chg]);
	       }
               dg.setCategory(GroupCats[chg]);
               tl = tl.rest();
               val++;
             }
           }
         }
      } else{
         // take the categories for the object from LinkAttrCat LAC
	 if(LinkComboBox.getSelectedIndex()<0){ // no references selected
	    MessageBox.showMessage("you have selected to use a manual link \n but the choice for this link is empty !!");
	    return;
	 }

         ListIterator li = Query.getGraphObjects().listIterator();
         int RefAttrIndex = LabelAList.indexOf(as);
         ListExpr AttrValues = getAttrValues(RefAttrIndex);
	  if(AttrValues==null){
	     MessageBox.showMessage("no reference attribute selected");
	     return;
	  }
	 // get the Manual Link from MLPool
	 String LinkName = (String) LinkComboBox.getSelectedItem();
	 AttrCatList ACL = ManualLinkPool.getLinkWithName(LinkName);
	 if(ACL==null){
	    MessageBox.showMessage("internal error: selected reference name not found");
	    return;
	 }
	 boolean found =false;
	 int number = ACL.numberOfLinksFor(AttrValues);
	 if(number<1){
	    MessageBox.showMessage("the selected reference contains no links for selected attribute");
	    return;
	 }
	 if(number < AttrValues.listLength()){
	    if (MessageBox.showQuestion("not all attribute values have a refered category \n "+
	                                "you want to use the default category ?")==MessageBox.NO)

                  return;
	 }

         // check for categories in selected references
         Iterator it = ACL.getCatNames();
	 int numberNames=0;
	 int failed =0;
	 while(it.hasNext()){
	    String CN = (String) it.next();
            numberNames++;
	    found = false;
            for (int i=0;i<mw.Cats.size() & ! found ;i++){
               if(CN.equals(((Category)mw.Cats.get(i)).getName()))
	         found = true;
	    }
	    if(!found) failed++;
	 }
	 if(numberNames==failed && numberNames>0){
	    MessageBox.showMessage("no categorie in the selected references is loaded \n abort");
	    return;
	 }

	 if(failed>0){
	    if(MessageBox.showQuestion("not all categories in the seleced references are loaded \n"+
	                                "you want to use the default category ?")==MessageBox.NO)
		return;
	 }

	 String CatName;
	 Category Cat=null;
	 ListExpr res = extractRelation();
         while(li.hasNext()){
            DsplGraph dg = (DsplGraph)li.next();
            if(dg.getAttrName().equals(AttrName)){
               ListExpr le = res.first();
               for(int i=1;i<RefAttrIndex;i++)  // read over non interesting attribute values
                  le = le.rest();
               CatName = ACL.getCatName(le.first());
	       for(int i=0;i<mw.Cats.size();i++){
	          if(((Category)mw.Cats.get(i)).getName().equals(CatName))
	             Cat = (Category) mw.Cats.get(i);
	       }
	       if(Cat!=null)
                  dg.setCategory(Cat);
               res = res.rest();
            }
         }
      }

    }
    setVisible(false);
    dispose();
  }             //GEN-LAST:event_OKBActionPerformed

  /**create new Cat for every GO with Attribute AttrName according to its ref. Tupel-Value
   */
  private Category calcCategory (int chg, int nr, Category templCat) {
    Category NewCat = null;
    try {
      NewCat = (Category)templCat.clone();
    } catch (Exception e) {
       if(Environment.DEBUG_MODE){
          System.err.println("Error in cloning of category");
       }
       return null;
    }
    NewCat.setName(NewCat.getName() + "V" + VariantNr);
    VariantNr++;
    Object o = RendTypeCB.getSelectedItem();
    if (o instanceof CalcCategory)
      return  ((CalcCategory)o).calcCategory(chg, nr, templCat);

    String RendType = (String)o;
    if (RendType.equals("Solid Gray Values")) {
      int inc = 255/nr;
      int cv = chg*inc;
      NewCat.setFillStyle(new Color(cv, cv, cv));
    }
    if (RendType.equals("Solid Blue Values")) {
      int inc = 255/nr;
      int cv = chg*inc;
      NewCat.setFillStyle(new Color(cv, cv, 255));
    }
    if (RendType.equals("Solid Red Values")) {
      int inc = 255/nr;
      int cv =  chg*inc;
      NewCat.setFillStyle(new Color(255, cv, cv));
    }
    if (RendType.equals("Solid Green Values")) {
      int inc = 255/nr;
      int cv =  chg*inc;
      NewCat.setFillStyle(new Color(cv, 255, cv));
    }
    if (RendType.startsWith("Images in ")) {
      FileFilter ImageFilter = new FileFilter() {
        public boolean accept (File f) {
          String name = f.getName().toLowerCase();
          return  name.endsWith(".gif") || name.endsWith(".jpg");
        }
        public String getDescription () {
          return  "Hintergrundkarten (*.gif , *.jpg)";
        }       //end getDescription
      };        // end newFileFilter
      File f = new File("res/" + RendType.substring(10));
      File[] fileList = f.listFiles(ImageFilter);
      if (fileList.length != 0) {
        chg = chg%fileList.length;
        ImageIcon ii = new ImageIcon(fileList[chg].getPath());
        BufferedImage bi = new BufferedImage(ii.getIconWidth(), ii.getIconHeight(),
            BufferedImage.TYPE_INT_ARGB);
        Graphics2D big = bi.createGraphics();
        big.drawImage(ii.getImage(), 0, 0, null);
        Rectangle r = new Rectangle(0, 0, ii.getIconWidth(), ii.getIconHeight());
        //NewCat.setPointasRect(true);
        NewCat.setFillStyle(new TexturePaint(bi, r));
        NewCat.setIconName(fileList[chg].getName());
      }
    }
    if (RendType.equals("Point Size <32Px")) {
      double inc = 32.0/(double)nr;
      double cv = 4 + chg*inc;
      NewCat.setPointSize(cv);
    }
    if (RendType.equals("Standard Palette")) {
      int inc = 64/nr;
      int cv = chg*inc;
      int red = (cv & 48)/16*64;
      int green = (cv & 12)/4*64;
      int blue = (cv & 3)*64;
      NewCat.setFillStyle(new Color(red, green, blue));
    }
    return  NewCat;
  }

  /**
   * Searches for registered (Config-file)classes of new rendering types that calc. categories
   */
  private void searchForCalcCats () {
    StringTokenizer ccc = new StringTokenizer(HoeseViewer.configuration.getProperty("CalcCats"),
        ",");
    while (ccc.hasMoreTokens())
      try {
        RendTypeCB.addItem(Class.forName("viewer.hoese." + ccc.nextToken()).newInstance());
      } catch (Exception except) {
        System.out.println("CCClass not found");
      }
  }

  /**
   * Searches the images/ directory for subdirectories with images. They are listed as
   * rendering type 'Images in <directory>'
   * @see <a href="ViewConfigsrc.html#searchForImagedirs">Source</a>
   */
  private void searchForImageDirs () {
    FileFilter DirFilter = new FileFilter() {

      public boolean accept (File f) {
        return  f.isDirectory();
      }
      public String getDescription () {
        return  "Directories";
      }         //end getDescription
    };          // end newFileFilter
    File f = new File("res/");
    File[] fileList = f.listFiles(DirFilter);
    for (int i = 0; i < fileList.length; i++)
      RendTypeCB.addItem(new String("Images in " + fileList[i].getName()));
  }

  /**
   * Calculates the minimum and maximum of an attribute over all tuples
   * @param ind The ind's attribute in a tuple
   * @return Min. and max. as Point2D.double
   */
  private Point2D.Double calcMinMax (int ind) {
    if (ind < 0) {// tuplenumber
      return  new Point2D.Double(0, (double)(TupelCount - 1));
    }  
    ListExpr tl = extractRelation();
    double min = Double.MAX_VALUE;
    double max = Double.MIN_VALUE;
    while (!tl.isEmpty()) {
      ListExpr le = tl.first(); // get the first tuple
      for (int i = 0; i < ind; i++) // read over attributes
        le = le.rest();
      double w = convertLEtoDouble(le.first()); // get value for desired attribute
      if (min >  w)
        min = w;
      if (max < w)
        max = w;
      tl = tl.rest();
    }
    return  new Point2D.Double(min, max);
  }

   /**
     * extracts the relation from a nautical map or from a relation 
     */
   private ListExpr extractRelation(){
      ListExpr TypeList = Query.LEResult.first();
      while(TypeList.atomType()==ListExpr.NO_ATOM && !TypeList.isEmpty())
          TypeList=TypeList.first();
      if(TypeList.atomType()!=ListExpr.SYMBOL_ATOM)
         return Query.LEResult.second();
      
      String TypeName = TypeList.symbolValue();
      
      if(TypeName.equals("nmap")){
         ListExpr Relations=Query.LEResult.second().rest().rest(); // take the value and read over name and scale
         ListExpr value;
         if(AttrName.equals("object")) 
           value=Relations.first();
         else if(AttrName.equals("sline"))
           value=Relations.second();
         else if(AttrName.equals("area"))
           value=Relations.third();
         else{
            value = new ListExpr();
            System.err.println("Unknow AttrName for nmap detected");
         }   
	 TupelCount = value.listLength();
	 return value;
      } else{
         return Query.LEResult.second();
      }
   }



  /**
   * Makes a double-value out of a special atom-type
   * @param le
   * @return double value
   */
  private double convertLEtoDouble (ListExpr le) {
   if (le.atomType() == ListExpr.STRING_ATOM){
      return le.stringValue().hashCode();
    }  
    if (le.atomType() == ListExpr.BOOL_ATOM)
      if (le.boolValue())
        return  1;
      else
        return  0;
    if (le.atomType() == ListExpr.INT_ATOM)
      return  (double)le.intValue();
    if (le.atomType() == ListExpr.REAL_ATOM)
      return  (double)le.realValue();
    else
      return  0;
  }


  /* returns a clone of a ListExpr */
  private static  ListExpr CloneListExpr(ListExpr Original){

     int AtomType = Original.atomType();
     if(Original.isEmpty())
        return ListExpr.theEmptyList();

     switch(AtomType){
       case ListExpr.INT_ATOM : return ListExpr.intAtom(Original.intValue());
       case ListExpr.REAL_ATOM : return ListExpr.realAtom(Original.realValue());
       case ListExpr.BOOL_ATOM : return ListExpr.boolAtom(Original.boolValue());
       case ListExpr.STRING_ATOM : return ListExpr.stringAtom(Original.stringValue());
       case ListExpr.SYMBOL_ATOM : return ListExpr.symbolAtom(Original.symbolValue());
       case ListExpr.TEXT_ATOM : return ListExpr.textAtom(Original.textValue());
       case ListExpr.NO_ATOM : { if(Original.listLength()<1){
                            System.out.println("viewer.hoese.ViewConfig.CloneListExpr :: error in ListExpr");
                            return null;
                         }
                         ListExpr First,Last;
                         First = CloneListExpr(Original.first());
                         Last = First;
                         ListExpr Next;
                         ListExpr Orig2 = Original.rest();
                         while(!Orig2.isEmpty()){
                            Next = CloneListExpr(Orig2.first());
                            Last = ListExpr.append(Last,Next);
                            Orig2 = Orig2.rest();
                         }

                         return First;

                       }
     }

     System.out.println("viewer.hoese.ViewConfig.ClonListExpr : unknow AtomType");
     return null;
  }

  private ListExpr getAttrValues(int index){
    if(index < 0)
       return null;
    return getAttrValues(index,extractRelation());
  }

  /* get a List of Values for specific index */
  private ListExpr getAttrValues(int index,ListExpr relation){
      if(index < 0)
        return null;
      ListExpr AllValues = relation;
      ListExpr Values=null;
      ListExpr CurrentTuple;
      ListExpr CurrentAttr;
      ListExpr Last=null,Next;
      while(!AllValues.isEmpty()){
         CurrentTuple = AllValues.first();
         if(index>CurrentTuple.listLength()){
             System.err.println("The selected Reference is outside the tuple");
             return null;
         }
         // search the attribute
         for(int i=1; i<index;i++)
            CurrentTuple = CurrentTuple.rest();
         CurrentAttr =  CurrentTuple.first();
         if(Values==null){
            Values = ListExpr.oneElemList(CloneListExpr(CurrentAttr));
            Last = Values;
         }else{
             Next = CloneListExpr(CurrentAttr);
             Last = ListExpr.append(Last,Next);
         }
         AllValues = AllValues.rest();
      }
      if(Values==null)
         Values = ListExpr.theEmptyList();
      return Values;
  }



  /**
   * Moves to the previous or the last tuple
   * @param evt
   */
  private void PrTuBActionPerformed (java.awt.event.ActionEvent evt) {    
    AktTupNr = (AktTupNr > 1) ? AktTupNr - 1 : TupelCount;
    updateFields();
  }            


  /**
   * Moves to the next or first tuple.
   * @param evt
   */
  private void NeTuBActionPerformed (java.awt.event.ActionEvent evt) {          //GEN-FIRST:event_NeTuBActionPerformed
    AktTupNr = (AktTupNr < TupelCount) ? AktTupNr + 1 : 1;
    updateFields();
  }             //GEN-LAST:event_NeTuBActionPerformed

  /**
   * This method is called after selecting ref. dep. rend.
   * @param evt
   * @see <a href="ViewConfigsrc.html#RefDepCBoActionPerformed">Source</a>
   */
  private void RefDepCBoActionPerformed (java.awt.event.ActionEvent evt) {      //GEN-FIRST:event_RefDepCBoActionPerformed
    // Add your handling code here:
    RefPanel.setVisible(RefDepCBo.isSelected());
    NavPanel.setVisible(!RefDepCBo.isSelected());
  }             //GEN-LAST:event_RefDepCBoActionPerformed



  /** check for new categories in HoeseViewer an show them */
  public void setVisible(boolean on){
    if(on){ // check for new Categorys
      if(CatCB.getItemCount()!=mw.Cats.size()){ // there are new Categories
        int index = CatCB.getSelectedIndex();
        CatCB.removeAllItems();
        for(int i=0;i<mw.Cats.size();i++)
           CatCB.addItem( (Category) mw.Cats.get(i));
        if(index<mw.Cats.size())
          CatCB.setSelectedIndex(index);
      }

    }
    super.setVisible(on);
  }




  /** Closes the dialog
   */
  private void closeDialog (java.awt.event.WindowEvent evt) {                   //GEN-FIRST:event_closeDialog
    setVisible(false);
    dispose();
  }             //GEN-LAST:event_closeDialog
  //  }
  // Variables declaration - do not modify//GEN-BEGIN:variables
  private JLabel GALabel;
  private JLabel VCLabel;
  private JComboBox CatCB;
  private JCheckBox RefDepCBo;
  private JLabel RALabel;
  private JLabel RTLabel;
  private JComboBox RefAttrCB;
  private JComboBox RendTypeCB;
  private JLabel NGLabel;
  private JSlider NoSlider;
  private JTextField NoText;
  private JTextField LabelText;
  private JTextField LabXOffText;
  private JTextField LabYOffText;
  private JCheckBox SingleTupelCBo;
  private JButton NeTuB;
  private JButton PrTuB;
  private JButton GATuB;
  private JButton ApplyTuB;
  private JButton OKB;
  private JButton CancelB;
  private JButton DefaultB;
  private JLabel LALabel;
  private JLabel LTLabel;
  private JComboBox LabelAttrCB;
  private JLabel GAAttrName, TupNrLabel;
  private JButton MLChangeBtn;
  private JButton MLNewBtn;
  private JButton MLRemoveBtn;
  private JCheckBox LinkCheckBox;
  private JComboBox LinkComboBox;

  private static int VariantNr = 0;
  // End of variables declaration//GEN-END:variables
}

