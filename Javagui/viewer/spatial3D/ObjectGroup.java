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

package viewer.spatial3D;

import gui.SecondoObject;
import sj.lang.*;
import tools.Reporter;
import viewer.Spatial3DViewer;

import java.awt.Color;
import java.util.Iterator;
import java.util.Vector;

import javax.media.j3d.*;
import javax.swing.JColorChooser;
import javax.vecmath.Color3f;
import javax.vecmath.Matrix3f;
import javax.vecmath.Matrix4f;
import javax.vecmath.Point3d;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import com.sun.j3d.utils.geometry.GeometryInfo;
import com.sun.j3d.utils.geometry.NormalGenerator;

/**
 * This Class represents a object which is shown in the
 * Spatial3DViewer
 */
public class ObjectGroup {

  // Mouse options
  private CustomPickZoom pickZoom = null;
  private CustomPickMouseWheelZoom wheelZoom = null;
  private CustomPickTranslate pickTranslate = null;
  private CustomPickRotate pickRotate = null;
  private RelationPicking relationPicking = null;

  private boolean isSinglePoint;
  private boolean isRelation;
  private boolean isVaryingColorViewSelected;
  private int currentTupleLength;
  private boolean drawGrid;
  private boolean isUndefined;
  private boolean isEmpty;

  private double scaleFactor = 0.05;
  private double zoomFactor = 1;
  private double translateFactor = 0.1;
  private double rotateFactor = 0.03;
  private boolean autoScaleFactor = true;
  private float pointSize = 5.0f;
  private float singlePointSize;

  private SecondoObject secondoObject;
  private ListExpr currentType;
  private ListExpr currentObjectList;
  private Shape3D currentShape;
  private String currentTypeString;
  private Spatial3DViewer viewer;

  private Background background;
  private Canvas3D canvas;
  private BranchGroup rootScene;
  private TransformGroup objectTransGrp;
  private TransformGroup scaleTransGrp;
  private TransformGroup translateTransGrp;
  
  private Point3d standardCenterTFG = new Point3d();
  private double standardRadTFG;
  
  
  /** textViewItems saves each elements(shapes and non shapes) and cut lines */
  private Vector<String> textViewItems = new Vector<String>();
  /** shapeVector saves each elements, if it is a non shape the value is null */
  private Vector<Vector<Shape3D>> shapeVector = new Vector<Vector<Shape3D>>();
  /** objectNameVector saves the names of all shapes */
  private Vector<String> objectNameVector = new Vector<String>();
  /** objectAppearanceVector saves all Appearances from the Shapes */
  private Vector<Appearance> objectAppearanceVector = new Vector<Appearance>();
  /** transform group vector for all transformgroups */
  private Vector <TransformGroup> tfgVector = new Vector<TransformGroup>();

  // Colors
  /* standard Background color is white */
  private static final Color3f DEFAULT_BACKGROUND_COLOR = new Color3f(1.0f,
  1.0f, 1.0f);
  /* standard Grid color is black */
  private static final Color3f DEFAULT_GRID_COLOR = new Color3f(0.0f, 0.0f,
  0.0f);
  /* standard Object color is grey */
  private static final Color3f DEFAULT_OBJECT_COLOR = new Color3f(0.23f,
  0.248f, 0.243f);
  private Color3f backgroundColor = new Color3f(1.0f, 1.0f, 1.0f);
  private Color3f gridColor = new Color3f(0.0f, 0.0f, 0.0f);
  private Color3f objectColor = new Color3f(0.23f, 0.248f, 0.243f);
  private Color firstSelCol;
  private Color secSelCol;
  
  // lighting
  private LightPanel lightingPanel;
  private BranchGroup lights;
  private BranchGroup ambLightBg;
  private AmbientLight ambLight;
  private Color3f lastColor;
  
