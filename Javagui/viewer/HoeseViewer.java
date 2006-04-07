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

import  javax.swing.*;
import  java.awt.*;
import  java.awt.event.*;
import  java.net.URL;
import  java.io.*;
import  java.util.Properties;
import  java.util.Vector;
import  java.util.StringTokenizer;
import  java.util.Enumeration;
import  sj.lang.ListExpr;
import  java.util.ListIterator;
import  javax.swing.event.*;
import  java.awt.geom.*;
import  javax.swing.border.*;
import  viewer.hoese.*;
import  gui.SecondoObject;
import  gui.idmanager.*;
import  project.*;
import  components.*;
import  java.awt.image.*;
import  viewer.hoese.algebras.Dsplpointsequence;
import javax.swing.plaf.basic.*;
import tools.Reporter;

/**
 * this is a viewer for spatial and temporal spatial objects
 * but this viewer can display other query results
 */
public class HoeseViewer extends SecondoViewer {
  private JOptionPane MessageControl = new JOptionPane();
  private MenuVector MenuExtension = new MenuVector();
  private QueryListSelectionListener DoQuerySelection;
  private JSplitPane VisualPanel;
  private JSplitPane VisComPanel;
  private JPanel dummy = new JPanel();  // a place holder for timepanel

  /** The top-right component for graph. objects in layer */
  public GraphWindow GraphDisplay;
  /** The left component for alphanumeric query-output */
  public TextWindow TextDisplay;
  private JPanel SpatioTempPanel;

  /** A link to the last query-result */
  public QueryResult CurrentQueryResult;
  private TimePanel TimeDisplay;

  /** The Zoomfactor of the GraphWindow */
  protected double ZoomFactor = 1;
  private AffineTransform ZoomTransform = new AffineTransform();
  private SelMouseAdapter SelectionControl;

  private Interval TimeBounds;
  private LongScrollBar TimeSlider;

  /** The list of categories present in the app. */
  public Vector Cats;

  /** The component which contains the togglebuttons for the layer at the left side of GraphWindow */
  public JPanel LayerSwitchBar;

  /** Rectangle of the WorldBoundingBox */
  public Rectangle2D.Double BBoxWC = new Rectangle2D.Double(0, 0, 0, 0);

  /** Rectangle of the Device-coordinates */
  public Rectangle BBoxDC = new Rectangle(0, 0, 0, 0);
  private int oldDL;

  private boolean isMouseSelected = false;
  private DsplGraph selGraphObj;
  private DsplBase selBaseObj;

  /** The projection from World (userspace) to Device-ccord. */
  public AffineTransform allProjection;

  /** The rectangle where device-coords are clipped. */
  public Rectangle ClipRect = null;
  private JLabel MouseKoordLabel;
  private JLabel actTimeLabel;
  private JButton DecrementSpeedBtn;
  private JButton IncrementSpeedBtn;
  public JScrollPane GeoScrollPane;
  private final static String CONFIGURATION_FILE = "GBS.cfg";
  private final static int NOT_ERROR_CODE = ServerErrorCodes.NOT_ERROR_CODE;


   /** The maximum length of a number for the mousekoordlabel */
   private static final int MAX_COORD_LENGTH = 12;
   /** Ensure that the next constant contains at least MAX_COORD_LENGTH whitespaces*/
   private static final String COORD_EXT="             ";

  /** The main configuration parameter hash-table */
  public static Properties configuration;

 /* File-Menu */
  private javax.swing.JMenu jMenu1;
  private javax.swing.JMenuItem jMenu_NewSession;
  private javax.swing.JMenuItem jMenu_OpenSession;
  private javax.swing.JMenuItem jMenu_CloseSession;
  private javax.swing.JMenuItem jMenu_SaveSession;
  private javax.swing.JMenuItem jMenu_SaveSessionAs;
  private javax.swing.JSeparator jSeparator1;
  private javax.swing.JMenuItem jMenu_Browse;
  private javax.swing.JSeparator jSeparator2;
  private javax.swing.JMenuItem MI_SaveAttrCatLink;
  private javax.swing.JMenuItem MI_LoadAttrCatLink;
  private javax.swing.JMenuItem MI_AppendAttrCatLink;


 /* settings-menu */
  private javax.swing.JMenu jMenuGui;
  private JMenuItem jMenuBackgroundColor;
  private static boolean changeAllBackgrounds=false; 
  private javax.swing.JMenuItem MINewCat;

  private JMenu Menu_Prj;
  private JMenuItem MI_PrjSettings;
  private ProjectionSelector PrjSelector;

  /** True if menu-entry 'automatic category' is selected */
  public javax.swing.JCheckBoxMenuItem isAutoCatMI;
  private JMenuItem selectSequenceCat;
  private JRadioButtonMenuItem create_Rectangle_MI;
  private JRadioButtonMenuItem create_PointSequence_MI;
  private JRadioButtonMenuItem create_FilledPointSequence_MI;
  private JRadioButtonMenuItem create_Points_MI;
  private JRadioButtonMenuItem create_Line_MI;
  private JRadioButtonMenuItem create_Point_MI;
  private JRadioButtonMenuItem create_Region_MI;


  private javax.swing.JSeparator jSeparator5;
  private javax.swing.JMenuItem jMIShowCE;
  private javax.swing.JMenuItem MIQueryRep;

  private javax.swing.JMenuItem MILayerMgmt;

  /** Object Menu */
  private javax.swing.JMenu MenuObject;
  private javax.swing.JMenuItem MIHideObject;
  private javax.swing.JMenuItem MIShowObject;
  private javax.swing.JMenuItem MILabelAttr;
  private javax.swing.JMenuItem jMenuItem8;
  private javax.swing.JMenuItem MIMoveTop;
  private javax.swing.JMenuItem MIMoveUp;
  private javax.swing.JMenuItem MIMoveDown;
  private javax.swing.JMenuItem MIMoveBottom;
  private JMenuItem MIZoomOut;
  private JMenuItem MIZoomMinus;
  private JMenuItem MIZoomPlus;
  private JRadioButtonMenuItem RBMICustTI;
  private AbstractAction AAZoomOut;
  private AbstractAction AACatEdit;
  private AbstractAction AAViewCat;
  private AbstractAction AASetBackground;
  private AbstractAction AARemoveBackground;
  private AbstractAction AACaptureBackground;
  private AbstractAction AALabelAttr;
  private String tok, PickTok;


  /** a FileChooser for Sessions */
  private JFileChooser FC_Session=new JFileChooser();
  /** a FileChooser for Categories*/
  private JFileChooser FC_Category=new JFileChooser();
  /** a FileChooser for References Attribute value -> Category */
  private JFileChooser FC_References = new JFileChooser();
  /** a FileChooser for sving images as pong graphics */
  private JFileChooser FC_Images = new JFileChooser();
  /** a dialog to input a time value */
  TimeInputDialog TimeInput = new TimeInputDialog(this.getMainFrame());

  private String TexturePath;
  private String CatPath;
  private String FileSeparator;


  /** a dialog for assigning a background image */
  private BackGroundImage bgImage;

  /** the maximum number of pixels for capturing the bakcground */
  private long MAXCAPTUREPIXELS = 10000000L;

  /** set the border when capturing the visible rectangle */
  private int CaptureBorder = 200;

  /** a point value for shipping data between this class and the ProjectionManager
    **/
  static java.awt.geom.Point2D.Double aPoint = new java.awt.geom.Point2D.Double();

