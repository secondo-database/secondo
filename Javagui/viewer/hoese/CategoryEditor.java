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

import  javax.swing.*;
import  java.io.*;
import  java.awt.*;
import  java.util.*;
import  java.awt.image.*;
import  javax.swing.tree.*;
import  viewer.HoeseViewer;
import tools.Reporter;

/**
 * A class for editing categories based on swing's JDialog
 * @author  hoese
 * @version
 */
public class CategoryEditor extends javax.swing.JDialog {
  /** A link to the main application window */
  private HoeseViewer mw;
  /** The actual index in the category list */
  private int aktIndex;
  /** A counter neccassary for creating unique names for new categories */
  public static int CpCnt = 1;
  /** If true then in mode: change the category for a graph. object */
  private boolean LeaveByApply;
  /** The internal File-object for the icon */
  private String IconFileName;

 /** a FileChooser to load textures */
  private static  JFileChooser Texture_FileChooser=new JFileChooser();

 /** Creates new instance of a CategoryEditor
  * @param parent The main application JFrame
  * @param modal True if modal dialog
  * @see <a href="CategoryEditorsrc.html#CategoryEditor1">Source</a>
  */
  public CategoryEditor (HoeseViewer aparent, boolean modal) {
    super(aparent.getMainFrame(), modal);
    mw = aparent;
    LeaveByApply = false;
    initComponents();
    ListIterator li = mw.Cats.listIterator();
    String s = " ";
    while (li.hasNext()) {
      s = ((Category)li.next()).getName();
      CatCB.addItem(s);
    }
    //CatCB.setSelectedIndex(0);
    CatCB.setEditable(true);
    pack();
  }

  /** set the directory for textures */
  public static void setTextureDirectory(File dir){
     Texture_FileChooser.setCurrentDirectory(dir);
  }

  /**
   * Constructor for editing graph. objects category
   * @param   HoeseViewer aparent
   * @param   boolean modal
   * @param   Category Cat actual category to edit
  * @see <a href="CategoryEditorsrc.html#CategoryEditor2">Source</a>
   */
  public CategoryEditor (HoeseViewer aparent, boolean modal, Category Cat) {
    this(aparent, modal);
    LeaveByApply = true;
    if (mw.Cats.indexOf(Cat) == -1) {           //if generated variant
      mw.Cats.add(Cat);
      CatCB.addItem(Cat.getName());
    }
    CatCB.setSelectedItem(Cat.getName());
    setTitle("Choose category for selected Object");
    ApplyB.setText("OK");
    setSize(400,500);
  }

