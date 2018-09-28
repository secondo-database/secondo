package mapviewer;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.FilenameFilter;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.ListSelectionModel;
import javax.swing.WindowConstants;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

import org.jxmapviewer.JXMapKit;
import org.jxmapviewer.JXMapViewer;
import org.jxmapviewer.painter.CompoundPainter;
import org.jxmapviewer.painter.Painter;
import org.jxmapviewer.viewer.GeoPosition;

import mapviewer.features.MapPaintable;
import mapviewer.importer.FileImporter;
import mapviewer.list.CheckboxListItem;
import mapviewer.list.CheckboxListItemFilter;
import mapviewer.list.CheckboxListItemMPoint;
import mapviewer.list.CheckboxListItemResult;
import mapviewer.list.CheckboxListRenderer;
import stlib.interfaces.moving.MovingPointIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.RegionIF;
import stlib.operations.interaction.Passes;
import stlib.operations.projection.Trajectory;

/**
 * Main window of the MapViewer
 * 
 * @author Markus Fuessel
 */
public class MapViewerSurface extends JFrame {

   private static final long serialVersionUID = 1L;

   private final String initialTitle;

   /**
    * Container
    */
   private Container mainContentPane;

   /**
    * The Main Menu
    */
   private JMenuBar mainMenuBar;

   /**
    * Panels
    */
   private JPanel mapPanel;
   private JPanel objectPanel;
   private JPanel propertiesPanel;

   private JSplitPane splittedPaneH;
   private JSplitPane splittedPaneV;

   /**
    * Reference to map kit which includes zoom buttons, zoom slider, a mini map
    */
   private JXMapKit mapKit;

   /**
    * Reference to the map
    */
   private JXMapViewer mainMap;

   /**
    * File chooser for gpx files
    */
   private JFileChooser gpxFileChooser;

   /**
    * Object Lists
    */
   private JList<CheckboxListItemMPoint> objectList;
   private JScrollPane objectScrollPane;
   private DefaultListModel<CheckboxListItemMPoint> chkBoxListMPointModel;

   private JList<CheckboxListItemFilter> filterList;
   private JScrollPane filterScrollPane;
   private DefaultListModel<CheckboxListItemFilter> chkBoxListFilterModel;

   private JList<CheckboxListItemResult> resultList;
   private JScrollPane resultScrollPane;
   private DefaultListModel<CheckboxListItemResult> listResultModel;

   /**
    * Actions for opening file chooser and closing application
    */
   private Action openFileAction;
   private Action exitAction;
   private Action noFilterAction;
   private Action passesFilterAction;
   private Action applyFilterAction;

   private Action clearObjectListAction;
   private Action clearFilterListAction;
   private Action clearResultListAction;
   private Action clearAllAction;

   /**
    * Listener
    */
   private SelectionAdapter selectionAdapter;

   /**
    * Painter
    */

   private SelectionPainter selectionPainter;

   /**
    * Current choosed filter
    */
   private FilterType filterType = FilterType.NO_FILTER;

   private FileImporter fileimporter = new FileImporter();

   /**
    * Main Constructor
    * 
    * @param title
    */
   public MapViewerSurface(String title) {

      super(title);
      initialTitle = title;

      mainContentPane = getContentPane();

      setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
      setPreferredSize(new Dimension(1000, 600));
      setLocation(100, 100);

      mapPanel = new JPanel(new BorderLayout());
      objectPanel = new JPanel(new BorderLayout());
      propertiesPanel = new JPanel(new BorderLayout());

      // splittedPaneV = new JSplitPane(JSplitPane.VERTICAL_SPLIT, true, mapPanel,
      // propertiesPanel);
      // splittedPaneV.setDividerLocation(400);

      splittedPaneH = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, true, objectPanel, mapPanel);
      splittedPaneH.setPreferredSize(new Dimension(800, 600));
      splittedPaneH.setDividerLocation(200);

      mainContentPane.setLayout(new BorderLayout());
      mainContentPane.add(splittedPaneH, BorderLayout.CENTER);

      initializeMap();
      initializeActions();
      initializeMenues();
      initializeToolBar();
      initializeListeners();

      initializeFileChooser();

      initializeObjectLists();