  /** an empty style for painting line objects */
  public static Composite emptyStyle = AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.0f);

  /** a button for creating a point sequence **/
  private JButton createPointSequenceBtn;
  private boolean createPointSequenceActivated=false;
  private CreatePointSequenceListener createPointSequenceListener;


  /**
   * Creates a MainWindow with all its components, initializes Secondo-Server, loads the
   * standard-categories, etc.
   */
  public HoeseViewer() {
    try {
      UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
    } catch (Exception exc) {
      Reporter.debug("Error loading L&F: " , exc);
    }

    Properties SystemProps = System.getProperties();
    FileSeparator = SystemProps.getProperty("file.separator");
    if(FileSeparator==null){
        FileSeparator ="/";
        Reporter.writeError("i can't determine the Systems file separator");
     }
     TexturePath = FileSeparator;

    initComponents();
    init();
    Cats = new Vector(30, 10);
    Cats.add(Category.getDefaultCat());
    //Load S Categories
    String Catfiles = configuration.getProperty("StandardCats");
    if (Catfiles != null) {
      StringTokenizer ST = new StringTokenizer(Catfiles," \n\t");
      while(ST.hasMoreTokens()){
        String FileName = CatPath+ST.nextToken().trim();
        ListExpr le = new ListExpr();
  File F1 = new File(FileName);
  if(!F1.exists()){
     Reporter.writeError("i can't find the StandardCategoryFile "+FileName);
  }else
           if(le.readFromFile(FileName)!=0){
              Reporter.writeError("i can't load the file "+FileName);
           } else{
             if(!readAllCats(le))
               Reporter.writeError("no categories in file "+FileName);
           }
      }
    }



    MouseKoordLabel = new JLabel("-------/-------");
    MouseKoordLabel.setFont(new Font("Monospaced", Font.PLAIN, 12));
    MouseKoordLabel.setForeground(Color.black);
    actTimeLabel = new JLabel("No Time");
    actTimeLabel.setFont(new Font("Monospaced", Font.PLAIN, 12));
    actTimeLabel.setForeground(Color.black);

    String Button_File = configuration.getProperty("FasterIcon");
    ImageIcon icon;
    if(Button_File!=null){
       icon = new ImageIcon(ClassLoader.getSystemResource(Button_File));
       if(icon.getImage()!=null){
          IncrementSpeedBtn = new JButton(icon);
          icon.setImage(icon.getImage().getScaledInstance(18,18,Image.SCALE_DEFAULT));
       } else
          IncrementSpeedBtn = new JButton("+");
    }
    else
       IncrementSpeedBtn = new JButton("+");

    Button_File = configuration.getProperty("SlowerIcon");
    if(Button_File!=null){
       icon = new ImageIcon(ClassLoader.getSystemResource(Button_File));
       if(icon.getImage()!=null){
          DecrementSpeedBtn = new JButton(icon);
          icon.setImage(icon.getImage().getScaledInstance(18,18,Image.SCALE_DEFAULT));
       } else
          DecrementSpeedBtn = new JButton("-");
    }
    else
       DecrementSpeedBtn = new JButton("-");


    createPointSequenceBtn = new JButton();
    createPointSequenceBtn.setOpaque(true);
    createPointSequenceListener=new CreatePointSequenceListener();
    
    ActionListener createPointSequenceAL = new ActionListener(){
          public void actionPerformed(ActionEvent evt){
              Object src = evt.getSource();
              if(!(src instanceof JButton))
                 return;
              JButton Butt = (JButton) src;
              if(HoeseViewer.this.createPointSequenceActivated){
                  // finish the input
                  if(!src.equals(lastSrc)) // ignore messages from the other button
                    return;   
                  Butt.setBackground(Butt.getParent().getBackground());
                  createPointSequenceActivated=false;
                  GraphDisplay.removeMouseListener(createPointSequenceListener);
                  GraphDisplay.removeMouseMotionListener(createPointSequenceListener);
                  createPointSequenceListener.closeSequence(); 
                  SelectionControl.enableSelection(true); 
                  GraphDisplay.repaint();
                  create_Rectangle_MI.setEnabled(true);
                  create_Points_MI.setEnabled(true);
                  create_Line_MI.setEnabled(true);
                  create_PointSequence_MI.setEnabled(true);
                  create_Point_MI.setEnabled(true);
                  create_FilledPointSequence_MI.setEnabled(true);
                  create_Region_MI.setEnabled(true);
              } else{ // start input
                  lastSrc = src;
                  Butt.setBackground(aColor);
                  createPointSequenceActivated=true; 
                  SelectionControl.enableSelection(false);
                  GraphDisplay.addMouseListener(createPointSequenceListener);
                  GraphDisplay.addMouseMotionListener(createPointSequenceListener);
                  create_Rectangle_MI.setEnabled(false);
                  create_Points_MI.setEnabled(false);
                  create_Line_MI.setEnabled(false);
                  create_PointSequence_MI.setEnabled(false);
                  create_FilledPointSequence_MI.setEnabled(false);
                  create_Point_MI.setEnabled(false);
                  create_Region_MI.setEnabled(false);
              }            
          }
          Color aColor = Color.GREEN;
//          Color dColor = Color.LIGHT_GRAY;
          Object lastSrc;
    };

    createPointSequenceBtn.addActionListener(createPointSequenceAL);
    ActionListener SpeedControlListener = new ActionListener(){
         public void actionPerformed(ActionEvent evt){
            Object src = evt.getSource();
            long ui = TimeSlider.getUnitIncrement();
            if(src.equals(DecrementSpeedBtn)){
                ui = Math.max(ui/2,1);
            } else if(src.equals(IncrementSpeedBtn)){
                ui = ui*2;
            }
            TimeSlider.setUnitIncrement(ui);
            TimeSlider.setBlockIncrement(ui*60);
         };
    };

    DecrementSpeedBtn.addActionListener(SpeedControlListener);
    IncrementSpeedBtn.addActionListener(SpeedControlListener);
    IncrementSpeedBtn.setPreferredSize(new Dimension(22,22));
    DecrementSpeedBtn.setPreferredSize(new Dimension(22,22));


    JToolBar SpeedControlPanel = new JToolBar();
    SpeedControlPanel.putClientProperty("JToolBar.isRollover", Boolean.TRUE);
    SpeedControlPanel.setFloatable(false);
    SpeedControlPanel.add(DecrementSpeedBtn);
    SpeedControlPanel.add(new JLabel("speed"));
    SpeedControlPanel.add(IncrementSpeedBtn);


    JToolBar jtb = new JToolBar();
    jtb.putClientProperty("JToolBar.isRollover", Boolean.TRUE);
    jtb.setFloatable(false);
    JButton ctrls[] = new JButton[6];
    Button_File = configuration.getProperty("PlayIcon");
    if(Button_File!=null)
        ctrls[0] = new JButton(new ImageIcon(ClassLoader.getSystemResource(Button_File)));
    else
        ctrls[0] = new JButton(">");

    Button_File = configuration.getProperty("ReverseIcon");
    if(Button_File!=null)
       ctrls[1] = new JButton(new ImageIcon(ClassLoader.getSystemResource(Button_File)));
    else
       ctrls[1] = new JButton("<");

    Button_File = configuration.getProperty("PlayDefIcon");
    if(Button_File!=null)
       ctrls[2] = new JButton(new ImageIcon(ClassLoader.getSystemResource(Button_File)));
    else
       ctrls[2] = new JButton(">:");

    Button_File = configuration.getProperty("ToendIcon");
    if(Button_File!=null)
       ctrls[3] = new JButton(new ImageIcon(ClassLoader.getSystemResource(Button_File)));
    else
       ctrls[3] = new JButton(">|");

    Button_File = configuration.getProperty("TostartIcon");
    if(Button_File!=null)
        ctrls[4] = new JButton(new ImageIcon(ClassLoader.getSystemResource(Button_File)));
    else
       ctrls[4] = new JButton("|<");


    Button_File = configuration.getProperty("StopIcon");
    if(Button_File!=null)
        ctrls[5] = new JButton(new ImageIcon(ClassLoader.getSystemResource(Button_File)));
    else
       ctrls[5] = new JButton("[]");


    JPanel AnimControlPanel = new JPanel(new GridLayout(1,9));
    ActionListener al = new AnimCtrlListener();
    for (int i = 0; i < ctrls.length; i++) {
      ctrls[i].setActionCommand(Integer.toString(i));
      ctrls[i].addActionListener(al);
      if(i!=2){  // ignore the playdef button
         AnimControlPanel.add(ctrls[i]);
      }
    }


    JPanel ControlPanel=new JPanel(new GridLayout(2,1));
    ControlPanel.add(AnimControlPanel);
    ControlPanel.add(SpeedControlPanel);
    AnimControlPanel.setPreferredSize(new Dimension(100,15));

    jtb.add(ControlPanel);
    jtb.addSeparator();


    TimeSlider = new LongScrollBar(0, 0, 1);
    TimeSlider.addChangeValueListener(new TimeChangeListener());
    TimeSlider.setPreferredSize(new Dimension(400, 20));
    TimeSlider.setUnitIncrement(1000); // 1 sec
    TimeSlider.setBlockIncrement(60000); // 1 min

    JPanel PositionsPanel = new JPanel(new GridLayout(1,2));
    PositionsPanel.add(actTimeLabel);
    PositionsPanel.add(MouseKoordLabel);
    JPanel aPanel = new JPanel(new GridLayout(2,1));
    aPanel.add(createPointSequenceBtn);
    aPanel.add(new JLabel());

    JPanel TimeSliderAndLabels = new JPanel(new BorderLayout());
    TimeSliderAndLabels.add(TimeSlider,BorderLayout.NORTH);
    TimeSliderAndLabels.add(PositionsPanel,BorderLayout.SOUTH);
    actTimeLabel.setHorizontalAlignment(SwingConstants.CENTER);
    MouseKoordLabel.setHorizontalAlignment(SwingConstants.CENTER);
    jtb.add(TimeSliderAndLabels);
    jtb.add(aPanel);   
 
    TextDisplay = new TextWindow(this);
    DoQuerySelection = new QueryListSelectionListener();
    allProjection = new AffineTransform();
    allProjection.scale(ZoomFactor, ZoomFactor);
    LayerSwitchBar = new JPanel();
    GraphDisplay = new GraphWindow(this);
    GraphDisplay.setOpaque(true); // needed for background-color
    MouseKoordLabel.setOpaque(true);


    MouseKoordLabel.setHorizontalAlignment(SwingConstants.RIGHT);
    GraphDisplay.addMouseMotionListener(new MouseMotionListener() {
      public void mouseMoved (MouseEvent e) {
        //Koordinaten in Weltkoordinaten umwandeln
        Point2D.Double p = new Point2D.Double();
        try {
          p = (Point2D.Double)allProjection.inverseTransform(e.getPoint(),p);
        } catch (Exception ex) {}
        // compute the inverse projection if possible 
        if(!ProjectionManager.isReversible()){
           MouseKoordLabel.setBackground(Color.PINK);
           MouseKoordLabel.setText(Double.toString(p.getX()) + " / " + 
                                   Double.toString(p.getY()).concat(COORD_EXT).substring(0,
                                   MAX_COORD_LENGTH));
        } else{
           MouseKoordLabel.setBackground(getBackground());
           double px = p.getX();
           double py = p.getY();
           if(ProjectionManager.getOrig(px,py,aPoint)){           
               double x = aPoint.x;
               double y = aPoint.y;
               MouseKoordLabel.setText( (""+x)+
                                         " / "+(""+y).concat(COORD_EXT).substring(0,MAX_COORD_LENGTH));    
          }else{
               MouseKoordLabel.setBackground(Color.RED);
               MouseKoordLabel.setText( (""+px) + " / "+ 
                                         (""+py).concat(COORD_EXT).substring(0,MAX_COORD_LENGTH));    
 
          }
        }
      }
      public void mouseDragged(MouseEvent evt){
         mouseMoved(evt);
      }
    });

    SelectionControl = new SelMouseAdapter();
    GraphDisplay.addMouseListener(SelectionControl);
    SpatioTempPanel = new JPanel(new BorderLayout());
    LayerSwitchBar.setPreferredSize(new Dimension(10, 10));
    LayerSwitchBar.setLayout(new BoxLayout(LayerSwitchBar, BoxLayout.Y_AXIS));
    GeoScrollPane = new JScrollPane(GraphDisplay);
    SpatioTempPanel.add(GeoScrollPane, BorderLayout.CENTER);
    SpatioTempPanel.add(LayerSwitchBar, BorderLayout.WEST);
    VisComPanel = new JSplitPane(JSplitPane.VERTICAL_SPLIT, SpatioTempPanel,dummy);
    
    VisComPanel.setOneTouchExpandable(true);
    VisComPanel.setResizeWeight(1);
    TimeDisplay = new TimePanel(this);
    VisualPanel = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, TextDisplay,VisComPanel);
    VisualPanel.setOneTouchExpandable(true);
    VisualPanel.setPreferredSize(new Dimension(800, 600));
    VisualPanel.setResizeWeight(0);

    JPanel pane = new JPanel();
    pane.setLayout(new BorderLayout());
    pane.add(jtb, BorderLayout.NORTH);
    pane.add(VisualPanel, BorderLayout.CENTER);
    pane.setPreferredSize(new Dimension(800, 600));
    setLayout(new BorderLayout());
    add(pane,BorderLayout.CENTER);
    setDivider();
    bgImage = new BackGroundImage(null);

    /* initialize the file chooser for saving images */ 
    FC_Images.setDialogTitle("Save PNG-Image");
    javax.swing.filechooser.FileFilter filter = new javax.swing.filechooser.FileFilter(){
        public boolean accept(File PathName){
           if(PathName==null) return false;
           if(PathName.isDirectory())
              return true;
           if(PathName.getName().endsWith(".png"))
               return true;
           else 
              return false;                
         }
         public String getDescription(){
           return "PNG images";
         }
    };
    FC_Images.setFileFilter(filter);           
    if(changeAllBackgrounds){
       setAllOpaque(this,false);
    }

  }

  /* sets all contained componenets to be opaque */
  public static void setAllOpaque(Component C,boolean enable){
      if(C instanceof JComponent)
         ((JComponent) C).setOpaque(enable);
      if(C instanceof Container){
         Component[] Comps = ((Container)C).getComponents();
         if(Comps!=null){
             for(int i=0;i<Comps.length;i++){
                setAllOpaque(Comps[i],enable);
             }
         }
      }
  }
  /* sets all contained componenets to the desired background */
  public static void setAllBackgrounds(Component C,Color color){
      C.setBackground(color);
      if(C instanceof Container){
         Component[] Comps = ((Container)C).getComponents();
         if(Comps!=null){
             for(int i=0;i<Comps.length;i++){
                setAllBackgrounds(Comps[i],color);
             }
         }
      }
  }


  /**
   * Sets the divider between split-components
   * @see <a href="MainWindowsrc.html#setdivider>Source</a>
   */
  public void setDivider () {
    VisComPanel.setDividerLocation(0.75);
    VisualPanel.setDividerLocation(200);
  }


  /**
   *
   * @return The zoomfactor of the GraphWindow
   * @see <a href="MainWindowsrc.html#getZoomFactor">Source</a>
   */
  public double getZoomFactor () {
    return  ZoomFactor;
  }

  /**
   * Sets the ZoomFactor to zf
   * @param zf
   * @see <a href="MainWindowsrc.html#setZoomFactor">Source</a>
   */
  public void setZoomFactor (double zf) {
    ZoomFactor = zf;
  }

  /**
   * Reads all categories out of the Listexpr le
   * @param le a ListExpr containing Categories
   */
  public boolean readAllCats (ListExpr le) {
    try{
      if(le.listLength()!=2)
         return false;
      if (le.first().atomType() != ListExpr.SYMBOL_ATOM)
         return false;
      if (!le.first().symbolValue().equals("Categories"))
         return false;
      le = le.second();
      if(le.atomType()!=ListExpr.NO_ATOM)
         return false;
      ListExpr aCat;
      while (!le.isEmpty()) {
         aCat = le.first();
         Category cat = Category.ConvertLEtoCat(aCat);
         if (cat != null && Cats.indexOf(cat)<0 )  // only new categories
             Cats.add(cat);
         le = le.rest();
       }
       return true;
     } catch(Exception e){
       return false;
     }
  }


  /**
   *
   * @return A listExpr of all the categories in Cats
   */
  public ListExpr writeAllCats () {
    ListExpr right = null;
    ListExpr theList = null;
    int size = Cats.size();
    for (int i = 0; i <size; i++){
      ListExpr catList = Category.ConvertCattoLE((Category)Cats.get(i));
      if(theList==null){
          theList = ListExpr.oneElemList(catList);
          right = theList;

      }   else{
          right = ListExpr.append(right,catList); 
      }
    }
    if(theList==null) {
       theList = ListExpr.theEmptyList();
    }
    return  ListExpr.twoElemList(ListExpr.symbolAtom("Categories"), theList);
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
   * Init. the menu entries.
   */
  private void initComponents () {

   /** file-menu */
    jMenu1 = new javax.swing.JMenu();
    jMenu_NewSession = new javax.swing.JMenuItem();
    jMenu_OpenSession = new javax.swing.JMenuItem();
    jMenu_SaveSession = new javax.swing.JMenuItem();
    jSeparator1 = new javax.swing.JSeparator();
    jMenu_Browse = new javax.swing.JMenuItem();
    MI_SaveAttrCatLink = new JMenuItem();
    MI_LoadAttrCatLink = new JMenuItem();
    MI_AppendAttrCatLink = new JMenuItem();


 /** Menu Settings */
    jMenuGui = new javax.swing.JMenu();
    jMenuBackgroundColor = new JMenuItem("Background Color");
    jMenuBackgroundColor.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
            Color OldBG = GraphDisplay.getBackground();
            Color BG = JColorChooser.showDialog(
                            HoeseViewer.this,
                            "Choose Background Color",
                             OldBG);
            if(!OldBG.equals(BG)){
                GraphDisplay.setBackground(BG);
                if(changeAllBackgrounds){
                   setAllOpaque(HoeseViewer.this,true); 
                   TimeSlider.setOpaque(false);
                   setAllBackgrounds(HoeseViewer.this,BG);
                }
            }
  }
    });

    MINewCat = new javax.swing.JMenuItem();
    isAutoCatMI = new javax.swing.JCheckBoxMenuItem();
    jSeparator5 = new javax.swing.JSeparator();
    MILayerMgmt = new javax.swing.JMenuItem();
    Menu_Prj = new JMenu("Projections");
    MI_PrjSettings = Menu_Prj.add("Settings");
    Menu_Prj.addSeparator();
    PrjSelector = new ProjectionSelector(Menu_Prj);
    PrjSelector.addProjection(ProjectionManager.getVoidProjection());