  /**
   * Constructor 
   * create the 3D object from the given SecondoObject which is shown
   * in Spatial3DViewer
   * 
   * @param o   SecondoObject
   * @param c   Canvas3D
   * @param v   Spatial3DViewer
   */
  public ObjectGroup(SecondoObject o, Canvas3D c, Spatial3DViewer v) {
    secondoObject = o;
    canvas = c;
    viewer =v;
    if (secondoObject !=null) {
      // get list and type
      currentObjectList = secondoObject.toListExpr();
      currentType = currentObjectList.first();
      if (!currentType.isAtom()) {
        currentType = currentType.first();                    // type is a relation
        }
      currentTypeString = currentType.symbolValue();

      // create rootScene and Transformgroup
      rootScene = new BranchGroup();
      rootScene.setCapability(BranchGroup.ALLOW_DETACH);

      objectTransGrp = new TransformGroup();
      objectTransGrp.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
      objectTransGrp.setCapability(TransformGroup.ALLOW_TRANSFORM_READ);
      objectTransGrp.setCapability(TransformGroup.ENABLE_PICK_REPORTING);

      scaleTransGrp = new TransformGroup();
      scaleTransGrp.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
      scaleTransGrp.setCapability(TransformGroup.ALLOW_TRANSFORM_READ);

      translateTransGrp = new TransformGroup();
      translateTransGrp.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
      translateTransGrp.setCapability(TransformGroup.ALLOW_TRANSFORM_READ);

      // Initializes vectors
      shapeVector.clear();
      textViewItems.clear();
      objectNameVector.clear();
      objectAppearanceVector.clear();
      tfgVector.clear();

      // create Appearance
      Appearance app = createAppearance();

      // create Object
      switch (currentTypeString) {
        // point3d
        case "point3d":
        case "vector3d":
        isSinglePoint = true;
        currentShape = createPoint(app, currentObjectList.second());
          break;
        // surface3d volume3d
        case "surface3d":
        case "volume3d":
        	
        currentShape = createVolume(app, currentObjectList.second());
          break;

        // relation
        case "rel":
        case "trel":
        case "mrel":
        isRelation = true;
        createRelationObjects(currentObjectList);
          break;
        
        default:return;
       }

      if (!isRelation) {                                    // it is a single object
        objectTransGrp.addChild(currentShape);
        objectNameVector.add(new String(currentTypeString.toString()));
        objectAppearanceVector.add(app);
        objectTransGrp.setCapability(TransformGroup.ALLOW_TRANSFORM_READ);
        objectTransGrp.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
        tfgVector.add(objectTransGrp);
      }
      
      // set lighting
      lightingPanel = new LightPanel(viewer);
      viewer.addLightingPanel(lightingPanel);

      lights = new BranchGroup();
      lights.setCapability(BranchGroup.ALLOW_DETACH);
      lights.setCapability(Group.ALLOW_CHILDREN_READ);
      lights.setCapability(Group.ALLOW_CHILDREN_WRITE);
      lights.setCapability(Group.ALLOW_CHILDREN_EXTEND);
      lastColor = new Color3f(1.0f, 1.0f, 1.0f);

      // set up ambient light
      ambLight = new AmbientLight();
      BoundingSphere ambBounds = new BoundingSphere(new Point3d(0.0,0.0,0.0), Double.MAX_VALUE);
      ambLight.setInfluencingBounds(ambBounds);
      ambLight.setEnable(true);
      ambLight.setCapability(Light.ALLOW_COLOR_READ);
      ambLight.setCapability(Light.ALLOW_COLOR_WRITE);
      ambLight.setCapability(Light.ALLOW_STATE_READ);
      ambLight.setCapability(Light.ALLOW_STATE_WRITE);
      ambLightBg = new BranchGroup();
      ambLightBg.setCapability(BranchGroup.ALLOW_DETACH);
      ambLightBg.addChild(ambLight);

      //set View
      updateView(addMouseBehavior(calcBounds(objectTransGrp)));
      
      //set Satandard TFG values
      Bounds tfgBounds = translateTransGrp.getBounds();
      BoundingSphere tfgSphere = new BoundingSphere(tfgBounds);
      tfgSphere.getCenter(standardCenterTFG);
      standardRadTFG = tfgSphere.getRadius();

   }
  }   // end constructor

  /**
   * showObjectGroup This method shows the created Objects on the 3D View. It
   * where called if the objects where already created.
   * 
   * @return root TransformGroup which contain the 3d Object(s)
   */
  public BranchGroup showObjectGroup() {
     return rootScene;
    }

  /**
   * createPoint 
   * creates a point with a given Appearance
   * 
   * @param app Appearance
   * @param value List Expression
   * @return the created Point as a Shape
   */
  private Shape3D createPoint(Appearance app, ListExpr value) {

    //check if Object is defined
    if (value.listLength()<0){
      isUndefined=true;
      return null;
    } 
    
    //check if Object is emty
    if ((value.listLength()==0)) {
      isEmpty=true;
      return null;
    }
      
    float x = (float) value.first().realValue();
    float y = (float) value.second().realValue();
    float z = (float) value.third().realValue();
    Point3f pntCoord[] = new Point3f[1];
    pntCoord[0] = new Point3f(x, y, z);

    // create point3d
    PointArray pnt = new PointArray(1, GeometryArray.COORDINATES);
    pnt.setCapability(GeometryArray.ALLOW_COORDINATE_READ);
    pnt.setCoordinates(0, pntCoord);

    // set standard size of point to 5.0f pixels
    PointAttributes pntAttr = new PointAttributes(pointSize, true);
    pntAttr.setCapability(PointAttributes.ALLOW_SIZE_WRITE);
    pntAttr.setCapability(PointAttributes.ALLOW_SIZE_READ);
    app.setPointAttributes(pntAttr);

    // create point shape
    Shape3D pntShape = new Shape3D(pnt, app);
    pntShape.setCapability(Shape3D.ALLOW_APPEARANCE_OVERRIDE_READ);
    pntShape.setCapability(Shape3D.ALLOW_APPEARANCE_OVERRIDE_WRITE);
    pntShape.setCapability(Shape3D.ALLOW_APPEARANCE_READ);
    pntShape.setCapability(Shape3D.ALLOW_APPEARANCE_WRITE);

    return pntShape;
    }

