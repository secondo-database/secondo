

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

  /** Creates new JDialog ViewConfig with the attribute-name an
   */
  public ViewConfig (HoeseViewer parent, String an) {
    super(parent.getMainFrame(), true);
    setTitle("Set Representation of GraphObjects");
    mw = parent;
    AttrName = an;
    JComboBox cb = mw.TextDisplay.getQueryCombo();
    Query = (QueryResult)cb.getSelectedItem();
    RefAList = getRefAttrList();
    initComponents();
    pack();
    setResizable(false);
  }

  /**
   * Searches the attributes for possible reference attributes (int,real,string,bool)
   * @return A Vector with aatrributes been found
   */
  private Vector getRefAttrList () {
    Vector v = new Vector(5, 1);
    LabelAList = new Vector(10, 5);
    if (Query.LEResult.first().isAtom())
      return  v;
    v.add("Tupel-No.");
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
    RendPosCB = new javax.swing.JComboBox();
    getContentPane().setLayout(new BoxLayout(getContentPane(), BoxLayout.Y_AXIS));
    //	setSize(300,300);
    InfoPanel = new JPanel();
    InfoPanel.setBorder(LineBorder.createGrayLineBorder());
    InfoPanel.setPreferredSize(new Dimension(400, 130));
    NavPanel = new JPanel();
    NavPanel.setBorder(LineBorder.createGrayLineBorder());
    NavPanel.setPreferredSize(new Dimension(300, 100));
    RefPanel = new JPanel();
    RefPanel.setBorder(LineBorder.createGrayLineBorder());
    RefPanel.setVisible(false);
    RefPanel.setPreferredSize(new Dimension(300, 100));
    GALabel.setText("Attribute: ");
    InfoPanel.add(GALabel);
    GAAttrName = new JLabel(AttrName);
    InfoPanel.add(GAAttrName);
    InfoPanel.add(Box.createHorizontalStrut(30));
    VCLabel.setText("View Category:");
    InfoPanel.add(VCLabel);
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
    InfoPanel.add(CatCB);
    InfoPanel.add(ShowCEB);
    LTLabel.setText("Label Text:");
    InfoPanel.add(LTLabel);
    LabelText.setColumns(10);
    InfoPanel.add(LabelText);
    LALabel.setText("Labelattribute");
    InfoPanel.add(LALabel);
    InfoPanel.add(LabelAttrCB);
    InfoPanel.add(new JLabel("Label Offset X Y"));
    InfoPanel.add(LabXOffText);
    InfoPanel.add(LabYOffText);
    RefDepCBo.setText("Reference dependant Rendering");
    RefDepCBo.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        RefDepCBoActionPerformed(evt);
      }
    });
    InfoPanel.add(RefDepCBo);
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
    NavPanel.add(SingleTupelCBo);
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
    NavPanel.add(PrTuB);
    NavPanel.add(NeTuB);
    NavPanel.add(Box.createHorizontalStrut(60));
    TupNrLabel = new JLabel("Tuples: " + TupelCount);
    NavPanel.add(TupNrLabel);
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
    NavPanel.add(ApplyTuB);
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
    NavPanel.add(GATuB);
    // RefDepCBo.setText ("jCheckBox1");
    RALabel.setText("Reference attribute");
    RefPanel.add(RALabel);
    RefPanel.add(Box.createHorizontalStrut(40));
    RTLabel.setText("Rendering Type");
    RefPanel.add(RTLabel);
    RefPanel.add(Box.createHorizontalStrut(60));
    RefPanel.add(RefAttrCB);
    RendTypeCB.addItem(new String("Solid Gray Values"));
    RendTypeCB.addItem(new String("Solid Red Values"));
    RendTypeCB.addItem(new String("Solid Green Values"));
    RendTypeCB.addItem(new String("Solid Blue Values"));
    RendTypeCB.addItem(new String("Point Size <32Px"));
    RendTypeCB.addItem(new String("Standard Palette"));
    searchForCalcCats();
    searchForImageDirs();
    RefPanel.add(RendTypeCB);
    RefPanel.add(Box.createHorizontalStrut(60));
    NGLabel.setText("No. of different groups");
    RefPanel.add(NGLabel);
    NoText.setColumns(4);
    NoText.setText("16");
    RefPanel.add(NoText);
    OKB.setText("OK");
    OKB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        OKBActionPerformed(evt);
      }
    });
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
   * @see <a href="ViewConfigsrc.html#updateFields">Source</a>
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

  /**
   * When 'OK' is pushed this method checks
   * 1. Not in single tuple mode:
   *    labeltext, labelpos, category is associated to all tupels of the graphic attribute
   * 2. Reference dependant rendering?
   *    calculates a category for each value of the ref. attr. ,based upon the selected 
   *    category and rendering type.
   * @param evt
   * @see <a href="ViewConfigsrc.html#OKBActionPerformed">Source</a>
   */
  private void OKBActionPerformed (java.awt.event.ActionEvent evt) {            //GEN-FIRST:event_OKBActionPerformed
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
            String text = Query.getModel().getElementAt(cnt*(AttrCount + 1)
                + LabIndex).toString();
            dg.setLabelText(text.substring(text.indexOf(":") + 1));
          } 
          else if (LabelText.getText().equals(""))
            dg.setLabelText(null); 
          else 
            dg.setLabelText(LabelText.getText());
          cnt++;
        }
      }
    }
    if (RefDepCBo.isSelected()) {
      String as = (String)RefAttrCB.getSelectedItem();
      int RefAttrIndex = LabelAList.indexOf(as);
      //int RendTypeIndex=RendTypeCB.getSelectedIndex());
      Point2D.Double p = calcMinMax(RefAttrIndex);
      double min = p.getX();
      double max = p.getY();
      //create number of groups cats
      int nr = 8;
      try {
        nr = Integer.parseInt(NoText.getText());
      } catch (NumberFormatException n) {}
      if (max != min) {         // no cat need to be created if =
        Category GroupCats[] = new Category[nr];
        for (int i = 0; i < nr; GroupCats[i] = calcCategory(i++, nr, (Category)CatCB.getSelectedItem()));
        ListIterator li = Query.getGraphObjects().listIterator();
        ListExpr tl = Query.LEResult.second();
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
            if (RefAttrIndex == -1)
              value = (double)val; 
            else 
              value = convertLEtoDouble(le.first());
            int chg = (int)((value - min)/(max - min)*nr);
            mw.Cats.add(GroupCats[chg]);
            dg.setCategory(GroupCats[chg]);
            tl = tl.rest();
            val++;
          }
        }
      }
    }
    setVisible(false);
    dispose();
  }             //GEN-LAST:event_OKBActionPerformed

  /**create new Cat for every GO with Attribute AttrName according to its ref. Tupel-Value
  * @see <a href="ViewConfigsrc.html#calcCategory">Source</a>
   */
  private Category calcCategory (int chg, int nr, Category templCat) {
    Category NewCat = null;
    try {
      NewCat = (Category)templCat.clone();
    } catch (Exception e) {}
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
      int dec = 240/nr;
      int cv = 240 - chg*dec;
      NewCat.setFillStyle(new Color(cv, cv, 255));
    }
    if (RendType.equals("Solid Red Values")) {
      int dec = 240/nr;
      int cv = 240 - chg*dec;
      NewCat.setFillStyle(new Color(240, cv, cv));
    }
    if (RendType.equals("Solid Green Values")) {
      int dec = 240/nr;
      int cv = 240 - chg*dec;
      NewCat.setFillStyle(new Color(cv, 240, cv));
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
      File f = new File("images/" + RendType.substring(10));
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
        NewCat.setIconPath(fileList[chg].getPath());
      }
    }
    if (RendType.equals("Point Size <32Px")) {
      double inc = 32.0/(double)nr;
      double cv = 4 + (double)chg*inc;
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
   * @see <a href="ViewConfigsrc.html#searchForCalcCats">Source</a>
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
    File f = new File("images/");
    File[] fileList = f.listFiles(DirFilter);
    for (int i = 0; i < fileList.length; i++)
      RendTypeCB.addItem(new String("Images in " + fileList[i].getName()));
  }

  /**
   * Calculates the minimum and maximum of an attribute over all tuples
   * @param ind The ind's attribute in a tuple
   * @return Min. and max. as Point2D.double
   * @see <a href="ViewConfigsrc.html#calcMinMax">Source</a>
   */
  private Point2D.Double calcMinMax (int ind) {
    if (ind == -1)
      return  new Point2D.Double(0, (double)(TupelCount - 1));
    ListExpr tl = Query.LEResult.second();
    double min = Double.MAX_VALUE;
    double max = Double.MIN_VALUE;
    while (!tl.isEmpty()) {
      ListExpr le = tl.first();
      for (int i = 1; i <= ind; i++)
        le = le.rest();
      double w = convertLEtoDouble(le.first());
      if (min > w)
        min = w;
      if (max < w)
        max = w;
      tl = tl.rest();
    }
    return  new Point2D.Double(min, max);
  }

  /**
   * Makes a double-value out of a special atom-type
   * @param le
   * @return double value
   * @see <a href="ViewConfigsrc.html#convertLEtoDouble">Source</a>
   */
  private double convertLEtoDouble (ListExpr le) {
    if (le.atomType() == ListExpr.STRING_ATOM)
      return  (double)le.stringValue().hashCode();
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

  /**
   * Moves to the previous or the last tuple
   * @param evt
   * @see <a href="ViewConfigsrc.html#PrTuBActionPerformed">Source</a>
   */
  private void PrTuBActionPerformed (java.awt.event.ActionEvent evt) {          //GEN-FIRST:event_PrTuBActionPerformed
    AktTupNr = (AktTupNr > 1) ? AktTupNr - 1 : TupelCount;
    updateFields();
  }             //GEN-LAST:event_PrTuBActionPerformed

  /**
   * Moves to the next or first tuple.
   * @param evt
   * @see <a href="ViewConfigsrc.html#NeTuBActionPerformed">Source</a>
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


  /** Closes the dialog 
  * @see <a href="ViewConfigsrc.html#closeDialog">Source</a>
   */
  private void closeDialog (java.awt.event.WindowEvent evt) {                   //GEN-FIRST:event_closeDialog
    setVisible(false);
    dispose();
  }             //GEN-LAST:event_closeDialog
  //  }
  // Variables declaration - do not modify//GEN-BEGIN:variables
  private javax.swing.JLabel GALabel;
  private javax.swing.JLabel VCLabel;
  private javax.swing.JComboBox CatCB;
  private javax.swing.JCheckBox RefDepCBo;
  private javax.swing.JLabel RALabel;
  private javax.swing.JLabel RTLabel;
  private javax.swing.JComboBox RefAttrCB;
  private javax.swing.JComboBox RendTypeCB;
  private javax.swing.JLabel NGLabel;
  private javax.swing.JSlider NoSlider;
  private javax.swing.JTextField NoText;
  private javax.swing.JTextField LabelText;
  private javax.swing.JTextField LabXOffText;
  private javax.swing.JTextField LabYOffText;
  private javax.swing.JCheckBox SingleTupelCBo;
  private javax.swing.JButton NeTuB;
  private javax.swing.JButton PrTuB;
  private javax.swing.JButton GATuB;
  private javax.swing.JButton ApplyTuB;
  private javax.swing.JButton OKB;
  private javax.swing.JButton CancelB;
  private javax.swing.JButton DefaultB;
  private javax.swing.JLabel LALabel;
  private javax.swing.JLabel LTLabel;
  private javax.swing.JComboBox LabelAttrCB;
  private javax.swing.JComboBox RendPosCB;
  private JLabel GAAttrName, TupNrLabel;
  private static int VariantNr = 0;
  // End of variables declaration//GEN-END:variables
}