/** Menu object */
    MenuObject = new javax.swing.JMenu();
    MIHideObject = new javax.swing.JMenuItem();
    MIShowObject = new javax.swing.JMenuItem();
    jMenuItem8 = new javax.swing.JMenuItem();
    MIMoveTop = new javax.swing.JMenuItem();
    MIMoveUp = new javax.swing.JMenuItem();
    MIMoveDown = new javax.swing.JMenuItem();
    MIMoveBottom = new javax.swing.JMenuItem();
    RBMICustTI = new JRadioButtonMenuItem();


    /** File -Menu **/
    jMenu1.setText("File");
    jMenu_NewSession.setText("New session");
    jMenu_NewSession.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
         on_jMenu_NewSession(evt);
      }
    });
    jMenu1.add(jMenu_NewSession);
    jMenu_OpenSession.setText("Open session");
    jMenu_OpenSession.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        on_jMenu_OpenSession(evt);
      }
    });
    jMenu1.add(jMenu_OpenSession);

    jMenu_SaveSession.setText("Save session");
    jMenu_SaveSession.addActionListener(new java.awt.event.ActionListener() {
       public void actionPerformed (java.awt.event.ActionEvent evt) {
          on_jMenu_SaveSession(evt);
       }
    });
    jMenu1.add(jMenu_SaveSession);
    jMenu1.add(jSeparator1);

    JMenuItem MIloadCat = new JMenuItem("Load categories");
    MIloadCat.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        FC_Category.setFileSelectionMode(JFileChooser.FILES_ONLY);
        int returnVal = FC_Category.showOpenDialog(HoeseViewer.this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
          File file = FC_Category.getSelectedFile();
          ListExpr le = new ListExpr();
          String suc;
          if (le.readFromFile(file.getPath()) == 0) {
            Cats = new Vector(30, 10);
            Cats.add(Category.getDefaultCat());
            suc ="OK";
      if (readAllCats(le))
         Reporter.showInfo("categories loaded");
      else
         Reporter.showError("error in categories");
          }
          else{
            suc = "Failed";
            Reporter.showError("error in categories");
    }
        }
      }
    });

    jMenu1.add(MIloadCat);

    JMenuItem MISaveCat = new JMenuItem("Save categories");
    MISaveCat.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        FC_Category.setFileSelectionMode(JFileChooser.FILES_ONLY);
  int returnVal = FC_Category.showSaveDialog(HoeseViewer.this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
          File file = FC_Category.getSelectedFile();
          ListExpr le = writeAllCats();
          String suc;
          if (le.writeToFile(file.getPath())!= 0)
             Reporter.showError("save category failed");
          else
             Reporter.showInfo("success");
        }
      }
    });


    jMenu1.add(MISaveCat);


    MI_SaveAttrCatLink.setText("Save  Attribute -> Category");
    MI_SaveAttrCatLink.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
           if(ManualLinkPool.numberOfLinks()==0){
              Reporter.showError("no references defined");
        return;
     }
     if(FC_References.showSaveDialog(HoeseViewer.this)==JFileChooser.APPROVE_OPTION){
              ListExpr LE = ManualLinkPool.toListExpr();
        File f = FC_References.getSelectedFile();
        if(LE.writeToFile(f.getPath())!=0)
             Reporter.showError("saves references failed");
        else
             Reporter.showError("save references successful");
     }
       }});

    MI_LoadAttrCatLink.setText("Load  Attribute -> Category");
    MI_LoadAttrCatLink.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          if(FC_References.showOpenDialog(HoeseViewer.this)==JFileChooser.APPROVE_OPTION)
       if (loadReferences(FC_References.getSelectedFile())){
           Reporter.showInfo("load references successful");
       }else{
              Reporter.showError("load references failed");
       }
    }});


    MI_AppendAttrCatLink.setText("Append Attribute -> Category");
    MI_AppendAttrCatLink.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
                Reporter.showError("this function is not implemented");

    }});




    jMenu1.add(new JSeparator());
    jMenu1.add(MI_SaveAttrCatLink);
    jMenu1.add(MI_LoadAttrCatLink);
    //jMenu1.add(MI_AppendAttrCatLink);

    JMenuItem SaveGraph = new JMenuItem("Save Graphic");
    SaveGraph.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
           // create the image
           Rectangle2D R = GraphDisplay.getBounds();
           Reporter.writeWarning("Restrict the size of the image  !!!");
           BufferedImage bi = new BufferedImage((int)R.getWidth(),(int)R.getHeight(),
                                                 BufferedImage.TYPE_INT_RGB);
           Graphics2D g = bi.createGraphics();
           GraphDisplay.printAll(g);
           if(FC_Images.showSaveDialog(HoeseViewer.this)==JFileChooser.APPROVE_OPTION){
              File F = FC_Images.getSelectedFile();
              try{
                 javax.imageio.ImageIO.write(bi,"png",F); 
              } catch(Exception e){
                 Reporter.showError("Error in saving image ");
              }
           }
           g.dispose(); 
           System.gc();
           System.runFinalization();
        }
    });

   jMenu1.addSeparator();
   jMenu1.add(SaveGraph);

    MenuExtension.addMenu(jMenu1);

    jMenuGui.setText("Settings");

    jMenuGui.add(Menu_Prj);
    MI_PrjSettings.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          ProjectionManager.showSettings();
       }});


    isAutoCatMI.setText("Auto category");
    jMenuGui.add(isAutoCatMI);
    JMenu createMenu = new JMenu("Object Creation");
   // jMenuGui.add(createMenu); // moved into the main menu
    selectSequenceCat = new JMenuItem("Select Category");
    createMenu.add(selectSequenceCat);

    create_Rectangle_MI=new JRadioButtonMenuItem("Rectangle");
    create_PointSequence_MI=new JRadioButtonMenuItem("Point sequence");
    create_FilledPointSequence_MI=new JRadioButtonMenuItem("Filled point sequence");
    create_Points_MI=new JRadioButtonMenuItem("Points");
    create_Point_MI=new JRadioButtonMenuItem("Point");
    create_Line_MI=new JRadioButtonMenuItem("Line");
    create_Region_MI = new JRadioButtonMenuItem("Region");
    
    createMenu.add(create_Rectangle_MI);
    createMenu.add(create_PointSequence_MI);
    createMenu.add(create_FilledPointSequence_MI);
    createMenu.add(create_Points_MI);
    createMenu.add(create_Point_MI);
    createMenu.add(create_Line_MI);
    createMenu.add(create_Region_MI);
    ButtonGroup createTypes = new ButtonGroup();
    createTypes.add(create_Rectangle_MI);
    createTypes.add(create_PointSequence_MI);
    createTypes.add(create_FilledPointSequence_MI);
    createTypes.add(create_Points_MI);
    createTypes.add(create_Point_MI);
    createTypes.add(create_Line_MI);
    createTypes.add(create_Region_MI);
    create_PointSequence_MI.setSelected(true);

    // add an changeListener for inrforming the createPointSequnceListener when needed
    ChangeListener cL = new ChangeListener(){
        public void stateChanged(ChangeEvent evt){
           if(create_Rectangle_MI.isSelected())
                createPointSequenceListener.setMode(CreatePointSequenceListener.RECTANGLE_MODE);
           if(create_PointSequence_MI.isSelected())
                createPointSequenceListener.setMode(CreatePointSequenceListener.POINT_SEQUENCE_MODE);
           if(create_FilledPointSequence_MI.isSelected())
                createPointSequenceListener.setMode(CreatePointSequenceListener.FILLED_POINT_SEQUENCE_MODE);
           if(create_Points_MI.isSelected())
                createPointSequenceListener.setMode(CreatePointSequenceListener.POINTS_MODE);
           if(create_Point_MI.isSelected())
                createPointSequenceListener.setMode(CreatePointSequenceListener.POINT_MODE);
           if(create_Line_MI.isSelected())
                createPointSequenceListener.setMode(CreatePointSequenceListener.LINE_MODE);
           if(create_Region_MI.isSelected())
                createPointSequenceListener.setMode(CreatePointSequenceListener.REGION_MODE);
        }
    };
   create_Rectangle_MI.addChangeListener(cL); 
   create_PointSequence_MI.addChangeListener(cL); 
   create_FilledPointSequence_MI.addChangeListener(cL); 
   create_Points_MI.addChangeListener(cL); 
   create_Point_MI.addChangeListener(cL); 
   create_Line_MI.addChangeListener(cL);
   create_Region_MI.addChangeListener(cL); 

   selectSequenceCat.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent evt){
             if(Cats==null || Cats.size()==0){
                Reporter.showError("No categories available");
                return;
             }
             Category[] catarray = new Category[Cats.size()];
             Category lastcat = createPointSequenceListener.getCat();
             for(int i=0;i<catarray.length;i++){
                  catarray[i] = (Category) Cats.get(i);
             } 
             Object res = JOptionPane.showInputDialog(HoeseViewer.this,
                                                      "Select a category",
                                                      "CategorySelector",
                                                      JOptionPane.YES_NO_OPTION,
                                                      null, // icon
                                                      catarray,lastcat);
             if(res!=null){
                createPointSequenceListener.setCategory((Category)res);
                if(createPointSequenceActivated && !res.equals(lastcat)){
                     GraphDisplay.repaint();
                }
             }
                                                       
         }
    });

    MIQueryRep = new JMenuItem("Query representation");
    MIQueryRep.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        QueryResult qr = (QueryResult)TextDisplay.getQueryCombo().getSelectedItem();
        if (qr!=null){
           GraphDisplay.newQueryRepresentation(qr.getGraphObjects());
           GraphDisplay.repaint();
        }
      }
    });

    jMenuGui.add(MIQueryRep);
    jMenuGui.add(jSeparator5);
    AACatEdit = new AbstractAction("Category editor"){
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        new CategoryEditor(HoeseViewer.this, true).show();
        GraphDisplay.repaint();
      }
    };

    jMIShowCE = jMenuGui.add(AACatEdit);
    
    JMenu BGMenu = new JMenu("BackGround");
    
    AASetBackground = new AbstractAction("set image") {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        bgImage.setVisible(true);
        GraphDisplay.updateBackground();
      }
    };

    AARemoveBackground = new AbstractAction("remove image"){
       public void actionPerformed(java.awt.event.ActionEvent evt){
         bgImage.setImage(null);
         GraphDisplay.updateBackground();
       }
    };

    AACaptureBackground = new AbstractAction("capture image"){
       public void actionPerformed(java.awt.event.ActionEvent evt){
           Rectangle2D r1 = GraphDisplay.getBounds();
           Rectangle2D R =  new Rectangle2D.Double(r1.getX(),r1.getY(),r1.getWidth(),r1.getHeight());
           double w =  R.getWidth();
           double h =  R.getHeight();
           if(w<=0 | h<=0){
              Reporter.showError("cannot capture the background");
              bgImage.setImage(null);
              GraphDisplay.updateBackground();
              return; 
           } 
           try{
              boolean scale = false; // image to scale ?
              double sf=1.0; // the scale factor
              long MAXPIXELS = MAXCAPTUREPIXELS;            
              if((long)(w*h) > MAXPIXELS){
                 sf = Math.sqrt( MAXPIXELS/ ((R.getWidth()*R.getHeight())));
                 w =  (w*sf);
                 h =  (h*sf);
                 scale = true;
              }

              BufferedImage bi = new BufferedImage((int)w,(int)h,BufferedImage.TYPE_3BYTE_BGR);
              Graphics2D g = bi.createGraphics();
              
              if(scale){
                AffineTransform sdat = new AffineTransform();
                sdat.setToScale(sf,sf);   
                g.setTransform(sdat);
              } 
              GraphDisplay.printAll(g);
              bgImage.setImage(bi);
              AffineTransform at = (AffineTransform) allProjection.clone();
              // convert the bounding bocx of the GraphDisplay into
              // world coordinates.
              try{
                  R.setRect(0,0,R.getWidth(),R.getHeight());    
                  R = at.createInverse().createTransformedShape(R).getBounds2D();
              } catch(Exception e){
                  Reporter.showError("Cannot determine the bounding box of this image");
              }
              bgImage.setBBox(R.getX(),R.getY(),R.getWidth(),R.getHeight());
              g.dispose();
              GraphDisplay.updateBackground();
            }catch(Exception e){
               // because large sized data are processed, a OutOfMemory error is possible
               Reporter.showError("an error in capturing the background is occured");
               bgImage.setImage(null); 
           }
           System.gc();
           System.runFinalization();
       }
    };
   
    AbstractAction AACaptureRect = new AbstractAction("capture visible rect"){
       public void actionPerformed(java.awt.event.ActionEvent evt){
           try{
              // we will not scale it beacuse the maximum size is restricted by
              // the screen size
              Rectangle VisRect = GeoScrollPane.getBounds();
              if(VisRect.getHeight()<=0 || VisRect.getWidth()<=0){
                Reporter.showError("cannot capture the rectangle");
              }
              int border = CaptureBorder; // later change for setting by the user
              int vw = (int)VisRect.getWidth()+2*border;
              int vh = (int)VisRect.getHeight()+2*border;
              Rectangle2D R = GraphDisplay.getBounds();
              BufferedImage bi = new BufferedImage(vw,vh,BufferedImage.TYPE_3BYTE_BGR);
              Graphics2D g = bi.createGraphics();
              g.setBackground(GraphDisplay.getBackground());
              g.clearRect(0,0,vw,vh);
              AffineTransform trans = new AffineTransform();
              double x = R.getX()+border;
              double y = R.getY()+border;
              trans.setToTranslation(x,y);   
              g.setTransform(trans);
              GraphDisplay.printAll(g);
              bgImage.setImage(bi);
              // convert the bounding bocx of the GraphDisplay into
              // world coordinates.
              Rectangle2D R2= new Rectangle2D.Double();
              AffineTransform at = (AffineTransform) allProjection.clone();
              try{
                  R2.setRect(-x,-y,vw,vh);    
                  R2 = at.createInverse().createTransformedShape(R2).getBounds2D();
              } catch(Exception e){
                  Reporter.showError("Cannot determine the bounding box of this image");
              }
              bgImage.setBBox(R2.getX(),R2.getY(),R2.getWidth(),R2.getHeight());
              g.dispose();
              GraphDisplay.updateBackground();
            }catch(Exception e){
               // because large sized data are processed, a OutOfMemory error is possible
               Reporter.showError("an error in capturing the background is occured");
               bgImage.setImage(null); 
           }
           System.gc();
           System.runFinalization();
       }
    };
    JMenu SelectBorder = new JMenu("Select Bordersize");
    JRadioButtonMenuItem Border_0 = new JRadioButtonMenuItem("0");
    SelectBorder.add(Border_0);
    
    JRadioButtonMenuItem Border_30 = new JRadioButtonMenuItem("30");
    SelectBorder.add(Border_30);

    JRadioButtonMenuItem Border_50 = new JRadioButtonMenuItem("50");
    SelectBorder.add(Border_50); 

    JRadioButtonMenuItem Border_100 = new JRadioButtonMenuItem("100");
    SelectBorder.add(Border_100); 

    JRadioButtonMenuItem Border_200 = new JRadioButtonMenuItem("200");
    SelectBorder.add(Border_200); 

    JRadioButtonMenuItem Border_500 = new JRadioButtonMenuItem("500");
    SelectBorder.add(Border_500); 
 
    ButtonGroup SelectBorderGroup = new ButtonGroup();
    SelectBorderGroup.add(Border_0);
    SelectBorderGroup.add(Border_30);
    SelectBorderGroup.add(Border_50);
    SelectBorderGroup.add(Border_100);
    SelectBorderGroup.add(Border_200);
    SelectBorderGroup.add(Border_500);
    CaptureBorder=30;
    Border_30.setSelected(true);
    ActionListener SelectBorderListener = new ActionListener(){
        public void actionPerformed(ActionEvent evt){
           Object src = evt.getSource();
           if(! (src instanceof JMenuItem)) 
               return;
           String Label = ((JMenuItem)src).getText().trim();
           try{
             CaptureBorder = Integer.parseInt(Label);
           }catch(Exception e){
              Reporter.showError("Cannot determine the size of the border");
           } 
        }
    };
    Border_0.addActionListener(SelectBorderListener);
    Border_30.addActionListener(SelectBorderListener);
    Border_50.addActionListener(SelectBorderListener);
    Border_100.addActionListener(SelectBorderListener);
    Border_200.addActionListener(SelectBorderListener);
    Border_500.addActionListener(SelectBorderListener);
    BGMenu.add(jMenuBackgroundColor);
    BGMenu.add(AASetBackground); 
    BGMenu.add(AARemoveBackground);
    BGMenu.add(AACaptureBackground);
    BGMenu.add(AACaptureRect);
    BGMenu.add(SelectBorder);

   jMenuGui.add(BGMenu); 

    AAZoomOut = new AbstractAction("Zoom out"){
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        double zf = 1/ZoomFactor;
        ZoomFactor = 1;
        ClipRect = null;
        Point p = GeoScrollPane.getViewport().getViewPosition();
        updateViewParameter();
        GeoScrollPane.getViewport().setViewPosition(new Point((int)(p.getX()*zf),
            (int)(p.getY()*zf)));
      }
    };
    MIZoomOut = jMenuGui.add(AAZoomOut);
    MIZoomOut.setAccelerator(KeyStroke.getKeyStroke("alt Z"));

    MIZoomMinus = new JMenuItem("Zoom -");
    MIZoomMinus.setAccelerator(KeyStroke.getKeyStroke("alt MINUS"));

    MIZoomMinus.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        double zf = 0.75;
        if (ZoomFactor*zf < 1) {
          zf = 1/ZoomFactor;
          ZoomFactor = 1;
          ClipRect = null;
          Point p = GeoScrollPane.getViewport().getViewPosition();
          updateViewParameter();
          //GeoScrollPane.getViewport().setViewPosition(new Point((int)(p.getX()*zf),(int)(p.getY()*zf)));
          return;
        }
        ZoomFactor *= zf;
        double m[] = new double[6];
        allProjection.getMatrix(m);
        m[0] *= zf;
        m[3] *= zf;
        m[4] *= zf;
        m[5] *= zf;
        allProjection = new AffineTransform(m);
        BBoxDC.setSize((int)(BBoxDC.getWidth()*zf), (int)(BBoxDC.getHeight()*zf));
        GraphDisplay.updateLayersSize(BBoxDC);
        Point p = GeoScrollPane.getViewport().getViewPosition();
        Rectangle VP = GeoScrollPane.getViewport().getViewRect();
        int x = (int)p.getX();
        int y = (int)p.getY();
        x = (int)(((x+VP.width/2)*zf)-VP.width/2);
        y = (int)(((y+VP.height/2)*zf)-VP.height/2);
        GeoScrollPane.getViewport().setViewPosition(new Point(x,y));
        GraphDisplay.repaint();
      }
    });

    jMenuGui.add(MIZoomMinus);



    MIZoomPlus = new JMenuItem("Zoom +");
    MIZoomPlus.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        double zf = 1.25;
        ZoomFactor *= zf;
        double m[] = new double[6];
        allProjection.getMatrix(m);
        m[0] *= zf;
        m[3] *= zf;
        m[4] *= zf;
        m[5] *= zf;
        allProjection = new AffineTransform(m);
        BBoxDC.setSize((int)(BBoxDC.getWidth()*zf), (int)(BBoxDC.getHeight()*zf));
        GraphDisplay.updateLayersSize(BBoxDC);
        Point p = GeoScrollPane.getViewport().getViewPosition(); // left-top
        Rectangle VP = GeoScrollPane.getViewport().getViewRect();
        int x = (int)p.getX();
        int y = (int)p.getY();
        x = (int)(((x+VP.width/2)*zf)-VP.width/2);
        y = (int)(((y+VP.height/2)*zf)-VP.height/2);
        //GeoScrollPane.getViewport().setViewPosition(new Point((int)(p.getX()*zf),(int)(p.getY()*zf)));
        GeoScrollPane.getViewport().setViewPosition(new Point(x,y));
        GraphDisplay.repaint();
      }
    });


    MIZoomPlus.setAccelerator(KeyStroke.getKeyStroke("alt PLUS"));
    jMenuGui.add(MIZoomPlus);


    MILayerMgmt.setText("Layer management");
    jMenuGui.add(MILayerMgmt);
    MILayerMgmt.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        new LayerMgmt(HoeseViewer.this, GraphDisplay.getComponents()).show();
        GraphDisplay.repaint();
      }
    });

    jMenuGui.add(new JSeparator());
    String TextPar[] =  {
      "ms/s", "s/m", "m/h", "h/d", "d/w", "w/M" ,"M/Y"
    };

    JRadioButtonMenuItem RBMITimeFrame[] = new JRadioButtonMenuItem[TextPar.length];
    ButtonGroup bg = new ButtonGroup();
    /*final int SliderPar[] =  {
      1, 60, 1440, 7*1440, 30*1440, (int)(365.25*1440)
    };*/
    // ms  s    min      h      d         week        month               year
    final long SliderPar[] ={
      1, 1000, 60000, 3600000, 86400000,  604800000,  2678400000L, 31536000000L
    };
    ActionListener TimeFrameListener = new ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        int index = Integer.parseInt(evt.getActionCommand());
        TimeSlider.setUnitIncrement(SliderPar[index]);
        TimeSlider.setBlockIncrement(SliderPar[index + 1]);
      }
    };

    tok = "time";
    for (int i = 0; i < RBMITimeFrame.length; i++) {
      RBMITimeFrame[i] = new JRadioButtonMenuItem(tok + " " + TextPar[i]);
      RBMITimeFrame[i].setActionCommand(Integer.toString(i));
      RBMITimeFrame[i].addActionListener(TimeFrameListener);
      jMenuGui.add(RBMITimeFrame[i]);
      bg.add(RBMITimeFrame[i]);
    }
    RBMITimeFrame[1].setSelected(true);
    RBMICustTI.setText(tok + " =");
    jMenuGui.add(RBMICustTI);
    bg.add(RBMICustTI);
    RBMICustTI.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        /*
  String input = JOptionPane.showInputDialog("Insert a time increment in msec.");
        long l = 1;
        try {
          l = Long.parseLong(input);
        } catch (NumberFormatException n) {}
        RBMICustTI.setText(tok + " = " + l + " msec.");
        TimeSlider.setUnitIncrement(l);
        TimeSlider.setBlockIncrement(l);
  */
  if(TimeInput.inputTime()==TimeInputDialog.OK){
            RBMICustTI.setText(tok + " = " +TimeInput.getTimeString());
            long t = TimeInput.getTime();
      TimeSlider.setUnitIncrement(t);
            TimeSlider.setBlockIncrement(t);
  }
      }
    });
    //RBMITimeFrame
    MenuExtension.addMenu(jMenuGui);
    MenuExtension.addMenu(createMenu);

    MenuObject.setText("Object");
    MIHideObject.setText("Hide");
    MIHideObject.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        QueryResult qr = null;
        qr = (QueryResult)TextDisplay.getQueryCombo().getSelectedItem();
        if (qr != null) {
          Object o = qr.getSelectedValue();
          if (o instanceof DsplBase) {
            qr.clearSelection();
            ((DsplBase)o).setVisible(false);
            GraphDisplay.repaint();
          } 
          else
            Reporter.showError("No DsplBase object selected!");
        } 
        else
          Reporter.showError("No query selected!");
      }
    });

    MenuObject.add(MIHideObject);

    MIShowObject.setText("Show");
    MIShowObject.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        QueryResult qr = null;
        qr = (QueryResult)TextDisplay.getQueryCombo().getSelectedItem();
        if (qr != null) {
          Object o = qr.getSelectedValue();
          if (o instanceof DsplBase) {
            ((DsplBase)o).setVisible(true);
            GraphDisplay.repaint();
            TextDisplay.repaint();
          } 
          else 
            Reporter.showError("No DsplBase object selected!");
        }
        else 
          Reporter.showError("No query selected!");
      }
    });

    MenuObject.add(MIShowObject);
    AAViewCat = new AbstractAction("Change category"){
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        if (selGraphObj == null) {
          Reporter.showError("No DsplGraph object selected!");
          return;
        }
        CategoryEditor ce = new CategoryEditor(HoeseViewer.this, true, selGraphObj.getCategory());
        ce.show();
        if (ce.getActualCategory() != null) {
          selGraphObj.setCategory(ce.getActualCategory());
          GraphDisplay.repaint();
        }
      }
    };

    MINewCat = MenuObject.add(AAViewCat);
    AALabelAttr = new AbstractAction("Label attributes"){
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        if (selGraphObj == null) {
          Reporter.showError("No DsplGraph object selected!");
          return;
        }
        new LabelAttrDlg(HoeseViewer.this, selGraphObj).show();
        GraphDisplay.repaint();
      }
    };
    MILabelAttr = MenuObject.add(AALabelAttr);
    MenuObject.add(new JSeparator());

    MIMoveTop.setText("Move to top");
    MIMoveTop.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        if (selGraphObj == null) {
          Reporter.showError("No DsplGraph object selected!");
          return;
        }
        //oldLayer.getGeoObjects().remove(selGraphObj);
        int newno = GraphDisplay.highestLayer();
        Component[] Comps = GraphDisplay.getComponentsInLayer(newno);
        Layer oldLayer = selGraphObj.getLayer();
        oldLayer.setSelectedButton(false);
        oldLayer.removeGO(selGraphObj);
        Layer newLayer = (Layer)Comps[0];
        //selGraphObj.getSelected = false;
        selGraphObj.setLayer(newLayer);
        newLayer.addGO(-1, selGraphObj);
        newLayer.setSelectedButton(true);
        GraphDisplay.repaint();
      }
    });
    MenuObject.add(MIMoveTop);

    MIMoveUp.setText("Move layer up");
    MIMoveUp.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        if (selGraphObj == null) {
          Reporter.showError("No DsplGraph object selected!");
          return;
        }
        //oldLayer.getGeoObjects().remove(selGraphObj);
        Layer oldLayer = selGraphObj.getLayer();
        int aktno = GraphDisplay.getIndexOf(oldLayer);
        if (aktno > 0)
          aktno--;
        oldLayer.setSelectedButton(false);
        oldLayer.removeGO(selGraphObj);
        Layer newLayer = (Layer)GraphDisplay.getComponent(aktno);
        //selGraphObj.getSelected = false;
        selGraphObj.setLayer(newLayer);
        newLayer.addGO(-1, selGraphObj);
        newLayer.setSelectedButton(true);
        GraphDisplay.repaint();
      }
    });
    MenuObject.add(MIMoveUp);

    MIMoveDown.setText("Move layer down");
    MIMoveDown.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        if (selGraphObj == null) {
          Reporter.showError("No DsplGraph object selected!");
          return;
        }
        //oldLayer.getGeoObjects().remove(selGraphObj);
        int min = 1;
        Layer oldLayer = selGraphObj.getLayer();
        int aktno = GraphDisplay.getIndexOf(oldLayer);
        if (aktno < GraphDisplay.getComponentCount() - min)
          aktno++;
        oldLayer.setSelectedButton(false);
        oldLayer.removeGO(selGraphObj);
        Layer newLayer = (Layer)GraphDisplay.getComponent(aktno);
        //selGraphObj.getSelected = false;
        selGraphObj.setLayer(newLayer);
        newLayer.addGO(0, selGraphObj);
        newLayer.setSelectedButton(true);
        GraphDisplay.repaint();
      }
    });
    MenuObject.add(MIMoveDown);

    MIMoveBottom.setText("Move to bottom");
    MIMoveBottom.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        if (selGraphObj == null) {
          Reporter.showError("No DsplGraph object selected!");
          return;
        }
        int min = 1;
        Layer oldLayer = selGraphObj.getLayer();
        oldLayer.setSelectedButton(false);
        oldLayer.removeGO(selGraphObj);
        Layer newLayer = (Layer)GraphDisplay.getComponent(GraphDisplay.getComponentCount()
            - min);
        //selGraphObj.getSelected = false;
        selGraphObj.setLayer(newLayer);
        newLayer.addGO(0, selGraphObj);
        newLayer.setSelectedButton(true);
        GraphDisplay.repaint();
      }
    });
    MenuObject.add(MIMoveBottom);
    MenuExtension.addMenu(MenuObject);
  }