  /**
   * createVolume 
   * creates a 3D Volume or a Surface
   * 
   * @param app Appearance
   * @param value List Expression from SecondoObject or Relation tuple
   * @return volume or surface as a shape
   */
  private Shape3D createVolume(Appearance app, ListExpr value) {
    // value=( ((xyz)(xyz)(xyz)) ((xyz)(xyz)(xyz)) ((xyz)(xyz)(xyz)) )

    //check if Object is defined
    if (value.listLength()<0){
      isUndefined=true;
      return null;
    } 
    
    //check if Object is emty
    if ((value.listLength()==0)) {
      isEmpty=true;
      return null;
    }

    Point3f[] volumePnts = new Point3f[value.listLength() * 3];
    int[] volumeIndices = new int[value.listLength() * 3];

    float x, y, z;
    ListExpr firstTriangle;
    ListExpr[] pnt = new ListExpr[3];
    ListExpr restTriangles = value;

    int length = value.listLength();
    for (int i = 0; i < length; i++) {
      firstTriangle = restTriangles.first();
      restTriangles = restTriangles.rest();

      pnt[0] = firstTriangle.first();
      pnt[1] = firstTriangle.second();
      pnt[2] = firstTriangle.third();

      for (int j = 0; j < 3; j++) {
        x = (float) pnt[j].first().realValue();
        y = (float) pnt[j].second().realValue();
        z = (float) pnt[j].third().realValue();

        volumePnts[(i * 3) + j] = new Point3f(x, y, z);
        volumeIndices[(i * 3) + j] = (i * 3) + j;
        }
      }

    GeometryInfo gi = new GeometryInfo(GeometryInfo.TRIANGLE_ARRAY);
    gi.setCoordinates(volumePnts);
    gi.setCoordinateIndices(volumeIndices);
    NormalGenerator ng = new NormalGenerator();
    ng.generateNormals(gi);
    GeometryArray te = gi.getGeometryArray();
    te.setCapability(GeometryArray.ALLOW_COORDINATE_READ);

    // create shape
    Shape3D volume = new Shape3D(te, app);
    volume.setCapability(Shape3D.ALLOW_APPEARANCE_OVERRIDE_READ);
    volume.setCapability(Shape3D.ALLOW_APPEARANCE_OVERRIDE_WRITE);
    volume.setCapability(Shape3D.ALLOW_APPEARANCE_READ);
    volume.setCapability(Shape3D.ALLOW_APPEARANCE_WRITE);

    return volume;
    }

  /**
   * createRelationObjects 
   * create all Objects form Relation. This Method also
   * create the TextView in TextWindow
   * 
   * @param list List Expression from secondo Object
   */
  private void createRelationObjects(ListExpr list) {
	  
    // set varying color menu unselected
    isVaryingColorViewSelected = false;
    // create a cutting line string
    String cutLine = "----------";

    // Analyze Relation
    if (list.listLength() != 2) {
    	Reporter.showError("List Expression is not Correct ");
      } else {
        ListExpr type = list.first();
        ListExpr value = list.second();
        // Analyze type
        ListExpr maintype = type.first();
        if (type.listLength() != 2
          || !maintype.isAtom()
          || maintype.atomType() != ListExpr.SYMBOL_ATOM
          || !(maintype.symbolValue().equals("rel")
            || maintype.symbolValue().equals("mrel") || maintype
            .symbolValue().equals("trel"))) {
              Reporter.showError("List Expression is not a Relation ");
              }
        ListExpr tupletype = type.second();
        // Analyze Tuple
        ListExpr TupleFirst = tupletype.first();
        if (tupletype.listLength() != 2
          || !TupleFirst.isAtom()
          || TupleFirst.atomType() != ListExpr.SYMBOL_ATOM
          || !(TupleFirst.symbolValue().equals("tuple") || TupleFirst
            .symbolValue().equals("mtuple"))) {
              Reporter.showError("List Expression is not a Tuple ");
              }
        // Analyze Types of objects
        ListExpr TupleTypeValue = tupletype.second();
  	  
        String[] objectTypes = new String[TupleTypeValue.listLength()];
        String[] objectNames = new String[TupleTypeValue.listLength()];
        for (int i = 0; !TupleTypeValue.isEmpty(); i++) {
          ListExpr TupleSubType = TupleTypeValue.first();
          objectNames[i] = TupleSubType.first().stringValue();
          objectTypes[i] = TupleSubType.second().stringValue();
          TupleTypeValue = TupleTypeValue.rest();
          }
        // Analyze objects values
        ListExpr valRow;
        // group for
        TransformGroup relationGroup = new TransformGroup();
        // relation

        int length = value.listLength();
        currentTupleLength = value.first().listLength();
        for (int i = 0; i < length; i++) {
          valRow = value.first();
         
          // Transform group for each tuple
          TransformGroup tupleGroup = new TransformGroup();
          // shape vector for each tuple
          Vector<Shape3D> shapesOfTupleVector = new Vector<Shape3D>();

          for (int j = 0; !valRow.isEmpty(); j++) {
            TransformGroup elemGroup = new TransformGroup();
            Shape3D elem = null;
            Appearance app = createAppearance();
            
            // get the type from String[]
            switch (objectTypes[j]) {
              case "point3d":
              case "vector3d":
              elem = createPoint(app, valRow.first());
                break;
              case "surface3d":
              case "volume3d":
              elem = createVolume(app, valRow.first());
                break;
              default:
              elem = null;
            }
            
            if (elem != null) {
              elemGroup.addChild(elem);
              tupleGroup.addChild(elemGroup);
              objectAppearanceVector.add(app);
              objectNameVector.add(new String(objectTypes[j]
                .toString()));
              setTextView(objectNames[j] + ": " + objectTypes[j],
                null);
              elemGroup.setCapability(TransformGroup.ALLOW_TRANSFORM_READ);
              elemGroup.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
              tfgVector.add(elemGroup);
              int index = i * (currentTupleLength + 1) + j;
              elem.setUserData(index);
            } else {
              setTextView(objectNames[j], valRow.first());
            }

            shapesOfTupleVector.add(elem);
            valRow = valRow.rest();
           }

          shapeVector.add(shapesOfTupleVector);
          setTextView(cutLine, null);
          relationGroup.addChild(tupleGroup);
          value = value.rest();
          }
        objectTransGrp.addChild(relationGroup);
        }
    }

