//This file is part of SECONDO.

//Copyright (C) 2006, University in Hagen, Department of Computer Science, 
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

// Written 2012 by Jan Kristof Nidzwetzki 

package progresswatcher;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;

/**
 * This abstract class provides a framework for the progress windows
 *
 */
public abstract class AbstractProgressWindow extends AbstractWindow {

	protected AbstractGraph progressHistoryGraph;
	protected ChartPanel progressChartPanel;
	protected JFreeChart chart = null;
	
	protected final JPanel southPanel 
		= new JPanel();
	
	protected final JCheckBox queryCheckbox 
		= new JCheckBox("Show Optimal progression");

	/**
	 * Do some basic initialization
	 * and build the gui
	 */
	public void init() {
		mainframe.setTitle(getTitle());
		
		final JButton showQueryProgressTable = new JButton("Table view");
		showQueryProgressTable.addActionListener(new ActionListener() {
			
			public void actionPerformed(ActionEvent e) {
				AppCtx.getInstance()
					.getQueryProgressTableWindow(getWindowType()).show();
			}
		});
		
		southPanel.add(showQueryProgressTable);
		southPanel.add(queryCheckbox);

		// Handle Window close button
		// Terminate application if the last window is closed
		final JButton closeButton = new JButton("Close");
		closeButton.addActionListener(new ActionListener() {
			
			public void actionPerformed(ActionEvent e) {	
				terminateApplicationIfLastWindowIsClosed();
				mainframe.setVisible(false);
				mainframe.dispose();
			}
		});
		
		// Handle Windows close event
		// Terminate application if the last window is closed
		mainframe.addWindowListener(new WindowAdapter() {
			
			public void windowClosing(WindowEvent arg0) {
				terminateApplicationIfLastWindowIsClosed();
			}
	
		});
		
		// Show show Time Buttom
		final JButton showEstTimeButton = new JButton("Show Time");
		showEstTimeButton.addActionListener(new ActionListener() {
			
			public void actionPerformed(ActionEvent arg0) {
				AppCtx.getInstance().getWindow(WindowType.TIME).show();
			}
		});
		
		// Show show card button
		final JButton showEstCardButton = new JButton("Show Card");
		showEstCardButton.addActionListener(new ActionListener() {
			
			public void actionPerformed(ActionEvent arg0) {
				AppCtx.getInstance().getWindow(WindowType.CARD).show();
			}
		});
		
		// Show show Progress button
		final JButton showProgressButton = new JButton("Show Progress");
		showProgressButton.addActionListener(new ActionListener() {
			
			public void actionPerformed(ActionEvent arg0) {
				AppCtx.getInstance().getWindow(WindowType.PROGRESS).show();
			}
		});
		
		// Export image button
		final JButton exportButton = new JButton("Export Image");
		exportButton.addActionListener(new ActionListener() {
			
			public void actionPerformed(ActionEvent arg0) {
				final String dirname = File.separator+"tmp";
				final JFileChooser fc = new SaveJFileChooser(new File(dirname));
				fc.showSaveDialog(mainframe);
				final File selFile = fc.getSelectedFile();	
				
				if(selFile != null) {
					final String filename = selFile.getName();
					if(filename.endsWith(".png")) {
						progressHistoryGraph.exportImage(selFile.getPath());
					} else if(filename.endsWith(".svg")) {
						progressHistoryGraph.exportChartAsSVG(selFile.getPath());
					} else {
						JOptionPane.showMessageDialog(mainframe, 
								"Filename must end with .png or .svg", 
								"Error", JOptionPane.ERROR_MESSAGE);
					} 
				}	
			}
		});

        // Place graph
        progressHistoryGraph = getGraphType();
        chart = progressHistoryGraph.getChart();
        progressChartPanel = new ChartPanel(chart);
        progressChartPanel.setPreferredSize(new java.awt.Dimension(600, 350));
        progressChartPanel.setMouseZoomable(false);
        progressChartPanel.setMouseWheelEnabled(false);
        
        
        if(! (this instanceof EstimatedProgressWindow)) {
        	southPanel.add(showProgressButton);
        }
        
        if(! (this instanceof EstimatedCardinalityWindow)) {
        	southPanel.add(showEstCardButton);
        }
        
        if(! (this instanceof EstimatedTimeWindow)) {
        	southPanel.add(showEstTimeButton);
        }
 
        southPanel.add(exportButton);
        southPanel.add(closeButton);
        
        mainframe.add(progressChartPanel, BorderLayout.NORTH);
        mainframe.add(southPanel, BorderLayout.SOUTH);
	}
	
	/**
	 * Terminate Application if the last windows is closed
	 */
	protected void terminateApplicationIfLastWindowIsClosed() {
		int visibleWindows = 0;
	
		for(WindowType type : WindowType.values()) {
			
			final AbstractWindow abstractProgressWindow 
				= AppCtx.getInstance().getWindow(type);
			
			if(abstractProgressWindow != null) {
				if(abstractProgressWindow.getMainframe().isVisible()) {
					visibleWindows++;
				}
			}
		}
			
		if(visibleWindows <= 1) {
			System.exit(0);
		} else {
			mainframe.setVisible(false);
		}
	}
	
	protected abstract AbstractGraph getGraphType();

	public AbstractGraph getGraph() {
		return progressHistoryGraph;
	}

	public JCheckBox getQueryCheckbox() {
		return queryCheckbox;
	}
}
