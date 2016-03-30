package appGui;

import convert.PostgresTypes;
import convert.SecondoTypes;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.IOException;
import java.io.LineNumberReader;
import java.io.StringReader;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.DefaultListModel;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreePath;

import postgis.ConnectPostgres;
import postgis.DatabaseName;
import postgis.IPGTextMessages;
import postgis.Spalte;
import postgis.Tabelle;
import secondo.ConnectSecondo;
import secondo.ISECTextMessages;
import secondo.MyInquiryViewer;
import secondo.MyRelViewer;
import secondo.MySecondoObject;
import secondo.SecondoObjectInfoClass;
//import sj.lang.IntByReference;
import secondoPostgisUtil.Configuration;
import secondoPostgisUtil.HTMLStrings;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import secondoPostgisUtil.UtilFunctions;
import appGuiUtil.IPublicTextMessages;
import appGuiUtil.JSplitPainInTheAss;
import appGuiUtil.Message;
import appGuiUtil.RefreshLeftSideTree;
import appGuiUtil.RefreshRightSideTree;
import appGuiUtil.SetUI;
import appGuiUtil.Warning;

/*
 * Diese Klass stellt die Haupt Seiter der Anwendung dar
 * 
 */

public class MainGui
implements ActionListener, IGlobalParameters, IPGTextMessages, IPublicTextMessages, ISECTextMessages, ItemListener
{
	JFrame mainFrame;
	JMenuBar jmenuBar;
	JMenu jmenu;
	JMenuItem jmenuItemSchliessen;
	JMenuItem jmenuItemRightZeigeERM;
	JMenuItem jmenuItemRightReconnect;
	JMenuItem jmenuItemRightConvert;
	JMenuItem jmenuItemRigtSupportedTypes;
	JMenuItem jmenuItemLeftReconnect;
	JMenu jmenuItemLeftSecCommands;
	JMenuItem jmenuItemLeftSecComDatabases;
	JMenuItem jmenuItemLeftSecComTypes;
	JMenuItem jmenuItemLeftSecComTypeConstructors;
	JMenuItem jmenuItemLeftSecComObjects;
	JMenuItem jmenuItemLeftSecComOperators;
	JMenuItem jmenuItemLeftSecComAlgebras;
	JMenuItem jmenuItemLeftConvert;
	JMenuItem jmenuItemLeftCopyMO2PG;
	JMenuItem jmenuItemLeftGenerateMO;
	JMenuItem jmenuItemLeftSupportedTypes;
	JMenuItem jmenuItemAbout;
	JMenuItem jmenuItemConfiguration;
	JMenuItem jmenuItemParameterSettings;
	JScrollPane jscrollpaneLinks;
	JScrollPane jscrollpaneRechts;
	JTree jtreeLinks;
	JTree jtreeRechts;
	JPanel jpanelLinks;
	JPanel jpanelRechts;
	JLabel jlabelLinks;
	JLabel jlabelRechts;
	Font fontHeadLine;
	JTextArea jtextareaLinks;
	JScrollPane jscrollpanetextareaLinks;
	JTextArea jtextareaRechts;
	JScrollPane jscrollpanetextareaRechts;
	volatile DefaultMutableTreeNode mdmtreenodeLinksServer;
	volatile DefaultMutableTreeNode mdmtreenodeLinksDatenbank;
	volatile DefaultMutableTreeNode mdmtreenodeLinksTabellen;
	volatile DefaultMutableTreeNode mdmtreenodeLinksSpalten;
	volatile DefaultMutableTreeNode mdmtreenodeLinksNamenTypen;
	volatile DefaultMutableTreeNode mdmDatenLinksServer;
	volatile DefaultMutableTreeNode mdmDatenLinksDatenbank;
	volatile DefaultMutableTreeNode mdmDatenLinksTabellen;
	volatile DefaultMutableTreeNode mdmDatenLinksSpalten;
	volatile DefaultMutableTreeNode mdmDatenLinksNamenTypen;
	volatile DefaultMutableTreeNode mdmtreenodeRechtsServer;
	volatile DefaultMutableTreeNode mdmtreenodeRechtsDatenbank;
	volatile DefaultMutableTreeNode mdmtreenodeRechtsTabellen;
	volatile DefaultMutableTreeNode mdmtreenodeRechtsSpalten;
	volatile DefaultMutableTreeNode mdmDatenRechtsServer;
	volatile DefaultMutableTreeNode mdmDatenRechtsDatenbank;
	volatile DefaultMutableTreeNode mdmDatenRechtsTabellen;
	volatile DefaultMutableTreeNode mdmDatenRechtsSpalten;
	JPopupMenu jpopupRechts;
	JMenuItem jmenuItemPopRightprERM;
	JMenuItem jmenuItemPopRightConvert;
	JMenuItem jmenuItemPopRightDropDatabase;
	JPopupMenu jpopupLinks;
	JMenu jmenuPopLeftSecCommands;
	JMenuItem jmenuItemPopLeftSecComDatabases;
	JMenuItem jmenuItemPopLeftSecComTypes;
	JMenuItem jmenuItemPopLeftSecComTypeConstructors;
	JMenuItem jmenuItemPopLeftSecComObjects;
	JMenuItem jmenuItemPopLeftSecComOperators;
	JMenuItem jmenuItemPopLeftSecComAlgebras;
	JMenuItem jmenuItemPopLeftSecRemoveDB;
	JMenuItem jmenuItemPopLeftConvert;
	JMenuItem jmenuItemPopLeftCopyMO2PG;
	JMenuItem jmenuItemPopLeftGenerateMO;
	HTMLStrings htmlstrings;
	LoadingGUI loadingGui;
	boolean bPressed_STRG;
	Thread thRechts;
	Thread thLinks;
	StringBuffer sbLinksCMD;
	TreePath selPathLinks;
	volatile boolean brunSecCommandOpenDB = false;
	volatile String strrunSecCommandCMD = "";
	volatile boolean bInitRechts;
	volatile boolean bInitLinks;
	ArrayList<String> alRelations = new ArrayList();
	ConnectPostgres cPostgres;
	ConnectSecondo cSecondo;
	TreePath selPath;
	
	public MainGui()
	{
	  this.bPressed_STRG = false;
	  
	  this.loadingGui = new LoadingGUI();
	  
	  this.sbLinksCMD = new StringBuffer();
	  init();
	  
	  this.loadingGui.closeLoadingWindow();
	}
	
	public void init()
	{
	  try
	  {
	    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
	  }
	  catch (ClassNotFoundException e)
	  {
	    e.printStackTrace();
	  }
	  catch (InstantiationException e)
	  {
	    e.printStackTrace();
	  }
	  catch (IllegalAccessException e)
	  {
	    e.printStackTrace();
	  }
	  catch (UnsupportedLookAndFeelException e)
	  {
	    e.printStackTrace();
	  }
	  this.jpopupRechts = new JPopupMenu();
	  this.jmenuItemPopRightprERM = new JMenuItem("Show table model");
	  
	  this.jmenuItemPopRightprERM.addActionListener(this.alPopupRechts);
	  this.jmenuItemPopRightConvert = new JMenuItem("Copy to SECONDO");
	  
	  this.jmenuItemPopRightConvert.addActionListener(this.alPopupRechts);
	  
	  this.jmenuItemPopRightDropDatabase = new JMenuItem("drop database");
	  this.jmenuItemPopRightDropDatabase.addActionListener(this.alPopupRechts);
	  
	  this.jpopupRechts.add(this.jmenuItemPopRightConvert);
	  this.jpopupRechts.add(new JSeparator());
	  this.jpopupRechts.add(this.jmenuItemPopRightprERM);
	  this.jpopupRechts.add(new JSeparator());
	  this.jpopupRechts.add(this.jmenuItemPopRightDropDatabase);
	  
	
	  this.fontHeadLine = new Font("Comic Sans MS", 1, 20);
	  this.jpanelLinks = new JPanel(new BorderLayout());
	  this.jpanelRechts = new JPanel(new BorderLayout());
	  
	  this.jlabelLinks = new JLabel("SECONDO", 0);
	  this.jlabelLinks.setForeground(new Color(0, 0, 255));
	  this.jlabelLinks.setFont(this.fontHeadLine);
	  
	  this.jlabelRechts = new JLabel("PostgreSQL/PostGIS", 0);
	  this.jlabelRechts.setForeground(new Color(200, 200, 100)); 
	  this.jlabelRechts.setFont(this.fontHeadLine);
	  
	
	  this.htmlstrings = new HTMLStrings();
	  
	  this.mdmtreenodeRechtsServer = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeServer(gsbPG_Host));
	  this.mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode();
	  this.mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode();
	  this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode();
	  
	  this.mdmDatenRechtsServer = new DefaultMutableTreeNode(gsbPG_Host);
	  this.mdmDatenRechtsDatenbank = new DefaultMutableTreeNode();
	  this.mdmDatenRechtsTabellen = new DefaultMutableTreeNode();
	  this.mdmDatenRechtsSpalten = new DefaultMutableTreeNode();
	  
	
	  this.mdmtreenodeLinksServer = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeServer(gsbSEC_Host));
	  this.mdmtreenodeLinksDatenbank = new DefaultMutableTreeNode();
	  this.mdmtreenodeLinksTabellen = new DefaultMutableTreeNode();
	  this.mdmtreenodeLinksSpalten = new DefaultMutableTreeNode();
	  this.mdmtreenodeLinksNamenTypen = new DefaultMutableTreeNode();
	  
	
	  this.mdmDatenLinksServer = new DefaultMutableTreeNode(gsbSEC_Host);
	  this.mdmDatenLinksDatenbank = new DefaultMutableTreeNode();
	  this.mdmDatenLinksTabellen = new DefaultMutableTreeNode();
	  this.mdmDatenLinksSpalten = new DefaultMutableTreeNode();
	  this.mdmDatenLinksNamenTypen = new DefaultMutableTreeNode();
	  
	  this.jtreeLinks = new JTree();
	  this.jtreeRechts = new JTree();
	  
	  this.thRechts = new Thread(this.runInitRechts);
	  this.thLinks = new Thread(this.runInitLinks);
	  this.thRechts.start();
	  this.thLinks.start();
	  try
	  {
	    this.thRechts.join();
	    this.thLinks.join();
	  }
	  catch (InterruptedException e1)
	  {
	    e1.printStackTrace();
	  }
	  if (!this.bInitRechts) {
	    new Warning("Can not connect to PostgreSQL/ PostGIS database.\nPlease check connection parameter.");
	  }
	  if (!this.bInitLinks) {
	    new Warning("Can not connect to SECONDO database.\nPlease checkconnection parameters.");
	  }
	  this.jpopupLinks = new JPopupMenu();
	  this.jtreeLinks = new JTree(this.mdmtreenodeLinksServer);
	  this.jtreeLinks.addMouseListener(this.mlLinks);
	  this.jtreeLinks.add(this.jpopupLinks);
	  
	  this.jtreeLinks.setRootVisible(true);
	  if ((this.jtreeLinks.getCellRenderer() instanceof DefaultTreeCellRenderer))
	  {
	    DefaultTreeCellRenderer renderer = 
	      (DefaultTreeCellRenderer)this.jtreeLinks.getCellRenderer();
	    renderer.setBackgroundSelectionColor(Color.ORANGE);
	    renderer.setTextNonSelectionColor(Color.RED);
	    renderer.setTextSelectionColor(Color.BLUE);
	  }
	  this.jtreeRechts = new JTree(this.mdmtreenodeRechtsServer);
	  this.jtreeRechts.addMouseListener(this.mlRechts);
	  this.jtreeRechts.add(this.jpopupRechts);
	  
	  this.jtreeRechts.setRootVisible(true);
	  if ((this.jtreeRechts.getCellRenderer() instanceof DefaultTreeCellRenderer))
	  {
	    DefaultTreeCellRenderer renderer = 
	      (DefaultTreeCellRenderer)this.jtreeRechts.getCellRenderer();
	    
	    renderer.setBackgroundSelectionColor(Color.ORANGE);
	    renderer.setTextNonSelectionColor(Color.RED);
	    renderer.setTextSelectionColor(Color.BLUE);
	  }
	  this.jtextareaLinks = new JTextArea(3, 1);
	  this.jtextareaLinks.setToolTipText("Please insert the query statement here and press 'ctrl + return' for sending. It is necessary to write the semicolon (;) at the end of a statement. You will get more information in the help document. Your statement gets an automatically limit.");
	  this.jtextareaLinks.addKeyListener(new KeyListener()
	  {
	    public void keyTyped(KeyEvent e) {}
	    
	    public void keyReleased(KeyEvent e)
	    {
	      if (e.getKeyCode() == 116) {
	        MainGui.this.sendSQLInput2Links();
	      } else if (e.getKeyCode() == 123) {
	    	  MainGui.this.jtextareaLinks.setText("");
	      }
	    }
	    
	    public void keyPressed(KeyEvent e)
	    {
	      if ((e.isControlDown()) && (e.getKeyCode() == 10)) {
	    	  MainGui.this.sendSQLInput2Links();
	      }
	    }
	  });
	  this.jscrollpanetextareaLinks = new JScrollPane(this.jtextareaLinks);
	  
	
	  this.jtextareaRechts = new JTextArea(3, 1);
	  this.jtextareaRechts.setToolTipText("Please insert the query statement here and press 'ctrl + return' for sending. It is necessary to write the semicolon (;) at the end of a statement. You will get more information in the help document. Your statement gets an automatically limit.");
	  this.jtextareaRechts.addKeyListener(new KeyListener()
	  {
	    public void keyTyped(KeyEvent e) {}
	    
	    public void keyReleased(KeyEvent e)
	    {
	      if (e.getKeyCode() == 116) {
	    	  MainGui.this.sendSQLInput2Rechts();
	      } else if (e.getKeyCode() == 123) {
	    	  MainGui.this.jtextareaRechts.setText("");
	      }
	    }
	    
	    public void keyPressed(KeyEvent e)
	    {
	      if ((e.isControlDown()) && (e.getKeyCode() == 10)) {
	    	  MainGui.this.sendSQLInput2Rechts();
	      }
	    }
	  });
	  this.jscrollpanetextareaRechts = new JScrollPane(this.jtextareaRechts);
	  
	  this.jscrollpaneLinks = new JScrollPane(this.jtreeLinks);
	  this.jscrollpaneRechts = new JScrollPane(this.jtreeRechts);
	  
	  this.jpanelLinks.add(this.jlabelLinks, "North");
	  this.jpanelLinks.add(this.jscrollpaneLinks, "Center");
	  this.jpanelLinks.add(this.jscrollpanetextareaLinks, "South");
	  
	  this.jpanelRechts.add(this.jlabelRechts, "North");
	  this.jpanelRechts.add(this.jscrollpaneRechts, "Center");
	  this.jpanelRechts.add(this.jscrollpanetextareaRechts, "South");
	  
	
	  this.jmenuBar = new JMenuBar();
	  
	  this.jmenu = new JMenu("File");
	  
	  this.jmenuItemSchliessen = new JMenuItem("Exit");
	  this.jmenuItemSchliessen.addActionListener(this);
	  this.jmenu.add(this.jmenuItemSchliessen);
	  
	  this.jmenuBar.add(this.jmenu);
	  
	  this.jmenu = new JMenu("View");
	  
	  this.jmenu = new JMenu("Left");
	  this.jmenuItemLeftReconnect = new JMenuItem("Reconnect Server");
	  this.jmenuItemLeftReconnect.addActionListener(this);
	  this.jmenu.add(this.jmenuItemLeftReconnect);
	  this.jmenu.addSeparator();
	  
	  this.jmenuItemLeftConvert = new JMenuItem("Copy Objects to PostgreSQL");
	  this.jmenuItemLeftConvert.addActionListener(this);
	  this.jmenu.add(this.jmenuItemLeftConvert);
	  
	  //this.jmenuItemLeftCopyMO2PG = new JMenuItem("Copy Moving Objects to PostgreSQL");
	  this.jmenuItemLeftCopyMO2PG = new JMenuItem("Create Upoints and splite UPoints");
	  this.jmenuItemLeftCopyMO2PG.addActionListener(this);
	  this.jmenu.add(this.jmenuItemLeftCopyMO2PG);
	  
	  this.jmenu.addSeparator();
	  
	  this.jmenuItemLeftGenerateMO = new JMenuItem("Generate Moving Objects");
	  this.jmenuItemLeftGenerateMO.addActionListener(this);
	  this.jmenu.add(this.jmenuItemLeftGenerateMO);
	  
	  this.jmenu.addSeparator();
	  
	  this.jmenuItemLeftSecCommands = new JMenu("SECONDO Commands");
	  this.jmenu.add(this.jmenuItemLeftSecCommands);
	  
	  this.jmenuItemLeftSecComDatabases = new JMenuItem("list databases");
	  this.jmenuItemLeftSecComDatabases.addActionListener(this);
	  this.jmenuItemLeftSecCommands.add(this.jmenuItemLeftSecComDatabases);
	  
	  this.jmenuItemLeftSecComTypes = new JMenuItem("list types");
	  this.jmenuItemLeftSecComTypes.addActionListener(this);
	  this.jmenuItemLeftSecCommands.add(this.jmenuItemLeftSecComTypes);
	  
	
	  this.jmenuItemLeftSecComTypeConstructors = new JMenuItem("list type constructors");
	  this.jmenuItemLeftSecComTypeConstructors.addActionListener(this);
	  this.jmenuItemLeftSecCommands.add(this.jmenuItemLeftSecComTypeConstructors);
	  
	  this.jmenuItemLeftSecComObjects = new JMenuItem("list objects");
	  this.jmenuItemLeftSecComObjects.addActionListener(this);
	  this.jmenuItemLeftSecCommands.add(this.jmenuItemLeftSecComObjects);
	  
	  this.jmenuItemLeftSecComOperators = new JMenuItem("list operators");
	  this.jmenuItemLeftSecComOperators.addActionListener(this);
	  this.jmenuItemLeftSecCommands.add(this.jmenuItemLeftSecComOperators);
	  
	  this.jmenuItemLeftSecComAlgebras = new JMenuItem("list algebras");
	  this.jmenuItemLeftSecComAlgebras.addActionListener(this);
	  this.jmenuItemLeftSecCommands.add(this.jmenuItemLeftSecComAlgebras);
	  
	
	  this.jmenu.addSeparator();
	  this.jmenuItemLeftSupportedTypes = new JMenuItem("Supported Types");
	  this.jmenuItemLeftSupportedTypes.addActionListener(this);
	  this.jmenu.add(this.jmenuItemLeftSupportedTypes);
	  
	
	  this.jmenuPopLeftSecCommands = new JMenu("Secondo Commands");
	  
	  this.jmenuItemPopLeftSecComDatabases = new JMenuItem("list databases");
	  this.jmenuItemPopLeftSecComDatabases.addActionListener(this.alPopupLinks);
	  this.jmenuPopLeftSecCommands.add(this.jmenuItemPopLeftSecComDatabases);
	  
	  this.jmenuItemPopLeftSecComTypes = new JMenuItem("list types");
	  this.jmenuItemPopLeftSecComTypes.addActionListener(this.alPopupLinks);
	  this.jmenuPopLeftSecCommands.add(this.jmenuItemPopLeftSecComTypes);
	  
	
	  this.jmenuItemPopLeftSecComTypeConstructors = new JMenuItem("list type constructors");
	  this.jmenuItemPopLeftSecComTypeConstructors.addActionListener(this.alPopupLinks);
	  this.jmenuPopLeftSecCommands.add(this.jmenuItemPopLeftSecComTypeConstructors);
	  
	  this.jmenuItemPopLeftSecComObjects = new JMenuItem("list objects");
	  this.jmenuItemPopLeftSecComObjects.addActionListener(this.alPopupLinks);
	  this.jmenuPopLeftSecCommands.add(this.jmenuItemPopLeftSecComObjects);
	  
	  this.jmenuItemPopLeftSecComOperators = new JMenuItem("list operators");
	  this.jmenuItemPopLeftSecComOperators.addActionListener(this.alPopupLinks);
	  this.jmenuPopLeftSecCommands.add(this.jmenuItemPopLeftSecComOperators);
	  
	  this.jmenuItemPopLeftSecComAlgebras = new JMenuItem("list algebras");
	  this.jmenuItemPopLeftSecComAlgebras.addActionListener(this.alPopupLinks);
	  this.jmenuPopLeftSecCommands.add(this.jmenuItemPopLeftSecComAlgebras);
	  
	  this.jmenuItemPopLeftSecRemoveDB = new JMenuItem("delete database");
	  this.jmenuItemPopLeftSecRemoveDB.addActionListener(this.alPopupLinks);
	  this.jmenuPopLeftSecCommands.add(new JSeparator());
	  this.jmenuPopLeftSecCommands.add(this.jmenuItemPopLeftSecRemoveDB);
	  
	  this.jpopupLinks.add(this.jmenuPopLeftSecCommands);
	  
	  this.jpopupLinks.add(new JSeparator());
	  this.jmenuItemPopLeftConvert = new JMenuItem("Copy Objects to PostgreSQL");
	  this.jmenuItemPopLeftConvert.addActionListener(this.alPopupLinks);
	  this.jpopupLinks.add(this.jmenuItemPopLeftConvert);
	  
	  this.jmenuItemPopLeftCopyMO2PG = new JMenuItem("Copy Moving Objects to PostgreSQL");
	  this.jmenuItemPopLeftCopyMO2PG.addActionListener(this.alPopupLinks);
	  this.jpopupLinks.add(this.jmenuItemPopLeftCopyMO2PG);
	  
	  this.jpopupLinks.add(new JSeparator());
	  this.jmenuItemPopLeftGenerateMO = new JMenuItem("Generate Moving Objects");
	  this.jmenuItemPopLeftGenerateMO.addActionListener(this.alPopupLinks);
	  this.jpopupLinks.add(this.jmenuItemPopLeftGenerateMO);
	  
	  this.jmenuBar.add(this.jmenu);
	  
	  this.jmenu = new JMenu("Right");
	  
	  this.jmenuItemRightReconnect = new JMenuItem("Reconnect Server");
	  this.jmenuItemRightReconnect.addActionListener(this);
	  this.jmenu.add(this.jmenuItemRightReconnect);
	  
	  this.jmenu.addSeparator();
	  this.jmenuItemRightConvert = new JMenuItem("Copy to SECONDO");
	  this.jmenuItemRightConvert.addActionListener(this);
	  this.jmenu.add(this.jmenuItemRightConvert);
	  
	  this.jmenu.addSeparator();
	  
	  this.jmenuItemRightZeigeERM = new JMenuItem("Show table model");
	  this.jmenuItemRightZeigeERM.addActionListener(this);
	  this.jmenu.add(this.jmenuItemRightZeigeERM);
	  
	  this.jmenu.addSeparator();
	  this.jmenuItemRigtSupportedTypes = new JMenuItem("Supported Types");
	  this.jmenuItemRigtSupportedTypes.addActionListener(this);
	  this.jmenu.add(this.jmenuItemRigtSupportedTypes);
	  
	  this.jmenuBar.add(this.jmenu);
	  
	  this.jmenu = new JMenu("Connect");
	  