  /**
   * setTextView 
   * creates the text view wich is shown in Spatial3DViewer 
   * fo Relations.
   * Each time it is invoked it put an item to the TextWindow.
   * 
   * @param objectName
   * @param objectValue
   */
  private void setTextView(String objectName, ListExpr objectValue) {
    if (objectValue != null) {
      String value = objectValue.writeListExprToString();
      value = value.replaceAll("\\r\\n|\\r|\\n", "");
      textViewItems.add(objectName + ": " + value);
      } else {
        textViewItems.add(objectName);
        }
    }

  /**
   * createAppearance 
   * create a standard appearance for the 3D object. In this Appearance
   * the last selected color is set. Also the distinction between 
   * grid view and full view is made.
   * 
   * @return new Appearance
   */
  private Appearance createAppearance() {
    Appearance app = new Appearance();
    PolygonAttributes polygAttr = new PolygonAttributes();
    polygAttr.setCullFace(PolygonAttributes.CULL_NONE);
    
    if (drawGrid) {
      setDefaultAppearance(app, gridColor);
//       PolygonAttributes polygAttr = new PolygonAttributes();
      polygAttr.setPolygonMode(PolygonAttributes.POLYGON_LINE);
//       app.setPolygonAttributes(polygAttr);
      } else {
        setDefaultAppearance(app, objectColor);
//         PolygonAttributes polygAttr = new PolygonAttributes();
//         polygAttr.setCullFace(PolygonAttributes.CULL_NONE);
//         app.setPolygonAttributes(polygAttr);
        }
    app.setPolygonAttributes(polygAttr);
    return app;
    }

/**
 * setDefaultAppearance 
 * This Mehtod is invoked from createAppearance() and creates a
 * default appearance.
 */

  private void setDefaultAppearance(Appearance app, Color3f col) {
    // set Material
    Material appMaterial = new Material(col, col, col, col, 120.0f);
    appMaterial.setCapability(Material.ALLOW_COMPONENT_READ);
    appMaterial.setCapability(Material.ALLOW_COMPONENT_WRITE);
    app.setMaterial(appMaterial);

    // set coloring attribute
    ColoringAttributes colAttr = new ColoringAttributes();
    colAttr.setCapability(ColoringAttributes.ALLOW_COLOR_WRITE);
    colAttr.setColor(col);
    app.setColoringAttributes(colAttr);

    // set Capability from Appearance
    app.setCapability(Appearance.ALLOW_COLORING_ATTRIBUTES_READ);
    app.setCapability(Appearance.ALLOW_COLORING_ATTRIBUTES_WRITE);
    app.setCapability(Appearance.ALLOW_MATERIAL_READ);
    app.setCapability(Appearance.ALLOW_MATERIAL_WRITE);
    app.setCapability(Appearance.ALLOW_POINT_ATTRIBUTES_READ);
    app.setCapability(Appearance.ALLOW_POINT_ATTRIBUTES_WRITE);
    app.setCapability(Appearance.ALLOW_POLYGON_ATTRIBUTES_READ);
    app.setCapability(Appearance.ALLOW_POLYGON_ATTRIBUTES_WRITE);
    }

  /**
   * changeObjectsColor 
   * change the color of all objects stored in this object group
   */
  public void changeObjectsColor() {
    int size = objectAppearanceVector.size();
    Iterator<Appearance> it = objectAppearanceVector.iterator();
    Appearance app;

    if (isVaryingColorViewSelected && isRelation) {
      Color3f[] varCol = calcVaryingColor(size);
      for (int i = 0; i < size; i++) {
        app = (Appearance) it.next();
        changeColor(app, varCol[i]);
        }
      } else {
        // set the standard color
        for (int i = 0; i < size; i++) {
          app = (Appearance) it.next();
          if (drawGrid)
          changeColor(app, gridColor);
          else
          changeColor(app, objectColor);
      }
    }
  }

  /**
  * calcVaryingColor 
  * determines the color of all objects. 
  * The colors approximate between two selected colors.
  */
  private Color3f[] calcVaryingColor(int size) {

	  Color3f[] newColor = new Color3f[size];
    
    // choose colors
     firstSelCol = JColorChooser.showDialog(null,
      "Choose first Color!", objectColor.get());
     secSelCol = JColorChooser.showDialog(null,
      "Choose second Color!", objectColor.get());
      
    //calc new colors in HSB color Space    
    float [] hsbFirst = Color.RGBtoHSB(firstSelCol.getRed(),
      firstSelCol.getGreen(), firstSelCol.getBlue(), null);
    float [] hsbSecond = Color.RGBtoHSB(secSelCol.getRed(),
      secSelCol.getGreen(), secSelCol.getBlue(), null);
    // calc hue
    float minHue = Math.min(hsbFirst[0], hsbSecond[0]);
    float maxHue = Math.max(hsbFirst[0], hsbSecond[0]);
    double offsetHue = (double)(maxHue - minHue)/(double)size;
    //calc saturation
    float minSat = Math.min(hsbFirst[1], hsbSecond[1]);
    float maxSat = Math.max(hsbFirst[1], hsbSecond[1]);
    double offsetSat = (double)(maxSat - minSat)/(double)size;
    // calc brightness
    float minBright = Math.min(hsbFirst[2], hsbSecond[2]);
    float maxBright = Math.max(hsbFirst[2], hsbSecond[2]);
    double offsetBright = (double)(minBright - maxBright)/(double)size;
    
    double newHue = minHue;
    double newSat = minSat;
    double newBright = minBright;
    int rgb;
    
    for (int i = 0; i < size; i++) {
      newHue=minHue+offsetHue*(double)i;
      newSat=minSat+offsetSat*(double)i;
      newBright=minBright+offsetBright*(double)i;
      
      rgb = Color.HSBtoRGB((float)newHue, (float)newSat, (float)newBright);
      
      newColor[i] = new Color3f(new Color((rgb>>16)&0xFF, (rgb>>8)&0xFF,
        rgb&0xFF));
    }

    return newColor;
   
    
    }


