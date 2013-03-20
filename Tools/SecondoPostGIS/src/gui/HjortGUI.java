/**
 * 
 */
package gui;


import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Label;
import java.awt.MenuItem;
import java.awt.PopupMenu;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreePath;

import com.sun.corba.se.impl.oa.poa.ActiveObjectMap.Key;



import postgres.ConnectPostgres;
import postgres.Datenbank;
import postgres.Spalte;
import postgres.Tabelle;

import util.Configuration;
import util.Global;
import util.HTMLStrings;
import utilgui.JSplitPainInTheAss;
import utilgui.Meldung;
import utilgui.Warnung;

/**
 * @author Bill
 *
 */
public class HjortGUI implements ActionListener,ItemListener{

	JFrame mainFrame;
	JMenuBar menuBar;
	JMenu menu, submenu;
	
	JButton buttonHelp, buttonCopy, buttonMove;
	JMenuItem menuItemVerbindungPruefen;
	JMenuItem menuItemSchliessen;
	JMenuItem menuItemRightZeigeERM;
	JMenuItem menuItemRightReconnect;
	
	JMenuItem menuItemAbout;
	JMenuItem menuItemConfiguration;
	JLabel labelStatusZeile;
	
	JScrollPane jscrollpaneLinks, jscrollpaneRechts;
	JTree jtreeLinks,jtreeRechts;
	JPanel jpanelLinks;
	JPanel jpanelRechts;
	
	JLabel jlabelLinks;
	JLabel jlabelRechts;
	Font fontHeadLine;
	JTextArea jtextareaLinks;
	JScrollPane jscrollpanetextareaLinks;
	JTextArea jtextareaRechts;
	JScrollPane jscrollpanetextareaRechts;
	
	DefaultMutableTreeNode mdmtreenodeLinksServer;
	DefaultMutableTreeNode mdmtreenodeLinksDatenbank;
	DefaultMutableTreeNode mdmtreenodeLinksTabellen;
	DefaultMutableTreeNode mdmtreenodeLinksSpalten;
	
	
	DefaultMutableTreeNode mdmtreenodeRechtsServer; 
	DefaultMutableTreeNode mdmtreenodeRechtsDatenbank; 
	DefaultMutableTreeNode mdmtreenodeRechtsTabellen; 
	DefaultMutableTreeNode mdmtreenodeRechtsSpalten; 
	
	PopupMenu popupRechts;
	MenuItem menuItemprERM;
	MenuItem menuItemConvert;
	HTMLStrings htmlstrings;
	LoadingGUI loadingGui;
	
	boolean bPressed_STRG;
	
	/**
	 * 
	 */
	public HjortGUI() {
		super();
		// TODO Auto-generated constructor stub
		bPressed_STRG = false;
		loadingGui = new LoadingGUI();
		init();
		loadingGui.closeLoadindWindow();
	}
	