/** returns the index of qr in TextDisplay-ComboBox, 
  * if qr not in this box then -1 is returned
  **/
private int getQueryIndex(QueryResult qr){
   JComboBox CB = TextDisplay.getQueryCombo();
   int count = CB.getItemCount();
   int pos = -1;
   boolean found =false;
   for(int i=0;i<count&&!found;i++)
      if (qr.equals(CB.getItemAt(i))) {pos = i;found=true;}
   return pos;
}


public MenuVector getMenuVector(){
    return MenuExtension;
}


private boolean loadReferences(File F){
  if(!F.exists())
     return false;
  ListExpr LE= new ListExpr();
  if (LE.readFromFile(F.getPath())!=0){
     return false;
  }
  if(!ManualLinkPool.readFromListExpr(LE))
     return false;
  return true;
}


/** Removes a SecondoObject */
public void removeObject(SecondoObject o){
   QueryResult qr = new QueryResult(o.getName(),o.toListExpr());
   int index = getQueryIndex(qr);
   if (index>=0){
       JComboBox CB = TextDisplay.getQueryCombo();
       qr = (QueryResult) CB.getItemAt(index);  // we need the original

       qr.clearSelection();

       // clear graphicDisplay
       ListIterator li = qr.getGraphObjects().listIterator();
       while (li.hasNext()) {
           DsplGraph dg = (DsplGraph)li.next();
           Layer l = dg.getLayer();
           Vector go = l.getGeoObjects();
           go.remove(dg);
           if (go.isEmpty()) {
              LayerSwitchBar.remove(l.button);
              GraphDisplay.remove(l);
           }
        }
        qr.getGraphObjects().clear();
        VisComPanel.setBottomComponent(dummy);

       qr.setListData(new Vector());
       qr.revalidate();
       qr.repaint();

       // remove from ComboBox
       CB.removeItemAt(index);
       if (CB.getItemCount()==0)
           TextDisplay.clearComboBox();
       CB = TextDisplay.getQueryCombo();
       CB.setSelectedIndex(-1);

        updateViewParameter();
    }

}