  /**
   * changeColor 
   * change Color from current Shape in the passed color
   * 
   * @param col Color which was chosen
   */
  public void changeColor(Color col) {
    Appearance app = currentShape.getAppearance();
    Color3f color = new Color3f();
    color.set(col);
    changeColor(app, color);
    }

  /**
   * changeColor 
   * changes the color of the passed appearance in the passed color
   * 
   * @param app in this appearance, the color is changed
   * @param col this is the chosen color
   */
  private void changeColor(Appearance app, Color3f col) {
    Material mat = app.getMaterial();
    mat.setAmbientColor(col);
    mat.setEmissiveColor(col);
    mat.setDiffuseColor(col);
    mat.setSpecularColor(col);
    ColoringAttributes colAttr = app.getColoringAttributes();
    colAttr.setColor(col);
    PolygonAttributes polygAttr = new PolygonAttributes();
    polygAttr.setCullFace(PolygonAttributes.CULL_NONE);
    if (drawGrid) {
      polygAttr.setPolygonMode(PolygonAttributes.POLYGON_LINE);
      app.setPolygonAttributes(polygAttr);
      } else {
        polygAttr.setPolygonMode(PolygonAttributes.POLYGON_FILL);
        app.setPolygonAttributes(polygAttr);
        }
    }

  /**
   * changeBackgroundColor 
   * opens a color dialog and set Background color tho the given color
   * in the global variable backgroundColor
   */
  public void changeBackgroundColor() {
    Color selColor = JColorChooser.showDialog(null,
      "Choose the background color!", backgroundColor.get());
    if (selColor != null) {
      if (!selColor.equals(backgroundColor.get())) {
        backgroundColor.set(selColor);
        background.setColor(backgroundColor);
        }
      }
    }

  /**
   * changeSinglePointSize 
   * change point size from the current Shape,  which is a point
   * 
   * @param pntSize is the selected pointSize
   */
  public void changeSinglePointSize(float pntSize) {
    PointAttributes pntAtr = currentShape.getAppearance()
    .getPointAttributes();
    if (pntAtr != null) {
      pntAtr.setPointSize(pntSize);
      }
    }

  /**
   * changeAllPointSize 
   * changes every point size in the selected pointSize
   * 
   * @param pntSize is the selected pointSize
   */
  public void changeAllPointSize(float pntSize) {
    pointSize = pntSize;
    int size = objectAppearanceVector.size();
    Iterator<Appearance> objAppIt = objectAppearanceVector.iterator();
    Iterator<String> objNameIt = objectNameVector.iterator();
    Appearance app;
    String name;

    for (int i = 0; i < size; i++) {
      app = (Appearance) objAppIt.next();
      name = (String) objNameIt.next();
      // set Pointsize
      if (name.equals("point3d"))
      app.getPointAttributes().setPointSize(pntSize);
      }
    }

  /**
   * changeScaleFactor 
   * change the scale factor for the current object group by
   * manipulating the scaleTransformGroup. This TransformGroup is not
   * used by any other method or Transforming operations,  therefore here 
   * is the scaleFactor the only factor which is changed.
   */
  private void changeScaleFactor() {
    Transform3D scale = new Transform3D();
    scale.setScale(scaleFactor);
    scaleTransGrp.setTransform(scale);
    }

  /**
   * addMouseBehavior 
   * adds the Mouse Behaviors to the object 
   * (translate, rotate, zoom,  pick)
   * 
   * @param object TransformGroup
   * @return object the same TransformGroup which was passed with 
   * mouse behaviors
   */
  private TransformGroup addMouseBehavior(TransformGroup object) {
    BoundingSphere behaveBounds = new BoundingSphere(new Point3d(0.0, 0.0,
      0.0), Double.MAX_VALUE);

    // Rotation
    pickRotate = new CustomPickRotate(rootScene, canvas, behaveBounds);
    object.addChild(pickRotate);
        
    // Translate
    pickTranslate = new CustomPickTranslate(rootScene, canvas,
      behaveBounds);
    object.addChild(pickTranslate);

    // Zoom
    pickZoom = new CustomPickZoom(rootScene, canvas, behaveBounds);
    object.addChild(pickZoom);

    // WheelZoom
    wheelZoom = new CustomPickMouseWheelZoom(rootScene, canvas,
      behaveBounds);
    object.addChild(wheelZoom);

    // RelationPick
    if (isRelation) {
      relationPicking = new RelationPicking( viewer, rootScene,
        canvas, behaveBounds);
      object.addChild(relationPicking);
    }
    
    return object;  
    }