	public void init()
	{
		try {
			 UIManager.setLookAndFeel( UIManager.getSystemLookAndFeelClassName() );
			// UIManager.setLookAndFeel( "com.sun.java.swing.plaf.nimbus.NimbusLookAndFeel" );
			 
			} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InstantiationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (UnsupportedLookAndFeelException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		popupRechts = new PopupMenu();
		menuItemprERM = new MenuItem();
		menuItemprERM.setLabel("show ERM");
		menuItemprERM.addActionListener(alPopupRechts);
		menuItemConvert = new MenuItem();
		//menuItemConvert.setLabel("Test");
		popupRechts.add(menuItemprERM);
		popupRechts.add(menuItemConvert);
		popupRechts.addActionListener(alPopupRechts);
		
		
		fontHeadLine = new Font("Comic Sans MS", Font.BOLD, 20);
		jpanelLinks = new JPanel(new BorderLayout());
		jpanelRechts = new JPanel(new BorderLayout());
		
		jlabelLinks = new JLabel("Secondo" ,JLabel.CENTER);
		jlabelLinks.setForeground(new Color(0, 0, 255));
		jlabelLinks.setFont(fontHeadLine);
		
		jlabelRechts = new JLabel("PostGIS", JLabel.CENTER);
		jlabelRechts.setForeground(new Color(100, 25, 12));
		jlabelRechts.setFont(fontHeadLine);
		
		
		htmlstrings = new HTMLStrings();
		
		mdmtreenodeLinksServer = new DefaultMutableTreeNode(htmlstrings.generateJTreeServer(Global.gsbSEC_Host));
		
		mdmtreenodeLinksDatenbank = new DefaultMutableTreeNode("no function");
		this.mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode();
		this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode();
		
		mdmtreenodeLinksServer.add(mdmtreenodeLinksDatenbank);
		
		
		
		
		//this.mdmtreenodeRechtsServer = new DefaultMutableTreeNode(htmlstrings.generateJTreeServer(new StringBuffer("Postgres")));
		this.mdmtreenodeRechtsServer = new DefaultMutableTreeNode(htmlstrings.generateJTreeServer(Global.gsbPG_Host));
		this.mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode();
		this.mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode();
		this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode();
		
		if(initRechts() == false)
		{
			new Warnung("Can not connect to postgres/postgis database!\nPlease check connection parameters.");
			//new Warnung("Can not connect to postgres/postgis database!\nPlease check connection parameters.");
		}
		 
		 
		jtreeLinks = new JTree(mdmtreenodeLinksServer);
		
		jtreeRechts = new JTree(this.mdmtreenodeRechtsServer);
		jtreeRechts.addMouseListener(mlRechts);
		jtreeRechts.add(popupRechts);
		
		jtreeRechts.setRootVisible(true);
		
		if (jtreeRechts.getCellRenderer() instanceof DefaultTreeCellRenderer)
		{
		    final DefaultTreeCellRenderer renderer = 
		        (DefaultTreeCellRenderer)(jtreeRechts.getCellRenderer());
		    //renderer.setBackgroundNonSelectionColor(Color.YELLOW);
		    renderer.setBackgroundSelectionColor(Color.ORANGE);
		    renderer.setTextNonSelectionColor(Color.RED);
		    renderer.setTextSelectionColor(Color.BLUE);
			
		}
		
		
		jtextareaLinks = new JTextArea(3,1);
		jtextareaLinks.addKeyListener(new KeyListener() {
			
			@Override
			public void keyTyped(KeyEvent e) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void keyReleased(KeyEvent e) {
				// TODO Auto-generated method stub
				
				//System.out.println(e.getKeyCode());
				
				if(e.getKeyCode() == KeyEvent.VK_F5)
				{
					
				}
			}
			
			@Override
			public void keyPressed(KeyEvent e) {
				// TODO Auto-generated method stub
				
			}
		});
		jscrollpanetextareaLinks = new JScrollPane(jtextareaLinks);
		
		
		jtextareaRechts = new JTextArea(3,1);
		jtextareaRechts.addKeyListener(new KeyListener() {
			
			@Override
			public void keyTyped(KeyEvent e) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void keyReleased(KeyEvent e) {
				// TODO Auto-generated method stub
				
				//System.out.println(e.getKeyCode());
				
				if(e.getKeyCode() == KeyEvent.VK_F5)
				{
					sendSQLInput2Rechts();	
				}
				else if(e.getKeyCode() == KeyEvent.VK_F12)
				{
					
					jtextareaRechts.setText("");
				}
			}
			
			@Override
			public void keyPressed(KeyEvent e) {
				// TODO Auto-generated method stub
				
				if(e.isControlDown() && e.getKeyCode() == KeyEvent.VK_ENTER)
				{
					sendSQLInput2Rechts();
				}
			}
		});
		jscrollpanetextareaRechts = new JScrollPane(jtextareaRechts);
		
		
		jscrollpaneLinks = new JScrollPane(jtreeLinks);
		jscrollpaneRechts = new JScrollPane(jtreeRechts);
		
		jpanelLinks.add(jlabelLinks,BorderLayout.NORTH);
		jpanelLinks.add(jscrollpaneLinks,BorderLayout.CENTER);
		jpanelLinks.add(jscrollpanetextareaLinks,BorderLayout.SOUTH);
		
		jpanelRechts.add(jlabelRechts,BorderLayout.NORTH);
		jpanelRechts.add(jscrollpaneRechts,BorderLayout.CENTER);
		//jpanelRechts.add(jtextfieldRechts,BorderLayout.SOUTH);
		jpanelRechts.add(jscrollpanetextareaRechts ,BorderLayout.SOUTH);
		
		
		
		menuBar = new JMenuBar();
		
		menu = new JMenu("File");
		menuItemVerbindungPruefen = new JMenuItem("Check Connection");
		menuItemVerbindungPruefen.addActionListener(this);
		//menu.add(menuItemVerbindungPruefen);
		
		menuItemSchliessen = new JMenuItem("Exit");
		menuItemSchliessen.addActionListener(this);
		menu.add(menuItemSchliessen);
		
		menuBar.add(menu);
		
		menu = new JMenu("View");
		menuBar.add(menu);
		
		
		menu = new JMenu("Left");
		menuBar.add(menu);
		
		menu = new JMenu("Right");
		menuItemRightZeigeERM = new JMenuItem("Open all Databases in ERM");
		menuItemRightZeigeERM.addActionListener(this);
		menu.add(menuItemRightZeigeERM);
		
		
		menuItemRightReconnect = new JMenuItem("Reconnect Server");
		menuItemRightReconnect.addActionListener(this);
		menu.add(menuItemRightReconnect);
		
		
		menuBar.add(menu);
		
		menu = new JMenu("Help");
		
		menuItemConfiguration = new JMenuItem("Set Configuration");
		menuItemConfiguration.addActionListener(this);
		menu.add(menuItemConfiguration);
		
		menu.addSeparator();
		
		menuItemAbout = new JMenuItem("About");
		menuItemAbout.addActionListener(this);
		menu.add(menuItemAbout);
		menuBar.add(menu);
		
		JToolBar toolbar = new JToolBar();
		toolbar.add(openAction);
		
		buttonCopy = new JButton("Copy");
		//buttonCopy.setPreferredSize(new Dimension(5,40));
		buttonHelp= new JButton("Help");
		buttonMove= new JButton("Move");
		labelStatusZeile = new JLabel("Text der StatusLeiste");
		    
		
		 JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
				 jpanelLinks, jpanelRechts);
			 splitPane.setOneTouchExpandable(true);
			
		splitPane.setPreferredSize(new Dimension(500,500));
		JSplitPainInTheAss.setDividerLocation(splitPane, 0.5D);
			
			
		
		GridBagConstraints c = new GridBagConstraints();
		GridBagLayout gb_layout = new GridBagLayout();

		
		/*c.gridx=0;
		c.gridy=0;
		c.ipadx= 250;
		c.ipady= 175;
		//c.gridheight=0;
		c.fill=GridBagConstraints.NONE;
		c.insets=new Insets(5,5,5,5);
		*/
		gb_layout.setConstraints(splitPane, c);
		
		c.gridx=0;
		c.gridy=1;
		c.fill=GridBagConstraints.HORIZONTAL;
		c.gridwidth=1;
		c.insets=new Insets(5,5,5,5);
		gb_layout.setConstraints(buttonCopy, c);
		
		
		c.gridx=1;
		c.gridy=1;
		c.gridwidth=1;
		c.fill=GridBagConstraints.HORIZONTAL;
		c.insets=new Insets(5,5,5,5);
		gb_layout.setConstraints(buttonMove, c);
		
		c.gridx=2;
		c.gridy=1;
		c.gridwidth=1;
		c.fill=GridBagConstraints.HORIZONTAL;
		c.insets=new Insets(5,5,5,5);
		gb_layout.setConstraints(buttonHelp, c);
		
		
		
		c.gridx=0;  //Links
		c.gridy=2;	//unten
		c.fill=GridBagConstraints.HORIZONTAL;
		c.gridwidth = 3;
		//c.ipadx = 250;      //make this component tall
		c.insets=new Insets(5,5,5,5);
		gb_layout.setConstraints(labelStatusZeile, c);
		
		
		
		JPanel panelMain =new JPanel();
		panelMain.setLayout(gb_layout);
		
		panelMain.add(buttonCopy);
		panelMain.add(buttonMove);
		panelMain.add(buttonHelp);
		
		
		//Vielleicht mit mehreren Panels arbeiten
	
		mainFrame = new JFrame("Secondo - PostGis Converter");
		mainFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	
		
		mainFrame.setLayout(new BorderLayout());
		
		mainFrame.add( toolbar, BorderLayout.PAGE_START );
		mainFrame.add(splitPane,BorderLayout.CENTER);
		
		JPanel panelMainFrame = new JPanel(new BorderLayout());
		panelMainFrame.add(panelMain,BorderLayout.NORTH);
		panelMainFrame.add(labelStatusZeile,BorderLayout.SOUTH);
		
		mainFrame.add(panelMainFrame,BorderLayout.SOUTH);
		
		mainFrame.setLocationByPlatform(true);
		
		mainFrame.pack();
		mainFrame.setJMenuBar(menuBar);
		mainFrame.setVisible(true);
		
		//http://docs.oracle.com/javase/tutorial/uiswing/layout/gridbag.html
			
			
		
	}

	
	Action openAction = new AbstractAction() {
		  { putValue( Action.NAME, "Open" );
		    putValue( Action.DISPLAYED_MNEMONIC_INDEX_KEY, 1 );
		   // putValue( Action.SMALL_ICON,     smallIcon );
		   /* putValue( Action.LARGE_ICON_KEY, largeIcon ); */}
		  public void actionPerformed( ActionEvent e ) {
		    System.out.println( "no function" );
		  }
		};
	