/** add a Projection with given ClassName */
public boolean addProjection(String Name){

 Name ="project."+Name.trim();
 Class cls;
 try{
     cls = Class.forName(Name);
     Object P;
     P = cls.newInstance();
     if(! (P instanceof Projection))
        return false;
     PrjSelector.addProjection((Projection)P);
     return true;
    }
 catch(Exception e){ 
     Reporter.debug(e);
     return false;}
}


/** remove all containing objects */
public void removeAll(){
  on_jMenu_NewSession(null);
}


/** return true if o is displayed false otherwise */
public boolean isDisplayed(SecondoObject o){
   if(!canDisplay(o))
      return false;
   QueryResult q = new QueryResult(o.getName(),o.toListExpr());
   int index = getQueryIndex(q);
   return index>=0;
}


/** select o */
public boolean selectObject(SecondoObject o){
    QueryResult q = new QueryResult(o.getName(),o.toListExpr());
    int index = getQueryIndex(q);
    if(index <0)
       return false;
    else{
      JComboBox CB = TextDisplay.getQueryCombo();
      CB.setSelectedIndex(index);
      return true;
    }
}  

/** all query results can be displayed (see work of Th.Hoese) **/
public boolean canDisplay(SecondoObject o){
  ListExpr LE = o.toListExpr();
  if(LE==null)
     return false;
  else{
     if(LE.listLength()!=2)
        return false;
     ListExpr Type = LE.first();
     while(Type.atomType()==ListExpr.NO_ATOM){
       if(Type.listLength()<1)
           return false;
       Type = Type.first();
     }

     if(Type.atomType()!=ListExpr.SYMBOL_ATOM)
        return false;
     try{
        Class.forName("viewer.hoese.algebras.Dspl"+Type.symbolValue());
        return true;
     }
     catch(ClassNotFoundException e){
        return false;
     }
  }
}


  /**
   * Adds a new QueryResult qr to the Textwindow
   * @param qr
   * @return True if no error has occured
   * @see <a href="MainWindowsrc.html#addQueryResult">Source</a> 
   */
  public boolean addQueryResult (QueryResult qr) {
    CurrentQueryResult = qr;
    CurrentQueryResult.addListSelectionListener(DoQuerySelection);
    ListExpr displayErrorList = TextDisplay.newQueryResult(CurrentQueryResult);
    int displayErrorCode = displayErrorList.first().intValue();
    // If an error happened when showing the query result, shows the error
    // message.
    if (displayErrorCode != NOT_ERROR_CODE) {
      Reporter.showError("add queryresult failed");
      return  false;
    }
    else {
      return  true;
      //QueryResultList.add (CurrentQueryResult);
    }
  }


 /** return the name of this Viewer */
 public String getName(){
    return "Th.Hoese-Viewer";
 }
  
 /** adds a new SecondoObject to this Viewer **/
  public boolean addObject(SecondoObject o){

    QueryResult qr= new QueryResult(o.getName(),o.toListExpr()); 
    if(getQueryIndex(qr)>=0)
      return false;
    else {
      if (addQueryResult(qr)) {
        if (!CurrentQueryResult.getGraphObjects().isEmpty()){
          addSwitch(GraphDisplay.addLayerObjects(CurrentQueryResult.getGraphObjects()),-1);
        }
        CurrentQueryResult.setSelectedIndex(0);
      }
      return true;
    }
  }

  /** switchs to the testmode **/
  public void enableTestmode(boolean on){
     isAutoCatMI.setSelected(on);
  }



  /**
   *
   * @return The selected grph. object 
   * @see <a href="MainWindowsrc.html#getSelGO">Source</a> 
   */
  public DsplGraph getSelGO () {
    return  selGraphObj;
  }


  /**
   * Sets actual global timeinterval
   * @param in Interval 
   * @see <a href="MainWindowsrc.html#setActualTime">Source</a> 
   */
  public void setActualTime (Interval in) {
    TimeBounds = in;
    if (in == null) {
      TimeSlider.setValues(0, 0, 1);
      actTimeLabel.setText("no time");
      //TimeSlider.setVisible(false);
      CurrentState.ActualTime = 0;
    } 
    else {
      TimeSlider.setVisible(true);
      CurrentState.ActualTime = TimeBounds.getStart();
      TimeSlider.setValues((long)Math.round(in.getStart()*86400000), (long)Math.round(in.getStart()*86400000),
          (long)Math.round(in.getEnd()*86400000) + 1);
      actTimeLabel.setText(LEUtils.convertTimeToString(CurrentState.ActualTime));
    }
  }

  /**
   * optional
   * @param evt
   * @see <a href="MainWindowsrc.html#on_jMenu_Browse">Source</a> 
   */
  private void on_jMenu_Browse (java.awt.event.ActionEvent evt) {               //GEN-FIRST:event_on_jMenu_Browse
    // Add your handling code here:
  }             //GEN-LAST:event_on_jMenu_Browse

/** Save session to selected file 
   */
  private void on_jMenu_SaveSession (java.awt.event.ActionEvent evt) {          //GEN-FIRST:event_on_jMenu_SaveSession
    // Add your handling code here:
    FC_Session.setDialogTitle("Save Session");
    int returnVal = FC_Session.showSaveDialog(HoeseViewer.this);
    if (returnVal == JFileChooser.APPROVE_OPTION) {
      File file = FC_Session.getSelectedFile();
      //          File file=new File("Session");
      String DirName = FC_Session.getCurrentDirectory().getAbsolutePath();
      ListExpr le = ListExpr.fourElemList(ListExpr.symbolAtom("session"), bgImage.toListExpr(DirName), 
          writeAllCats(), TextDisplay.convertAllQueryResults());
      String suc;
      if(le.writeToFile(file.getPath()) == 0)
         Reporter.showInfo("session written");
      else
         Reporter.showError("save session failed");
    }
  }             