  /** This method is called from within the constructor to
   * initialize the dialog.
  * @see <a href="CategoryEditorsrc.html#initComponents">Source</a>
   */
  private void initComponents () {              //GEN-BEGIN:initComponents
    PointFormsP = new javax.swing.JPanel();
    CircleRB = new javax.swing.JRadioButton();
    RectRB = new javax.swing.JRadioButton();
    SizeL = new javax.swing.JLabel();
    SizeT = new javax.swing.JTextField();
    LineP = new javax.swing.JPanel();
    WidthL = new javax.swing.JLabel();
    WidthT = new javax.swing.JTextField();
    TypeL = new javax.swing.JLabel();
    TypeCB = new javax.swing.JComboBox(Category.StrokeStrings);
    ColorL = new javax.swing.JLabel();
    ColorB = new javax.swing.JButton();
    FillStyleP = new javax.swing.JPanel();
    TransparencyT = new javax.swing.JTextField();
    TransparencyL = new javax.swing.JLabel();
    TextureRB = new javax.swing.JRadioButton();
    TextureIconB = new javax.swing.JButton();
    IconRB = new JRadioButton();
    ResizeIconCB = new JCheckBox();
    SolidRB = new javax.swing.JRadioButton();
    SolidColorB = new javax.swing.JButton();
    GradientRB = new javax.swing.JRadioButton();
    GradientColorB = new javax.swing.JButton();
    NoFillRB = new javax.swing.JRadioButton();
    CatCB = new javax.swing.JComboBox();
    CatNameL = new javax.swing.JLabel();
    ComP = new java.awt.Panel();
    NewB = new javax.swing.JButton();
    DeleteB = new javax.swing.JButton();
    ApplyB = new javax.swing.JButton();
    CancelB = new javax.swing.JButton();
    String[] capOptions = {"butt","round","square"};
    capStyle = new JComboBox(capOptions);
    String[] joinOptions = {"bevel","miter","round"};
    joinStyle= new JComboBox(joinOptions);




    getContentPane().setLayout(new java.awt.GridBagLayout());
    java.awt.GridBagConstraints gridBagConstraints1;
    //setResizable(false);
    setTitle("Category Editor");
    setModal(true);
    addWindowListener(new java.awt.event.WindowAdapter() {
      public void windowClosing (java.awt.event.WindowEvent evt) {
        closeDialog(evt);
      }
    });
    PointFormsP.setLayout(new java.awt.FlowLayout(0, 5, 5));
    PointFormsP.setBorder(new javax.swing.border.TitledBorder("Point Forms"));
    CircleRB.setText("as Circle");
    PointFormsP.add(CircleRB);
    RectRB.setText("as Rectangle");
    PointFormsP.add(RectRB);
    ButtonGroup group = new ButtonGroup();
    group.add(CircleRB);
    group.add(RectRB);
    SizeL.setText("Size");
    SizeL.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
    PointFormsP.add(SizeL);
    SizeT.setColumns(4);
    SizeT.setText("8");
    PointFormsP.add(SizeT);
    gridBagConstraints1 = new java.awt.GridBagConstraints();
    gridBagConstraints1.gridx = 0;
    gridBagConstraints1.gridy = 1;
    gridBagConstraints1.gridwidth = 2;
    gridBagConstraints1.anchor = java.awt.GridBagConstraints.WEST;
    getContentPane().add(PointFormsP, gridBagConstraints1);
    LineP.setLayout(new java.awt.GridBagLayout());
    java.awt.GridBagConstraints gridBagConstraints2;
    LineP.setBorder(new javax.swing.border.TitledBorder("(Out)Line-Selection"));
    WidthL.setText("Width");
    WidthL.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
    gridBagConstraints2 = new java.awt.GridBagConstraints();
    gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
    LineP.add(WidthL, gridBagConstraints2);
    WidthT.setColumns(4);
    WidthT.setText("4");
    gridBagConstraints2 = new java.awt.GridBagConstraints();
    gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
    LineP.add(WidthT, gridBagConstraints2);
    TypeL.setText("Type");
    TypeL.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
    gridBagConstraints2 = new java.awt.GridBagConstraints();
    gridBagConstraints2.gridx = 0;
    gridBagConstraints2.gridy = 1;
    gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
    LineP.add(TypeL, gridBagConstraints2);
    gridBagConstraints2 = new java.awt.GridBagConstraints();
    gridBagConstraints2.gridx = 1;
    gridBagConstraints2.gridy = 1;
    gridBagConstraints2.gridwidth = 3;
    gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
    LineP.add(TypeCB, gridBagConstraints2);
    ColorL.setText("Color");
    ColorL.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
    gridBagConstraints2 = new java.awt.GridBagConstraints();
    gridBagConstraints2.insets = new java.awt.Insets(0, 16, 0, 0);
    gridBagConstraints2.anchor = java.awt.GridBagConstraints.EAST;
    LineP.add(ColorL, gridBagConstraints2);
    ColorB.setPreferredSize(new java.awt.Dimension(20, 20));
    ColorB.setBackground(java.awt.Color.pink);
    ColorB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        ColorBActionPerformed(evt);
      }
    });
    gridBagConstraints2 = new java.awt.GridBagConstraints();
    gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
    LineP.add(ColorB, gridBagConstraints2);
    

    gridBagConstraints2 = new GridBagConstraints();
    LineP.add(new JLabel("cap style"), gridBagConstraints2);
    gridBagConstraints2 = new GridBagConstraints();
    LineP.add(capStyle, gridBagConstraints2);
    
    gridBagConstraints2 = new GridBagConstraints();
    gridBagConstraints2.gridx=4;
    gridBagConstraints2.gridy=1;
    gridBagConstraints2.gridwidth=1;
    LineP.add(new JLabel("join style"), gridBagConstraints2);
    gridBagConstraints2 = new GridBagConstraints();
    gridBagConstraints2.gridx=5;
    gridBagConstraints2.gridy=1;
    gridBagConstraints2.gridwidth=1;
    LineP.add(joinStyle, gridBagConstraints2);
    gridBagConstraints1 = new java.awt.GridBagConstraints();
    gridBagConstraints1.gridx = 0;
    gridBagConstraints1.gridy = 2;
    gridBagConstraints1.gridwidth = 2;
    gridBagConstraints1.anchor = java.awt.GridBagConstraints.NORTHWEST;
   
    // set the cap and join style to be round
    capStyle.setSelectedIndex(1);
    joinStyle.setSelectedIndex(2); 

    getContentPane().add(LineP, gridBagConstraints1);
    FillStyleP.setLayout(new java.awt.GridBagLayout());
    
    java.awt.GridBagConstraints gridBagConstraints3;
    FillStyleP.setBorder(new javax.swing.border.TitledBorder("Fill Style"));
    TransparencyT.setColumns(6);
    TransparencyT.setText("0.0");
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(TransparencyT, gridBagConstraints3);
    TransparencyL.setText("Transparency in %");
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(TransparencyL, gridBagConstraints3);

    // texture

    TextureRB.setText("Texture");
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.gridx = 0;
    gridBagConstraints3.gridy = 1;
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(TextureRB, gridBagConstraints3);
    String filesep = "/";
    try{
       String tmpfilesep = System.getProperty("file.separator");
       if(tmpfilesep!=null)
           filesep = tmpfilesep;
    }catch(Exception e){} // ignore this exception

    // set the standardicon
    IconFileName = "res/duke.gif";
    
    try{
       TextureIconB.setIcon(new ImageIcon(ClassLoader.getSystemResource(IconFileName)));
    }catch(Exception e){
        Reporter.writeError("Cannot find the resource " + IconFileName);
    }

    TextureIconB.setPreferredSize(new java.awt.Dimension(40, 40));
    TextureIconB.setMaximumSize(new java.awt.Dimension(50, 75));
    //   TextureIconB.setBackground (java.awt.Color.blue);
    TextureIconB.setMinimumSize(new java.awt.Dimension(50, 75));
    TextureIconB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        TextureIconBActionPerformed(evt);
      }
    });
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.gridx = 1;
    gridBagConstraints3.gridy = 1;
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(TextureIconB, gridBagConstraints3);

    // iconFill
    IconRB.setText("Icon");
    gridBagConstraints3 = new GridBagConstraints();
    gridBagConstraints3.gridx = 0;
    gridBagConstraints3.gridy = 2;
    gridBagConstraints3.anchor = GridBagConstraints.WEST;
    FillStyleP.add(IconRB,gridBagConstraints3);
    ResizeIconCB.setText("resize");
    ResizeIconCB.setSelected(false);
    gridBagConstraints3 = new GridBagConstraints();
    gridBagConstraints3.gridx = 1;
    gridBagConstraints3.gridy = 2;
    gridBagConstraints3.anchor = GridBagConstraints.WEST;
    FillStyleP.add(ResizeIconCB,gridBagConstraints3);
     
    // solid fill

    SolidRB.setText("Solid");
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.gridx = 0;
    gridBagConstraints3.gridy = 3;
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(SolidRB, gridBagConstraints3);
    SolidColorB.setPreferredSize(new java.awt.Dimension(20, 20));
    //     SolidColorB.setBackground (java.awt.Color.pink);
    SolidColorB.addActionListener(new java.awt.event.ActionListener() {

      public void actionPerformed (java.awt.event.ActionEvent evt) {
        SolidColorBActionPerformed(evt);
      }
    });
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.gridx = 1;
    gridBagConstraints3.gridy = 3;
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(SolidColorB, gridBagConstraints3);

    // gradient fill
    GradientRB.setText("Gradient");
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.gridx = 0;
    gridBagConstraints3.gridy = 4;
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(GradientRB, gridBagConstraints3);
    GradientColorB.setPreferredSize(new java.awt.Dimension(20, 20));
    // GradientColorB.setBackground (java.awt.Color.pink);
    GradientColorB.addActionListener(new java.awt.event.ActionListener() {

      public void actionPerformed (java.awt.event.ActionEvent evt) {
        GradientColorBActionPerformed(evt);
      }
    });
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.gridx = 1;
    gridBagConstraints3.gridy = 4;
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(GradientColorB, gridBagConstraints3);
    
    // No Filling

    NoFillRB.setText("No Filling");
    gridBagConstraints3 = new java.awt.GridBagConstraints();
    gridBagConstraints3.gridx = 0;
    gridBagConstraints3.gridy = 5;
    gridBagConstraints3.anchor = java.awt.GridBagConstraints.WEST;
    FillStyleP.add(NoFillRB, gridBagConstraints3);
    gridBagConstraints1 = new java.awt.GridBagConstraints();
    gridBagConstraints1.gridx = 0;
    gridBagConstraints1.gridy = 5;
    gridBagConstraints1.gridwidth = 2;
    gridBagConstraints1.anchor = java.awt.GridBagConstraints.NORTHWEST;
    getContentPane().add(FillStyleP, gridBagConstraints1);
    CatCB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        CatCBActionPerformed(evt);
      }
    });
    gridBagConstraints1 = new java.awt.GridBagConstraints();
    gridBagConstraints1.gridx = 1;
    gridBagConstraints1.gridy = 0;
    gridBagConstraints1.anchor = java.awt.GridBagConstraints.WEST;
    getContentPane().add(CatCB, gridBagConstraints1);
    CatNameL.setText("Name");
    gridBagConstraints1 = new java.awt.GridBagConstraints();
    gridBagConstraints1.gridx = 0;
    gridBagConstraints1.gridy = 0;
    gridBagConstraints1.anchor = java.awt.GridBagConstraints.EAST;
    getContentPane().add(CatNameL, gridBagConstraints1);
    ComP.setLayout(new javax.swing.BoxLayout(ComP, 1));
    ComP.setBackground(new java.awt.Color(204, 204, 204));
    ComP.setForeground(java.awt.Color.black);
    ComP.setName("panel1");
    ComP.setFont(new java.awt.Font("Dialog", 0, 11));
    NewB.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
    NewB.setText("New");
    NewB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        NewBActionPerformed(evt);
      }
    });
    ComP.add(NewB);
    DeleteB.setText("Delete");
    DeleteB.addActionListener(new java.awt.event.ActionListener() {

      public void actionPerformed (java.awt.event.ActionEvent evt) {
        DeleteBActionPerformed(evt);
      }
    });
    ComP.add(DeleteB);
    ApplyB.setText("Apply");
    ApplyB.addActionListener(new java.awt.event.ActionListener() {

      public void actionPerformed (java.awt.event.ActionEvent evt) {
        ApplyBActionPerformed(evt);
      }
    });
    ComP.add(ApplyB);
    CancelB.setText("Close");
    CancelB.addActionListener(new java.awt.event.ActionListener() {

      public void actionPerformed (java.awt.event.ActionEvent evt) {
        CancelBActionPerformed(evt);
      }
    });
    ComP.add(CancelB);
    gridBagConstraints1 = new java.awt.GridBagConstraints();
    gridBagConstraints1.gridx = 2;
    gridBagConstraints1.gridy = 2;
    gridBagConstraints1.gridheight = 2;
    group = new ButtonGroup();
    group.add(TextureRB);
    group.add(SolidRB);
    group.add(IconRB);
    group.add(GradientRB);
    group.add(NoFillRB);
    getContentPane().add(ComP, gridBagConstraints1);
  }             //GEN-END:initComponents

  /**
   * After Close-button this mezhod is called. Closes the dialog
   * @param evt
  * @see <a href="CategoryEditorsrc.html#CancelBActionPerformed">Source</a>
   */
  private void CancelBActionPerformed (java.awt.event.ActionEvent evt) {        //GEN-FIRST:event_CancelBActionPerformed
    aktIndex = -1;
    closeDialog(null);
  }             //GEN-LAST:event_CancelBActionPerformed

  /**
   * Applies the changes to the category-list
   * @param evt
   * @see <a href="CategoryEditorsrc.html#ApplyBActionPerformed">Source</a>
   */
  private void ApplyBActionPerformed (java.awt.event.ActionEvent evt) {         //GEN-FIRST:event_ApplyBActionPerformed
    Category aktCat = (Category)mw.Cats.elementAt(aktIndex);
    getDialog(aktCat);
    CatCB.insertItemAt(aktCat.getName(), aktIndex);
    CatCB.removeItemAt(aktIndex + 1);
    if (LeaveByApply) {
      closeDialog(null);
    }  else{
      mw.repaint();
    }

  }             //GEN-LAST:event_ApplyBActionPerformed

  /**
   * Deletes a category from list if not used and if not the default-category
   * @param evt
   * @see <a href="CategoryEditorsrc.html#DeleteBActionPerformed">Source</a>
   */
  private void DeleteBActionPerformed (java.awt.event.ActionEvent evt) {        //GEN-FIRST:event_DeleteBActionPerformed
    // Check if used
    boolean isused = false;
    Category aktCat = (Category)mw.Cats.elementAt(aktIndex);
    if (aktCat == Category.getDefaultCat()) {
      Reporter.showError("This is the Default !");
      return;
    }
    JComboBox cb = mw.TextDisplay.getQueryCombo();
    for (int j = 0; j < cb.getItemCount(); j++) {
      ListIterator li2 = ((QueryResult)cb.getItemAt(j)).getGraphObjects().listIterator();
      while (li2.hasNext()) {
        DsplGraph dg = (DsplGraph)li2.next();
        if (dg.getCategory() == aktCat)
          isused = true;
      }
    }
    if (isused) {
      Reporter.showError("Category is in use!");
      return;
    } 
    else {
      Category c = (Category)mw.Cats.remove(aktIndex);
      CatCB.removeItem(c.getName());
    }
  }             //GEN-LAST:event_DeleteBActionPerformed

  /**
   * Creates a new category by copying the actual selected and add a suffix to the name
   * @param evt
  * @see <a href="CategoryEditorsrc.html#NewBActionPerformed">Source</a>
   */
  private void NewBActionPerformed (java.awt.event.ActionEvent evt) {           //GEN-FIRST:event_NewBActionPerformed
    Category NewCat = new Category();
    getDialog(NewCat);
    String NewName = NewCat.getName() + "_" + Integer.toString(CpCnt++);
    NewCat.setName(NewName);
    mw.Cats.add(NewCat);
    CatCB.addItem(NewName);
    CatCB.setSelectedItem(NewName);
  }             //GEN-LAST:event_NewBActionPerformed

  /**
   * Called if new category is selected in combobox, set the dialog according to the new selection
   * @param evt
  * @see <a href="CategoryEditorsrc.html#CatCBActionPerformed">Source</a>
   */
  private void CatCBActionPerformed (java.awt.event.ActionEvent evt) {          //GEN-FIRST:event_CatCBActionPerformed
    JComboBox cb = (JComboBox)evt.getSource();
    int i = cb.getSelectedIndex();
    if ((i >= 0) && (i < mw.Cats.size())) {
      aktIndex = i;
      Category aktCat = (Category)mw.Cats.elementAt(i);
      setDialog(aktCat);
    }
  }             //GEN-LAST:event_CatCBActionPerformed

  /**
   * Selects the gradient-color
   * @param evt
  * @see <a href="CategoryEditorsrc.html#GradientColorBActionPerformed">Source</a>
   */
  private void GradientColorBActionPerformed (java.awt.event.ActionEvent evt) {                 //GEN-FIRST:event_GradientColorBActionPerformed
    Color newColor = JColorChooser.showDialog(this, "Choose  Gradient Color", 
        GradientColorB.getBackground());
    if (newColor != null) {
      GradientColorB.setBackground(newColor);
    }
  }             //GEN-LAST:event_GradientColorBActionPerformed

  /**
   * Selects the solid-color
   * @param evt
  * @see <a href="CategoryEditorsrc.html#SolidColorBActionPerformed">Source</a>
   */
  private void SolidColorBActionPerformed (java.awt.event.ActionEvent evt) {                    //GEN-FIRST:event_SolidColorBActionPerformed
    Color newColor = JColorChooser.showDialog(this, "Choose  Solid Color", 
        SolidColorB.getBackground());
    if (newColor != null) {
      SolidColorB.setBackground(newColor);
    }
  }             //GEN-LAST:event_SolidColorBActionPerformed

  /**
   * Selects the texture-icon.
   * @param evt
  * @see <a href="CategoryEditorsrc.html#TextureIconBActionPerformed">Source</a>
   */
  private void TextureIconBActionPerformed (java.awt.event.ActionEvent evt) {                   //GEN-FIRST:event_TextureIconBActionPerformed
    int returnVal = Texture_FileChooser.showOpenDialog(this);
    if (returnVal == JFileChooser.APPROVE_OPTION) {
      IconFileName = Texture_FileChooser.getSelectedFile().getPath();
      TextureIconB.setIcon(new ImageIcon(IconFileName));
    }
  }             //GEN-LAST:event_TextureIconBActionPerformed

  /**
   * Selects the outline-color
   * @param evt
  * @see <a href="CategoryEditorsrc.html#ColorBActionPerformed">Source</a>
   */
  private void ColorBActionPerformed (java.awt.event.ActionEvent evt) {         //GEN-FIRST:event_ColorBActionPerformed
    Color newColor = JColorChooser.showDialog(this, "Choose  Color", ColorB.getBackground());
    if (newColor != null) {
      ColorB.setBackground(newColor);
    }
  }             //GEN-LAST:event_ColorBActionPerformed

  /** Closes the dialog 
   * @param evt
  * @see <a href="CategoryEditorsrc.html#closeDialog">Source</a>
   */
  private void closeDialog (java.awt.event.WindowEvent evt) {                   //GEN-FIRST:event_closeDialog
    setVisible(false);
    dispose();
  }             //GEN-LAST:event_closeDialog

  /**
   * Sets all the elements in the dialog according to cat
   * @param cat A category
  * @see <a href="CategoryEditorsrc.html#setDialog">Source</a>
   */
  public void setDialog (Category cat) {
    if (cat.getPointasRect())
      RectRB.setSelected(true); 
    else 
      CircleRB.setSelected(true);
    // ignore reference depending rendeing
    SizeT.setText(Double.toString(cat.getPointSize(null,0)));
    //	  BasicStroke bs=Category.getLineStroke(cat.getLineStyle());
    WidthT.setText(Float.toString(cat.getLineWidth(null,0)));
    ColorB.setBackground(cat.getLineColor());
    TypeCB.setSelectedIndex(cat.getLineStyle());
    int cap = cat.getCapStyle();
    int join = cat.getJoinStyle();
    switch(cap){
       case BasicStroke.CAP_BUTT: capStyle.setSelectedIndex(0);break;
       case BasicStroke.CAP_ROUND: capStyle.setSelectedIndex(1);break;
       case BasicStroke.CAP_SQUARE: capStyle.setSelectedIndex(2); break;
       default: Reporter.writeError("unknown cap style detected in category");
    }
    switch(join){
       case BasicStroke.JOIN_BEVEL: joinStyle.setSelectedIndex(0);break;
       case BasicStroke.JOIN_MITER: joinStyle.setSelectedIndex(1);break;
       case BasicStroke.JOIN_ROUND: joinStyle.setSelectedIndex(2); break;
       default: Reporter.writeError("unknown join style detected in category");
    }

    TransparencyT.setText(Float.toString(100.0f - ((AlphaComposite)cat.getAlphaStyle()).getAlpha()*100));
    GradientColorB.setBackground(Color.lightGray);
    SolidColorB.setBackground(Color.lightGray);
    TextureIconB.setBackground(Color.lightGray);
    Paint fillStyle = cat.getFillStyle(null,0); // not supported
    if (fillStyle == null)
      NoFillRB.setSelected(true); 
    else if (fillStyle instanceof TexturePaint) {
      IconFileName=(new File(cat.getIconPath())).getName();
      TextureRB.setSelected(true);
      TextureIconB.setIcon(new ImageIcon(((TexturePaint)fillStyle).getImage()));
    } 
    else if (fillStyle instanceof GradientPaint) {
      GradientRB.setSelected(true);
      SolidColorB.setBackground(((GradientPaint)fillStyle).getColor1());
      GradientColorB.setBackground(((GradientPaint)fillStyle).getColor2());
    } 
    else {
      SolidRB.setSelected(true);
      SolidColorB.setBackground((Color)fillStyle);
    } 
    if(cat.getIconFill()){
       IconRB.setSelected(true);
       IconFileName=(new File(cat.getIconPath())).getName();
       TextureIconB.setIcon(new ImageIcon(cat.getTextureImage()));
    }
    ResizeIconCB.setSelected(cat.getIconResizeToBox());
    
  }

  /**
   * 
   * @return The actual selected category
  * @see <a href="CategoryEditorsrc.html#getActualCategory">Source</a>
   */
  public Category getActualCategory () {
    if (aktIndex == -1)
      return  null;
    return  (Category)mw.Cats.elementAt(aktIndex);
  }

  /**
   * Reads the dialog parameters and place it in the category cat
   * @param cat
  * @see <a href="CategoryEditorsrc.html#getDialog">Source</a>
   */
  public void getDialog (Category cat) {
    //Category cat = new Category();
    cat.setName((String)CatCB.getSelectedItem());
    cat.setPointasRect(RectRB.isSelected());
    cat.setPointSize(Double.parseDouble(SizeT.getText()));
    double linewidth = Double.parseDouble(WidthT.getText());
    int cap=BasicStroke.CAP_BUTT;
    switch(capStyle.getSelectedIndex()){
        case 0 : cap = BasicStroke.CAP_BUTT;break;
        case 1 : cap = BasicStroke.CAP_ROUND; break;
        case 2 : cap = BasicStroke.CAP_SQUARE; break;
        default: Reporter.writeError("invalid value for cap style");
    }    
    int join = BasicStroke.JOIN_BEVEL;
    switch(joinStyle.getSelectedIndex()){
        case 0 : join = BasicStroke.JOIN_BEVEL;break;
        case 1 : join = BasicStroke.JOIN_MITER;break;
        case 2 : join = BasicStroke.JOIN_ROUND;break;
        default: Reporter.writeError("invalid value for join style");
    }
    int dash = TypeCB.getSelectedIndex();
    cat.setLineStyle(dash,cap,join,linewidth);
    cat.setLineColor(ColorB.getBackground());
    float f = -Float.parseFloat(TransparencyT.getText())/100 + 1.0f;
    if (f > 1.0f)
      f = 1.0f;
    if (f < 0.0f)
      f = 0.0f;
    cat.setAlphaStyle(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, f));
    cat.setIconFill(false); // default
    if (NoFillRB.isSelected()){
      cat.setFillStyle(null); 
    }  else if (SolidRB.isSelected()) {
      cat.setFillStyle(SolidColorB.getBackground());
    }  else if (GradientRB.isSelected()) {
      cat.setFillStyle(new GradientPaint(0.0f, 0.0f, SolidColorB.getBackground(),
          20.0f, 20.0f, GradientColorB.getBackground(), true));
    } else if (TextureRB.isSelected()) {
      cat.setIconName(IconFileName);
      ImageIcon ii = (ImageIcon)TextureIconB.getIcon();
      BufferedImage bi = new BufferedImage(ii.getIconWidth(), ii.getIconHeight(),
          BufferedImage.TYPE_INT_ARGB);
      Graphics2D big = bi.createGraphics();
      big.drawImage(ii.getImage(), 0, 0, null);
      Rectangle r = new Rectangle(0, 0, ii.getIconWidth(), ii.getIconHeight());
      cat.setFillStyle(new TexturePaint(bi, r));
    } else if(IconRB.isSelected()){
      cat.setIconFill(true);
      cat.setIconResizeToBox(ResizeIconCB.isSelected());
      cat.setIconName(IconFileName);
      ImageIcon ii = (ImageIcon)TextureIconB.getIcon();
      BufferedImage bi = new BufferedImage(ii.getIconWidth(), ii.getIconHeight(),
          BufferedImage.TYPE_INT_ARGB);
      Graphics2D big = bi.createGraphics();
      big.drawImage(ii.getImage(), 0, 0, null);
      big.dispose();
      cat.TextureImage = bi;
      // create a resized version
      cat.resizedImage=null;
      int ps = (int) cat.getPointSize(null,0);
      if(ps>0){
        cat.resizedImage= new BufferedImage(ps,ps,BufferedImage.TYPE_INT_ARGB);
        big = cat.resizedImage.createGraphics();
        big.drawImage(ii.getImage().getScaledInstance(ps,ps,Image.SCALE_DEFAULT),0,0,null);
        big.dispose();
      }

    }
    //		return cat;
  }

  // Variables declaration - do not modify//GEN-BEGIN:variables
  private javax.swing.JPanel PointFormsP;
  private javax.swing.JRadioButton CircleRB;
  private javax.swing.JRadioButton RectRB;
  private javax.swing.JLabel SizeL;
  private javax.swing.JTextField SizeT;
  private javax.swing.JPanel LineP;
  private javax.swing.JLabel WidthL;
  private javax.swing.JTextField WidthT;
  private javax.swing.JLabel TypeL;
  private javax.swing.JComboBox TypeCB;
  private javax.swing.JLabel ColorL;
  private javax.swing.JButton ColorB;
  private javax.swing.JPanel FillStyleP;
  private javax.swing.JTextField TransparencyT;
  private javax.swing.JLabel TransparencyL;
  private javax.swing.JRadioButton TextureRB;
  private javax.swing.JButton TextureIconB;
  private JRadioButton IconRB;
	private JCheckBox ResizeIconCB;
  private javax.swing.JRadioButton SolidRB;
  private javax.swing.JButton SolidColorB;
  private javax.swing.JRadioButton GradientRB;
  private javax.swing.JButton GradientColorB;
  private javax.swing.JRadioButton NoFillRB;
  private javax.swing.JComboBox CatCB;
  private javax.swing.JLabel CatNameL;
  private java.awt.Panel ComP;
  private javax.swing.JButton NewB;
  private javax.swing.JButton DeleteB;
  private javax.swing.JButton ApplyB;
  private javax.swing.JButton CancelB;
  private JComboBox capStyle;
  private JComboBox joinStyle;
  // End of variables declaration//GEN-END:variables
}