//	  this.jmenuItemConfiguration = new JMenuItem("Set Configuration");
//	  this.jmenuItemConfiguration.addActionListener(this);
//	  this.jmenu.add(this.jmenuItemConfiguration);
//	  
	  this.jmenuItemParameterSettings = new JMenuItem("Parameter Settings");
	  this.jmenuItemParameterSettings.addActionListener(this);
	  this.jmenu.add(this.jmenuItemParameterSettings);
	  
	  this.jmenu.addSeparator();
	  
//	  this.jmenuItemAbout = new JMenuItem("About");
//	  this.jmenuItemAbout.addActionListener(this);
//	  this.jmenu.add(this.jmenuItemAbout);
	  this.jmenuBar.add(this.jmenu);
	  
	  this.jmenu = new JMenu("Help");
	  this.jmenuItemAbout = new JMenuItem("About");
	  this.jmenuItemAbout.addActionListener(this);
	  this.jmenu.add(this.jmenuItemAbout);
	  this.jmenuBar.add(this.jmenu);
	  
	  JSplitPane jsplitPane = new JSplitPane(1, 
	    this.jpanelLinks, this.jpanelRechts);
	  jsplitPane.setOneTouchExpandable(true);
	  
	  jsplitPane.setPreferredSize(new Dimension(500, 500));
	  JSplitPainInTheAss.setDividerLocation(jsplitPane, 0.5D);
	  
	  this.mainFrame = new JFrame("SecondoPostGIS Converter");
	  this.mainFrame.setDefaultCloseOperation(3);
	  
	  this.mainFrame.setIconImage(gimp_S2P);
	  this.mainFrame.setLayout(new BorderLayout());
	  
	  ActionToolbar toolbar = new ActionToolbar(this);
	  
	  this.mainFrame.add(toolbar.jtoolbar, "First");
	  this.mainFrame.add(jsplitPane, "Center");
	  
	  JPanel panelMainFrame = new JPanel(new BorderLayout(5, 5));
	  
	  panelMainFrame.add(new JLabel(" "), "North");
	  
	  this.mainFrame.add(panelMainFrame, "South");
	  
	  this.mainFrame.setLocationByPlatform(true);
	  
	  this.mainFrame.pack();
	  this.mainFrame.setJMenuBar(this.jmenuBar);
	  this.mainFrame.setVisible(true);
	  
	  LogFileHandler.mlogger.info("show S2P gui");
	}
	
	private void exitSPC()
	{
	  if (this.cPostgres.conn != null) {
	    try
	    {
	      this.cPostgres.conn.close();
	    }
	    catch (SQLException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	  if (this.cSecondo.isSecondoConnected()) {
	    this.cSecondo.closeConnection();
	  }
	  UtilFunctions u = new UtilFunctions();
	  u.removeAllTempFiles();
	  
	  this.mainFrame.setVisible(false);
	  if (LogFileHandler.mFH != null) {
	    LogFileHandler.mFH.close();
	  }
	  System.exit(0);
	}
	
	private Runnable runSecCommand = new Runnable()
	{
	  public void run()
	  {
	    String strSelDB = MainGui.this.getLeftSelectedDatabase(new StringBuffer("Before you send a statement you must select a database."));
	    if (strSelDB != null)
	    {
	      if (MainGui.this.cSecondo.isSecondoConnected()) {
	        MainGui.this.cSecondo.sendCommand(new StringBuffer("close database;"));
	      }
	      MainGui.this.sbLinksCMD.delete(0, MainGui.this.sbLinksCMD.length());
	      if (!MainGui.this.cSecondo.isSecondoConnected()) {
	        MainGui.this.cSecondo.connect();
	      }
	      if (MainGui.this.brunSecCommandOpenDB)
	      {
	        MainGui.this.cSecondo.sendCommand(new StringBuffer("open database " + strSelDB + ";"));
	        MainGui.this.sbLinksCMD.append(MainGui.this.strrunSecCommandCMD);
	        new SecInquiryGUI().init(MainGui.this.cSecondo, MainGui.this.sbLinksCMD);
	        MainGui.this.cSecondo.sendCommand(new StringBuffer("close database"));
	        
	        MainGui.this.cSecondo.closeConnection();
	      }
	      else
	      {
	        MainGui.this.cSecondo.sendCommand(new StringBuffer(MainGui.this.strrunSecCommandCMD + strSelDB + ";"));
	        try
	        {
	          MainGui.this.thLinks = new Thread(MainGui.this.runInitLinks);
	          MainGui.this.thLinks.start();
	          MainGui.this.thLinks.join();
	        }
	        catch (InterruptedException e1)
	        {
	          LogFileHandler.mlogger.severe(e1.getMessage());
	        }
	        finally
	        {
	          if (!MainGui.this.bInitLinks) {
	            new Warning("Can not connect to PostgreSQL/ PostGIS database.\nPlease check connection parameter.");
	          }
	        }
	      }
	    }
	  }
	};
	
	private void simpleInquiryCommand(StringBuffer _sbCMD)
	{
	  LogFileHandler.mlogger.info("send inquiry command");
	  this.sbLinksCMD.delete(0, this.sbLinksCMD.length());
	  this.sbLinksCMD.append(_sbCMD);
	  
	  new SecInquiryGUI().init(this.cSecondo, this.sbLinksCMD);
	}
	
	private void simpleSecCommandDBOpen(String _strCMD, boolean _bOpenDB)
	{
	  this.brunSecCommandOpenDB = _bOpenDB;
	  this.strrunSecCommandCMD = _strCMD;
	  Thread thtmp = new Thread(this.runSecCommand);
	  thtmp.start();
	}
	
	public boolean getInitLinks()
	{
	  return this.bInitLinks;
	}
	
	public boolean getInitRechts()
	{
	  return this.bInitRechts;
	}
	
	Runnable runInitRechts = new Runnable()
	{
	  public void run()
	  {
	    MainGui.this.bInitRechts = false;
	    MainGui.this.bInitRechts = MainGui.this.initRechts();
	  }
	};
	
	public Runnable getRunInitRechts()
	{
	  return this.runInitRechts;
	}
	
	Runnable runInitLinks = new Runnable()
	{
	  public void run()
	  {
	    MainGui.this.bInitLinks = false;
	    MainGui.this.bInitLinks = MainGui.this.initLinks();
	  }
	};
	
	public Runnable getRunInitLinks()
	{
	  return this.runInitLinks;
	}
	
	public boolean initRechts()
	{
	  this.cPostgres = new ConnectPostgres(gsbPG_Host, Integer.valueOf(gsbPG_Port.toString()).intValue(), 
	    gsbPG_User, gsbPG_Pwd);
	  if (!this.cPostgres.connect()) {
	    return false;
	  }
	  HTMLStrings htmlstrings = new HTMLStrings();
	  
	  DefaultTreeModel model = (DefaultTreeModel)this.jtreeRechts.getModel();
	  if (this.jtreeRechts.getRowCount() > 1)
	  {
	    this.jtreeRechts.setSelectionRow(1);
	    TreePath path = this.jtreeRechts.getSelectionPath();
	    MutableTreeNode node = (MutableTreeNode)path.getLastPathComponent();
	    model.removeNodeFromParent(node);
	  }
	  model.setRoot(null);
	  
	  this.mdmtreenodeRechtsServer = new DefaultMutableTreeNode(htmlstrings.generateJTreeServer(gsbPG_Host));
	  this.mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode();
	  this.mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode();
	  this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode();
	  
	  this.mdmDatenRechtsServer = new DefaultMutableTreeNode(gsbPG_Host);
	  this.mdmDatenRechtsDatenbank = new DefaultMutableTreeNode();
	  this.mdmDatenRechtsTabellen = new DefaultMutableTreeNode();
	  this.mdmDatenRechtsSpalten = new DefaultMutableTreeNode();
	  
	  Tabelle tabelle = new Tabelle();
	  DatabaseName db = new DatabaseName();
	  
	  LinkedList<DatabaseName> llDatenbanknamen = new LinkedList();
	  LinkedList<Tabelle> llTabellennamen = new LinkedList();
	  LinkedList<Spalte> llSpaltennamen = new LinkedList();
	  
	
	  llDatenbanknamen = this.cPostgres.getDatabaseNames();
	  
	  Iterator<DatabaseName> itDatenbanknamen = llDatenbanknamen.iterator();
	  while (itDatenbanknamen.hasNext())
	  {
	    db = (DatabaseName)itDatenbanknamen.next();
	    
	    this.mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode(htmlstrings.generateJTreeDatenbank(db.getSbName()));
	    this.mdmDatenRechtsDatenbank = new DefaultMutableTreeNode(db.getSbName().toString());
	    try
	    {
	      this.cPostgres.conn.close();
	    }
	    catch (SQLException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	    if (!this.cPostgres.connect(db.getSbName())) {
	      return false;
	    }
	    llTabellennamen.clear();
	    llTabellennamen = this.cPostgres.getTableNamesFromDB();
	    Iterator<Tabelle> itTabellennamen = llTabellennamen.iterator();
	    while (itTabellennamen.hasNext())
	    {
	      tabelle = new Tabelle();
	      
	      tabelle = (Tabelle)itTabellennamen.next();
	      
	      tabelle.setiRows(this.cPostgres.generateRowsFromTable(tabelle.getSbName().toString()));
	      
	
	      this.mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode(htmlstrings.generateJTreeTabelle(
	        tabelle.getShowText()));
	      
	      this.mdmDatenRechtsTabellen = new DefaultMutableTreeNode(tabelle.getSbName().toString());
	      
	
	      llSpaltennamen.clear();
	      llSpaltennamen = this.cPostgres.getColumnsNamesFromTable(tabelle.getSbName().toString(), db.getSbName());
	      
	
	      HashSet<String> hsPrimaryKeys = new HashSet();
	      hsPrimaryKeys = this.cPostgres.getPrimaryKeyFromTable(tabelle.getSbName().toString());
	      
	
	
	      HashSet<String> hsForeignKeys = new HashSet();
	      hsForeignKeys = this.cPostgres.getForeignKeyFromTable(tabelle.getSbName().toString());
	      for (int i = 0; i < llSpaltennamen.size(); i++)
	      {
	        if (hsPrimaryKeys.contains(((Spalte)llSpaltennamen.get(i)).getSbName().toString())) {
	          this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpaltePrimaryKey(
	            ((Spalte)llSpaltennamen.get(i)).getSbName(), ((Spalte)llSpaltennamen.get(i)).getSbTyp()));
	        } else if (hsForeignKeys.contains(((Spalte)llSpaltennamen.get(i)).getSbName().toString())) {
	          this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpalteForeignKey(
	            ((Spalte)llSpaltennamen.get(i)).getSbName(), ((Spalte)llSpaltennamen.get(i)).getSbTyp()));
	        } else {
	          this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpalte(
	            ((Spalte)llSpaltennamen.get(i)).getSbName(), ((Spalte)llSpaltennamen.get(i)).getSbTyp()));
	        }
	        this.mdmDatenRechtsSpalten = new DefaultMutableTreeNode(((Spalte)llSpaltennamen.get(i)).getSbName().toString() + ":" + ((Spalte)llSpaltennamen.get(i)).getSbTyp().toString());
	        
	        this.mdmtreenodeRechtsTabellen.add(this.mdmtreenodeRechtsSpalten);
	        this.mdmDatenRechtsTabellen.add(this.mdmDatenRechtsSpalten);
	      }
	      this.mdmtreenodeRechtsDatenbank.add(this.mdmtreenodeRechtsTabellen);
	      this.mdmDatenRechtsDatenbank.add(this.mdmDatenRechtsTabellen);
	    }
	    db = new DatabaseName();
	    
	
	
	    this.mdmtreenodeRechtsServer.add(this.mdmtreenodeRechtsDatenbank);
	    this.mdmDatenRechtsServer.add(this.mdmDatenRechtsDatenbank);
	  }
	  model.setRoot(this.mdmtreenodeRechtsServer);
	  try
	  {
	    this.cPostgres.conn.close();
	  }
	  catch (SQLException e)
	  {
	    LogFileHandler.mlogger.severe(e.getMessage());
	  }
	  return true;
	}
	
	public boolean initLinks()
	{
	  boolean bRet = false;
	  try
	  {
	    this.cSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
	      gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
	  }
	  catch (SecurityException e)
	  {
	    bRet = false;
	    LogFileHandler.mlogger.severe(e.getMessage());
	    return bRet;
	  }
	  catch (IOException e)
	  {
	    LogFileHandler.mlogger.severe(e.getMessage());
	    return bRet;
	  }
	  if (!this.cSecondo.connect()) {
	    return bRet;
	  }
	  DefaultTreeModel model = (DefaultTreeModel)this.jtreeLinks.getModel();
	  if (this.jtreeLinks.getRowCount() > 1)
	  {
	    this.jtreeLinks.setSelectionRow(1);
	    TreePath path = this.jtreeLinks.getSelectionPath();
	    MutableTreeNode node = (MutableTreeNode)path.getLastPathComponent();
	    model.removeNodeFromParent(node);
	  }
	  model.setRoot(null);
	  
	  this.mdmtreenodeLinksServer = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeServer(gsbSEC_Host));
	  this.mdmtreenodeLinksDatenbank = new DefaultMutableTreeNode();
	  this.mdmtreenodeLinksTabellen = new DefaultMutableTreeNode();
	  this.mdmtreenodeLinksSpalten = new DefaultMutableTreeNode();
	  this.mdmtreenodeLinksNamenTypen = new DefaultMutableTreeNode();
	  
	  this.mdmDatenLinksServer = new DefaultMutableTreeNode(gsbSEC_Host);
	  this.mdmDatenLinksDatenbank = new DefaultMutableTreeNode();
	  this.mdmDatenLinksTabellen = new DefaultMutableTreeNode();
	  this.mdmDatenLinksSpalten = new DefaultMutableTreeNode();
	  this.mdmDatenLinksNamenTypen = new DefaultMutableTreeNode();
	  
	  LinkedList<String> llDatabasenames = new LinkedList();
	  
	  llDatabasenames = this.cSecondo.getDatabaseNames();
	  if (llDatabasenames.size() != 0)
	  {
	    LinkedList<String> llTypes = new LinkedList();
	    LinkedList<SecondoObjectInfoClass> llObjects = new LinkedList();
	    for (int i = 0; i < llDatabasenames.size(); i++)
	    {
	      llTypes.clear();
	      llObjects.clear();
	      
	      llTypes = this.cSecondo.getTypes((String)llDatabasenames.get(i));
	      llObjects = this.cSecondo.getObjects((String)llDatabasenames.get(i));
	      
	      this.mdmtreenodeLinksDatenbank = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeDatenbank(new StringBuffer((String)llDatabasenames.get(i))));
	      this.mdmDatenLinksDatenbank = new DefaultMutableTreeNode(new StringBuffer((String)llDatabasenames.get(i)));
	      
	
	      this.mdmtreenodeLinksTabellen = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeTabelle(new StringBuffer("TYPES")));
	      this.mdmDatenLinksTabellen = new DefaultMutableTreeNode(new StringBuffer("TYPES"));
	      for (int k = 0; k < llTypes.size(); k++)
	      {
	        this.mdmtreenodeLinksSpalten = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeSpalte(
	          new StringBuffer((String)llTypes.get(k)), new StringBuffer("")));
	        this.mdmtreenodeLinksTabellen.add(this.mdmtreenodeLinksSpalten);
	        
	        this.mdmDatenLinksSpalten = new DefaultMutableTreeNode(new StringBuffer((String)llTypes.get(k)));
	        this.mdmDatenLinksTabellen.add(this.mdmDatenLinksSpalten);
	      }
	      this.mdmtreenodeLinksDatenbank.add(this.mdmtreenodeLinksTabellen);
	      this.mdmDatenLinksDatenbank.add(this.mdmDatenLinksTabellen);
	      
	
	      this.mdmtreenodeLinksTabellen = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeTabelle(new StringBuffer("OBJECTS")));
	      this.mdmDatenLinksTabellen = new DefaultMutableTreeNode(new StringBuffer("OBJECTS"));
	      for (int k = 0; k < llObjects.size(); k++)
	      {
	        this.mdmtreenodeLinksSpalten = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeSpalte(
	          new StringBuffer(((SecondoObjectInfoClass)llObjects.get(k)).getStrObjName()), new StringBuffer(String.valueOf(((SecondoObjectInfoClass)llObjects.get(k)).getiCount()))));
	        
	
	
	        this.mdmDatenLinksSpalten = new DefaultMutableTreeNode(new StringBuffer(((SecondoObjectInfoClass)llObjects.get(k)).getStrObjName()));
	        for (int m = 0; m < ((SecondoObjectInfoClass)llObjects.get(k)).sizeNames(); m++)
	        {
	          this.mdmtreenodeLinksNamenTypen = new DefaultMutableTreeNode(this.htmlstrings.generateJTreeSpalte(
	            ((SecondoObjectInfoClass)llObjects.get(k)).getColName(m), ((SecondoObjectInfoClass)llObjects.get(k)).getColTypes(m)));
	          this.mdmtreenodeLinksSpalten.add(this.mdmtreenodeLinksNamenTypen);
	          
	          this.mdmDatenLinksNamenTypen = new DefaultMutableTreeNode(((SecondoObjectInfoClass)llObjects.get(k)).getColName(m) + ":" + ((SecondoObjectInfoClass)llObjects.get(k)).getColTypes(m));
	          this.mdmDatenLinksSpalten.add(this.mdmDatenLinksNamenTypen);
	        }
	        this.mdmtreenodeLinksTabellen.add(this.mdmtreenodeLinksSpalten);
	        
	        this.mdmDatenLinksTabellen.add(this.mdmDatenLinksSpalten);
	      }
	      this.mdmtreenodeLinksDatenbank.add(this.mdmtreenodeLinksTabellen);
	      this.mdmDatenLinksDatenbank.add(this.mdmDatenLinksTabellen);
	      
	
	      this.mdmtreenodeLinksServer.add(this.mdmtreenodeLinksDatenbank);
	      this.mdmDatenLinksServer.add(this.mdmDatenLinksDatenbank);
	    }
	  }
	  this.mdmtreenodeLinksServer.add(this.mdmtreenodeLinksDatenbank);
	  this.mdmDatenLinksServer.add(this.mdmDatenLinksDatenbank);
	  
	  model.setRoot(this.mdmtreenodeLinksServer);
	  if (this.cSecondo.isSecondoConnected()) {
	    this.cSecondo.closeConnection();
	  }
	  return true;
	}
	
	Runnable runshowERMGUI = new Runnable()
	{
	  public void run()
	  {
	    MainGui.this.showERMGUI();
	  }
	};
	
	private void showERMGUI()
	{
	  String strDBName = getRightSelectedDatabase(new StringBuffer("Please select a database."));
	  if (strDBName != null)
	  {
	    LogFileHandler.mlogger.info("try to show erm gui");
	    
	    ERMGUI erm = new ERMGUI();
	    this.alRelations.clear();
	    
	    this.cPostgres.connect(new StringBuffer(strDBName));
	    
	    this.alRelations = this.cPostgres.getForeignRelations();
	    try
	    {
	      this.cPostgres.conn.close();
	    }
	    catch (SQLException e1)
	    {
	      LogFileHandler.mlogger.severe(e1.getMessage());
	    }
	    erm.init(this.mdmtreenodeRechtsServer, strDBName, this.alRelations);
	  }
	}
	
	private void sendSQLInput2Links()
	{
	  LogFileHandler.mlogger.info("send input left");
	  
	  LineNumberReader lnr = new LineNumberReader(new StringReader(this.jtextareaLinks.getText()));
	  
	  StringBuffer sbCommand = new StringBuffer();
	  ArrayList<String> altmpCommands = new ArrayList();
	  try
	  {
	    String line;
	    while ((line = lnr.readLine()) != null)
	    {
	      String line2;
	      if (line.length() > 0)
	      {
	        if (line.charAt(line.length() - 1) == ';')
	        {
	          sbCommand.append(line.substring(0, line.length() - 1));
	          altmpCommands.add(sbCommand.toString());
	          
	          sbCommand.delete(0, sbCommand.length());
	        }
	        else
	        {
	          sbCommand.append(line);
	        }
	      }
	      else {
	        sbCommand.append(line);
	      }
	    }
	  }
	  catch (IOException e)
	  {
	    LogFileHandler.mlogger.severe(e.getMessage());
	  }
	  if (altmpCommands.size() == 0) {
	    return;
	  }
	  String[] strATextAreaLinks = new UtilFunctions().toStringArray(altmpCommands);
	  
	
	  String strSelDB = getLeftSelectedDatabase(new StringBuffer("A database shall be selected before you can send a statement."));
	  if (strSelDB != null)
	  {
	    if (this.cSecondo != null) {
	      this.cSecondo.connect();
	    }
	    if (!this.cSecondo.isSecondoConnected()) {
	      this.cSecondo.connect();
	    }
	    if (!this.cSecondo.isSecondoConnected()) {
	      return;
	    }
	    UtilFunctions utilf = new UtilFunctions();
	    
	    this.cSecondo.sendCommand(new StringBuffer("close database;"));
	    
	
	    this.cSecondo.sendCommand(new StringBuffer("open database " + strSelDB + ";"));
	    this.sbLinksCMD.delete(0, this.sbLinksCMD.length());
	    
	    Pattern pHeadConsume = Pattern.compile("(head\\s?[\\[]{1}[0-9]+[\\]]{1})");
	    
	
	
	    this.cSecondo.setQueryResults2Null();
	    int iStatus = 0;
	    for (int i = 0; i < strATextAreaLinks.length; i++)
	    {
	      iStatus = 0;
	      if (strATextAreaLinks[i].length() > 0)
	      {
	        Matcher m = pHeadConsume.matcher(strATextAreaLinks[i]);
	        while (m.find())
	        {
	          strATextAreaLinks[i].replaceAll(pHeadConsume.pattern(), "head [50]");
	          iStatus = 1;
	        }
	        if (iStatus != 1) {
	          if (strATextAreaLinks[i].contains("consume"))
	          {
	            strATextAreaLinks[i] = strATextAreaLinks[i].replaceAll("consume", "head[50] consume");
	          }
	          else if (utilf.countChar(strATextAreaLinks[i], ' ') == 1)
	          {
	            strATextAreaLinks[i] = (strATextAreaLinks[i] + " feed head[" + "50" + "] consume");
	          }
	          else
	          {
	            new SetUI().setUIAndLanguage();
	            
	            int n = JOptionPane.showConfirmDialog(
	              null, 
	              "Your query can not be analysed \nand may need a lot of time.\nDo you really want to query?\nYour query was:\n" + strATextAreaLinks[i], 
	              "Question", 
	              0);
	            if (n == 1) {
	              continue;
	            }
	          }
	        }
	        this.sbLinksCMD.delete(0, this.sbLinksCMD.length());
	        this.sbLinksCMD.append(strATextAreaLinks[i]);
	        if (this.cSecondo.isSecondoConnected()) {
	          if (this.cSecondo.sendCommand(this.sbLinksCMD))
	          {
	            MySecondoObject secObj = new MySecondoObject("", this.cSecondo.getResultList());
	            
	
	            MyRelViewer myRelViewer = new MyRelViewer();
	            MyInquiryViewer myInqViewer = new MyInquiryViewer();
	            if (myRelViewer.canDisplay(secObj))
	            {
	              TableSECConvertGUI tblSecGUI = new TableSECConvertGUI(this);
	              tblSecGUI.init(new MyRelViewer().createTableFrom(secObj.toListExpr()), "");
	            }
	            else if (myInqViewer.canDisplay(secObj))
	            {
	              new SecInquiryGUI().init(myInqViewer.getHTMLCode(secObj));
	            }
	            else
	            {
	              new SecStandardGUI().init(secObj);
	            }
	            this.cSecondo.setQueryResults2Null();
	          }
	          else
	          {
	            if (this.cSecondo.getErrorCode().value != 0) {
	              new Message(this.cSecondo.getErrorMessage().toString());
	            }
	            this.cSecondo.setQueryResults2Null();
	          }
	        }
	      }
	    }
	    this.cSecondo.sendCommand(new StringBuffer("close database"));
	    this.cSecondo.closeConnection();
	  }
	}
	
	private String getRightSelectedDatabase(StringBuffer sbMeldung)
	{
	  if (this.jtreeRechts.isSelectionEmpty())
	  {
	    new Message(sbMeldung.toString());
	    return null;
	  }
	  TreePath selPath = this.jtreeRechts.getSelectionPath();
	  
	  String[] selPathArray = selPath.toString().split(",");
	  if (selPathArray.length >= 2)
	  {
	    selPathArray[1] = (selPathArray[1].indexOf("]") == -1 ? selPathArray[1] : selPathArray[1].substring(0, selPathArray[1].length() - 1));
	    
	    selPathArray[1] = ERMGUI.strReplace(selPathArray[1]);
	    
	
	    return selPathArray[1];
	  }
	  new Message(sbMeldung.toString());
	  return null;
	}
	
	private String getRightSelectedTable(StringBuffer sbMeldung)
	{
	  if (this.jtreeRechts.isSelectionEmpty())
	  {
	    new Message(sbMeldung.toString());
	    return null;
	  }
	  TreePath selPath = this.jtreeRechts.getSelectionPath();
	  
	
	  String[] selPathArray = selPath.toString().split(",");
	  if (selPathArray.length >= 3)
	  {
	    selPathArray[2] = (selPathArray[2].indexOf("]") == -1 ? selPathArray[2] : selPathArray[2].substring(0, selPathArray[2].length() - 1));
	    selPathArray[2] = ERMGUI.strReplace(selPathArray[2]);
	    selPathArray[2] = selPathArray[2].substring(0, selPathArray[2].indexOf(" "));
	    
	    return selPathArray[2];
	  }
	  new Message(sbMeldung.toString());
	  return null;
	}
	
	private String getLeftSelectedDatabase(StringBuffer sbMeldung)
	{
	  if (this.jtreeLinks.isSelectionEmpty())
	  {
	    new Message(sbMeldung.toString());
	    return null;
	  }
	  TreePath selPath = this.jtreeLinks.getSelectionPath();
	  
	  String[] selPathArray = selPath.toString().split(",");
	  if (selPathArray.length >= 2)
	  {
	    selPathArray[1] = (selPathArray[1].indexOf("]") == -1 ? selPathArray[1] : selPathArray[1].substring(0, selPathArray[1].length() - 1));
	    
	    selPathArray[1] = ERMGUI.strReplace(selPathArray[1]);
	    
	    return selPathArray[1];
	  }
	  new Message(sbMeldung.toString());
	  return null;
	}
	
	private String getLeftSelectedTable(StringBuffer sbMeldung)
	{
	  if (this.jtreeLinks.isSelectionEmpty())
	  {
	    new Message(sbMeldung.toString());
	    return null;
	  }
	  TreePath selPath = this.jtreeLinks.getSelectionPath();
	  
	  String[] selPathArray = selPath.toString().split(",");
	  if (selPathArray.length >= 4)
	  {
	    if (selPathArray[2].indexOf("TYPES") != -1)
	    {
	      LogFileHandler.mlogger.info("Can not used at TYPES");
	      return null;
	    }
	    selPathArray[3] = (selPathArray[3].indexOf("]") == -1 ? selPathArray[3] : selPathArray[3].substring(0, selPathArray[3].length() - 1));
	    selPathArray[3] = ERMGUI.strReplace(selPathArray[3]);
	    selPathArray[3] = selPathArray[3].substring(0, selPathArray[3].indexOf(":"));
	    
	    return selPathArray[3];
	  }
	  new Message(sbMeldung.toString());
	  return null;
	}
	
	public void showRightConvert()
	{
	  LogFileHandler.mlogger.info("try to show right copy");
	  String strSelTBL = getRightSelectedTable(new StringBuffer("Please select a table."));
	  if (strSelTBL != null)
	  {
	    String strSelDB = getRightSelectedDatabase(new StringBuffer("Please select a database."));
	    DefaultListModel dlm = null;
	    dlm = new PostgresSelectionGUI(new StringBuffer(strSelDB), new StringBuffer(strSelTBL), this)
	      .convertDefaultTreeNodeModelToDefaultListModel(this.mdmDatenRechtsServer, strSelDB, strSelTBL);
	    new PostgresSelectionGUI(new StringBuffer(strSelDB), new StringBuffer(strSelTBL), this).init(dlm);
	  }
	}
	
	public void showLeftConvert()
	{
	  LogFileHandler.mlogger.info("try to show left copy");
	  String strSelTBL = getLeftSelectedTable(new StringBuffer("Please select a object."));
	  if (strSelTBL != null)
	  {
	    String strSelDB = getLeftSelectedDatabase(new StringBuffer("Please select a database."));
	    DefaultListModel dlm = null;
	    dlm = new SecondoSelectionGUI(new StringBuffer(strSelDB), new StringBuffer(strSelTBL), this)
	      .convertDefaultTreeNodeModelToDefaultListModel(this.mdmDatenLinksServer, strSelDB, strSelTBL);
	    new SecondoSelectionGUI(new StringBuffer(strSelDB), new StringBuffer(strSelTBL), this).init(dlm);
	  }
	}
	
	public void showLeftGenerateMO()
	{
	  LogFileHandler.mlogger.info("try to show generateMO gui");
	  String strSelTBL = getLeftSelectedTable(new StringBuffer("Please select a object."));
	  if (strSelTBL != null)
	  {
	    String strSelDB = getLeftSelectedDatabase(new StringBuffer("Please select a database."));
	    
	    new GenerateMOGUI(this).init(strSelTBL, strSelDB);
	  }
	}
	
	public void showLeftCopyMO2PG()
	{
	  LogFileHandler.mlogger.info("try to show copy mo 2 pg");
	  String strSelTBL = getLeftSelectedTable(new StringBuffer("Please select a object."));
	  if (strSelTBL != null)
	  {
	    String strSelDB = getLeftSelectedDatabase(new StringBuffer("Please select a database."));
	    
	    new MO2Postgres(this).init(strSelTBL, strSelDB);
	  }
	}
	
	private void sendSQLInput2Rechts()
	{
	  LogFileHandler.mlogger.info("try to send right sql input");
	  
	  LineNumberReader lnr = new LineNumberReader(new StringReader(this.jtextareaRechts.getText()));
	  
	  StringBuffer sbCommand = new StringBuffer();
	  
	  ArrayList<String> altmpCommands = new ArrayList();
	  try
	  {
	    String line;
	    while ((line = lnr.readLine()) != null)
	    {
	     // String line1 = null;
	      if (line.length() > 0)
	      {
	        if (line.charAt(line.length() - 1) == ';')
	        {
	          sbCommand.append(line.substring(0, line.length() - 1));
	          altmpCommands.add(sbCommand.toString());
	          
	          sbCommand.delete(0, sbCommand.length());
	        }
	        else
	        {
	          sbCommand.append(line);
	        }
	      }
	      else {
	        sbCommand.append(line);
	      }
	    }
	  }
	  catch (IOException e)
	  {
	    LogFileHandler.mlogger.severe(e.getMessage());
	  }
	  if (altmpCommands.size() == 0) {
	    return;
	  }
	  String[] strATextAreaRechts = new UtilFunctions().toStringArray(altmpCommands);
	  
	  String strSelDB = getRightSelectedDatabase(new StringBuffer("Before you send a statement you must select a database."));
	  if (strSelDB != null) {
	    if (this.cPostgres.connect(new StringBuffer(strSelDB))) {
	      if (this.cPostgres.makeExecuteQuerysTo2TableView(strATextAreaRechts, this.mainFrame, this)) {
	        try
	        {
	          this.cPostgres.conn.close();
	        }
	        catch (SQLException e1)
	        {
	          LogFileHandler.mlogger.severe(e1.getMessage());
	        }
	      }
	    }
	  }
	}
	
	private boolean reallyDeleteDatabase()
	{
	  new SetUI().setUIAndLanguage();
	  int n = JOptionPane.showConfirmDialog(
	    null, 
	    "Do you want to delete a database?", 
	    "Question", 
	    0);
	  if (n == 1) {
	    return false;
	  }
	  return true;
	}
	
	public void itemStateChanged(ItemEvent arg0) {}
	
	public void actionPerformed(ActionEvent arg0)
	{
	  if (arg0.getSource() == this.jmenuItemRightZeigeERM)
	  {
	    Thread th = new Thread(this.runshowERMGUI);
	    th.start();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemRightReconnect)
	  {
	    new RefreshRightSideTree(this).start();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemRightConvert)
	  {
	    showRightConvert();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftConvert)
	  {
	    showLeftConvert();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftCopyMO2PG)
	  {
	    showLeftCopyMO2PG();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftGenerateMO)
	  {
	    showLeftGenerateMO();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemRigtSupportedTypes)
	  {
	    new PostgresTypes().showSupportedTypes();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftReconnect)
	  {
	    new RefreshLeftSideTree(this).start();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftSupportedTypes)
	  {
	    new SecondoTypes().showSupportedTypes();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftSecComDatabases)
	  {
	    simpleInquiryCommand(new StringBuffer("list databases;"));
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftSecComTypeConstructors)
	  {
	    simpleInquiryCommand(new StringBuffer("list type constructors;"));
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftSecComObjects)
	  {
	    simpleSecCommandDBOpen("list objects", true);
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftSecComTypes)
	  {
	    simpleSecCommandDBOpen("list types", true);
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftSecComOperators)
	  {
	    simpleInquiryCommand(new StringBuffer("list operators;"));
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemLeftSecComAlgebras)
	  {
	    simpleInquiryCommand(new StringBuffer("list algebras;"));
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemConfiguration)
	  {
	    Configuration conf = new Configuration(0);
	    conf.insertDialogs();
	    conf.write();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemParameterSettings)
	  {
	    new ParametersGUI().initView(gsbSEC_Host.toString(), gsbSEC_Port.toString(), gsbSEC_User.toString(), 
	      gsbSEC_Pwd.toString(), gsbSEC_UseBinaryList.toString(), gsbPG_Host.toString(), 
	      gsbPG_Port.toString(), gsbPG_User.toString(), gsbPG_Pwd.toString());
	    
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemAbout)
	  {
	    new HelpGUI().init();
	    return;
	  }
	  if (arg0.getSource() == this.jmenuItemSchliessen) {
	    exitSPC();
	  }
	}
	
	MouseListener mlRechts = new MouseAdapter()
	{
	  public void mouseReleased(MouseEvent e)
	  {
	    if (e.getButton() == 3) {
	      MainGui.this.jpopupRechts.show(e.getComponent(), e.getX(), e.getY());
	    }
	  }
	};
	ActionListener alPopupRechts = new ActionListener()
	{
	  public void actionPerformed(ActionEvent e)
	  {
	    if (e.getSource() == MainGui.this.jmenuItemPopRightprERM)
	    {
	      MainGui.this.showERMGUI();
	    }
	    else if (e.getSource() == MainGui.this.jmenuItemPopRightConvert)
	    {
	      MainGui.this.showRightConvert();
	    }
	    else if (e.getSource() == MainGui.this.jmenuItemPopRightDropDatabase)
	    {
	      Runnable runDropDatabase = new Runnable()
	      {
	        public void run()
	        {
	          LogFileHandler.mlogger.info("try to delete a database at pg");
	          String strDBName = MainGui.this.getRightSelectedDatabase(new StringBuffer("Please select a database."));
	          if ((strDBName != null) && (MainGui.this.cPostgres.connect())) {
	            try
	            {
	              Statement statement = MainGui.this.cPostgres.conn.createStatement();
	              
	              statement.execute("drop database " + strDBName + ";");
	              
	              Thread th = new Thread(MainGui.this.runInitRechts);
	              th.start();
	            }
	            catch (SQLException e1)
	            {
	              new Warning(e1.toString());
	            }
	              try
	              {
	                MainGui.this.cPostgres.conn.close();
	              }
	              catch (SQLException e11)
	              {
	                e11.printStackTrace();
	              }
	            
	            finally
	            {
	              try
	              {
	                MainGui.this.cPostgres.conn.close();
	              }
	              catch (SQLException e1)
	              {
	                e1.printStackTrace();
	              }
	            }
	          }
	        }
	      };
	      if (MainGui.this.reallyDeleteDatabase())
	      {
	        Thread thdrop = new Thread(runDropDatabase);
	        thdrop.start();
	      }
	    }
	  }
	};
	MouseListener mlLinks = new MouseAdapter()
	{
	  public void mouseReleased(MouseEvent e)
	  {
	    if (e.getButton() == 3) {
	      MainGui.this.jpopupLinks.show(e.getComponent(), e.getX(), e.getY());
	    }
	  }
	};
	ActionListener alPopupLinks = new ActionListener()
	{
	  public void actionPerformed(ActionEvent e)
	  {
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftSecComDatabases)
	    {
	      MainGui.this.simpleInquiryCommand(new StringBuffer("list databases;"));
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftSecComTypeConstructors)
	    {
	      MainGui.this.simpleInquiryCommand(new StringBuffer("list type constructors;"));
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftSecComObjects)
	    {
	      MainGui.this.simpleSecCommandDBOpen("list objects", true);
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftSecComTypes)
	    {
	      MainGui.this.simpleSecCommandDBOpen("list types", true);
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftSecRemoveDB)
	    {
	      if (MainGui.this.reallyDeleteDatabase()) {
	        MainGui.this.simpleSecCommandDBOpen("delete database ", false);
	      }
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftSecComOperators)
	    {
	      MainGui.this.simpleInquiryCommand(new StringBuffer("list operators;"));
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftSecComAlgebras)
	    {
	      MainGui.this.simpleInquiryCommand(new StringBuffer("list algebras;"));
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftConvert)
	    {
	      MainGui.this.showLeftConvert();
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftGenerateMO)
	    {
	      MainGui.this.showLeftGenerateMO();
	      return;
	    }
	    if (e.getSource() == MainGui.this.jmenuItemPopLeftCopyMO2PG)
	    {
	      MainGui.this.showLeftCopyMO2PG();
	      return;
	    }
	  }
	};
}






 