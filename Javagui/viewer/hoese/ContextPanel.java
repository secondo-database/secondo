

package  viewer.hoese;

import  javax.swing.*;
import  java.io.*;
import  java.awt.*;
import  java.awt.event.*;
import  java.awt.geom.*;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;


/**
 * A swing JPanel that could be displayed in the main window. It allows to define the 
 * World bounding box, transformation and background image. 
 */
public class ContextPanel extends JPanel {
/** Application frame */
  HoeseViewer mw;
/** The rectangle that contains the selected image */
  Rectangle ImageRect;
/** Position of the blue cross, set during defining of projection */
  Point2D.Double ProjPoint1 = new Point2D.Double(50, 50);
/** Position of the red cross, set during defining of projection */
  Point2D.Double ProjPoint2 = new Point2D.Double(250, 250);
/**  World coordinates under the blue cross */
  Point2D.Double World1 = new Point2D.Double(50, 50);
/**  World coordinates under the red cross */
  Point2D.Double World2 = new Point2D.Double(250, 250);
  private Point2D.Double Dev1 = new Point2D.Double(50, 50);
  private Point2D.Double Dev2 = new Point2D.Double(250, 250);
/** The offset of the image */
  Point MapOfs = new Point(0, 0);
  private ProjLabel Proj;
/** The Image-pathname of the background image */
  public String ImagePath = null;
  private double ClipWidth = 900, ClipHeight = 900;
  private javax.swing.JPanel ConfP;
  private javax.swing.JRadioButton NoProjRB;
  private javax.swing.JRadioButton ManProjRB;
  private javax.swing.JTextField C1XTF;
  private javax.swing.JTextField C1YTF;
  private javax.swing.JLabel C1L;
  private javax.swing.JLabel C2L;
  private javax.swing.JTextField C2XTF;
  private javax.swing.JTextField C2YTF;
/** The button which indicates automatic projection */
  public javax.swing.JRadioButton AutoProjRB;
  private javax.swing.JRadioButton NoClipRB;
  private javax.swing.JRadioButton SizeClipRB;
  private javax.swing.JRadioButton ImageSizeClipRB;
  private javax.swing.JTextField WBx1TF;
  private javax.swing.JTextField WBy1TF;
  private javax.swing.JTextField WBx2TF;
  private javax.swing.JTextField WBy2TF;
  private JLabel WBx1L;
  private JLabel WBy1L;
  private JLabel WBx2L;
  private JLabel WBy2L;
  private JLabel WBc1L;
  private JLabel WBc2L;
  private JLabel WBL;
  private JLabel bordspcL;
  private JTextField bordspcT;
  private javax.swing.JLabel ImageNameL;
  private javax.swing.JButton ApplyB;
  private javax.swing.JButton CancelB;
  private javax.swing.JSeparator jSeparator1;
  private javax.swing.JButton ImageB;
  private MouseAdapter ProjectionControl;

