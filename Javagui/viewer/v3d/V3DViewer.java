package viewer.v3d;

import com.sun.j3d.exp.swing.JCanvas3D;
import com.sun.j3d.utils.behaviors.mouse.MouseBehavior;
import com.sun.j3d.utils.behaviors.mouse.MouseRotate;
import com.sun.j3d.utils.behaviors.mouse.MouseTranslate;
import com.sun.j3d.utils.behaviors.mouse.MouseWheelZoom;
import com.sun.j3d.utils.universe.SimpleUniverse;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import gui.SecondoObject;
import java.util.LinkedList;
import javax.media.j3d.Appearance;
import javax.media.j3d.Background;
import javax.media.j3d.BoundingSphere;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.GraphicsConfigTemplate3D;
import javax.media.j3d.PolygonAttributes;
import javax.media.j3d.Shape3D;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.media.j3d.TriangleArray;
import javax.vecmath.Point3d;
import javax.vecmath.Vector3f;
import sj.lang.ListExpr;
import viewer.MenuVector;
import viewer.SecondoViewer;

/* this viewer shows an MRegion or a URegion as a 3D-Object */
public class V3DViewer extends SecondoViewer {

    private final MenuVector MV = new MenuVector();
    private final JMenuItem MI_CageModel, MI_LightBackground;
    private final JMenuItem MI_CTransl, MI_AntiAliasing;
    private JComboBox ComboBox = new JComboBox();
    private java.util.List ItemObjects = new LinkedList();
    private SecondoObject CurrentObject = null;
    private final JScrollPane ScrollPane = new JScrollPane();
    private final SimpleUniverse universe;

