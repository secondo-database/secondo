package viewer.v3d;

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
import java.util.List;
import javax.media.j3d.Appearance;
import javax.media.j3d.BoundingSphere;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Canvas3D;
import javax.media.j3d.PolygonAttributes;
import javax.media.j3d.Shape3D;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.media.j3d.TriangleArray;
import javax.vecmath.Color3b;
import javax.vecmath.Point3d;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;
import sj.lang.ListExpr;
import viewer.MenuVector;
import viewer.SecondoViewer;

/* this viewer shows an MRegion as 3D-Object */
public class V3DViewer extends SecondoViewer {

    private JScrollPane ScrollPane = new JScrollPane();
    private SimpleUniverse universe;
    private JComboBox ComboBox = new JComboBox();
    private java.util.List ItemObjects = new LinkedList();
    private MenuVector MV = new MenuVector();
    private SecondoObject CurrentObject = null;
    private JMenuItem MI_CageModel;
    private JMenuItem MI_CTransl;

    /* create a new StandardViewer */
    public V3DViewer() {
        setLayout(new BorderLayout());
        add(BorderLayout.NORTH, ComboBox);
        Canvas3D c3d = new Canvas3D(SimpleUniverse.getPreferredConfiguration());
//        GraphicsConfigTemplate3D gct3d = new GraphicsConfigTemplate3D();
//        JCanvas3D c3d = new JCanvas3D(gct3d);

        universe = new SimpleUniverse(c3d);

        ScrollPane.add(c3d);
        ScrollPane.setViewportView(universe.getCanvas());

//        JPanel jp = new JPanel(new BorderLayout());
//        jp.add(BorderLayout.CENTER, ScrollPane);
//        jp.setDoubleBuffered(true);
        add(BorderLayout.CENTER, ScrollPane);

        //   tg.addChild(shape);
        //  ScrollPane.add(TextArea);
        JMenu StdMenu = new JMenu("V3DViewer");
        JMenuItem MI_Remove = StdMenu.add("remove");
        JMenuItem MI_RemoveAll = StdMenu.add("remove all");
        MI_CageModel = StdMenu.add(new JCheckBoxMenuItem("Cage-Model"));
        MI_CTransl = StdMenu.add(new JCheckBoxMenuItem("Compensate Translation"));
        MV.addMenu(StdMenu);

        MI_CageModel.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (CurrentObject != null) {
                    showObject();
                }
            }
        });

        MI_CTransl.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (CurrentObject != null) {
                    showObject();
                }
            }
        });

        MI_Remove.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                if (CurrentObject != null) {
                    SecondoObject Victim = CurrentObject;
                    if (ItemObjects.remove(CurrentObject)) {
                        ComboBox.removeItem(CurrentObject.getName());
                        CurrentObject = null;
                        int index = ComboBox.getSelectedIndex();          // the new current object
                        if (index >= 0) {
                            CurrentObject = (SecondoObject) ItemObjects.get(index);
                            showObject();
                        }
                    }
                    if (VC != null) {
                        VC.removeObject(Victim);  // inform the ViewerControl
                    }
                }
            }
        });

        MI_RemoveAll.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                ItemObjects.clear();
                ComboBox.removeAllItems();
                CurrentObject = null;
                if (VC != null) {
                    VC.removeObject(null);
                }
                showObject();
            }
        });

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
        return ItemObjects.indexOf(o) >= 0;

    }

    /**
     * remove o from this Viewer
     */
    @Override
    public void removeObject(SecondoObject o) {
        if (ItemObjects.remove(o)) {
            ComboBox.removeItem(o.getName());
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


    /* returns true if the object is a uregion or an mregion */
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


    /* returns the Menuextension of this viewer */
    public MenuVector getMenuVector() {
        return MV;
    }

    /* returns Standard */
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
    int cnr = 0;

    private Color3b nextColor() {
        Color3b c = new Color3b();

        switch (cnr) {
            case 0:
                c.set(Color.RED);
                break;
            case 1:
                c.set(Color.BLUE);
                break;
            case 2:
                c.set(Color.YELLOW);
                break;
            case 3:
                c.set(Color.GREEN);
                break;
            case 4:
                c.set(Color.MAGENTA);
                break;
            case 5:
                c.set(Color.CYAN);
                break;
            case 6:
                c.set(Color.ORANGE);
                break;
            case 7:
                c.set(Color.PINK);
                break;
        }
        cnr = (cnr + 1) % 8;

        return c;
    }
    
    private void addURegionFace(ListExpr ll, LinkedList<Point3f> pl, LinkedList<Color3b> cl, float zoff) {
        ll = ll.first();
        System.err.println("Displaying: " + ll.toString());
        boolean finish = false;
        ListExpr fp = ll.first();
        float scale = 200f;
        float height = 1f;
        float minsx = 999999, minsy = 999999, minfx = 999999, minfy = 999999;
        do {
            ListExpr p = ll.first();
            if (p == null) {
                System.err.println("p is null");
                p = fp;
                finish = true;
            }
            System.err.println("p is "+p.toString());            
            float sx1 = (float) p.first().realValue();
            float sy1 = (float) p.second().realValue();
            float fx1 = (float) p.third().realValue();
            float fy1 = (float) p.fourth().realValue();
            p = ll.rest().first();
            if (finish) {
                System.err.println("2p is null");
                break;
            }
            if (p == null) {
                p = fp;
                finish = true;
            }
            float sx2 = (float) p.first().realValue();
            float sy2 = (float) p.second().realValue();
            float fx2 = (float) p.third().realValue();
            float fy2 = (float) p.fourth().realValue();

            if ((sx1 != fx1) || (sy1 != fy1)) {
                if (minsx > sx1) {
                    minsx = sx1;
                }
                if (minsy > sy1) {
                    minsy = sy1;
                }
                if (minfx > fx1) {
                    minfx = fx1;
                }
                if (minfx > fx2) {
                    minfx = fx2;
                }
                if (minfy > fy1) {
                    minfy = fy1;
                }
                if (minfy > fy2) {
                    minfy = fy2;
                }
                pl.add(new Point3f(sx1 / scale, sy1 / scale, zoff * height));
                pl.add(new Point3f(fx1 / scale, fy1 / scale, (zoff + 1) * height));
                pl.add(new Point3f(fx2 / scale, fy2 / scale, (zoff + 1) * height));
                Color3b c = nextColor();
                cl.add(c);
                cl.add(c);
                cl.add(c);
//                        pl.add(new Point3f(sx1 / scale, sy1 / scale, 0f));
//                        pl.add(new Point3f(fx2 / scale, fy2 / scale, height));
//                        pl.add(new Point3f(fx1 / scale, fy1 / scale, height));
            }
            if ((sx2 != fx2) || (sy2 != fy2)) {
                if (minfx > fx1) {
                    minfx = fx1;
                }
                if (minfy > fy1) {
                    minfy = fy1;
                }
                if (minsx > sx1) {
                    minsx = sx1;
                }
                if (minsx > sx2) {
                    minsx = sx2;
                }
                if (minsy > sy1) {
                    minsy = sy1;
                }
                if (minsy > sy2) {
                    minsy = sy2;
                }
                pl.add(new Point3f(sx1 / scale, sy1 / scale, zoff * height));
                pl.add(new Point3f(sx2 / scale, sy2 / scale, zoff * height));
                pl.add(new Point3f(fx2 / scale, fy2 / scale, (zoff + 1) * height));
                Color3b c = nextColor();
                cl.add(c);
                cl.add(c);
                cl.add(c);
//                        pl.add(new Point3f(sx1 / scale, sy1 / scale, 0));
//                        pl.add(new Point3f(fx2 / scale, fy2 / scale, height));
//                        pl.add(new Point3f(sx2 / scale, sy2 / scale, 0));
            }

            ll = ll.rest();
        } while (!finish);
        float minx = Math.min(minsx, minfx);
        float miny = Math.min(minsy, minfy);
        for (int i = 0; i < pl.size(); i++) {
            Point3f p = pl.get(i);
            if (MI_CTransl.isSelected()) {
                if (p.z == zoff * height) {
                    p.x -= minsx / scale;
                    p.y -= minsy / scale;
                } else {
                    p.x -= minfx / scale;
                    p.y -= minfy / scale;
                }
            } else {
//                p.x -= minx / scale;
//                p.y -= miny / scale;
            }
            p.z -= height / 20;
        }
    }

    private void addURegion(ListExpr ll, LinkedList<Point3f> pl, LinkedList<Color3b> cl, float zoff) {
        ll = ll.second();
        
        System.out.println("Adding URegion "+ll.toString());

        while (ll != null && !ll.isEmpty()) {
            addURegionFace(ll.first(), pl, cl, zoff);
            ll = ll.rest();
        }
    }

    private void showObject() {

        int index = ComboBox.getSelectedIndex();
        if (index >= 0) {
            try {
                CurrentObject = (SecondoObject) ItemObjects.get(index);
                LinkedList<Point3f> pl = new LinkedList();
                LinkedList<Color3b> cl = new LinkedList();
                
                List<Point3d> px = Face.MRegionList2Triangles(CurrentObject.toListExpr());

//                ListExpr ll = CurrentObject.toListExpr().second();
//                float zoff = 0;
//                while (ll != null && !ll.isEmpty()) {
//                    addURegion(ll.first(), pl, cl, zoff);
//                    ll = ll.rest();
//                    zoff++;
//                    System.out.println("Ok, size: "+pl.size());
//                }

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
                TriangleArray tri = new TriangleArray(px.size(), TriangleArray.COORDINATES | TriangleArray.COLOR_3);
                Color3b c = nextColor();
                for (int i = 0; i < px.size(); i++) {
                    if (i%3 == 0)
                        c = nextColor();
                    tri.setCoordinate(i, px.get(i));
                    System.err.println("YYY "+px.get(i).toString());
                    tri.setColor(i, c);
                }
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
                bg.addChild(tg);
                bg.addChild(rotor);
                bg.addChild(trans);

                MouseWheelZoom mwz = new MouseWheelZoom(MouseBehavior.INVERT_INPUT);
                mwz.setTransformGroup(universe.getViewingPlatform().getViewPlatformTransform());
                mwz.setSchedulingBounds(new BoundingSphere());
                bg.addChild(mwz);

//                bg.addChild(new MouseWheelZoom());
                bg.setCapability(BranchGroup.ALLOW_DETACH);
                bg.compile();
                universe.addBranchGraph(bg);
                universe.getViewingPlatform().setNominalViewingTransform();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
