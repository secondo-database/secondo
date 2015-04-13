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

package  viewer;

import gui.SecondoObject;
import sj.lang.*;
import tools.Reporter;

import java.io.*;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Toolkit;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Color;
import java.awt.Frame;
import java.awt.event.*;
import java.util.Vector;
import java.util.BitSet;
import java.util.Iterator;

import javax.swing.JComboBox;
import javax.swing.JEditorPane;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JSplitPane;
import javax.swing.JFormattedTextField;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JTabbedPane;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.JColorChooser;
import javax.swing.JTabbedPane;

import java.text.DecimalFormat;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import javax.vecmath.*;

import com.sun.j3d.utils.geometry.*;
import com.sun.j3d.utils.universe.*;

import javax.media.j3d.*;

import com.sun.j3d.utils.behaviors.vp.*;
import com.sun.j3d.utils.pickfast.behaviors.*;

import viewer.spatial3D.*;


/**
 * this is a viewer for single and relations of spatial 3D objects
 * this viewer displays objects from relations also in an textual representation
 */
public class Spatial3DViewer extends SecondoViewer {
  
  private enum AllowedObjects {point3d,surface3d,vector3d,volume3d}; //can be changed according to development of algebra

  private JTabbedPane configPane;
  private JComboBox itemsNameBox;   
  private Vector<ObjectGroup> itemsObject = new Vector<ObjectGroup>(10, 5);
  private JSplitPane splitPane;

  private JPanel graphicPanel;
  private JPanel boxPanel;
  
  JLabel scaleFactorLabel=new JLabel("Scale Factor:");
  private JFormattedTextField scaleFactorField = new JFormattedTextField(new DecimalFormat());
  
  private SecondoObject currentObject;
  
  private Canvas3D canvas;
  private SimpleUniverse universe;
  private int coordIndices[];
  private BranchGroup rootScene;
  
  // lighting
  private BranchGroup lights;
  private BranchGroup ambLightBg;
  
  //Dialogs
  private ScaleFactorDialog scaleFactorDialog;
  private ZoomFactorDialog zoomfactorDialog;
  private TranslateFactorDialog translateFactorDialog;
  private RotateFactorDialog rotateFactorDialog;
  private SetPointSizeDialog setPointSizeDialog;
  
  //Textview
  private TextWindow textWindow;
  
  //menus
  private MenuVector MenuExtension = new MenuVector();
  private JMenu settingsMenu;
  private JMenuItem menuItem_Scale;
  private JCheckBoxMenuItem menuItem_ScaleView;
  private JMenuItem menuItem_Zoom;
  private JMenuItem menuItem_Translate;
  private JMenuItem menuItem_Rotate;
  private JMenuItem menuItem_setPointSize;
  private JMenu appearanceMenu;
  private JCheckBoxMenuItem menuItem_GridView;
  private JCheckBoxMenuItem menuItem_varyingColorView;
  private JMenu setColorsMenu;
  private JMenuItem menuItem_setBackgroundColor;
  private JMenuItem menuItem_setGridColor;
  private JMenuItem menuItem_setObjectColor;
  private JMenu setDefaultColorsMenu;
  private JMenuItem menuItem_setDefaultBackgroundColor;
  private JMenuItem menuItem_setDefaultGridColor;
  private JMenuItem menuItem_setDefaultObjectColor;
  private JMenu viewMenu;
  private JMenuItem menuItem_centerView;
  private JMenuItem menuItem_resetView;
  private boolean showScaleMenu = false;
  
  private float pointSize =10.0f;
  private float singlePointSize;
  
  private ObjectGroup objectGroup;
  private ObjectGroup dummy;
  
  private boolean colorChanged;
  
  private OrbitBehavior ob;
  private Transform3D home = new Transform3D();
  
