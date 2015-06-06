package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.Stroke;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import javax.swing.AbstractAction;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.AbstractTableModel;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.datastax.driver.core.Host;

public class CassandraGUI {
	
	protected JFrame mainframe;
	protected JPanel cassandraPanel;
	protected JMenuBar menuBar;
	protected AbstractTableModel tableModell;
	protected Map<String, CassandraNode> cassandraNodes;
	protected CassandraGUIModel guiModel;
	protected int totalTokenRanges;
	
	protected final static Logger logger = LoggerFactory.getLogger(CassandraGUI.class);

	public final static int SIZE = 400;
	public final Point upperPoint = new Point(200, 30);
	public final Point centerPoint = new Point(upperPoint.x + SIZE/2,  upperPoint.y + SIZE/2);
	public final SimpleDateFormat sdfDate = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	public volatile boolean shutdown = false;
	
	public CassandraGUI() {
		
	}

	public CassandraGUI(CassandraGUIModel guiModel) {
		this.guiModel = guiModel;
	}

	/**
	 * Build the DSECONDO dialog, init GUI components
	 * and assemble the dialog
	 */
	public void run() {
		
		mainframe = new JFrame("Distributed SECONDO");
		
		setupMenu();
		setupCassandraPanel();
		
		tableModell = getTableModel();
		final JTable table = new JTable(tableModell);
		table.getColumnModel().getColumn(0).setMaxWidth(40);
		table.getColumnModel().getColumn(2).setMinWidth(100);
		table.getColumnModel().getColumn(2).setMaxWidth(100);
		
		table.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
	        public void valueChanged(ListSelectionEvent event) {
	        	guiModel.setObservedQueryId(
	        			Integer.parseInt((table.getValueAt(table.getSelectedRow(), 0).toString())));
	        }
	    });

		JScrollPane scrollPane = new JScrollPane(table);
		
		Dimension d = table.getPreferredSize();
		scrollPane.setPreferredSize(
		    new Dimension(d.width,table.getRowHeight()*7));
		
		mainframe.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		mainframe.setLayout(new BorderLayout());
		mainframe.add(cassandraPanel, BorderLayout.CENTER);
		mainframe.add(scrollPane, BorderLayout.SOUTH);

		mainframe.pack();
		GuiHelper.locateOnScreen(mainframe);
		mainframe.setVisible(true);
	}

	/** 
	 * Dispose the main frame
	 */
	public void dispose() {
		mainframe.dispose();
	}
	
	/**
	 * Get the table model for the schedules queries
	 * @return The table model
	 */
	private AbstractTableModel getTableModel() {
		return new AbstractTableModel() {
			
			private static final long serialVersionUID = 8593512480994197794L;
			
			@Override
			public Object getValueAt(int rowIndex, int columnIndex) {
				final CassandraQuery query = guiModel.getQueryCache().get(rowIndex);
				
				if(guiModel.getQueryCache().size() < rowIndex) {
					return "";
				}
				
				if(query == null) {
					return "";
				}
				
				if(columnIndex == 0) {
					return query.getId();
				}
				
				if(columnIndex == 1) {
					return query.getQuery();
				}
				
				if(columnIndex == 2) {
					return query.getVersion();
				}
				
				return "";
				
			}
			
			@Override
			public int getRowCount() {
				return guiModel.getQueryCache().size();
			}
			
			@Override
			public int getColumnCount() {
				return 3;
			}
			
			@Override
			public boolean isCellEditable(int rowIndex, int columnIndex) {
				return false;
			}
			
			@Override
			public String getColumnName(int column) {
		   	   if(column == 0) {
				   return "Id";
			   } else if(column == 1) {
					return "Query";
			   } 
				
			   return "Version";
			}
		};
	}

	/**
	 * Initalize the cassandra GUI panel
	 * 
	 */
	protected void setupCassandraPanel() {
		cassandraPanel = new JPanel() {
		
			private static final long serialVersionUID = -248493308846818192L;

			@Override
			protected void paintComponent(Graphics g) {
			
				super.paintComponent(g);
				
	            Graphics2D graphics2D = (Graphics2D)g;
	            
	            graphics2D.setRenderingHint(
	                    RenderingHints.KEY_ANTIALIASING, 
	                    RenderingHints.VALUE_ANTIALIAS_ON);
			
				Stroke previousStroke = graphics2D.getStroke();
				graphics2D.setStroke(new BasicStroke(2.0f));
				g.drawOval(upperPoint.x, upperPoint.y, SIZE, SIZE);
				graphics2D.setStroke(previousStroke);
				

				int processed = 0;
				for(CassandraNode node : cassandraNodes.values()) {
					node.paintComponent(g);
					processed = processed + node.getTokenRangeCount();
				}
				
				g.drawString("Observed query: " + guiModel.getObservedQueryId(), 10, 470);
				g.drawString("Processed token ranges: " + processed + " of " + totalTokenRanges, 10, 490);

			}
			
			@Override
			public String getToolTipText(MouseEvent event) {
				
				for(CassandraNode node : cassandraNodes.values()) {
					if(node.isMouseOver(event)) {
						final Map<String, CassandraSystemState> heartbeatData = guiModel.getNodeHeartbeat();
						final CassandraSystemState systemState = heartbeatData.get(node.getName());
			
						final Date heartbeat = new Date(systemState.getHeartbeat());
			
						final StringBuilder sb = new StringBuilder();
						sb.append("<html>Heart beat: " + sdfDate.format(heartbeat) + "<br>");
						
						if(systemState.getCputype() != null)
					   	   sb.append("CPU-Type: " + systemState.getCputype() + "<br>");
						
						if(systemState.getMemory() != 0) 
						   sb.append("Memory (MB): " + systemState.getMemory() + "<br>");
						
						if(systemState.getThreads() != 0)
						   sb.append("SECONDO Threads: " + systemState.getThreads() + "<br>");
						
						sb.append("State: " + node.getState());
						sb.append("</html>");
						return sb.toString();
					}
				}
				
				return "";
			}
		};
		
		cassandraPanel.setToolTipText("");
		
		cassandraPanel.setPreferredSize(new Dimension(800, 500));
		insertCassandraNodes();
	}

	/**
	 * Place the cassandra nodes on the GUI panel
	 */
	protected void insertCassandraNodes() {
		cassandraNodes = new HashMap<String, CassandraNode>();
		
		for(Host host : guiModel.getAllHosts()) {
			String ip = host.getAddress().getHostAddress();
			
			if(ip.equals("127.0.0.1")) {
				ip = "132.176.69.181";
			}
			
			cassandraNodes.put(ip, new CassandraNode(ip));
		}
		
		double angle = 2 * Math.PI / cassandraNodes.size();
		int i = 0;
		
		ArrayList<CassandraNode> nodes = new ArrayList<CassandraNode>(cassandraNodes.values().size());
		nodes.addAll(cassandraNodes.values());
		Collections.sort(nodes, new Comparator<CassandraNode>() {

			@Override
			public int compare(CassandraNode o1, CassandraNode o2) {
				return o1.getName().compareTo(o2.getName());
			}
		});

		for (CassandraNode node : nodes) {
			
			int x = (int) Math.round((SIZE/2 * Math.cos(i * angle) + centerPoint.x));
			int y = (int) Math.round((SIZE/2 * Math.sin(i * angle) + centerPoint.y));
			
			node.setPosition(new Point(x, y));
			node.setState(CassandraNodeState.MISSING);
			i++;
		}
	}

	/**
	 * Create the menu of the main window
	 */
	protected void setupMenu() {
		menuBar = new JMenuBar();
		JMenu menu = new JMenu("File");
		menuBar.add(menu);
		
		JMenuItem menuItem = new JMenuItem("Close");
		menuItem.addActionListener(new AbstractAction() {
			
			private static final long serialVersionUID = -5380326547117916348L;

			public void actionPerformed(ActionEvent e) {
				shutdown = true;
			}
			
		});
		menu.add(menuItem);
        mainframe.setJMenuBar(menuBar);
	}
	
	/**
	 * Update the gui model, e.g. update token count,
	 * heartbeat data...
	 */
	public synchronized void updateStatus() {
		long curTime = System.currentTimeMillis();
		
		totalTokenRanges = guiModel.getTotalTokenRanges();
		guiModel.updateModel();
		
		for(CassandraNode node : cassandraNodes.values()) {
			node.setState(CassandraNodeState.MISSING);
			node.setTokenRangeCount(0);
		}

		for(String ip : guiModel.getTokenCache().keySet()) {
			CassandraNode node = cassandraNodes.get(ip);
			if(node == null) {
				logger.warn("Node " + ip + " not found on GUI, ignoring");
				continue;
			}
			node.setTokenRangeCount(guiModel.getTokenCache().get(ip));
		}
		
		Map<String, CassandraSystemState> result = guiModel.getNodeHeartbeat();
		for(String ip : result.keySet()) {
			final CassandraSystemState state = result.get(ip);
			long heartbeat = state.getHeartbeat();
			CassandraNode node = cassandraNodes.get(ip);
			
			if(node == null) {
				logger.warn("Node " + ip + " not found on GUI, ignoring");
				continue;
			}
			
			if(curTime - 30 * 1000 > heartbeat) {
				node.setState(CassandraNodeState.DOWN);
			} else {
				node.setState(CassandraNodeState.UP);
			}
		}
	
	}
	
	/**
	 * Update the view. This method should be called periodically
	 */
	public void updateView() {
		updateStatus();
		mainframe.repaint();
	}
	
}