  /**
   * calcBounds 
   * calculates the Bounds of the TransformGroup. Then the method 
   * scale the passed TransformGroup in the View automatically.
   * 
   * @param group
   * @return translateTransGrp a new TransformGroup which 
   * contains the passed TransformGroup group
   */
  private TransformGroup calcBounds(TransformGroup group) {
    Bounds tfGoupBounds = group.getBounds();
    BoundingSphere sphere = new BoundingSphere(tfGoupBounds);
    translateTransGrp.setTransform(calcAutoTransform(sphere, !isRelation));
    translateTransGrp.addChild(group);
    
   
    // set scaleFactor automatically
    if (autoScaleFactor && !isSinglePoint) {
      scaleFactor = calcAutoScaleFactor(sphere);
      } else {
        if (autoScaleFactor && isSinglePoint)
        scaleFactor = 0.1;
        }
    return translateTransGrp;
    }

  /**
   * calcAutoTransform 
   * Calculate a Transform3D objet to positions the TransformGroup, which 
   * contains the Object(s), in the center of the view.
   * 
   * @param sphere BoundingSphere from TransformGroup which contains the object(s)
   * @param boolean is True if the method should also calculate in Z direction
   * @return posMiddle Transform3D which centers the TransformGroup
   */
  private Transform3D calcAutoTransform(BoundingSphere sphere, boolean calcZ) {
    // put TF group in the middle of view
    Point3d tfCenter = new Point3d();           // get the center coordinates of
    // sphere
    sphere.getCenter(tfCenter);
    double[] coord = new double[3];
    tfCenter.get(coord);
    float zOffset;

    float xOffset = (float) (coord[0] * -1.0);  // calc offset of coordinates
    float yOffset = (float) (coord[1] * -1.0);
    if (calcZ)
      zOffset = (float) ((coord[2]+4.0f) * -1.0 );
    else
      zOffset = 0.0f;                                     

    Transform3D posMiddle = new Transform3D();  // put object in the middle
    posMiddle.setTranslation(new Vector3f(xOffset, yOffset, zOffset));

    return posMiddle;

    }
  

  /**
   * calcAutoScaleFactor 
   * calculate automatically the scale factor to fit
   * objects into canvas
   * 
   * @param sphere BoundingSphere from Object which should center in the view
   * @return factor is the scale factor
   */
  private double calcAutoScaleFactor(BoundingSphere sphere) {
    double factor;
    factor= 1.0/sphere.getRadius();
    if (factor > 1.0) {
      factor = 1.0;
      }
    if (factor < 0.0001) {
      factor = 0.0001;
      }
    return factor;
    }

  /**
   * updateView 
   * put all transformgroups tho the scene graph
   * 
   * @param group
   */
  private void updateView(TransformGroup group) {
    
    // set Background color
    BoundingSphere bounds = new BoundingSphere(new Point3d(0.0, 0.0, 0.0),
      Double.MAX_VALUE);
    background = new Background(backgroundColor);
    background.setApplicationBounds(bounds);
    background.setCapability(Background.ALLOW_COLOR_WRITE);
    rootScene.addChild(background);

    try {
      pickZoom.setFactor(zoomFactor);     // standard is 0.04
      wheelZoom.setFactor(zoomFactor);    // standard is 0.1
      // standard is 0.2
      pickTranslate.setFactor(translateFactor);
      pickRotate.setFactor(rotateFactor); // standard is 0.03
      } catch (NullPointerException e) {
        }

    // set Scale factor
    Transform3D scaleTrans = new Transform3D(); // set scaleFactor to scale
    // Transform3D
    scaleTrans.setScale(scaleFactor);
    scaleTransGrp.setTransform(scaleTrans);

    // put Object 4 meters away
    Transform3D transform = new Transform3D();
    transform.setTranslation(new Vector3f(0.0f, 0.0f, -4.0f));
    TransformGroup trans = new TransformGroup(transform);

    // add children
    scaleTransGrp.addChild(group);
    trans.addChild(scaleTransGrp);
    rootScene.addChild(trans);
    // compile Scene
    rootScene.compile();

    }

  /**
   * showSelectedObject 
   * select Object which was choose from TextView. The current 
   * object will be rotated back and centered
   * 
   * @param index from selected object
   */
  public void showSelectedObject(int index) {
    setCurrentShape(index);
    if (currentShape != null) {
      Bounds shapeBound = currentShape.getBounds();
      BoundingSphere shapeSphere = new BoundingSphere(shapeBound);
      //if it is not point set scale Factor
      if (! (currentShape.getGeometry() instanceof PointArray)) {
          setScaleFactor((calcAutoScaleFactor(shapeSphere)), false);
      }
      // set rotation
      rotateRelationBack();

      //calc center position
      Transform3D transTFGTransform = new Transform3D();
      translateTransGrp.getTransform(transTFGTransform);
      calcCenterPosition(currentShape, translateTransGrp, transTFGTransform);
      translateTransGrp.setTransform(transTFGTransform);
    }
  }
  