      initializePainters();

      mainMap.setPreferredSize(new Dimension(800, 600));
      mapPanel.add(mapKit, BorderLayout.CENTER);

      addWindowListener(new WindowAdapter() {
         @Override
         public void windowClosing(WindowEvent e) {
            exit();
         }
      });
   }

   /**
    * Initializes actions, set operations of actions
    */
   protected void initializeActions() {

      openFileAction = new AbstractAction("Open...") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {

            openFile();

         }
      };

      exitAction = new AbstractAction("Exit") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {
            exit();
         }
      };

      noFilterAction = new AbstractAction("no filter") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {
            filterType = FilterType.NO_FILTER;

         }
      };

      passesFilterAction = new AbstractAction("passes filter") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {
            filterType = FilterType.PASSES_FILTER;
         }
      };

      applyFilterAction = new AbstractAction("apply filter") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {
            applyFilter();
         }
      };

      clearObjectListAction = new AbstractAction("clear object list") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {
            chkBoxListMPointModel.removeAllElements();

            mapPanel.repaint();
         }
      };

      clearFilterListAction = new AbstractAction("clear filter list") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {
            chkBoxListFilterModel.removeAllElements();

            mapPanel.repaint();
         }
      };

      clearResultListAction = new AbstractAction("clear result list") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {
            listResultModel.removeAllElements();

            mapPanel.repaint();
         }
      };

      clearAllAction = new AbstractAction("clear all lists") {

         /**
          * default serial version ID
          */
         private static final long serialVersionUID = 1L;

         @Override
         public void actionPerformed(ActionEvent e) {
            chkBoxListMPointModel.removeAllElements();
            chkBoxListFilterModel.removeAllElements();
            listResultModel.removeAllElements();

            mapPanel.repaint();
         }
      };
   }

   /**
    * Create and initialize all menues
    */
   protected void initializeMenues() {

      JMenu fileMenu = new JMenu("File");

      fileMenu.add(new JMenuItem(openFileAction));
      fileMenu.addSeparator();
      fileMenu.add(exitAction);

      mainMenuBar = new JMenuBar();
      mainMenuBar.add(fileMenu);

      setJMenuBar(mainMenuBar);

   }

   /**
    * initialize the filter toolbar
    */
   protected void initializeToolBar() {
      JToolBar toolBar = new JToolBar();
      toolBar.setFloatable(false);
      toolBar.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

      final JToggleButton noFilterB = new JToggleButton(noFilterAction);
      noFilterB.setSelected(true);

      final JToggleButton passesFilterB = new JToggleButton(passesFilterAction);
      passesFilterB.setToolTipText("press the right mousebutton on the map to paint a filter");

      final JButton applyFilterB = new JButton(applyFilterAction);

      final JButton clearObjectsB = new JButton(clearObjectListAction);
      final JButton clearFilterB = new JButton(clearFilterListAction);
      final JButton clearResultB = new JButton(clearResultListAction);
      final JButton claerAllB = new JButton(clearAllAction);

      ButtonGroup bg = new ButtonGroup();

      bg.add(noFilterB);
      bg.add(passesFilterB);

      toolBar.add(noFilterB);
      toolBar.add(passesFilterB);

      toolBar.addSeparator();

      toolBar.add(applyFilterB);

      toolBar.addSeparator();

      toolBar.add(clearObjectsB);
      toolBar.add(clearFilterB);
      toolBar.add(clearResultB);
      toolBar.add(claerAllB);

      mainContentPane.add(toolBar, BorderLayout.NORTH);
   }

   /**
    * Initialzes the map and the map kit, set default location
    */
   protected void initializeMap() {
      mapKit = new JXMapKit();
      mainMap = mapKit.getMainMap();

      mapKit.setZoom(7);
      mapKit.setAddressLocation(new GeoPosition(52.5, 13.4));
   }

   /**
    * Initialize the Object Lists
    */
   protected void initializeObjectLists() {

      chkBoxListMPointModel = new DefaultListModel<>();
      objectList = new JList<>(chkBoxListMPointModel);

      objectList.setCellRenderer(new CheckboxListRenderer());
      objectList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

      objectList.addMouseListener(new MouseAdapter() {
         public void mouseClicked(MouseEvent event) {
            @SuppressWarnings("unchecked")
            JList<CheckboxListItemMPoint> list = (JList<CheckboxListItemMPoint>) event.getSource();

            // Get index of item clicked

            int index = list.locationToIndex(event.getPoint());
            CheckboxListItemMPoint item = list.getModel().getElementAt(index);

            // Toggle selected state

            item.setSelected(!item.isSelected());

            if (item.isSelected()) {
               PointIF point = item.getStartPoint();

               if (point.isDefined()) {

                  mapKit.setAddressLocation(new GeoPosition(point.getXValue(), point.getYValue()));
               }
            }

            // Repaint cell

            list.repaint(list.getCellBounds(index, index));
            repaint();
         }
      });

      objectScrollPane = new JScrollPane(objectList);

      chkBoxListFilterModel = new DefaultListModel<>();
      filterList = new JList<>(chkBoxListFilterModel);

      filterList.setCellRenderer(new CheckboxListRenderer());
      filterList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

      filterList.addMouseListener(new MouseAdapter() {
         public void mouseClicked(MouseEvent event) {
            @SuppressWarnings("unchecked")
            JList<CheckboxListItem> list = (JList<CheckboxListItem>) event.getSource();

            // Get index of item clicked

            int index = list.locationToIndex(event.getPoint());
            CheckboxListItem item = list.getModel().getElementAt(index);

            // Toggle selected state

            item.setSelected(!item.isSelected());

            // Repaint cell

            list.repaint(list.getCellBounds(index, index));
            repaint();
         }
      });

      filterScrollPane = new JScrollPane(filterList);

      listResultModel = new DefaultListModel<>();
      resultList = new JList<>(listResultModel);
      resultList.setCellRenderer(new CheckboxListRenderer());
      resultList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

      resultList.addMouseListener(new MouseAdapter() {
         public void mouseClicked(MouseEvent event) {
            @SuppressWarnings("unchecked")
            JList<CheckboxListItem> list = (JList<CheckboxListItem>) event.getSource();

            // Get index of item clicked

            int index = list.locationToIndex(event.getPoint());
            CheckboxListItem item = list.getModel().getElementAt(index);

            // Toggle selected state

            item.setSelected(!item.isSelected());

            // Repaint cell

            list.repaint(list.getCellBounds(index, index));
            repaint();
         }
      });

      resultScrollPane = new JScrollPane(resultList);

      objectPanel.add(objectScrollPane, BorderLayout.NORTH);
      objectPanel.add(filterScrollPane, BorderLayout.CENTER);
      objectPanel.add(resultScrollPane, BorderLayout.SOUTH);
   }

   /**
    * Initializes all Listeners of the application
    */
   protected void initializeListeners() {

      selectionAdapter = new SelectionAdapter(mainMap, this);

      mainMap.addMouseListener(selectionAdapter);
      mainMap.addMouseMotionListener(selectionAdapter);

   }

   /**
    * Initialize the file chooser with filter for gpx files
    * 
    */
   protected void initializeFileChooser() {
      gpxFileChooser = new JFileChooser();
      FileFilter filter = new FileNameExtensionFilter("GPX-Files", "gpx");

      gpxFileChooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);

      gpxFileChooser.setFileFilter(filter);
      gpxFileChooser.setMultiSelectionEnabled(true);

   }

   /**
    * Initializes the painters
    */
   protected void initializePainters() {
      ObjectListPainter<CheckboxListItemMPoint> objectsPainter = new ObjectListPainter<>(chkBoxListMPointModel);
      ObjectListPainter<CheckboxListItemFilter> filterPainter = new ObjectListPainter<>(chkBoxListFilterModel);
      ObjectListPainter<CheckboxListItemResult> resultPainter = new ObjectListPainter<>(listResultModel);

      selectionPainter = new SelectionPainter(selectionAdapter);

      List<Painter<JXMapViewer>> painters = new ArrayList<>();
      painters.add(objectsPainter);
      painters.add(filterPainter);
      painters.add(resultPainter);
      painters.add(selectionPainter);

      CompoundPainter<JXMapViewer> painter = new CompoundPainter<>(painters);
      mainMap.setOverlayPainter(painter);
   }

   /**
    * Open the file chooser
    */
   protected void openFile() {
      File[] files;

      if (gpxFileChooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {

         File file = gpxFileChooser.getSelectedFile();

         if (file.isDirectory()) {

            files = file.listFiles(new FilenameFilter() {

               @Override
               public boolean accept(File dir, String name) {
                  return name.toLowerCase().endsWith(".gpx");
               }
            });
         } else {

            files = gpxFileChooser.getSelectedFiles();
         }

         if (files.length == 0) {
            JOptionPane.showMessageDialog(null, "No files were selected!", "No files selected",
                  JOptionPane.INFORMATION_MESSAGE);
         } else {

            for (File gpxFile : files) {

               List<MovingPointIF> mpoints = fileimporter.loadFile(gpxFile);

               for (MovingPointIF mpoint : mpoints) {

                  chkBoxListMPointModel.addElement(new CheckboxListItemMPoint(gpxFile.getName(), mpoint));

                  System.out.println("Datei " + gpxFile.getName() + " geladen. ("
                        + Trajectory.execute(mpoint).getHalfsegments().size() + " Halfsegments)");
                  System.out.println(String.format("Länge: %.3f km", Trajectory.execute(mpoint).length(true) / 1000));
               }

            }
            repaint();

         }

      }
   }

   /**
    * EXIT
    */
   protected void exit() {
      System.exit(0);
   }

   /**
    * get the title string
    * 
    * @return title string
    */
   public String getInitialTitle() {
      return initialTitle;
   }

   /**
    * @return the filterType
    */
   public FilterType getFilterType() {
      return filterType;
   }

   /**
    * Add a Filter to the filter list
    * 
    * @param filterRegion
    * @param filterType
    */
   public void addFilter(final RegionIF filterRegion, final FilterType filterType) {

      if (filterType != FilterType.NO_FILTER) {
         CheckboxListItemFilter filterItem = new CheckboxListItemFilter(filterRegion, filterType);
         filterItem.setSelected(true);

         chkBoxListFilterModel.addElement(filterItem);
      }

   }

   public void applyFilter() {
      List<MapPaintable> resultObjects = new ArrayList<>();
      List<RegionIF> filterObjects = new ArrayList<>();

      Instant beginExecution = Instant.now();
      int executionCount = 0;

      for (int j = 0; j < chkBoxListFilterModel.size(); j++) {
         if (chkBoxListFilterModel.getElementAt(j).isSelected()) {
            CheckboxListItemFilter listElemFilter = chkBoxListFilterModel.getElementAt(j);

            resultObjects.add(new CheckboxListItemFilter(listElemFilter));

            filterObjects.add(listElemFilter.getRegionFilter());
         }
      }

      if (!filterObjects.isEmpty()) {

         for (int i = 0; i < chkBoxListMPointModel.size(); i++) {

            if (chkBoxListMPointModel.getElementAt(i).isSelected()) {

               int filterPassedCount = 0;

               CheckboxListItemMPoint listElemMPoint = chkBoxListMPointModel.getElementAt(i);
               MovingPointIF mpoint = listElemMPoint.getMPoint();

               for (int j = 0; j < filterObjects.size(); j++) {

                  RegionIF region = filterObjects.get(j);

                  boolean passes = Passes.execute(mpoint, region);

                  executionCount++;

                  System.out.println(chkBoxListMPointModel.getElementAt(i).toString() + " "
                        + chkBoxListFilterModel.getElementAt(j).toString() + ": " + passes);

                  if (passes) {
                     filterPassedCount++;
                  }
               }

               if (filterPassedCount == filterObjects.size() && !resultObjects.contains(listElemMPoint)) {
                  resultObjects.add(new CheckboxListItemMPoint(listElemMPoint));
               }
            }
         }
      }

      Instant endExecution = Instant.now();

      System.out.println(executionCount + " Ausführungen in "
            + Duration.between(beginExecution, endExecution).toMillis() + " Millisekunden.");

      if (resultObjects.isEmpty()) {
         listResultModel.addElement(new CheckboxListItemResult("No Results", resultObjects));
      } else {
         listResultModel.addElement(new CheckboxListItemResult(resultObjects));
      }

   }
}