  /**
   * Constructor of a contextpanel. There exist only one instance of a ContextPanel. 
   * It may be hide or shown by the main app.
   * @param   HoeseViewer main
   * @see <a href="ContextPanelsrc.html#ContextPanel">Source</a>
   */
  public ContextPanel (HoeseViewer main) {
    mw = main;
    Proj = new ProjLabel();
    ProjectionControl = (new MouseAdapter() {
      public void mouseClicked (MouseEvent e) {
        //Koordinaten in Weltkoordinaten umwandeln
        double sf = mw.getZoomFactor();
        Point2D.Double p = new Point2D.Double(((double)e.getX())/sf, ((double)e.getY())/sf);
        if (SwingUtilities.isLeftMouseButton(e)) {
          ProjPoint1 = p;
          // ImageL.repaint();
        } 
        else if (SwingUtilities.isRightMouseButton(e)) {
          ProjPoint2 = p;
          //ImageL.repaint();
        }
        Proj.repaint();
      }
    });
    setLayout(new java.awt.GridBagLayout());
    NoProjRB = new javax.swing.JRadioButton();
    ManProjRB = new javax.swing.JRadioButton();
    C1XTF = new javax.swing.JTextField();
    C1YTF = new javax.swing.JTextField();
    C1L = new javax.swing.JLabel();
    C2L = new javax.swing.JLabel();
    C2XTF = new javax.swing.JTextField();
    C2YTF = new javax.swing.JTextField();
    AutoProjRB = new javax.swing.JRadioButton();
    NoClipRB = new javax.swing.JRadioButton();
    SizeClipRB = new javax.swing.JRadioButton();
    ImageSizeClipRB = new javax.swing.JRadioButton();
    WBx1TF = new javax.swing.JTextField();
    WBy1TF = new javax.swing.JTextField();
    WBx2TF = new javax.swing.JTextField();
    WBy2TF = new javax.swing.JTextField();
    bordspcT = new javax.swing.JTextField();
    bordspcL = new JLabel();
    WBx1L = new JLabel();
    WBy1L = new JLabel();
    WBx2L = new JLabel();
    WBy2L = new JLabel();
    WBc1L = new JLabel();
    WBc2L = new JLabel();
    WBL = new JLabel();
    ImageNameL = new javax.swing.JLabel();
    ApplyB = new javax.swing.JButton();
    CancelB = new javax.swing.JButton();
    jSeparator1 = new javax.swing.JSeparator();
    ImageB = new javax.swing.JButton();
    java.awt.GridBagConstraints gbc;
    //      NoProjRB.setSelected (true);
    WBL.setText("World Bounding Box");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridwidth = 3;
    gbc.anchor = java.awt.GridBagConstraints.WEST;
    add(WBL, gbc);
    WBx1TF.setColumns(6);
    WBx1TF.setText("");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 1;
    gbc.gridy = 3;
    add(WBx1TF, gbc);
    gbc.gridy = 2;
    add(WBx1L, gbc);
    WBy1TF.setColumns(6);
    WBy1TF.setText("");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 2;
    gbc.gridy = 3;
    add(WBy1TF, gbc);
    gbc.gridy = 2;
    add(WBy1L, gbc);
    WBc1L.setText("Left [x,[y,");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 0;
    gbc.gridy = 2;
    gbc.anchor = java.awt.GridBagConstraints.EAST;
    add(WBc1L, gbc);
    WBc2L.setText("Right x] y]");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 0;
    gbc.gridy = 4;
    gbc.anchor = java.awt.GridBagConstraints.EAST;
    add(WBc2L, gbc);
    WBx2TF.setColumns(6);
    WBx2TF.setText("");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 1;
    gbc.gridy = 5;
    add(WBx2TF, gbc);
    gbc.gridy = 4;
    add(WBx2L, gbc);
    WBy2TF.setColumns(6);
    WBy2TF.setText("");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 2;
    gbc.gridy = 5;
    add(WBy2TF, gbc);
    gbc.gridy = 4;
    add(WBy2L, gbc);
    bordspcT.setColumns(3);
    bordspcT.setText("30");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 2;
    gbc.gridy = 6;
    add(bordspcT, gbc);
    gbc.gridwidth = 2;
    gbc.ipady = 10;
    gbc.gridx = 0;
    bordspcL.setText("border space");
    add(bordspcL, gbc);
    AutoProjRB.setText("auto projection ");
    AutoProjRB.setSelected(true);
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 0;
    gbc.gridy = 7;
    gbc.ipady = 5;
    gbc.gridwidth = 3;
    gbc.anchor = java.awt.GridBagConstraints.WEST;
    add(AutoProjRB, gbc);
    ManProjRB.setText("manual projection");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 0;
    gbc.gridy = 8;
    gbc.gridwidth = 3;
    gbc.anchor = java.awt.GridBagConstraints.WEST;
    add(ManProjRB, gbc);
    C1XTF.setColumns(6);
    C1XTF.setText("50.0");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 1;
    gbc.gridy = 9;
    add(C1XTF, gbc);
    C1YTF.setColumns(6);
    C1YTF.setText("50.0");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 2;
    gbc.gridy = 9;
    add(C1YTF, gbc);
    C1L.setForeground(Color.blue);
    C1L.setText("Coord 1");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 0;
    gbc.gridy = 9;
    gbc.anchor = java.awt.GridBagConstraints.EAST;
    add(C1L, gbc);
    C2L.setForeground(Color.red);
    C2L.setText("Coord 2");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 0;
    gbc.gridy = 10;
    gbc.anchor = java.awt.GridBagConstraints.EAST;
    add(C2L, gbc);
    C2XTF.setColumns(6);
    C2XTF.setText("250.0");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 1;
    gbc.gridy = 10;
    add(C2XTF, gbc);
    C2YTF.setColumns(6);
    C2YTF.setText("250.0");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 2;
    gbc.gridy = 10;
    add(C2YTF, gbc);
    ButtonGroup group = new ButtonGroup();
    group.add(ManProjRB);
    group.add(AutoProjRB);
    ImageNameL.setText("no Image");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 0;
    gbc.gridy = 11;
    gbc.gridwidth = 3;
    gbc.ipady = 10;
    gbc.anchor = java.awt.GridBagConstraints.WEST;
    add(ImageNameL, gbc);
    JButton SPB = new JButton("Set Points");
    //gbc = new java.awt.GridBagConstraints();
    gbc.gridy = 12;
    SPB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        Proj.setVisible(true);
        mw.allProjection = new AffineTransform();
        if (ImagePath != null)
          mw.GraphDisplay.updateLayersSize(ImageRect); 
        else 
          mw.GraphDisplay.updateLayersSize(new Rectangle(0, 0, 800, 800));
        mw.GraphDisplay.repaint();
      }
    });
    add(SPB, gbc);
    ApplyB.setText("Apply");
    gbc = new java.awt.GridBagConstraints();
    gbc.weighty = 1;
    gbc.gridx = 0;
    gbc.gridy = 13;
    ApplyB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        applyButtonPressed();
      }
    });
    add(ApplyB, gbc);
    CancelB.setText("Hide");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 1;
    gbc.gridy = 13;
    CancelB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        mw.MIsetKontext.setSelected(false);
        mw.on_Set_Kontext();
      }
    });
    add(CancelB, gbc);
    ImageB.setText("Image");
    gbc = new java.awt.GridBagConstraints();
    gbc.gridx = 2;
    gbc.gridy = 13;
    ImageB.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        final JFileChooser fc = new JFileChooser(mw.configuration.getProperty("WorkingDir", 
            "/"));
        int returnVal = fc.showOpenDialog(mw);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
          File file = fc.getSelectedFile();
          //this is where a real application would open the file.
          if (file.exists()) {
            ImageSizeClipRB.setEnabled(true);
            ImagePath = file.getPath();
            ImageNameL.setText(file.getName());
            ImageIcon ii = new ImageIcon(ClassLoader.getSystemResource(ImagePath));
            ImageRect = new Rectangle(0, 0, ii.getIconWidth(), ii.getIconHeight());
            //mw.allProjection=new AffineTransform();
            if (mw.hasBackImage)
              mw.LayerSwitchBar.remove(0);
            if (ImagePath != null)
              mw.addSwitch(mw.GraphDisplay.createBackLayer(ImagePath, 0, 0), 
                  0); 
            else 
              mw.GraphDisplay.createBackLayer(null, 0, 0);
            mw.hasBackImage = (ImagePath != null);
          } 
          else {
            ImageSizeClipRB.setEnabled(false);
            ImagePath = null;
            ImageNameL.setText("no image");
          }
        }
      }
    });
    add(ImageB, gbc);
  }

  /**
   * Applies the context-parameters
   * @see <a href="ContextPanelsrc.html#applyButtonPressed">Source</a> 
   */
  public void applyButtonPressed () {
    if (AutoProjRB.isSelected()) {
      if (mw.hasBackImage)
        mw.LayerSwitchBar.remove(0);
      if (ImagePath != null)
        mw.addSwitch(mw.GraphDisplay.createBackLayer(ImagePath, 0, 0), 0); 
      else 
        mw.GraphDisplay.createBackLayer(null, 0, 0);
      mw.hasBackImage = (ImagePath != null);
      mw.updateViewParameter();
    }
    if (ManProjRB.isSelected()) {
      mw.updateViewParameter();
      setWBLabel();
      mw.GraphDisplay.repaint();
    }
  }

  /**
   * Create a ListExpr out of the context-parameters. Used in session-saving .
   * @return A ListExpr
   * @see <a href="ContextPanelsrc.html#getContextLE">Source</a> 
   */
  public ListExpr getContextLE () {
    String projType = "";
    if (ManProjRB.isSelected())
      projType = "manprojection"; 
    else if (AutoProjRB.isSelected())
      projType = "autoprojection";
    ListExpr l = ListExpr.oneElemList(ListExpr.symbolAtom(projType));
    ListExpr le = ListExpr.append(l, ListExpr.realAtom(ProjPoint1.getX()));
    le = ListExpr.append(le, ListExpr.realAtom(ProjPoint1.getY()));
    le = ListExpr.append(le, ListExpr.realAtom(ProjPoint2.getX()));
    le = ListExpr.append(le, ListExpr.realAtom(ProjPoint2.getY()));
    le = ListExpr.append(le, ListExpr.realAtom(World1.getX()));
    le = ListExpr.append(le, ListExpr.realAtom(World1.getY()));
    le = ListExpr.append(le, ListExpr.realAtom(World2.getX()));
    le = ListExpr.append(le, ListExpr.realAtom(World2.getY()));
    le = ListExpr.append(le, ListExpr.stringAtom(WBx1TF.getText()));
    le = ListExpr.append(le, ListExpr.stringAtom(WBy1TF.getText()));
    le = ListExpr.append(le, ListExpr.stringAtom(WBx2TF.getText()));
    le = ListExpr.append(le, ListExpr.stringAtom(WBy2TF.getText()));
    le = ListExpr.append(le, ListExpr.stringAtom(bordspcT.getText()));
    String path = (ImagePath == null) ? "" : ImagePath;
    le = ListExpr.append(le, ListExpr.stringAtom(path));
    return  l;
  }

  /**
   * Reconstruct the context out of a ListExpr. Used in session-loading.
   * @param le A ListExpr containing the context.
   * @return True if no error had occured.
   * @see <a href="ContextPanelsrc.html#setContextLE">Source</a> 
   */
  public boolean setContextLE (ListExpr le) {
    if (le.listLength() != 15) {
      System.out.println("Error: No correct context expression: 13 elements needed");
      return  false;
    }
    if (le.first().atomType() != ListExpr.SYMBOL_ATOM)
      return  false;
    String projStyle = le.first().symbolValue();
    if (projStyle.equals("manprojection"))
      ManProjRB.setSelected(true); 
    else 
      AutoProjRB.setSelected(true);
    le = le.rest();
    if ((le.first().atomType() != ListExpr.REAL_ATOM) || (le.second().atomType()
        != ListExpr.REAL_ATOM) || (le.third().atomType() != ListExpr.REAL_ATOM)
        || (le.fourth().atomType() != ListExpr.REAL_ATOM))
      return  false;
    double r = le.first().realValue();
    ProjPoint1.setLocation(le.first().realValue(), le.second().realValue());
    ProjPoint2.setLocation(le.third().realValue(), le.fourth().realValue());
    le = le.rest().rest().rest().rest();
    if ((le.first().atomType() != ListExpr.REAL_ATOM) || (le.second().atomType()
        != ListExpr.REAL_ATOM) || (le.third().atomType() != ListExpr.REAL_ATOM)
        || (le.fourth().atomType() != ListExpr.REAL_ATOM))
      return  false;
    C1XTF.setText(Double.toString(le.first().realValue()));
    C1YTF.setText(Double.toString(le.second().realValue()));
    C2XTF.setText(Double.toString(le.third().realValue()));
    C2YTF.setText(Double.toString(le.fourth().realValue()));
    le = le.rest().rest().rest().rest();
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  false;
    WBx1TF.setText(le.first().stringValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  false;
    WBy1TF.setText(le.first().stringValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  false;
    WBx2TF.setText(le.first().stringValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  false;
    WBy2TF.setText(le.first().stringValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  false;
    bordspcT.setText(le.first().stringValue());
    le = le.rest();
    if (le.first().atomType() != ListExpr.STRING_ATOM)
      return  false;
    File file = new File(le.first().stringValue());
    //this is where a real application would open the file.
    if (file.exists()) {
      ImageSizeClipRB.setEnabled(true);
      ImagePath = file.getPath();
      ImageNameL.setText(file.getName());
      ImageIcon ii = new ImageIcon(ImagePath);
      ImageRect = new Rectangle(0, 0, ii.getIconWidth(), ii.getIconHeight());
    } 
    else {
      ImageSizeClipRB.setEnabled(false);
      ImagePath = null;
      ImageNameL.setText("no image");
    }
    applyButtonPressed();
    return  true;
  }

  /**
   * The projectionlabel is a special JLabel for drawing the crosses on the screen.
   * @return JLabel
   * @see <a href="ContextPanelsrc.html#getProjectionLabel">Source</a> 
   */
  public JLabel getProjectionLabel () {
    setWBLabel();
    return  Proj;
  }

  /**
   * The ProjectionControl controls the point setting for calc. a transformation
   * @return A MouseListener controlling mouse pointing
   * @see <a href="ContextPanelsrc.html#getProjectionControl">Source</a> 
   */
  public MouseListener getProjectionControl () {
    return  ProjectionControl;
  }

  /**
   * Calc. manual Projection out of the proj. points.
   * @return AffineTransform containing transformation-mater
   * @see <a href="ContextPanelsrc.html#calcManProjection">Source</a> 
   */
  public AffineTransform calcManProjection () {
    double extra = getBordSpc();                //extra portion at every border of 30 pix
    double _wp1x = Double.parseDouble(C1XTF.getText());
    double _wp1y = Double.parseDouble(C1YTF.getText());
    double _wp2x = Double.parseDouble(C2XTF.getText());
    double _wp2y = Double.parseDouble(C2YTF.getText());
    boolean isvalid = ((ProjPoint1.getX() != ProjPoint2.getX()) && (ProjPoint1.getY()
        != ProjPoint2.getY()) && (_wp1x != _wp2x) && (_wp1y != _wp2y));
    if (!isvalid) {
      JOptionPane.showMessageDialog(mw, "Projection is invalid! Old values will be restored!");
      ProjPoint1.setLocation(Dev1);
      ProjPoint2.setLocation(Dev2);
      C1XTF.setText(Double.toString(World1.getX()));
      C1YTF.setText(Double.toString(World1.getY()));
      C2XTF.setText(Double.toString(World2.getX()));
      C2YTF.setText(Double.toString(World2.getY()));
      mw.GraphDisplay.repaint();
      return  mw.allProjection;
    }
    World1.setLocation(_wp1x, _wp1y);
    World2.setLocation(_wp2x, _wp2y);
    Dev1.setLocation(Math.min(ProjPoint1.getX(), ProjPoint2.getX()), Math.min(ProjPoint1.getY(), 
        ProjPoint2.getY()));
    Dev2.setLocation(Math.max(ProjPoint1.getX(), ProjPoint2.getX()), Math.max(ProjPoint1.getY(), 
        ProjPoint2.getY()));
    //Dev2.setLocation(ProjPoint2.getX(),ProjPoint2.getY());
    Rectangle2D.Double r = getWBB();
    double m00 = (Dev2.getX() - Dev1.getX())/(_wp2x - _wp1x);
    double m02 = Dev1.getX() - m00*_wp1x;
    double m11 = (Dev2.getY() - Dev1.getY())/(_wp2y - _wp1y);
    double m12 = Dev1.getY() - m11*_wp1y;
    Point2D DevBB1 = new Point2D.Double(r.getX()*m00 + m02,                     //+Dev1.getX()+extra,
    r.getY()*m11 + m12);        //+Dev1.getY()+extra);
    Point2D DevBB2 = new Point2D.Double((r.getX() + r.getWidth())*m00 + m02,                    //+Dev2.getX()+extra,
    (r.getY() + r.getHeight())*m11 + m12);      //+Dev2.getY()+extra);
    //m00 = (DevBB2.getX() - DevBB1.getX())/(r.getWidth());
    m02 -= Math.min(DevBB1.getX(), DevBB2.getX()) - extra;      //(- DevBB1.getX()+extra)-r.getX()*m00;
    ///m11 = (DevBB2.getY() - DevBB1.getY())/(r.getHeight());
    m12 -= Math.min(DevBB1.getY(), DevBB2.getY()) - extra;      //= (-DevBB1.getY()+extra)-r.getY()*m11;
    MapOfs = new Point((int)(-Math.min(DevBB1.getX(), DevBB2.getX()) + extra), 
        (int)(-Math.min(DevBB1.getY(), DevBB2.getY()) + extra));
    double m01 = 0.0;
    double m10 = 0.0;
    return  new AffineTransform(m00, m10, m01, m11, m02, m12);
  }

  /**
   * Calculates the actual WorldBoundingBox (WBB)-intervals and display them in the upper left
   * @see <a href="ContextPanelsrc.html#setWBLabel">Source</a> 
   */
  private void setWBLabel () {
    if (mw.BBoxWC == null)
      return;
    double _wp1x = mw.BBoxWC.getX();
    double _wp1y = mw.BBoxWC.getY();
    double _wp2x = _wp1x + mw.BBoxWC.getWidth();
    double _wp2y = _wp1y + mw.BBoxWC.getHeight();
    WBx1L.setText(Double.toString(_wp1x).concat("       ").substring(0, 7));
    WBy1L.setText(Double.toString(_wp1y).concat("       ").substring(0, 7));
    WBx2L.setText(Double.toString(_wp2x).concat("       ").substring(0, 7));
    WBy2L.setText(Double.toString(_wp2y).concat("       ").substring(0, 7));
  }

  /**
   * 
   * @return The offset for displaying the background image
   * @see <a href="ContextPanelsrc.html#getMapOfs">Source</a> 
   */
  public Point getMapOfs () {
    return  MapOfs;
  }

  /**
   * 
   * @return The manual input of WBB-intervals
   * @see <a href="ContextPanelsrc.html#getWBB">Source</a> 
   */
  public Rectangle2D.Double getWBB () {
    double _wp1x = mw.BBoxWC.getX();
    double _wp1y = mw.BBoxWC.getY();
    double _wp2x = _wp1x + mw.BBoxWC.getWidth();
    double _wp2y = _wp1y + mw.BBoxWC.getHeight();
    try {
      _wp1x = Double.parseDouble(WBx1TF.getText());
    } catch (NumberFormatException n) {}
    try {
      _wp1y = Double.parseDouble(WBy1TF.getText());
    } catch (NumberFormatException n) {}
    try {
      _wp2x = Double.parseDouble(WBx2TF.getText());
    } catch (NumberFormatException n) {}
    try {
      _wp2y = Double.parseDouble(WBy2TF.getText());
    } catch (NumberFormatException n) {}
    return  new Rectangle2D.Double(_wp1x, _wp1y, _wp2x - _wp1x, _wp2y - _wp1y);
  }

  /**
   * Calculates the auto-projection, so that all objects fit into displayed window.
   * @return The transformation matrix
   * @see <a href="ContextPanelsrc.html#calcAutoProjection">Source</a> 
   */
  public AffineTransform calcAutoProjection () {
    double _wp1x = mw.BBoxWC.getX();
    double _wp1y = mw.BBoxWC.getY();
    double _wp2x = _wp1x + mw.BBoxWC.getWidth();
    double _wp2y = _wp1y + mw.BBoxWC.getHeight();
    if (NoClipRB.isSelected()) {
      ProjPoint1.setLocation(0.0, 0.0);
      ProjPoint2.setLocation(mw.BBoxWC.getWidth(), mw.BBoxWC.getHeight());
      double m02 = ProjPoint2.getX() - _wp2x;
      double m12 = ProjPoint2.getY() - _wp2y;
      return  new AffineTransform(1.0, 0.0, 0.0, 1.0, m02, m12);
    } 
    else {
      ProjPoint1.setLocation(0.0, 0.0);
      ProjPoint2.setLocation(mw.ClipRect.getWidth(), mw.ClipRect.getHeight());
      boolean isvalid = ((ProjPoint1.getX() != ProjPoint2.getX()) && (ProjPoint1.getY()
          != ProjPoint2.getY()) && (_wp1x != _wp2x) && (_wp1y != _wp2y));
      if (!isvalid) {
        JOptionPane.showMessageDialog(mw, "No BoundingBox present!\n Reset <No Projection");
        NoProjRB.setSelected(true);
        return  mw.allProjection;
      }
      double m00 = (ProjPoint2.getX() - ProjPoint1.getX())/(_wp2x - _wp1x);
      double m02 = ProjPoint2.getX() - m00*_wp2x;
      double m11 = (ProjPoint2.getY() - ProjPoint1.getY())/(_wp2y - _wp1y);
      double m12 = ProjPoint2.getY() - m11*_wp2y;
      System.out.println("m00=" + m00);
      System.out.println("m02=" + m02);
      System.out.println("m11=" + m11);
      System.out.println("m12=" + m12);
      double m01 = 0.0;
      double m10 = 0.0;
      return  new AffineTransform(m00, m10, m01, m11, m02, m12);
    }
  }

  /**
   * 
   * @return The extra space in pixel around WBB
   * @see <a href="ContextPanelsrc.html#getBordSpc">Source</a> 
   */
  public double getBordSpc () {
    int bs = 30;
    try {
      bs = Integer.parseInt(bordspcT.getText());
      //ClipHeight=Double.parseDouble(YClipTF.getText());
    } catch (NumberFormatException n) {}
    return  (double)bs;
  }
/** A special JLAbel, that paints corsses under mouse-coordinates when pressed.
* @see <a href="ContextPanelsrc.html#ProjLabel">Source</a> 
   */
  class ProjLabel extends JLabel {

    private void paintTarget (Point2D.Double p, Color c, Graphics g) {
      g.setColor(c);
      double sf = mw.getZoomFactor();
      p = new Point2D.Double(p.getX()*sf, p.getY()*sf);
      g.drawLine((int)p.getX(), (int)p.getY() - 10, (int)p.getX(), (int)p.getY()
          + 10);
      g.drawLine((int)p.getX() - 10, (int)p.getY(), (int)p.getX() + 10, (int)p.getY());
    }
    public void paintComponent (Graphics g) {
      super.paintComponent(g);
      paintTarget(ProjPoint1, Color.blue, g);
      paintTarget(ProjPoint2, Color.red, g);
    }
  }
}