/** Loads session from a file
  */  
  private void on_jMenu_OpenSession (java.awt.event.ActionEvent evt) {          //GEN-FIRST:event_on_jMenu_OpenSession
    FC_Session.setDialogTitle("Open Session");
    int returnVal = FC_Session.showOpenDialog(HoeseViewer.this);
    if (returnVal == JFileChooser.APPROVE_OPTION) {
      on_jMenu_NewSession(evt);
      File file = FC_Session.getSelectedFile();
      ListExpr le=null;
      boolean ok = true;
      // try to load the ListExpr
      try{
        le = ListExpr.getListExprFromFile(file.getPath());
        if(le==null)
           ok = false;
        }
        catch(Exception e){
          ok=false;
        }
      if(!ok){
         Reporter.showError("i can't load the file");
         return;
      }
      
      // check ListExprFormat
      if (le.listLength()!=4){
         Reporter.showError(" the file contains not a session ");
         return;
      }
         
      ListExpr type = le.first();

      if (!type.isAtom() || !(type.atomType()==ListExpr.SYMBOL_ATOM) || !type.symbolValue().equals("session")){
         Reporter.showError(" the file contains no session ");
         le.destroy();
         return;
      }

      le = le.rest();
      type.destroy();
      Cats = new Vector(10, 5);
      String DirName = FC_Session.getCurrentDirectory().getAbsolutePath();
      bgImage.readFromListExpr(le.first(),DirName);
      readAllCats(le.second());
      if(!Cats.contains(Category.getDefaultCat()))
         Cats.add(Category.getDefaultCat());
      
      le.first().destroy();
      le.second().destroy();

      TextDisplay.readAllQueryResults(le.third());

      // inform the ViewerControl over the new Objects
      if(VC!=null){
         JComboBox CB = TextDisplay.getQueryCombo();
         int count = CB.getItemCount();
         SecondoObject SO;
         QueryResult QR;
         for(int i=0;i<count;i++){
            QR = (QueryResult)CB.getItemAt(i);
            SO = new SecondoObject(IDManager.getNextID());
            SO.setName(QR.getCommand());
            SO.fromList(QR.getListExpr());
            VC.addObject(SO);
         }
      }
      GraphDisplay.updateBackground(); 

    }
    // Add your handling code here:
  }             //GEN-LAST:event_on_jMenu_OpenSession

  /**
   * Creates new session by constructing a new instance of mainWindow
   * @param evt
   * @return A new MainWindow
   */

  private void on_jMenu_NewSession (java.awt.event.ActionEvent evt) {                     //GEN-FIRST:event_on_jMenu_NewSession

 // remove querys
   JComboBox CB = TextDisplay.getQueryCombo();

   // all queries
   int count = CB.getItemCount();
   QueryResult qr=null;
   while (count!=0) {
      qr = (QueryResult) CB.getItemAt(0);
      qr.clearSelection();
      ListIterator li = qr.getGraphObjects().listIterator();
      while (li.hasNext()) {
         DsplGraph dg = (DsplGraph)li.next();
         Layer l = dg.getLayer();
         Vector go = l.getGeoObjects();
         go.remove(dg);
         if (go.isEmpty()) {
             LayerSwitchBar.remove(l.button);
             GraphDisplay.remove(l);
        }
      }
      qr.getGraphObjects().clear();
      updateViewParameter();
      TextDisplay.getQueryCombo().removeItem(qr);
      CB = TextDisplay.getQueryCombo();
      count = CB.getItemCount();
   }
   if(qr!=null)
      qr.setListData(new Vector());
   CB.removeAllItems();
   TextDisplay.clearComboBox();

   // remove the categorys
    Cats = new  Vector(30,10);
    Cats.add(Category.getDefaultCat());

    VisComPanel.setBottomComponent(dummy);
    String Catfile = configuration.getProperty("StandardCat");
    if (Catfile != null) {
      ListExpr le = new ListExpr();
      le.readFromFile(Catfile);
      readAllCats(le);
    }

 }             //GEN-LAST:event_on_jMenu_NewSession



  /**
  Adds the scale of the ZoomFactor to the matrix of th e at transform

   * @see <a href="MainWindowsrc.html#addScaling">Source</a>
   */
  public AffineTransform addScaling (AffineTransform at) {
    //at.scale(ZoomFactor,ZoomFactor);
    double w[] = new double[6];
    at.getMatrix(w);
    double z = ZoomFactor;
    w[0] *= z;
    w[3] *= z;
    w[4] *= z;
    w[5] *= z;
    at = new AffineTransform(w);
    return  at;
  }

  /**
   * Calc. the projection that all objects fit into visible window with border
   * @return Thecalc. transformation
   * @see <a href="MainWindowsrc.html#calcProjection">Source</a>
   */
  private AffineTransform calcProjection () {
    double extra = 30;   //extra portion at every border of 30 pix
    Rectangle2D.Double BBWC = BBoxWC;
    double wp1x = BBWC.getX();
    double wp1y = BBWC.getY();
    double wpw = BBWC.getWidth();
    double wph = BBWC.getHeight();
    double w = ClipRect.getWidth();             //(double) GeoScrollPane.getViewport().getWidth();
    double h = ClipRect.getHeight();            //(double) GeoScrollPane.getViewport().getHeight();
    //ClipRect.setSize((int)w,(int)h);
    // if no objects or only a single point,line is visible
    if ((wpw == 0) && (wph == 0)) {
      return  new AffineTransform(1, 0, 0, 1, -wp1x + extra, -wp1y + extra);
    } 
    else if (wpw == 0)
      wpw = 1; 
    else if (wph == 0)
      wph = 1;
    // now division by zero impossible  
    double m00, m11;
    if (w/wpw > h/wph) {        //keep aspect ratio
      //h-=60;  
      m11 = (2*extra - h)/wph;
      m00 = -m11;
    }
    else {
      //w-=60;
      m00 = (w - 2*extra)/wpw;
      m11 = -m00;
    }
    double m02 = extra - m00*wp1x;
    double m12 = extra - (wph + wp1y)*m11;
    double m01 = 0.0;
    double m10 = 0.0;
    return  new AffineTransform(m00, m10, m01, m11, m02, m12);
  }

  /**
   * When new objects are added or old areremoved or the context has change this
   * method must be called to calc. Layersize and transformation again.
   * @see <a href="MainWindowsrc.html#updateViewParameter">Source</a>
   */
  public void updateViewParameter () {
    GraphDisplay.updateBoundingBox();           //fuer neue oder geloeschte Objekte
    if (ClipRect == null) {
      double w = (double)GeoScrollPane.getViewport().getWidth();
      double h = (double)GeoScrollPane.getViewport().getHeight();
      ClipRect = new Rectangle(0, 0, (int)w, (int)h);
    }
    allProjection = calcProjection();         //addScaling(calcProjection());
    Rectangle2D rDC = (Rectangle2D)allProjection.createTransformedShape(BBoxWC).getBounds();
    double x = rDC.getX();
    double y = rDC.getY();
    double w = rDC.getWidth();
    double h = rDC.getHeight();
    w += 60;                //plus extra space
    h += 60;
    BBoxDC = new Rectangle(0, 0, (int)w, (int)h);
    GraphDisplay.updateLayersSize(BBoxDC);
    GraphDisplay.repaint();
    // compute the bounds of all contained Timed objects
    ListModel listModel = TextDisplay.getQueryCombo().getModel();
    int size = listModel.getSize();
    Interval interval=null;
    for(int i=0;i<size;i++){
      QueryResult qr = (QueryResult) listModel.getElementAt(i);
      Interval qrInterval = qr.getTimeBounds();
      if(qrInterval!=null){
         if(interval==null){
          interval = qrInterval.copy();
         }else{
          interval.unionInternal(qrInterval);
         }
      }
    }
    setActualTime(interval);
  }


  /**
   * The selected object should be visible even when it moves. This method keeps GO visible
   */
  public void makeSelectionVisible () {
    if (selGraphObj == null)
      return;
    if (selGraphObj.getRenderObject(allProjection) == null)
      return;
    Rectangle2D r = selGraphObj.getRenderObject(allProjection).getBounds2D();
    if (r == null)            // an emtpy spatial object or an undefined timed object
       return;
    //try{
    Shape s = allProjection.createTransformedShape(r);
    r = s.getBounds2D();
    if (!isMouseSelected) {
      double w = (double)GeoScrollPane.getViewport().getWidth();
      double h = (double)GeoScrollPane.getViewport().getHeight();
      double x = (double)-GraphDisplay.getX();
      double y = (double)-GraphDisplay.getY();
      double rmid_x = r.getX()+r.getWidth()/2;
      double rmid_y = r.getY()+r.getHeight()/2;
      int border = 60; 
      if(rmid_x-x<border || // left 
         rmid_x>x+w-border || // right
         rmid_y-y<border || // above
         rmid_y>y+h-border){ // under
         GeoScrollPane.getHorizontalScrollBar().setValue((int)(rmid_x - w/2));
         GeoScrollPane.getVerticalScrollBar().setValue((int)(rmid_y - h/2));
      } 
    }
    isMouseSelected = false;
  }


  /**
   * Adds a layer-switch to LayerSwitchbar
   * @param tb The button
   * @param index The position
   */
  public void addSwitch (JToggleButton tb, int index) {
    if (index < 0)
      LayerSwitchBar.add(tb);
    else
      LayerSwitchBar.add(tb, index);
    LayerSwitchBar.revalidate();
    LayerSwitchBar.repaint();
  }

  /** returns the actual backgroundimage 
    */
  public BackGroundImage getBackgroundImage(){
    return bgImage;
  }



/** Listens to a selection change in a query list
  * @see <a href="MainWindowsrc.html#QueryListSelectionListener">Source</a>
   */

  class QueryListSelectionListener
      implements ListSelectionListener {
    public void valueChanged (ListSelectionEvent e) {
      Object o;
      if (e.getValueIsAdjusting())
        return;
      QueryResult theList = (QueryResult)e.getSource();
      if (selBaseObj != null) {
        selBaseObj.setSelected(false);
        if (selBaseObj.getFrame() != null) {
          selBaseObj.getFrame().select(null);
          //selBaseObj.getFrame().show(false);
        }
        selBaseObj = null;
      }
      if (selGraphObj != null) {
        selGraphObj.getLayer().setSelectedButton(false);
        selGraphObj.setSelected(false);
        selGraphObj = null;
        //selGraphObj.getLayer().repaint();
      }
      if(theList.getModel().getSize()<1){// an empty list
         GraphDisplay.repaint(); // remove an old selection
   return;
      }
      o = theList.getSelectedValue();
      if (o instanceof Timed) {
        TimeDisplay.setTimeObject((Timed)o);
        oldDL = VisComPanel.getLastDividerLocation();
        VisComPanel.setBottomComponent(TimeDisplay);
        VisComPanel.setDividerLocation(0.8);
      }
      else
        //showCommandPanel()
       ;
      if (o instanceof DsplGraph) {
        DsplGraph dgorig = (DsplGraph)o;
        dgorig.setSelected(true);
        dgorig.getLayer().setSelectedButton(true);
        selGraphObj = dgorig;
        if (!isMouseSelected && (selGraphObj instanceof Timed)){
          Interval tb = ((Timed)selGraphObj).getTimeBounds();
          if(tb!=null) 
             TimeSlider.setValue((long)Math.round(tb.getStart()*86400000));
        }
        makeSelectionVisible();
  GraphDisplay.repaint();
      }
      else if (o instanceof DsplBase) {
        selBaseObj = (DsplBase)o;
        selBaseObj.setSelected(true);
        if (selBaseObj.getFrame() != null) {
          selBaseObj.getFrame().select(o);
          //selBaseObj.getFrame().show(true);
        }
      }
      else
        GraphDisplay.repaint();
    }

  }

/** Manages mouseclicks in the GraphWindow. It is placed here for textual-interaction
  */
    class SelMouseAdapter extends MouseAdapter
      implements MouseMotionListener {

    private int startX;  // start-x of selection rectangle
    private int startY;  // start-y of selection rectangle
    private int targetX;    // last end-x position of selection rectangle
    private int targetY;    // last end-y position of selection rectangle
    private boolean isPainting = false; // is the selection rectangle painted ?
    private boolean isEnabled = false;
    private boolean selectionEnabled=true;


    public void drawRectangle(){
         if(!isEnabled) return;
   int x1 = startX;
   int x2 = targetX;
   int y1 = startY;
   int y2 = targetY;
         Graphics2D G = (Graphics2D) GraphDisplay.getGraphics();
   G.setXORMode(Color.white);
         int x = Math.min(x1,x2);
         int w = Math.abs(x1-x2);
         int y = Math.min(y1,y2);
         int h= Math.abs(y1-y2);
         G.drawRect(x,y,w,h);
  }


    /** enabled or disables the sleection of objects **/
    public void enableSelection(boolean enable){
        if(enable==selectionEnabled) return;
        selectionEnabled= enable;
        JComboBox cb = TextDisplay.getQueryCombo();
        QueryResult qr = (QueryResult)cb.getSelectedItem();
        if(qr!=null)
           qr.clearSelection();
    }


    public void mouseReleased (MouseEvent e) {
      if ((e.getModifiers() & InputEvent.BUTTON3_MASK) == InputEvent.BUTTON3_MASK & isEnabled) {
      {
    if(isPainting)
             drawRectangle();
          isPainting=false;
          int x = Math.min(startX,targetX);
          int wi = Math.abs(startX-targetX);
    int y = Math.min(startY,targetY);
          int he = Math.abs(startY-targetY);


    Rectangle2D r = new Rectangle2D.Double(x,y,wi,he);
          GraphDisplay.removeMouseMotionListener(this);



          if ((r.getHeight() < 1) || (r.getWidth() < 1))
            return;
          double w = (double)GeoScrollPane.getViewport().getWidth();
          double h = (double)GeoScrollPane.getViewport().getHeight();
          double zf = Math.min(w/r.getWidth(), h/r.getHeight());
          ZoomFactor *= zf;
          double m[] = new double[6];
          allProjection.getMatrix(m);
          //double z=ZoomFactor;
          m[0] *= zf;
          m[3] *= zf;
          m[4] *= zf;
          m[5] *= zf;
          allProjection = new AffineTransform(m);
          BBoxDC.setSize((int)(BBoxDC.getWidth()*zf), (int)(BBoxDC.getHeight()*zf));
          GraphDisplay.updateLayersSize(BBoxDC);
          GraphDisplay.scrollRectToVisible(new Rectangle((int)(r.getX()*zf),
              (int)(r.getY()*zf), (int)w, (int)h));
          //GeoScrollPane.getViewport().setViewPosition(new Point((int)(r.getX()*zf),(int)(r.getY()*zf)));
          GraphDisplay.repaint();
    isEnabled = false;
        }
      }
    }

    public void mousePressed (MouseEvent e) {
      //Koordinaten in Weltkoordinaten umwandeln
      if ((e.getModifiers() & InputEvent.BUTTON3_MASK) == InputEvent.BUTTON3_MASK) {
         startX = e.getX();
         startY = e.getY();
   targetX = startX;
   targetY = startY;
   drawRectangle();
         isPainting=true;
   isEnabled = true;
   GraphDisplay.addMouseMotionListener(this);

      }
    }
    public void mouseDragged (MouseEvent e) {
      if ((e.getModifiers() & InputEvent.BUTTON3_MASK) == InputEvent.BUTTON3_MASK & isEnabled) {
          if(isPainting)
            drawRectangle();
          targetX = e.getX();
          targetY = e.getY();
          drawRectangle();
      }
    }

    public void mouseMoved (MouseEvent e) {}

    public void mouseClicked (MouseEvent e) {
      if(!selectionEnabled)
          return;
      if (!((e.getModifiers() & InputEvent.BUTTON1_MASK) == InputEvent.BUTTON1_MASK))
         return;
      Point2D.Double p = new Point2D.Double();
      try {
        p = (Point2D.Double)allProjection.inverseTransform(e.getPoint(), p);
      } catch (Exception ex) {}
      //int hits = 0;
      double SelIndex = 10000, BestIndex = -1, TopIndex = -1;
      //boolean selectionfound = false;
      DsplGraph Obj2sel = null, top = null;
      int ComboIndex = -1, TopComboIndex = -1;
      double scalex = 1/Math.abs(allProjection.getScaleX());
      double scaley = 1/Math.abs(allProjection.getScaleY());
      //ListIterator li=QueryResultList.listIterator();
      if ((selGraphObj != null) && (selGraphObj.contains(p.getX(), p.getY(),
          scalex, scaley)))
        SelIndex = selGraphObj.getLayer().getObjIndex(selGraphObj);
      JComboBox cb = TextDisplay.getQueryCombo();
      for (int j = 0; j < cb.getItemCount(); j++) {
        ListIterator li2 = ((QueryResult)cb.getItemAt(j)).getGraphObjects().listIterator();
        while (li2.hasNext()) {
          DsplGraph dg = (DsplGraph)li2.next();
          if (!dg.getVisible())
            continue;
          if (dg.contains(p.getX(), p.getY(), scalex, scaley)) {
            double AktIndex = (dg.getSelected()) ? SelIndex : dg.getLayer().getObjIndex(dg);
            //top is the topmost GO
            if (AktIndex > TopIndex) {
              TopIndex = AktIndex;
              top = dg;
              TopComboIndex = j;
            }
            if ((selGraphObj != null) && (AktIndex < SelIndex) && (AktIndex > BestIndex)) {
              //the next GO smaller than selindex and greater than best until now
              Obj2sel = dg;
              BestIndex = AktIndex;
              ComboIndex = j;
            }
          }
        }
      }
      if (Obj2sel == null) {
        Obj2sel = top;
        ComboIndex = TopComboIndex;
      }
      QueryResult qr;
      if (Obj2sel == null) {
        qr = (QueryResult)cb.getSelectedItem();
        if (qr != null)
          qr.clearSelection();
      }
      else {
        if (Obj2sel.getSelected())
          return;
        isMouseSelected = true;
        cb.setSelectedIndex(ComboIndex);
        qr = (QueryResult)cb.getSelectedItem();
        //qr.clearSelection();
        qr.setSelectedValue(Obj2sel, true);
        TextDisplay.ensureSelectedIndexIsVisible();
      }
    }


  }