    /* create a new StandardViewer */
    public V3DViewer() {
        setLayout(new BorderLayout());
        add(BorderLayout.NORTH, ComboBox);
        
        GraphicsConfigTemplate3D template = new GraphicsConfigTemplate3D();
        template.setDoubleBuffer(GraphicsConfigTemplate3D.PREFERRED);
        template.setSceneAntialiasing(GraphicsConfigTemplate3D.PREFERRED);
        JCanvas3D j3d = new JCanvas3D(template);
        universe = new SimpleUniverse(j3d.getOffscreenCanvas3D());

        ScrollPane.add(j3d);
        ScrollPane.setViewportView(universe.getCanvas());

        add(BorderLayout.CENTER, ScrollPane);

        JMenu StdMenu = new JMenu("V3DViewer");
        MI_CageModel = StdMenu.add(new JCheckBoxMenuItem("Cage-Model"));
        MI_CTransl = StdMenu.add(new JCheckBoxMenuItem("Compensate Translation"));
        MI_AntiAliasing = StdMenu.add(new JCheckBoxMenuItem("Anti-Aliasing"));
        MI_LightBackground = StdMenu.add(new JCheckBoxMenuItem("Light Background"));
        MV.addMenu(StdMenu);

        ActionListener redraw = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (CurrentObject != null) {
                    showObject();
                }
            }
        };

        MI_CageModel.addActionListener(redraw);
        MI_AntiAliasing.addActionListener(redraw);
        MI_CTransl.addActionListener(redraw);
        MI_LightBackground.addActionListener(redraw);

        ComboBox.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent evt) {
                showObject();
                if (VC != null) {
                    int index = ComboBox.getSelectedIndex();
                    if (index >= 0) {
                        try {
                            CurrentObject = (SecondoObject) ItemObjects.get(index);
                            VC.selectObject(V3DViewer.this, CurrentObject);
                        } catch (Exception e) {
                        }
                    }
                }
            }
        });

    }


    /* adds a new Object to this Viewer and display it */
    @Override
    public boolean addObject(SecondoObject o) {
        if (isDisplayed(o)) {
            selectObject(o);
        } else {
            ItemObjects.add(o);
            ComboBox.addItem(o.getName());
            try {
                ComboBox.setSelectedIndex(ComboBox.getItemCount() - 1);  // make the new object to active object
                showObject();
            } catch (Exception e) {
            }
        }
        return true;
    }

    /* returns true if o is a SecondoObject in this viewer */
    public boolean isDisplayed(SecondoObject o) {
        return CurrentObject == o;
    }

    /**
     * remove o from this Viewer
     */
    @Override
    public void removeObject(SecondoObject o) {
        if (ItemObjects.remove(o)) {
            ComboBox.removeItem(o.getName());
        }
        if (CurrentObject == o) {
            CurrentObject = null;
        }
    }

    /**
     * remove all containing objects
     */
    @Override
    public void removeAll() {
        ItemObjects.clear();
        ComboBox.removeAllItems();
        CurrentObject = null;
        if (VC != null) {
            VC.removeObject(null);
        }
        showObject();
    }


    /* returns true if the object is a uregion or an mregion, which is all this
    viewer can display.
    */
    @Override
    public boolean canDisplay(SecondoObject o) {
        ListExpr LE = o.toListExpr();
        if (LE.listLength() != 2) {
            return false;
        }
        ListExpr type = LE.first();
        if (!(type.isAtom() && type.atomType() == ListExpr.SYMBOL_ATOM)) {
            return false;
        }
        String TypeName = type.symbolValue();
        return (TypeName.equals("uregion") || TypeName.equals("mregion"));
    }


    /* returns the additional Menu-Items of this viewer */
    public MenuVector getMenuVector() {
        return MV;
    }

    /* returns the name of the V3D-Viewer*/
    public String getName() {
        return "V3D";
    }

    /* select O */
    public boolean selectObject(SecondoObject O) {
        int i = ItemObjects.indexOf(O);
        if (i >= 0) {
            ComboBox.setSelectedIndex(i);
            showObject();
            return true;
        } else //object not found
        {
            return false;
        }
    }
    
    BranchGroup bg = null;

    private void showObject() {

        int index = ComboBox.getSelectedIndex();
        if (index >= 0) {
            try {
                CurrentObject = (SecondoObject) ItemObjects.get(index);
                Face f = new Face(CurrentObject, MI_CTransl.isSelected(),
                        MI_LightBackground.isSelected());
                TriangleArray tri = f.GetTriangleArray();

                Appearance app = new Appearance();
                PolygonAttributes pa = new PolygonAttributes();
                if (MI_CageModel.isSelected()) {
                    pa.setPolygonMode(PolygonAttributes.POLYGON_LINE);
                } else {
                    pa.setPolygonMode(PolygonAttributes.POLYGON_FILL);
                }
                pa.setCullFace(PolygonAttributes.CULL_NONE);
                app.setPolygonAttributes(pa);
                Shape3D shape = new Shape3D();
                shape.setAppearance(app);
                shape.setGeometry(tri);
                Transform3D viewtransform3d = new Transform3D();
                viewtransform3d.setTranslation(new Vector3f(0.0f, 0.0f, 1.0f));
                TransformGroup tg = new TransformGroup(viewtransform3d);
                tg.setCapability(TransformGroup.ALLOW_TRANSFORM_READ);
                tg.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
                tg.addChild(shape);
                MouseRotate rotor = new MouseRotate();
                MouseTranslate trans = new MouseTranslate();
                rotor.setFactor(0.001f);
                rotor.setSchedulingBounds(new BoundingSphere());
                rotor.setTransformGroup(tg);
                trans.setSchedulingBounds(new BoundingSphere());
                trans.setTransformGroup(tg);
                if (bg != null) {
                    universe.getLocale().removeBranchGraph(bg);
                }
                bg = new BranchGroup();
                if (MI_LightBackground.isSelected()) {
                    Background background = new Background(255, 255, 255);
                    background.setCapability(Background.ALLOW_COLOR_WRITE);
                    BoundingSphere sphere = new BoundingSphere(new Point3d(0.0, 0.0,
                            0.0),
                            1000.0);
                    background.setApplicationBounds(sphere);
                    bg.addChild(background);
                }
                bg.addChild(tg);
                bg.addChild(rotor);
                bg.addChild(trans);
                MouseWheelZoom mwz = new MouseWheelZoom(MouseBehavior.INVERT_INPUT);
                mwz.setTransformGroup(universe.getViewingPlatform().getViewPlatformTransform());
                mwz.setSchedulingBounds(new BoundingSphere());
                bg.addChild(mwz);
                bg.setCapability(BranchGroup.ALLOW_DETACH);
                bg.compile();
                universe.addBranchGraph(bg);
                TransformGroup tg3 = universe.getViewingPlatform().getViewPlatformTransform();
                universe.getViewer().getView().setSceneAntialiasingEnable(MI_AntiAliasing.isSelected());
                Transform3D t3d = new Transform3D();
                t3d.setTranslation(new Vector3f(0.0f, 0.0f, 25));
                tg3.setTransform(t3d);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