  /**
   * rotateRelationBack
   * brings the relation with respect to rotation in starting position
   */
  private void rotateRelationBack() {
    Matrix3f rotationTFG = new Matrix3f();
    //get rotation from shape
    rotationTFG= pickRotate.getRotation();
    rotationTFG.invert();
    //rotate each shape in Relation
    Iterator<TransformGroup> tfgIt = tfgVector.iterator();
    Shape3D shape;
    while (tfgIt.hasNext()) {
      Transform3D currTF = new Transform3D();
      TransformGroup grp =tfgIt.next();    
      grp.getTransform(currTF);
      currTF.setRotation(rotationTFG);
      grp.setTransform(currTF);
    }
  }
  
  /**
   * calcCenterPosition
   * calculate from a given object in a Relation the position
   * and centered it in the display
   * 
   * @param shape is the object which should be centered
   * @param group is the TransformGroup which contains all objects
   * @param transform is the Transform3D object which should be transformed
   */
  private void calcCenterPosition(Shape3D shape,  TransformGroup group,  Transform3D transform) {
    group.getTransform(transform);
    //get shape position
    Bounds shapeBound = shape.getBounds();
    BoundingSphere shapeSphere = new BoundingSphere(shapeBound);
    double radiusShape = shapeSphere.getRadius() ;
    Point3d centerShape = new Point3d();
    shapeSphere.getCenter(centerShape);
    
    //get transform group position
    Bounds groupBound = group.getBounds();
    BoundingSphere groupSphere = new BoundingSphere(groupBound);
    double radiusGroup = groupSphere.getRadius() ;
    Point3d centerGroup = new Point3d();
    groupSphere.getCenter(centerGroup);
    
    double[] centerShapeCoord = new double[3];
    double[] centerGroupCoord = new double[3];
    double[] centerStandTFGCoord = new double[3];
    
    centerShape.get(centerShapeCoord);
    centerGroup.get(centerGroupCoord);
    standardCenterTFG.get(centerStandTFGCoord);
    float[] newCoord = new float[3];
    for (int i=0; i<3; i++) {
      newCoord[i]=(float) (centerShapeCoord[i]+centerGroupCoord[i]-centerStandTFGCoord[i])*-1.0f;
    }
    transform.setTranslation(new Vector3f(newCoord));
  }
 
  /**
   * selectObject 
   * returns the desired object that would be searched via the passed index. 
   * If index ==(-1) currentShape = Null.
   * 
   * @param index
   * @return shape
   */
  private Shape3D selectObject(int index) {
    if (index < 0) {
      return null;
      } else {
        Shape3D shape = null;
        // index is not a split line
        if (((index + 1) % (currentTupleLength + 1)) != 0) {
          int cntRows = shapeVector.size();

          if (cntRows != 0 && currentTupleLength != 0) {
            int row = (int) Math.floor(index / (currentTupleLength + 1));
            // +1 because of split Line
            int column = (int) index % (currentTupleLength + 1);
            try {
              shape = shapeVector.get(row).get(column);
              } catch (Exception e) {
                Reporter.writeError("Error in showObject= "
                  + e.getStackTrace());
                Reporter.writeError("Error in showObject= "
                  + e.getClass() + " - " + e.getMessage() + " - "
                  + e.getCause());
                }
            }
          }
        return shape;
    }
  }
      
  /**
   * centeringView 
   * Returns the display in starting position while retaining 
   * the settings of the objects
   */
  public void centeringView() {
    rotateRelationBack();
    //calc bounds
    Bounds tfGoupBounds = translateTransGrp.getBounds();
    BoundingSphere sphere = new BoundingSphere(tfGoupBounds);
    // set translation to middle position
    translateTransGrp.setTransform(calcAutoTransform(sphere, true));
    // calc automatic scale factor
    setScaleFactor(calcAutoScaleFactor(sphere), false);
//     
  }

/** getter and setter --------------------------------- */

  /** SecondoObject */
  public SecondoObject getSecondoObject() {
    return secondoObject;
    }

  /** isRelation */
  public boolean isRelation() {
    return isRelation;
    }
  
  /** isUndefined*/
  public boolean isUndefined() {
    return isUndefined;
  }
  
  /**isEmpty*/
  public boolean isEmpty() {
    return isEmpty;
  }

  /** textViewItems */
  public Vector<String> getTextViewItems() {
    return textViewItems;
    }

  /** currentTypeString */
  public String getCurrentType() {
    return currentTypeString;
    }

  /** objectTfGrp */
  public TransformGroup getObjetTrfGrp() {
    return objectTransGrp;
    }

  /** isVaryingColorViewSelected */
  public boolean getVaryingColorViewSelected() {
    return isVaryingColorViewSelected;
    }

  public void setVaryingColorViewSelected(boolean isSelected) {
    isVaryingColorViewSelected = isSelected;
    }

  /** drawGrid */
  public boolean getDrawGrid() {
    return drawGrid;
    }

  public void setDrawGrid(boolean grid) {
    if (grid != drawGrid) {
      drawGrid = grid;
      changeObjectsColor();
      }

    }

  /** currentShape */
  public Shape3D getCurrentShape() {
    return currentShape;
    }

  public void setCurrentShape(int index) {
    currentShape = selectObject(index);
    }

  /** scaleFactor */
  public double getScaleFactor() {
    return scaleFactor;
    }

  public void setScaleFactor(double factor, boolean setAuto) {
    scaleFactor = factor;
    autoScaleFactor = setAuto;
    changeScaleFactor();
    }