	@Override
	public void itemStateChanged(ItemEvent arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void actionPerformed(ActionEvent arg0) {
		// TODO Auto-generated method stub
		if(arg0.getSource() == menuItemVerbindungPruefen)
			System.out.println(arg0.getID());
		
		if(arg0.getSource() == menuItemRightZeigeERM)
		{	alRelations.clear();
			
			cPostgres.connect();
			
			LinkedList<Datenbank> llDatenbanknamen = new LinkedList<Datenbank>();
			
			llDatenbanknamen = cPostgres.getDatabaseNames();
			
			try {
				cPostgres.conn.close();
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			

			Iterator<Datenbank> itDatenbanknamen = llDatenbanknamen.iterator();
			Datenbank db = new Datenbank();
			
			while(itDatenbanknamen.hasNext())
			{
			
				ERMGUI erm = new ERMGUI();
				
				db = itDatenbanknamen.next();
				
				if(cPostgres.connect(db.getSbName())==true)
				{
					alRelations = cPostgres.getForeignRelations();
					
					erm.init(mdmtreenodeRechtsServer,db.getSbName().toString(),alRelations);
					
					try {
						cPostgres.conn.close();
					} catch (SQLException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				
			}
			
		}
		
		if(arg0.getSource() == menuItemRightReconnect)
		{
			
			System.out.println("reconnect");
			
			/*
			 DefaultMutableTreeNode node;
			    DefaultTreeModel model = (DefaultTreeModel) (jtreeRechts.getModel());
			    jtreeRechts.setSelectionRow(1);
			    TreePath[] paths = jtreeRechts.getSelectionPaths();
			    for (int i = 0; i < paths.length; i++) {
			      node = (DefaultMutableTreeNode) (paths[i].getLastPathComponent());
			      model.removeNodeFromParent(node);
			    }
			*/
			
			DefaultTreeModel model = (DefaultTreeModel) jtreeRechts.getModel();
			
			if(jtreeRechts.getRowCount() > 1)  //sonst kommt es hier zu einem fehler
			{
				jtreeRechts.setSelectionRow(1);
			    TreePath path = jtreeRechts.getSelectionPath();
			    MutableTreeNode node = (MutableTreeNode) path.getLastPathComponent();
			    model.removeNodeFromParent(node);
			}
		    model.setRoot(null);
		    
		    this.mdmtreenodeRechtsServer = new DefaultMutableTreeNode(htmlstrings.generateJTreeServer(Global.gsbPG_Host));
			mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode();
			mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode();
			mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode();
		    
		   
			if(initRechts() == false)
			{
				new Warnung("Can not connect to postgres/postgis database!\nPlease check connection parameters.");
			}
			
			model.setRoot(mdmtreenodeRechtsServer);
			
			
			
		}
			
		if(arg0.getSource() == menuItemConfiguration)
		{
			Configuration conf = new Configuration(0);
			
			conf.insertDialogs();
			conf.write();
			
		}
		
		if(arg0.getSource() == menuItemAbout)
		{
			new HelpGUI().init();
		}
		
		
		if(arg0.getSource() == menuItemSchliessen)
		{
			mainFrame.setVisible(false);
			System.exit(0);
			//alles beenden würde ich sagen
		}
			
		//System.out.println(menuItem);
	}
	
	
	ArrayList<String> alRelations = new ArrayList<String>();
	ConnectPostgres cPostgres;
	
	
	
	private boolean initRechts()
	{
		
		
		cPostgres = new ConnectPostgres(Global.gsbPG_Host, Integer.valueOf(Global.gsbPG_Port.toString()),
				Global.gsbPG_User, Global.gsbPG_Pwd);
		
		if(cPostgres.connect()==false)
			return false;
		
		HTMLStrings htmlstrings = new HTMLStrings();
		
		/*DefaultMutableTreeNode mdmtreenodeRechtsServer = new DefaultMutableTreeNode(htmlstrings.generateJTreeServer(new StringBuffer("Postgres")));
		DefaultMutableTreeNode mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode();
		DefaultMutableTreeNode mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode();
		DefaultMutableTreeNode mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode();
		*/
		
		DefaultMutableTreeNode mdmDatenRechtsServer = new DefaultMutableTreeNode("Postgres");
		DefaultMutableTreeNode mdmDatenRechtsDatenbank = new DefaultMutableTreeNode();
		DefaultMutableTreeNode mdmDatenRechtsTabellen = new DefaultMutableTreeNode();
		DefaultMutableTreeNode mdmDatenRechtsSpalten = new DefaultMutableTreeNode();
		
		
		Tabelle tabelle = new Tabelle();
		Datenbank db = new Datenbank();
		
		LinkedList<Datenbank> llDatenbanknamen = new LinkedList<Datenbank>();
		LinkedList<Tabelle> llTabellennamen = new LinkedList<Tabelle>();
		LinkedList<Spalte> llSpaltennamen = new LinkedList<Spalte>();
		
		//Datenbank-namen holen
		llDatenbanknamen = cPostgres.getDatabaseNames();	
		
		Iterator<Datenbank> itDatenbanknamen = llDatenbanknamen.iterator();
		
		
		while(itDatenbanknamen.hasNext())
		{
			
			db = itDatenbanknamen.next();
			
			this.mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode(htmlstrings.generateJTreeDatenbank(db.getSbName()));
			mdmDatenRechtsDatenbank = new DefaultMutableTreeNode(db.getSbName().toString());
			
			try {
				cPostgres.conn.close();
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			if(cPostgres.connect(db.getSbName())==false)
				System.exit(1);
		
			llTabellennamen.clear();
			llTabellennamen = cPostgres.getTableNamesFromDB();
			Iterator<Tabelle> itTabellennamen = llTabellennamen.iterator();
			
			
				while(itTabellennamen.hasNext())
				{
					tabelle = new Tabelle();
					
					tabelle = itTabellennamen.next();
					
					tabelle.setiRows(cPostgres.generateRowsFromTable(tabelle.getSbName().toString()));
					
					
					this.mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode(htmlstrings.generateJTreeTabelle(
												tabelle.getShowText()));
					
					mdmDatenRechtsTabellen = new DefaultMutableTreeNode(tabelle.getSbName().toString());
					
					
					llSpaltennamen.clear();
					llSpaltennamen = cPostgres.getColumnsNamesFromTable(tabelle.getSbName().toString());
					
					//PrimaryKeys bekommen:
					HashSet<String> hsPrimaryKeys = new HashSet<String>();
					hsPrimaryKeys = cPostgres.getPrimaryKeyFromTable(tabelle.getSbName().toString());
					
					
					//Foreign Key bekommen:
					HashSet<String> hsForeignKeys = new HashSet<String>();
					hsForeignKeys = cPostgres.getForeignKeyFromTable(tabelle.getSbName().toString());
					
					
					
					for(int i=0; i < llSpaltennamen.size(); i++)
					{
						
						if(hsPrimaryKeys.contains(llSpaltennamen.get(i).getSbName().toString())== true)
						{
							this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpaltePrimaryKey(
									llSpaltennamen.get(i).getSbName(),llSpaltennamen.get(i).getSbTyp()));
		
						}
						else if(hsForeignKeys.contains(llSpaltennamen.get(i).getSbName().toString())==true)
						{
							this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpalteForeignKey(
									llSpaltennamen.get(i).getSbName(),llSpaltennamen.get(i).getSbTyp()));
						}
						else
						{
							this.mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpalte(
									llSpaltennamen.get(i).getSbName(),llSpaltennamen.get(i).getSbTyp()));
		
						}
							
						mdmDatenRechtsSpalten = new DefaultMutableTreeNode(llSpaltennamen.get(i).getSbName().toString() +":" + llSpaltennamen.get(i).getSbTyp().toString());
						
						this.mdmtreenodeRechtsTabellen.add(this.mdmtreenodeRechtsSpalten);
						mdmDatenRechtsTabellen.add(mdmDatenRechtsSpalten);
						
					}
					
					this.mdmtreenodeRechtsDatenbank.add(this.mdmtreenodeRechtsTabellen);
					mdmDatenRechtsDatenbank.add(mdmDatenRechtsTabellen);
				}
			
			db = new Datenbank();
				
			//jetzt muss das abgespeichert werden
				
			//mdmtreenodeRechtsDatenbank.add(mdmtreenodeRechtsTabellen);
			
			this.mdmtreenodeRechtsServer.add(this.mdmtreenodeRechtsDatenbank);
			mdmDatenRechtsServer.add(mdmDatenRechtsDatenbank);
			
		}
		
		//alRelations = cPostgres.getForeignRelations();
		
		///anzahl der row per tabelle
		//select count(*) from wetter;
		
		
		try {
			cPostgres.conn.close();
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		//if != 0 dann kanns weiter gehen
		
		
		
		
		/*if (jtree.getCellRenderer() instanceof DefaultTreeCellRenderer)
		{
		   final DefaultTreeCellRenderer renderer = 
		        (DefaultTreeCellRenderer)(jtree.getCellRenderer());
		    renderer.setBackgroundNonSelectionColor(Color.YELLOW);
		    renderer.setBackgroundSelectionColor(Color.ORANGE);
		    renderer.setTextNonSelectionColor(Color.RED);
		    renderer.setTextSelectionColor(Color.BLUE);
			
		}
		else
		{
		    System.err.println("Sorry, no special colors today.");
		}
		*/
		
		return true;
		
	}
	
	
	TreePath selPath ;
	MouseListener mlRechts = new MouseAdapter() {
		 public void mouseReleased(MouseEvent e)
		 {
			 if(e.getButton() == MouseEvent.BUTTON3)  //rechts klick
			 {
				
				 int selRow = jtreeRechts.getRowForLocation(e.getX(), e.getY());
		         
		         selPath = jtreeRechts.getPathForLocation(e.getX(), e.getY());
		         
		        //System.out.println("Treepath:" + selPath.toString());
		         
		         jtreeRechts.setSelectionPath(selPath);
		         
		         popupRechts.show(e.getComponent(), e.getX(), e.getY());
		         
		         /*if(selRow != -1) {
		             if(e.getClickCount() == 1) {
		                // System.out.println("Single");
		             }
		             else if(e.getClickCount() == 2) {
		                 System.out.println("doppel klick");//myDoubleClick(selRow, selPath);
		             }
		         }*/
				 
				 
				// popup.show(e.getComponent(), e.getX(), e.getY());
				 
			 }
		 }
	
	};
	
	ActionListener alPopupRechts = new ActionListener()
	{

		@Override
		public void actionPerformed(ActionEvent e) {
			// TODO Auto-generated method stub
			
			if(e.getSource() == menuItemprERM)
			{
				//System.out.println("Open ERM");
		        //System.out.println(selPath.toString()); 
				String strSelPath[] = selPath.toString().split(",");
		         
				if(strSelPath.length >=2)
				{
					ERMGUI erm = new ERMGUI();
					
					strSelPath[1] = erm.strReplace(strSelPath[1]);
					
					alRelations.clear();
					
					cPostgres.connect(new StringBuffer(strSelPath[1]));
					
					alRelations = cPostgres.getForeignRelations();
					
					try {
						cPostgres.conn.close();
					} catch (SQLException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					erm.init(mdmtreenodeRechtsServer,strSelPath[1],alRelations);
				
					
				}
				
				
			}
		}
		
	};
	
	
	private void sendSQLInput2Rechts()
	{
		String strTextAreaRechts = jtextareaRechts.getText();
		String strATextAreaRechts[] = strTextAreaRechts.split(";");
		
		if(strTextAreaRechts.length()==0)
			return;
		
		
		for (int i =0 ; i < strATextAreaRechts.length; i++)
		{
			strATextAreaRechts[i] = strATextAreaRechts[i].replace("\r", "");
			strATextAreaRechts[i] = strATextAreaRechts[i].replace("\n", "");
		}
			
		
		
		//Text-Area ggf. wieder leeren
		
		//Selektierte Datenbank auswählen:
		selPath = jtreeRechts.getSelectionPath();
		
		if(selPath == null)
		{
			new Meldung("Before you send a statement you must select a database.");
			return;
		}
			
		jtextareaRechts.setText("");
		
		
		String strSelPath[] = selPath.toString().split(",");
         
		if(strSelPath.length >=2)
		{
			strSelPath[1] = htmlstrings.strHTMLReplace(strSelPath[1]);
			
			if(cPostgres.connect(new StringBuffer(strSelPath[1]))==true)
			{
				if(cPostgres.makeExecuteQuerysTo2TableView(strATextAreaRechts,mainFrame))
				/*	this.
				else
					System.out.println(false);
				*/
				try {
					cPostgres.conn.close();
				} catch (SQLException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		}
		
	}
	
	
}