  /**
   * Creates a MainWindow with all its components
  */
  public Spatial3DViewer(){

    this.setLayout(new BorderLayout());
    
    // TextPanel
    textWindow = new TextWindow(this);
    configPane = new JTabbedPane();
    configPane.addTab("Text", textWindow);
    
    // combobox items
    itemsNameBox = new JComboBox();
    itemsNameBox.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        if(VC !=null){  // VC..ViewerControl
          int index = itemsNameBox.getSelectedIndex();
          if (index>=0){
            try{
              objectGroup=itemsObject.get(index); 
              currentObject = objectGroup.getSecondoObject() ;
              VC.selectObject(Spatial3DViewer.this,currentObject);
              showObjectGroup();
            }
            catch(Exception e){
              Reporter.writeError("Error in addActionListener= " + e.getStackTrace());
              Reporter.writeError("Error in addActionListener= " + e.getClass() + " - "
                + e.getMessage()+" - "+ e.getCause() );
            }
            }
          }
        }});
        
    JLabel itemsNameBoxLabel = new JLabel("Item name:");    
    
    itemsNameBox.setLightWeightPopupEnabled(false);
    itemsNameBox.setPreferredSize(new Dimension(230,itemsNameBox.getPreferredSize().height));
    
    boxPanel = new JPanel();
    boxPanel.setLayout(new FlowLayout());
    boxPanel.add(itemsNameBoxLabel);
    boxPanel.add(itemsNameBox); 
    
    // 3D Graphic pane 
    graphicPanel =  new JPanel();
    graphicPanel.setLayout(new BorderLayout());
      
    //create Canvas and put it on GraphicPane
    canvas= new Canvas3D(SimpleUniverse.getPreferredConfiguration());
    
    //set universe
    universe = new SimpleUniverse(canvas);
    //standardposition of viewer
    universe.getViewingPlatform().setNominalViewingTransform();
    //set root Scene
    rootScene = new BranchGroup();
    rootScene.setCapability(BranchGroup.ALLOW_DETACH);
    
    // Lighting
    ambLightBg = new BranchGroup();
    ambLightBg.setCapability(BranchGroup.ALLOW_DETACH);
    lights = new BranchGroup();
    lights.setCapability(BranchGroup.ALLOW_DETACH);
    
    //set back clip distance
    try{
      canvas.getView().setBackClipDistance(100);
    }
    catch (NullPointerException e) {
      Reporter.writeError("canvas=null. Can not setBackClipDistance!");
    }
    
    //set panels
    graphicPanel.add(boxPanel,BorderLayout.NORTH);
    graphicPanel.add(canvas,BorderLayout.CENTER);
    graphicPanel.setMinimumSize(new Dimension(100,50));
    
    splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,configPane,graphicPanel);
    splitPane.setOneTouchExpandable(true);
    splitPane.setDividerLocation(230);   
    
    this.add(splitPane,BorderLayout.CENTER);               
    
    // set a dummy objectGroup to avoid nullPointerException
    dummy = new ObjectGroup(null, canvas, this);         
    objectGroup=dummy;
    createMenuExtension();
    
    // disable menü
    settingsMenu.setEnabled(false);
    appearanceMenu.setEnabled(false);
    viewMenu.setEnabled(false);
    
  }
  
  /**
   * getName
   * @return the name of this viewer
   */
  public String getName(){
    return "Spatial3DViewer";
    }

  /**
   * addObject
   * add the Secondo object
   * @param o This Method analyse the value o
   * @return true if the object could display
   */
  public boolean addObject(SecondoObject o){
    if (!canDisplay(o))
    return false;
    if (isDisplayed(o)) 
    selectObject(o); 
    else{
      try{
        objectGroup = new ObjectGroup(o, canvas, this);
        itemsObject.add(objectGroup);
        itemsNameBox.addItem(o.getName());
        showObjectGroup();
        //report if it is an undefined Object
        if (objectGroup.isUndefined()) {
          if (objectGroup.isRelation())
          Reporter.showInfo("Your Relation contains an undefined object");
          else
          Reporter.showInfo("Your Object is undefined. As a result, the object is not visible.");
        }
        //report if it is an empty Object
        if (objectGroup.isEmpty()) {
          if (objectGroup.isRelation())
          Reporter.showInfo("Your Relation contains an empty object");
          else
          Reporter.showInfo("You have an empty Object.");
        }
        // make the new object to active object
        itemsNameBox.setSelectedIndex(itemsNameBox.getItemCount()-1);
        }
      catch(Exception e){
        Reporter.writeError("Error in addObject= " + e.getStackTrace());
        Reporter.writeError("Error in addObject= " + e.getClass() + " - "
          + e.getMessage()+" - "+ e.getCause() );
        }
      }
    return true;
  }
  
  /**
   * isDisplayed
   * @return true if o is contained in this viewer
   */
  public boolean isDisplayed(SecondoObject o){
    Iterator<ObjectGroup> it = itemsObject.iterator();
    while (it.hasNext()) {
      if (o.equals(it.next().getSecondoObject())) {
        return true;
      }
    }
    return false;
  }
    
  /**
   * removeAll
   * removes all objects from this viewer
   */
  public void removeAll(){
    itemsObject.removeAllElements();   // remove all entrys from vector
    itemsNameBox.removeAllItems();        // remove all entrys from combobox
    currentObject =null;
    if (VC !=null)
      VC.removeObject(null);
    clearView();
  }
  
  /**
   * removeObject
   * removes SecondoObject o from this viewer
   */
  public void removeObject (SecondoObject o){
    Iterator<ObjectGroup> it = itemsObject.iterator();
    ObjectGroup objGrp = null;
    boolean itemFound=false;
    while (it.hasNext() && !itemFound) {
      objGrp =it.next();
      if (o.equals(objGrp.getSecondoObject())) {
        itemsObject.remove(objGrp);
        itemsNameBox.removeItem(o.getName());
        itemFound=true;
        }
      }
    if (objGrp.equals(objectGroup)) {
      clearView();                                       
      }
    }
 

  /**
   * canDisplay 
   * @return true if this viewer is able to display the given object
   * this viwer can show Spatial3D objects, their relation, and relations of their tuples
   * since algebra structure of Spatial3D migth be still changed, it is tentative.
   */
  @Override
  public boolean canDisplay(SecondoObject o){
    ListExpr LE = o.toListExpr();
    return typeCheck(LE,false);
  }
  
   /**
   * a (recursive) method to test if the given List Expression matches for the viewer.
   * It contains a recursive call, so that it checks nested relation of allowed types.
   * @param ListExpr to be tested
   * @param attr : true-> for attribute check / false -> atom or relation check
   * @return boolean wether it matches to the viewer
   */
  private boolean typeCheck(ListExpr LE, boolean attr){  

    if(LE.listLength()!=2){
      Reporter.showError("the list has wrong element numbers. It must have 2 elements. but it has " + LE.listLength() + " elements.");
      return false;
    }
    
    ListExpr type = attr?LE.second():LE.first(); // LE.secondo() is for attribute check
    
    if(!type.isAtom() && !type.isEmpty()){
      return typeCheck(type,false); // goes one level deeper. recursive call;
    }

    if(!(type.isAtom() && type.atomType() == ListExpr.SYMBOL_ATOM)){
      Reporter.showError("the " + (attr?"second":"first") + " element is not of type of Symbol atom");
      return false;
    }  
    
    String typeName = type.symbolValue();
    if(typeName.equals("rel") || typeName.equals("mrel") || typeName.equals("trel")) {
      return typeCheck(LE.second(),false); //recursive call
    }
    
    if(typeName.equals("tuple")){
      ListExpr values = LE.second();
      //check if one of the attributes has matched type.      
      while(!values.isEmpty()){
        if(typeCheck(values.first(),true)) return true; //recursive call for attribute check
        //one of the attribute matches for the type, it returns true
        values=values.rest();
      }
      return false;
    }

    // check for single atom type
    for(AllowedObjects ao: AllowedObjects.values()){
      if(typeName.equals(ao.toString()))return true;
    }
    return false;
  }

  /**
   * select a object in this viewer
   * @param SecondoObject o
   */
  public boolean selectObject(SecondoObject o){
    
    Iterator<ObjectGroup> it = itemsObject.iterator();
    ObjectGroup objGrp;
    int index = -1;
    while (it.hasNext()) {
      objGrp =it.next();
      if (o.equals(objGrp.getSecondoObject())) {
        index=itemsObject.indexOf(objGrp);
        if (index<0)
          return false;
        else{
          itemsNameBox.setSelectedIndex(index);
          objectGroup=objGrp;
          showObjectGroup();  
          return true;
          }
        }
      }
     return false;
   }
   
  /**
   * getMenuVector
   * @return menu extension for this viewer
   */
  public MenuVector getMenuVector(){
    return MenuExtension;
    }

  /**
   * getDisplayQuality
   * @return [0,1]
   *     0.. viewer can´t display this object
   *     1.. viewr is the best to display the given object
   */
  public double getDisplayQuality(SecondoObject so){
    if(canDisplay(so))
    return 1;  // because this is the only Viewer which can display 3D objects
    else
    return 0;
  }
  
  /**
   * showObjectGroup
   * shows the current object group from the selected Secondo Object
   */
  public void showObjectGroup() {
    
    boolean rootSceneExists = false;
    try{
      rootSceneExists=rootScene.equals(objectGroup.showObjectGroup());
    }catch (NullPointerException e) {}
    
    if (!rootSceneExists) {
      //set Menu
      if (!settingsMenu.isEnabled())
      settingsMenu.setEnabled(true);
      if (!appearanceMenu.isEnabled())
      appearanceMenu.setEnabled(true);
      if (!viewMenu.isEnabled())
      viewMenu.setEnabled(true);
      menuItem_varyingColorView.setEnabled(objectGroup.isRelation());
      colorChanged=true;                                      // set true in order to objectGroup.showObjectGroup() will be not invoked
      menuItem_varyingColorView.setSelected(objectGroup.getVaryingColorViewSelected());
      menuItem_GridView.setSelected(objectGroup.getDrawGrid());

      try{
        //set universe
        rootScene.detach();
        rootScene = objectGroup.showObjectGroup();
        ambLightBg.detach();
        ambLightBg = objectGroup.getAmLightBg();
        lights.detach();
        lights=objectGroup.getLights();
        universe.addBranchGraph(rootScene);
        universe.addBranchGraph(ambLightBg);
        universe.addBranchGraph(lights);
        } catch (NullPointerException e) {
          Reporter.writeError("error at adding branch groups");
          Reporter.writeError("Error in clearView= " + e.getStackTrace());
          Reporter.writeError("Error in clearView= " + e.getClass() + " - "
            + e.getMessage()+" - "+ e.getCause() );
       }

      // set Text view
      textWindow.clearView();
      if (objectGroup.isRelation())
      textWindow.setView(objectGroup.getTextViewItems());

      //set LightPanel
      try{
        configPane.removeTabAt(1);
        }catch (Exception E) {}
      configPane.addTab("Lighting", objectGroup.getLightingPanel());

      //set Scale Factor
      try{
        scaleFactorField.setValue(new Double(objectGroup.getScaleFactor()));
        }catch (NullPointerException e) {
          Reporter.writeError("error at setting scaleFactorField");
        }
    }
  }

  /**
   * clearView
   * removes objects from view
   */
  private void clearView() {
    currentObject=null;
    try{
      //clear universe
      rootScene.detach();
      ambLightBg.detach();
      lights.detach();
     }catch (Exception e) {
        Reporter.writeError("Error in clearView rootScene.detach");
        Reporter.writeError("Error in clearView= " + e.getStackTrace());
        Reporter.writeError("Error in clearView= " + e.getClass() + " - "
          + e.getMessage()+" - "+ e.getCause() );
    }
    //delete LightTab
    try{
      configPane.removeTabAt(1);
    }catch (Exception E) {}
    
    //set new Branch Groups
    rootScene = new BranchGroup();
    rootScene.setCapability(BranchGroup.ALLOW_DETACH);
    ambLightBg = new BranchGroup();
    ambLightBg.setCapability(BranchGroup.ALLOW_DETACH);
    lights = new BranchGroup();
    lights.setCapability(BranchGroup.ALLOW_DETACH);

    // set rest of view to standard
    textWindow.clearView();
    settingsMenu.setEnabled(false);
    appearanceMenu.setEnabled(false);
    viewMenu.setEnabled(false);
    }
    
  /**
   * resettingView
   * creates the current Object new and saves it in itemsObject
   */  
  private void resettingView() {
    //get Index in itemsObject
    Iterator<ObjectGroup> it = itemsObject.iterator();
    ObjectGroup objGrp;
    int index = -1;
    boolean hasFound = false;
    while (it.hasNext() && ! hasFound) {
      objGrp =it.next();
      if (objectGroup.equals(objGrp)) {
        hasFound = true;
        index=itemsObject.indexOf(objGrp);
        }
      }
    if (hasFound) {
      objectGroup = objectGroup = new ObjectGroup(currentObject, canvas, this);
      itemsObject.setElementAt(objectGroup, index);
      showObjectGroup();
    }
  }

  /**
   * showSelectedObject
   * select the current object which is choose from TextWindow
   * and shows it centered on view
   * @param index comes from TextWindow
   */
  public void showSelectedObject(int index) {   
    objectGroup.showSelectedObject(index);
    try{
      scaleFactorField.setValue(new Double(objectGroup.getScaleFactor()));
      }catch (NullPointerException e) {
    	  Reporter.writeError("Error at showSelectedObject");
      }
  }
  
  /**
   * setSelectedObject
   * set Object on TextView which was choose from 3D View. This 
   * Method is invoked from RelationPicking.java.
   * 
   * @param index comes from picked shape
   * @param True if no item should select. 
   *            This is when not item is picked on View
   */
  public void setSelectedObject(int index, boolean clear) {
    
    if ((!clear) && (index != -1)) {
      objectGroup.setCurrentShape(index);
      textWindow.setSelection(index);
    } 
    else{
      textWindow.clearSelection();
      objectGroup.setCurrentShape(-1);
    }
    textWindow.repaint();
  }

  
  /**
   * setObjectsColor
   * set the new color for all Objects
   */
  private void setObjectsColor() {
    objectGroup.changeObjectsColor(); 
  }
  
  /**
   * changeSinglePointSize
   * if it is possible it set Pointsize from current Object
   */
  public float changeSinglePointSize() {
    if (objectGroup.getCurrentShape().getAppearance().getPointAttributes() !=null) {
      setPointSizeDialog.setVisible(true, true);
      } else{
        Reporter.showInfo("Your Object isn't a Point. "+
          "This option is only available for Points!");
      }
    return objectGroup.getSinglePointSize();
  }
  
  /**
   * change size of a single point
   * @param size of point
   */
  public void changeSinglePointSize(float pntSize) {
    objectGroup.changeSinglePointSize(pntSize); 
  }
  
  /**
   * changeAllPointSize
   * set the size of Points in Relations
   * @param size of point
   */
  public void changeAllPointSize(float pntSize) {
    objectGroup.changeAllPointSize(pntSize);
  }

  /**
   * changeColor
   * public Method to change Color for current Shape
   */
  public void changeColor(Color col) {
    objectGroup.changeColor(col);
  }

  /**
   * createMenuExtension
   * create the extension from MenuVector
   */
  private void createMenuExtension() {
    
    //######### settings menu ##############################
    settingsMenu = new JMenu();
    settingsMenu.setText("Settings");
    
    // scale factor menu
    menuItem_Scale = new JMenuItem("Scale Factor");
    settingsMenu.add(menuItem_Scale);
    scaleFactorDialog = new ScaleFactorDialog(this);
    menuItem_Scale.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        scaleFactorDialog.setVisible(true);
        }
      });
    
    //show scale option in pane
    menuItem_ScaleView = new JCheckBoxMenuItem ("Show Scale option");
    settingsMenu.add(menuItem_ScaleView);
    menuItem_ScaleView.setSelected(showScaleMenu);
    menuItem_ScaleView.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent evt) {
        if (showScaleMenu != menuItem_ScaleView.isSelected()) {
          showScaleMenu=menuItem_ScaleView.isSelected();
          toggleScaleLabel(showScaleMenu);
          }
        }
      });
    
    // Zoom speed menu
    menuItem_Zoom = new JMenuItem("Zoom Speed");
    settingsMenu.add(menuItem_Zoom);
    zoomfactorDialog = new ZoomFactorDialog(this);
    menuItem_Zoom.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        zoomfactorDialog.setVisible(true);
        }
      });
    
    // Translate speed menu
   menuItem_Translate = new JMenuItem("Transl. Speed");
   settingsMenu.add(menuItem_Translate);
   translateFactorDialog = new TranslateFactorDialog(this);
    menuItem_Translate.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        translateFactorDialog.setVisible(true);
        }
      });
    
    // Rotate speed menu
    menuItem_Rotate = new JMenuItem("Rotate Speed");
    settingsMenu.add(menuItem_Rotate);
    rotateFactorDialog = new RotateFactorDialog(this);
    menuItem_Rotate.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        rotateFactorDialog.setVisible(true);
        }
      });
    
    //######## Appearance menu ###########################
    appearanceMenu = new JMenu();
    appearanceMenu.setText("Appearance");
    
    //Grid View menu
    menuItem_GridView = new JCheckBoxMenuItem ("Grid view");
    appearanceMenu.add(menuItem_GridView);
    menuItem_GridView.setSelected(objectGroup.getDrawGrid()); 
    menuItem_GridView.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent evt) {
        if (objectGroup.getDrawGrid() != menuItem_GridView.isSelected()) {
          objectGroup.setDrawGrid(menuItem_GridView.isSelected());
          }
        }
    });

    //varying Color View menu
    menuItem_varyingColorView = new JCheckBoxMenuItem ("varying colors (Rel)");
    appearanceMenu.add(menuItem_varyingColorView);
    menuItem_varyingColorView.setEnabled(objectGroup.isRelation());
    menuItem_varyingColorView.setSelected(objectGroup.getVaryingColorViewSelected());
    menuItem_varyingColorView.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent evt) {
        if (objectGroup.getVaryingColorViewSelected() != menuItem_varyingColorView.isSelected()) {
          if (colorChanged) {
            colorChanged=false;
          }else{
            objectGroup.setVaryingColorViewSelected(menuItem_varyingColorView.isSelected());
            objectGroup.changeObjectsColor();  
          }
         }
       }
     });
    
    //set point size menu
    menuItem_setPointSize = new JMenuItem("Point Size");
    appearanceMenu.add(menuItem_setPointSize);
    setPointSizeDialog = new SetPointSizeDialog(this);
    menuItem_setPointSize.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        setPointSizeDialog.setVisible(true, false);
        }
      }); 
    
    //set Colors menu...
    setColorsMenu = new JMenu("Set Colors");
    appearanceMenu.add(setColorsMenu);
    
    //set Background Color
    menuItem_setBackgroundColor = new JMenuItem("Background");
    setColorsMenu.add(menuItem_setBackgroundColor);
    menuItem_setBackgroundColor.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        objectGroup.changeBackgroundColor();
        }
      });
    
    //set Object Color
    menuItem_setObjectColor = new JMenuItem("Object");
    setColorsMenu.add(menuItem_setObjectColor);
    menuItem_setObjectColor.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        Color selColor = JColorChooser.showDialog(null,
          "Choose the object color!", objectGroup.getObjectColor().get());
        if (selColor !=null) {
          if (!selColor.equals(objectGroup.getObjectColor().get())) {
            objectGroup.setObjectColor(selColor);
            if (!objectGroup.getDrawGrid()) {
              colorChanged=true;
              menuItem_varyingColorView.setSelected(objectGroup.getVaryingColorViewSelected());
            }
          }
         }
        }
      });
    
    //set Grid Color
    menuItem_setGridColor = new JMenuItem("Grid");
    setColorsMenu.add(menuItem_setGridColor);
    menuItem_setGridColor.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        Color selColor = JColorChooser.showDialog(null,
          "Choose the grid color!", objectGroup.getGridColor().get());
        if (selColor !=null) {
          if (!selColor.equals(objectGroup.getGridColor().get())) {
            objectGroup.setGridColor(selColor);
            if (objectGroup.getDrawGrid()) {
              colorChanged=true;
              menuItem_varyingColorView.setSelected(objectGroup.getVaryingColorViewSelected());
            }
            }
          }
        }
      });
    
    //set default...
    setDefaultColorsMenu = new JMenu("Set Default Colors");
    appearanceMenu.add(setDefaultColorsMenu);
    
    //set Background Color default
    menuItem_setDefaultBackgroundColor= new JMenuItem("Background");
    setDefaultColorsMenu.add(menuItem_setDefaultBackgroundColor);
    menuItem_setDefaultBackgroundColor.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        objectGroup.setDefaultBackgroundColor();
        }
      });
    
    //set Object Color default
    menuItem_setDefaultObjectColor = new JMenuItem("Object");
    setDefaultColorsMenu.add(menuItem_setDefaultObjectColor);
    menuItem_setDefaultObjectColor.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        objectGroup.setDefaultObjectColor();
        if (!objectGroup.getDrawGrid()) {
          colorChanged=true;
          menuItem_varyingColorView.setSelected(objectGroup.getVaryingColorViewSelected());
          }                       
        }
      });
    
    //set Grid Color default
    menuItem_setDefaultGridColor = new JMenuItem("Grid");
    setDefaultColorsMenu.add(menuItem_setDefaultGridColor);
    menuItem_setDefaultGridColor.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        objectGroup.setDefaultGridColor();
        if (objectGroup.getDrawGrid()) {
          colorChanged=true;
          menuItem_varyingColorView.setSelected(objectGroup.getVaryingColorViewSelected());
          }
        }  
    });
    
    //set View Menu
    viewMenu= new JMenu("View");

    //set center view 
    menuItem_centerView = new JMenuItem("centering");
    viewMenu.add(menuItem_centerView);
    menuItem_centerView.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        objectGroup.centeringView(); 
        try{
          scaleFactorField.setValue(new Double(objectGroup.getScaleFactor()));
          }catch (NullPointerException e) {
            Reporter.writeError("Error at showSelectedObject");
          }
        }
      });

    //ser reset view
    menuItem_resetView = new JMenuItem("resetting");
    viewMenu.add(menuItem_resetView);
    menuItem_resetView.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        resettingView();
        }
      });
   
   
    // ########add menus to MenuExtension ###############
    MenuExtension.addMenu(settingsMenu);
    MenuExtension.addMenu(appearanceMenu);
    MenuExtension.addMenu(viewMenu);
    
  }// end createMenuExtension
  
  /**
   * toggleScaleLabel
   * add and remove Scale Label to the panel; If this Label is visible
   * it is much easier to change scale factor
   */
  private void toggleScaleLabel(boolean addLabel) { 
    if (addLabel) {
      scaleFactorField.setValue(new Double(objectGroup.getScaleFactor()));
      scaleFactorField.setColumns(4);
      scaleFactorField.addPropertyChangeListener("value", 
        new PropertyChangeListener()
        {
          @Override
          public void propertyChange(PropertyChangeEvent evt)
          {
            if (!objectGroup.getAutoScaleFactor()) {
              double factor=((Number)scaleFactorField.getValue()).doubleValue();
              if (factor>100.0) {
                factor=100;
                }
              if (factor<0.0001) {
                factor=0.0001;
                }
              objectGroup.setScaleFactor(factor, false);                         // and show object
            }  
          }
        });
        
      scaleFactorField.addMouseListener(new MouseAdapter()
        {
          public void mouseReleased(MouseEvent e) {
            objectGroup.setAutoScaleFactor(false);          
            }
        }); 

      boxPanel.add(scaleFactorLabel);
      boxPanel.add(scaleFactorField);
      boxPanel.revalidate();
      boxPanel.repaint();
      }
    else
    {
      try{
        boxPanel.remove(scaleFactorLabel);
        boxPanel.remove(scaleFactorField);
        boxPanel.revalidate();
        boxPanel.repaint();
      }
      catch (Exception e) {}
    }
  }   // end toggleScaleLabel
  
    
  /** getter and setter --------------------------------- */
  /** scaleFactor */
  public double getScaleFactor() {
    if(objectGroup==null) return 1.0;
    return objectGroup.getScaleFactor();
    }
  
  /**
   * to set factor for scaling
   * @param scale factor
   * @param weather auto or not
   */
  public void setScaleFactor(double factor, boolean setAuto) {
    if(objectGroup == null) return;
    objectGroup.setScaleFactor(factor, setAuto);
    try{
      scaleFactorField.setValue(new Double(objectGroup.getScaleFactor()));
      }catch (NullPointerException e) {
        Reporter.writeError("Error at setScaleFactor");
        }
    }

  /** 
   * getter zoomFactor
   * @return zoomfactor 
   * */
  public double getZoomFactor() {
    if(objectGroup==null)return 1.0;
    return objectGroup.getZoomFactor();
    }

  /**
   * to set factor of zooming
   * @param zooming factor
   */
  public void setZoomFactor(double factor) {
    if(objectGroup==null) return;
    objectGroup.setZoomFactor(factor);
    }

  /** 
   * getter translateFactor 
   * @return factor of translate
   * */
  public double getTranslateFactor() {
    if(objectGroup==null)return 1.0;
    return objectGroup.getTranslateFactor();
    }

  /**
   * to set factor of translate
   * @param translate factor to be set
   */
  public void setTranslateFactor(double factor) {
    if(objectGroup==null) return;
    objectGroup.setTranslateFactor(factor);
    }

  /** 
   * getter rotateFactor
   * @return rotate factor 
   */
  public double getRotateFactor() {
    if(objectGroup==null)return 1.0;
    return objectGroup.getRotateFactor();
    }
  
  /**
   * to set factor of rotation.
   * @param rotate factor to be set.
   */
  public void setRotateFactor(double factor) {
    if(objectGroup==null) return;
    objectGroup.setRotateFactor(factor);
    }

  /**
   * getter pointSize
   * @return size of point
   */
  public double getPointSize() {
    if(objectGroup==null) return 1.0;
    return objectGroup.getPointSize();
    }
  
  /**
   * to set size of single point
   * @param size to be set
   */
  public void setSinglePointSize(float sPntSize) {
    if(objectGroup==null) return;
    objectGroup.setSinglePointSize(sPntSize);
    }

  /**
   * getter ObjectColor
   * @return color of object
   */
  public Color3f getObjectColor() {
    if(objectGroup==null) return null;
    return objectGroup.getObjectColor();
    }

  /** 
   * getter CurrentShape
   * taken from Object group
   * @return CurrentShape  
   */
  public Shape3D getCurrentShape() {
    if(objectGroup==null)return null;
    return objectGroup.getCurrentShape();
    }

  /** 
   * getter TextWindow
   * @return Text Window 
   */
  public TextWindow getTextWindow() {
    return textWindow;
    }
    
   
  /** returns the MainFrame of application
   * needed for showing dialogs
   */
  public Frame getMainFrame(){
    if (VC!=null)
      return VC.getMainFrame();
    else
      return null;
  }
  
  /**
   * add an individual light
   */
  public void addLight(){
    LightBranchGroup lightBg = new LightBranchGroup(objectGroup.getLightingPanel());
    objectGroup.addLight(lightBg);
  }
  
  /**
   * return the light at index
   * @param index: index for selected light
   * @return returns the LightBranchGroup at the given index
   */
  public LightBranchGroup getLight(int index){
    return objectGroup.getLight(index);
  }
  
  /**
   * delete a light at index
   * @param index: index for selected light
   */
  public void delLight(int index){
    objectGroup.delLight(index);
  }
  
  /**
   * set lighting area of light at lindex with radius of "distance"
   * @param distance: radius of reaching sphere of the light
   * @param index: index for selected light
   */
  public void delAllLight(int indeces){
    for(int i=0; i<indeces; i++){
      delLight(0);
    }
  }

  /**
   * getter LightColor
   * @return color of light
   */
  public Color3f getLightColor(){
    return objectGroup.getLightColor();
  }
  
  /**
   * setter LightColor
   * @param color to be set
   */
  public void setLightColor(Color color){
    objectGroup.setLightColor(color);
  }
  
  /**
   * setter ambLight
   * @param on/off
   */
  public void setMainLightSwitch(boolean sw){
      objectGroup.setMainLightSwitch(sw);
  }
  
  /**
   * getter on/off ambLight
   * @return if light is on
   */
  public boolean getMainLightSwitch(){
    return objectGroup.getMainLightSwitch();
  }
  
  /**
   * to activate mouse movement only selected light at the index
   * @param lindex of light
   */
  public void setLightMouseMove(int index){
    objectGroup.setLightMouseMove(index);
 }
  
  /**
   * getter for light panel
   * @return light panel
   */
  public LightPanel getLightPanel(){
    return objectGroup.getLightingPanel();
  }
  
  /**
   * to add Lighting Panel
   * @param obsolete dummy 
   */
  public void addLightingPanel(LightPanel tab) {
    try{
      configPane.removeTabAt(1);
    }catch (Exception E) {}
    configPane.addTab("Lighting", objectGroup.getLightingPanel());
  }
 
}