  /** autoScaleFactor */
  public boolean getAutoScaleFactor() {
    return autoScaleFactor;
    }

  public void setAutoScaleFactor(boolean auto) {
    autoScaleFactor = auto;
    }

  /** zoomFactor */
  public double getZoomFactor() {
    return zoomFactor;
    }

  public void setZoomFactor(double factor) {
    zoomFactor = factor;
    //TODO new
    lightingPanel.setZChangeFactor(factor);
    try {
      pickZoom.setFactor(zoomFactor);     // standard is 0.04
      wheelZoom.setFactor(zoomFactor);    // standard is 0.1
      } catch (NullPointerException e) {
        }
    }

  /** translateFactor */
  public double getTranslateFactor() {
    return translateFactor;
    }

  public void setTranslateFactor(double factor) {
    translateFactor = factor;
    //TODO new
    lightingPanel.setXChangeFactor(factor);
    lightingPanel.setYChangeFactor(factor);
    try {
      // standard is 0.02
      pickTranslate.setFactor(translateFactor);
      } catch (NullPointerException e) {
        }
    }

  /** rotateFactor */
  public double getRotateFactor() {
    return rotateFactor;
    }

  public void setRotateFactor(double factor) {
    rotateFactor = factor;
    try {
      pickRotate.setFactor(rotateFactor); // standard is 0.03
      } catch (NullPointerException e) {
        }
    }
  
  /** pointSize */
  public double getPointSize() {
    return pointSize;
    }

  public void setSinglePointSize(float sPntSize) {
    singlePointSize = sPntSize;
    }

  public float getSinglePointSize() {
    return singlePointSize;
    }

  /** objectColor */
  public Color3f getObjectColor() {
    return objectColor;
    }

  public void setObjectColor(Color color) {
    objectColor.set(color);
    if(!drawGrid){
      setVaryingColorViewSelected(false);
      changeObjectsColor();
      }
    }

  /** gridColor */
  public Color3f getGridColor() {
    return gridColor;
    }

  public void setGridColor(Color color) {
    gridColor.set(color);
    if(drawGrid){
      setVaryingColorViewSelected(false);
      changeObjectsColor();
      }
    }

  /** set default colors */
  public void setDefaultBackgroundColor() {
    backgroundColor.set(DEFAULT_BACKGROUND_COLOR.get());
    background.setColor(backgroundColor);
    }

  public void setDefaultObjectColor() {
    objectColor.set(DEFAULT_OBJECT_COLOR.get());
    if(!drawGrid){
      setVaryingColorViewSelected(false);
      changeObjectsColor();
      }
    }

  public void setDefaultGridColor() {
    gridColor.set(DEFAULT_GRID_COLOR.get());
    if(drawGrid){
      setVaryingColorViewSelected(false);
      changeObjectsColor();
    }
  }

 /** end of getter and setter ----------------------- */
 
  /**
   * add an individual light
   * @param light branch group, to which a light gets added.
   */
  public void addLight(BranchGroup lightBg){
    lights.addChild(lightBg);
    }
  /**
   * return the light at index
   * @param index: index for selected light
   * @return returns the LightBranchGroup at the given index
   */
  public LightBranchGroup getLight(int index){
    if(index < 0 || index >= lights.numChildren()) return null;
    int j=0;
    for(int i=0;i<index;i++){
      if(!(lights.getChild(i) instanceof LightBranchGroup) ){
        j++;
        }
      j++;
      }
    if(j >= lights.numChildren()) return null;
    return (LightBranchGroup)lights.getChild(j);
    }
  /**
   * delete a light at index
   * @param index: index for selected light
   */
  public void delLight(int index){
    if(index<0 || index > lights.numChildren()-1) return;
    lights.removeChild(index);
    }

  /**
   * getter LightColor
   * @return color of ambient light
   */
  public Color3f getLightColor(){
    Color3f color = new Color3f();
    ambLight.getColor(color);
    return color;
    }

  /**
   * setter LightColor
   * @param color
   */
  public void setLightColor(Color color){
    if(color==null)return;
    Color3f color3f = new Color3f(color);
    ambLight.setColor(color3f);
    }

  /**
   * setter on / off ambLight
   * @param switch on/off
   */
  public void setMainLightSwitch(boolean sw){
    if(ambLight!=null) ambLight.setEnable(sw);
    }

  /**
   * getter on/off ambLight
   * @return on/off of AmbientLight
   */
  public boolean getMainLightSwitch(){
    if(ambLight==null)return false;
    return ambLight.getEnable();
    }

  /**
   * activate mouse movement only selected light at index
   * @param index of the light
   */
  public void setLightMouseMove(int index){
    for(int i = 0; i<lights.numChildren()-1 ; i++){
      if(lights.getChild(i) instanceof LightBranchGroup){
        ((LightBranchGroup)lights.getChild(i)).setMouseMoveEnable(false);
        }
      }
    if(lights.getChild(index) instanceof LightBranchGroup)((LightBranchGroup)lights.getChild(index)).setMouseMoveEnable(true);
    }

  /**
   * getter Ambient Lights
   * @return Ambient Lights
   */
  public BranchGroup getAmLightBg() {
    return ambLightBg;
    }
  
  /**
   * getter lights
   * @return lights
   */
  public BranchGroup getLights() {
    return lights;
    }

  /**
   * getLightingPanel
   */
  public LightPanel getLightingPanel() {
    return lightingPanel;
    }
        
}