/** This class controls movement of the timeslider

   * @see <a href="MainWindowsrc.html#TimeAdjustmentListener">Source</a>
   */
  class TimeChangeListener
      implements ChangeValueListener {

    public void valueChanged (ChangeValueEvent e) {
      if (TimeBounds == null) {
        TimeSlider.setValue(0);
        return;
      }
      long v = e.getValue();
      double anf;
      if (v == TimeSlider.getMinimum())
        anf = TimeBounds.getStart();
      else {
        anf = (double)v/86400000.0;
        if (anf > TimeBounds.getEnd())
          anf = TimeBounds.getEnd();
      }
      if (anf == CurrentState.ActualTime)
        return;
      GraphDisplay.setIgnorePaint(true);
      CurrentState.ActualTime = anf;
      actTimeLabel.setText(LEUtils.convertTimeToString(CurrentState.ActualTime));
      makeSelectionVisible();
      GraphDisplay.setIgnorePaint(false);
      GraphDisplay.repaint();
      SelectionControl.drawRectangle();
      
    }
  }



  private void init() {
    // Once the address of the configuration file is known, it tries to
    configuration = new Properties();
    File CF = new File(CONFIGURATION_FILE);
    if(!CF.exists()){
       Reporter.showError("HoeseViewer : configuration file not found");
       return;
    }
    try {
      FileInputStream Cfg = new FileInputStream(CF);
      configuration.load(Cfg);
      Cfg.close();
      }
    catch(Exception e){
       Reporter.debug(e);
       return;
    }
       CatPath="";
       String SessionPath="";
       String ReferencePath ="";
       TexturePath ="";

       String SecondoHome = configuration.getProperty("SECONDO_HOME");
       if(SecondoHome==null){
           String T = (new File("")).getAbsolutePath();
     T = T.substring(0,T.length()-8)+FileSeparator;
           if(! (new File(T)).exists())
        SecondoHome ="";
     else
        SecondoHome =T;
       }

       String HoeseHome = SecondoHome+"Data"+FileSeparator+"GuiData"+FileSeparator+"hoese"+FileSeparator;
       CatPath = HoeseHome+"categories";
       SessionPath = HoeseHome+"sessions";
       TexturePath = HoeseHome+"textures";
       ReferencePath = HoeseHome+"references";



       // set special category path
       String TmpCatPath = configuration.getProperty("CATEGORY_PATH");
       if(TmpCatPath!=null)
          CatPath = TmpCatPath.trim();

       if(!CatPath.endsWith(FileSeparator))
      CatPath += FileSeparator;


       File F;
       F = new File(CatPath);
       if(!F.exists())
          Reporter.writeError("wrong categorypath "+CatPath);
       else
          FC_Category.setCurrentDirectory(new File(CatPath));



       String TMPSessionPath = configuration.getProperty("SESSION_PATH");
       if(TMPSessionPath!=null)
           SessionPath = TMPSessionPath.trim();
       F = new File(SessionPath);
       if(!F.exists())
          Reporter.writeError("wrong SessionPath "+SessionPath);
       else
          FC_Session.setCurrentDirectory(new File(SessionPath));



       String TMPTexturePath = configuration.getProperty("TEXTURE_PATH");
       if(TMPTexturePath!=null)
          TexturePath = TMPTexturePath.trim();

       if(!TexturePath.endsWith(FileSeparator))
     TexturePath += FileSeparator;

       F = new File(TexturePath);
       if(!F.exists()){
          Reporter.writeWarning("the TEXTURE_PATH in "+CONFIGURATION_FILE+" is setted to a non existing Path");
          Reporter.writeWarning("please set this variable to a existing non relative pathname");
       }else{
    CategoryEditor.setTextureDirectory(new File(TexturePath));
    Reporter.writeInfo("set TexturePath to "+TexturePath);
    Category.setTexturePath(TexturePath);
       }

    String MaxPixels = configuration.getProperty("MAXPIXELS");
    if(MaxPixels!=null){
          MaxPixels=MaxPixels.trim();
          try{
             long mp = Long.parseLong(MaxPixels);
             ScalableImage.setMaxPixels(mp); 
          } catch(Exception e){
            Reporter.writeError("Error in reading MaxPixels");
          } 
    }
    
    String MaxCapPixels = configuration.getProperty("MAXCAPTUREPIXELS");
    if(MaxCapPixels!=null){
          MaxCapPixels=MaxCapPixels.trim();
          try{
             long mp = Long.parseLong(MaxCapPixels);
             MAXCAPTUREPIXELS=mp; 
          } catch(Exception e){
            Reporter.writeError("Error in readng MaxCapturePixels");
          } 
    }


       String TMPReferencePath = configuration.getProperty("REFERENCE_PATH");
       if(TMPReferencePath!=null)
         ReferencePath = TMPReferencePath;

       if(!ReferencePath.endsWith(FileSeparator))
           ReferencePath += FileSeparator;


       F = new File(ReferencePath);
       if(!F.exists())
          Reporter.writeError("wrong ReferencePath "+ReferencePath);
       else
          FC_References.setCurrentDirectory(new File(ReferencePath));


       String StdRef = configuration.getProperty("STD_REFERENCE");
       if(StdRef!=null){
         F = new File(ReferencePath+StdRef);
   if(!F.exists())
     Reporter.writeError("the Reference-File "+StdRef+" not exists");
   else
     if(!loadReferences(F))
       Reporter.writeError("i can't load the reference file :"+StdRef);
       }

       String Prjs = configuration.getProperty("PROJECTIONS");
       if(Prjs==null)
          Reporter.writeWarning("PROJECTIONS not found in " +CONFIGURATION_FILE);
       else{
          boolean ok = true;
    String Errors ="";
          StringTokenizer ST = new StringTokenizer(Prjs.trim());
    while(ST.hasMoreTokens()){
       String name = ST.nextToken();
             if(!addProjection(name)){
          ok = false;
    Errors +="  "+name;
       }
    }
    if(!ok){
      Reporter.showError("not all projections loaded \n errors in \n"+Errors);
    }


       }
    String AutoCat = configuration.getProperty("AUTOCAT");
    if(AutoCat!=null && AutoCat.trim().toUpperCase().equals("TRUE")){
      isAutoCatMI.setSelected(true);
    }
 }



/** This class manages animation-events * @see <a href="MainWindowsrc.html#MainWindow">Source</a>
   * @see <a href="MainWindowsrc.html#AnimCtrlListener">Source</a>
   */
  class AnimCtrlListener
      implements ActionListener {
    long inc = 1;
    boolean forward = true;
    boolean onlyDefined;
    Vector TimeObjects;
    Timer AnimTimer = new Timer(50, new ActionListener() {

      public void actionPerformed (java.awt.event.ActionEvent evt) {
        long v = TimeSlider.getValue();
        if (onlyDefined) {
          v++;
          long min = Long.MAX_VALUE;
          ListIterator li = TimeObjects.listIterator();
          while (li.hasNext()) {
            Timed t = (Timed)li.next();
            min = Math.min(Interval.getMinGT(t.getIntervals(), (double)v/86400000.0),
                min);
          }
          if (min < Long.MAX_VALUE) {
            TimeSlider.setValue(min);
            AnimTimer.setDelay((1000 - TimeObjects.size() < 50) ? 50 : 1000
                - TimeObjects.size());
          }
          else
            AnimTimer.stop();
        }
        else {
          //inc = (dir*TimeSlider.getUnitIncrement());
          //TimeSlider.setValue(v + inc);
    if(forward && !TimeSlider.next()){
             AnimTimer.stop();
          }
    if(!forward && !TimeSlider.back()){
       AnimTimer.stop();
    }
        }
      }
    });

    public void actionPerformed (java.awt.event.ActionEvent evt) {
      switch (Integer.parseInt(evt.getActionCommand())) {
        case 0:                 //play
          onlyDefined = false;
          forward = true;
          AnimTimer.start();
          break;
        case 1:                 //reverse
          onlyDefined = false;
          forward = false;
          AnimTimer.start();
          break;
        case 2:                 //show at defined Times
          TimeObjects = new Vector(50, 10);
          JComboBox cb = TextDisplay.getQueryCombo();
          for (int j = 0; j < cb.getItemCount(); j++) {
            ListIterator li2 = ((QueryResult)cb.getItemAt(j)).getGraphObjects().listIterator();
            while (li2.hasNext()) {
              Object o = li2.next();
              if ((o instanceof DsplGraph) && (o instanceof Timed))
                TimeObjects.add(o);
            }
          }
          onlyDefined = true;
          forward = true;
          AnimTimer.start();
          break;
        case 3:                 //to end
          TimeSlider.setValue(TimeSlider.getMaximum());
          break;
        case 4:                 //to start
          TimeSlider.setValue(TimeSlider.getMinimum());
          break;
        case 5:                 //stop
          AnimTimer.stop();
          break;
          //StartButton.setSelected(!StartButton.isSelected());
      }
  }
    }



  private class ProjectionSelector implements ActionListener{

     public ProjectionSelector(JMenu M){
        Menu=M;
     }

     public boolean addProjection(Projection P){
        if(P==null) return false;
        if(Projections.contains(P)) return false;
  String Name = P.getName();
  JCheckBoxMenuItem MI = new JCheckBoxMenuItem(Name);
  Menu.add(MI);
  if(Projections.size()==0){
      MI.setState(true);
      selectedIndex = 0;
  }

  MI.addActionListener(this);
  MenuItems.add(MI);
  Projections.add(P);
  return true;
     }

     public void actionPerformed(ActionEvent evt){
        Object source = evt.getSource();
  int index = MenuItems.indexOf(source);
  if(index<0) return;
  if(index==selectedIndex) return;


  ProjectionManager.setProjection((Projection)Projections.get(index));
  if(selectedIndex>=0)
    ((JCheckBoxMenuItem)MenuItems.get(selectedIndex)).setState(false);
        selectedIndex=index;
  ((JCheckBoxMenuItem)source).setState(true);
     }

     private Vector Projections = new Vector();
     private Vector MenuItems = new Vector();
     private JMenu Menu;
     private int selectedIndex = -1;
  }


 class CreatePointSequenceListener extends MouseInputAdapter{

    /** Will be called if an mouse click is performed. 
      * This event is evaluated when the current mode is 
      * unequal to the rectangle mode which have a special treatment.
      * In all other modes. the new point is added to the current
      * point sequence. 
      **/
        public void mouseClicked(MouseEvent evt){
          if(mode==RECTANGLE_MODE){
             return;
          } 
          if(evt.getButton()!=MouseEvent.BUTTON1)
             return;
      
          if(!computeOrig(evt.getPoint(),aPoint)){
              Reporter.showError("Error in computing Projection");  
              return;// ignore this point 
          }
          double x = aPoint.x;
          double y = aPoint.y;

          if(mode==POINT_MODE){
               ListExpr pointList = ListExpr.twoElemList( 
                                            ListExpr.symbolAtom("point"),
                                            ListExpr.twoElemList(
                                            ListExpr.realAtom(x),
                                            ListExpr.realAtom(y)));
               String Name = getNameFromUser();
               if(Name!=null){
                   processSecondoObject(Name,"point",pointList);
               }
               return;
          }

          if(ps.isEmpty()){
              points = new  Vector();
          }
          Point2D.Double thePoint = new Point2D.Double(x,y);
          if(mode==REGION_MODE){
              if(haveIntersections(points,thePoint)){
                  Reporter.showError("Intersection detected");
                  return;
              }
          }

           boolean repchanged = ps.add(x,y) || mode==FILLED_POINT_SEQUENCE_MODE ||
                                               mode==REGION_MODE;
           GraphDisplay.paintAdditional(ps); 
           if(repchanged)
              GraphDisplay.repaint();
           points.add(thePoint);
           Graphics2D G = (Graphics2D)GraphDisplay.getGraphics();
           ps.draw(G,allProjection,CurrentState.ActualTime);
}

      /** This function computes the coordinates in the 'world' from the 
        * given mouse coordinates. 
        **/
      private boolean computeOrig(java.awt.Point orig, java.awt.geom.Point2D.Double result){
          // first compute the virtual screen coordinates
          Point2D.Double p = new Point2D.Double();
          try{
            p = (Point2D.Double) allProjection.inverseTransform(orig,p);
          }catch(Exception e){} 
          double x = p.getX();
          double y = p.getY();
          return ProjectionManager.estimateOrig(x,y,result);
      }

     /** Sets the current point sequnce to be empty. And removes a rectangle if there is one. 
       **/ 
      public void reset(){
          ps.reset();
          points=null;
          GraphDisplay.paintAdditional(null);
          if(isPainted)
              paintRectangle();
          rectangle_start=false;
          isPainted=false;
      }

      /** Sets the category for a point sequence.
        * The drawing of a rectangle is not affected by this method 
        **/
      public void setCategory(Category cat){
          ps.setCategory(cat);
      }
     
      /** Returns the category which is curretly used for painting a
        * point sequence 
        **/ 
      public Category getCat(){
          return ps.getCategory();
      }

      /** Will be called when the mouse is pressed.
       * This event is only processed in the rectangle mode.
       * It starts the painting and creating of a single rectangle. 
       */
      public void mousePressed(MouseEvent evt){
         if(mode!=RECTANGLE_MODE)
           return;
         if(evt.getButton()!=MouseEvent.BUTTON1)
           return;
         x1 = evt.getX();
         y1 = evt.getY();
         rectangle_start=true;
      }

      /** Will be called if the mouse is released.
        * This event is only processed in rectangle mode.
        * The user is asked for a name of the created rectangle.
        * From the name and the rectangle, a SecondoObject instance
        * will be created and stored in the object list and the
        * database. 
        */
      public void mouseReleased(MouseEvent evt){
         if(mode!=RECTANGLE_MODE)
            return;
         if(evt.getButton()!=MouseEvent.BUTTON1)
            return;
         if(!rectangle_start)
            return;

         if(isPainted)
           paintRectangle();
         isPainted=false;
         x2 = evt.getX();
         y2 = evt.getY();
         rectangle_start=false;
         // ask for the object name 
         if(x1==x2 || y1==y2) // not a rectangle
            return; 
         String Name=getNameFromUser();
         if(Name!=null){
              Point p1 = new Point(Math.min(x1,x2),Math.min(y1,y2));
              Point p2 = new Point(Math.max(x1,x2),Math.max(y1,y2));
              if(!computeOrig(p1,aPoint)){
                  Reporter.showError("Error in computing a point from mouse coordinates");
                  return;
              }
              double dx1 = aPoint.x;
              double dy1 = aPoint.y;
              if(!computeOrig(p2,aPoint)){
                  Reporter.showError("Error in computing a point from mouse coordinates");
                  return;
              }
              double dx2 = aPoint.x;
              double dy2 = aPoint.y;
              ListExpr value = ListExpr.fourElemList(ListExpr.realAtom(Math.min(dx1,dx2)),
                                                     ListExpr.realAtom(Math.max(dx1,dx2)),
                                                     ListExpr.realAtom(Math.min(dy1,dy2)),
                                                     ListExpr.realAtom(Math.max(dy1,dy2)));
              ListExpr result = ListExpr.twoElemList(ListExpr.symbolAtom("rect"),value);
              processSecondoObject(Name,"rect",result);
         }
         reset();
      }

      /** Called when the mouse is dragged.
        * This event is for the rectangle  mode only.
        * A rectangle from the starting coordinates up to the current
        * coordinates will be drawn.
        */
      public void mouseDragged(MouseEvent evt){
          if(mode!=RECTANGLE_MODE)
              return;
          if(!rectangle_start)
              return;
          if(isPainted)
              paintRectangle();
          x2 = evt.getX();
          y2 = evt.getY();
          paintRectangle();
          isPainted=true;
      }

      /** Paints a rectangle in XOR mode from the current coordinates. */
      private void paintRectangle(){
         Graphics2D G = (Graphics2D) GraphDisplay.getGraphics();
          G.setXORMode(Color.white);
         int x = Math.min(x1,x2);
         int w = Math.abs(x1-x2);
         int y = Math.min(y1,y2);
         int h= Math.abs(y1-y2);
         G.drawRect(x,y,w,h);
      }

     
      
      /** Checks whether the given vector of  points is a simple
        *  Region.
        **/ 
       private boolean isPolygon(Vector points){
           // in the pointvector cannot be intersection segments
           int size = points.size();
           if(size<3)
              return false;
           if(haveIntersections(points,(Point2D.Double)points.get(0)))
              return false;
           // missing check for building a region (points not on a single segment
           return true; 
       }

       /** Checks whether the segment from the last point in the points vector
         * has an intersection with a segment in the pointsequence represented
         * by points.
         **/
       private boolean haveIntersections(Vector points, Point2D.Double p){
            int size = points.size();
            if(size<2) // no segments in points
               return false;
            Point2D.Double p1 = (Point2D.Double) points.get(size-1);
            Point2D.Double p2;
            // create the line from the last point to the new point
            Line2D.Double line = new Line2D.Double(p1.x,p1.y,p.x,p.y);
            // iterate over all segments in points
            p1 = (Point2D.Double) points.get(0);
            for(int i=1;i<points.size()-1;i++){
               p2 = (Point2D.Double)points.get(i);
               if(line.intersectsLine(p1.x,p1.y,p2.x,p2.y))
                  return true;
               p1 = p2;
            }
            return false;
       }

      /** Closed a Point sequence.
        * The user will be asked for an object name.
        * From the name and the current point sequence, an
        * object is created. The type of the object depends on the
        * current mode. After creating the object, it is stored in
        * the object list and in the database. In the rectangle mode,
        * this method will be have no effect.
        **/
      public void closeSequence(){
         if(mode==RECTANGLE_MODE || mode==POINT_MODE)
             return;
         if(points!=null){ // points are available
            ListExpr value=new ListExpr();
            ListExpr last=null;
            Point2D.Double P;
            if(mode!=LINE_MODE ){
              if(points.size()>0){
                P=(Point2D.Double) points.get(0);
                value.destroy();
                value = ListExpr.oneElemList(
                           ListExpr.twoElemList(
                                 ListExpr.realAtom(P.getX()),
                                  ListExpr.realAtom(P.getY())));
                last=value;
              }
              for(int i=1;i<points.size();i++){
                 P = (Point2D.Double) points.get(i);
                 last = ListExpr.append(last,ListExpr.twoElemList(
                             ListExpr.realAtom(P.getX()),
                             ListExpr.realAtom(P.getY())));
              }
             }
             if(mode==LINE_MODE){ 
               if(points.size()>2){
                  double lastx;
                  double lasty;
                  double x,y;
                  P = (Point2D.Double)points.get(0);
                  lastx = P.x;
                  lasty = P.y;
                  boolean first=true;
                  //ListExpr last; // the last list element
                  for(int i=1;i<points.size();i++){
                     P = (Point2D.Double)points.get(i);
                     x = P.x;
                     y = P.y;
                     if(x!=lastx || y != lasty){
                         ListExpr Segment = ListExpr.fourElemList(ListExpr.realAtom(lastx),
                                                                  ListExpr.realAtom(lasty),
                                                                  ListExpr.realAtom(x),
                                                                  ListExpr.realAtom(y));
                         lastx=x;
                         lasty=y; 
                         if(first){
                             value.destroy();
                             value = ListExpr.oneElemList(Segment);
                             last  = value;
                             first = false;
                         } else{
                           last = ListExpr.append(last,Segment);
                         }
                                          
                     } 
                  } // for 
                } // more than two points
             } // LINE_MODE
             if(mode==REGION_MODE){
              // first component, face
               if(points.size()<3){
                 Reporter.showError("Points don't build a valid polygon.");
                 reset();
                 return;
               } 
               value = ListExpr.oneElemList(ListExpr.oneElemList(value));
             }
             String Name=getNameFromUser();
             if(Name!=null){
                  String TypeName=null;
                  switch(mode){
                    case POINT_SEQUENCE_MODE : TypeName = "pointsequence"; break;
                    case FILLED_POINT_SEQUENCE_MODE : TypeName = "pointsequence"; break;
                    case LINE_MODE : TypeName ="line"; break;
                    case POINTS_MODE : TypeName ="points";break;
                    case REGION_MODE : TypeName ="region";break;
                    default : TypeName ="unknown"; Reporter.writeError("invalid mode detected");
                  }
                  ListExpr result = ListExpr.twoElemList(ListExpr.symbolAtom(TypeName),value);
                  processSecondoObject(Name,TypeName,result);
            }
         } // points available
         reset();
      }

      private boolean processSecondoObject(String Name,String typeName,ListExpr content){
        SecondoObject o = new SecondoObject(IDManager.getNextID());
        o.setName(Name);
        o.fromList(content);
        if(VC!=null){
           VC.addObject(o);
          String cmd = "let "+Name+" = [const "+typeName+" value "+content.second()+"]";
          sj.lang.IntByReference errorCode = new sj.lang.IntByReference(0);
          ListExpr resultList = ListExpr.theEmptyList();
           StringBuffer errorMessage= new StringBuffer();
          if(!VC.execCommand(cmd,errorCode,resultList,errorMessage)){
               Reporter.showError("Error in storing pointsequence\n"+ 
                                        sj.lang.ServerErrorCodes.getErrorMessageText(errorCode.value)+"\n"+
                                        errorMessage);
                return false;
          }
          return true;
        }
          return false;
      }
 

      /** Asks the user for a name until the name is a correct symbol **/
      private String getNameFromUser(){
         String Name=null;
         do{
           Name = JOptionPane.showInputDialog("Please enter a name for the object");
           if(Name!=null) Name = Name.trim();
         } while(Name!=null && !LEUtils.isIdent(Name));
         return Name;
      } 


      public void setMode(int mode){
         if(mode>=0 && mode<7 ){
             this.mode=mode;
             if(mode==POINT_SEQUENCE_MODE)
                ps.setPaintMode(Dsplpointsequence.LINE_MODE);
             if(mode==FILLED_POINT_SEQUENCE_MODE)
                ps.setPaintMode(Dsplpointsequence.AREA_MODE);
             if(mode==POINTS_MODE)
                ps.setPaintMode(Dsplpointsequence.POINTS_MODE);
             if(mode==LINE_MODE)
                ps.setPaintMode(Dsplpointsequence.LINE_MODE);
             if(mode==POINT_MODE)
                ps.setPaintMode(Dsplpointsequence.POINTS_MODE);
             if(mode==REGION_MODE)
                ps.setPaintMode(Dsplpointsequence.AREA_MODE);
         }
      }  

      public int getMode(){
         return mode;
      }


      private Dsplpointsequence ps =new Dsplpointsequence();
      private Vector points=null;
      private int mode=RECTANGLE_MODE;
      // points for the rectangle
      private int x1;
      private int y1;
      private int x2;
      private int y2;
      private boolean rectangle_start=false;
      private boolean isPainted=false; // true if an rectangle is painted 
      private boolean active=false;

      private static final int POINT_SEQUENCE_MODE=0;
      private static final int RECTANGLE_MODE=1;
      private static final int FILLED_POINT_SEQUENCE_MODE=2;
      private static final int POINTS_MODE=3;
      private static final int LINE_MODE=4;
      private static final int POINT_MODE=5;
      private static final int REGION_MODE=6;
 }

}

