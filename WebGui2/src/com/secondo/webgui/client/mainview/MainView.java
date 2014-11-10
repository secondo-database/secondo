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

package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import java.util.Map.Entry;

import java_cup.non_terminal;

import com.google.gwt.event.dom.client.ChangeEvent;
import com.google.gwt.event.dom.client.ChangeHandler;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.logical.shared.ResizeEvent;
import com.google.gwt.event.logical.shared.ResizeHandler;
import com.google.gwt.event.logical.shared.ValueChangeEvent;
import com.google.gwt.event.logical.shared.ValueChangeHandler;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.AbsolutePanel;
import com.google.gwt.user.client.ui.CheckBox;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.RadioButton;
import com.secondo.webgui.shared.model.DataType;
import com.secondo.webgui.shared.model.Point;
import com.secondo.webgui.shared.model.Polygon;
import com.secondo.webgui.shared.model.Polyline;

/**
 * This class represents the main view of the application and includes all
 * elements that are displayed in the main view.
 * 
 * @author Kristina Steiger
 */
public class MainView extends Composite {

	/**The main panel of the main view*/
	private HorizontalPanel mainPanel = new HorizontalPanel();
	private AbsolutePanel contentPanel = new AbsolutePanel();

	// main elements of the application
	private Header header = new Header();
	private StatusBar statusBar = new StatusBar();
	private SideBar sidebar = new SideBar();
	private FlowPanel commandPanelWrapper = new FlowPanel();
	private CommandPanel commandPanel = new CommandPanel();
	private OptionsTabPanel optionsTabPanel = new OptionsTabPanel();

	// different views that can be displayed in the viewpanel
	private HorizontalPanel view = new HorizontalPanel();
	private RawDataView rawDataView = new RawDataView();
	private TextPanel textView = new TextPanel();
	private GraphicalView graphicalView = new GraphicalView();
	private MapView mapView = new MapView();
	private ToolBox toolbox = new ToolBox();
	

	// boolean values to show if panels are visible or not
	private boolean cpTurnedOn = true;
	private boolean textTurnedOn = false;
	private boolean mapTurnedOn = false;

	// Commands for optimizer
	private Command optimizerOn;
	private Command optimizerOff;
	
	 /**
     * default value=0, means "doesn't show symtraj"
     */
    private int modeForSymTraj=0;

	public MainView() {

		contentPanel.add(view);		
		contentPanel.add(optionsTabPanel.getOptionsTabPanel(), 10,0);
		contentPanel.add(commandPanelWrapper);
		contentPanel.add(statusBar.gethPanel());

		mainPanel.add(sidebar.getSidebar());
		mainPanel.add(contentPanel);

		//initialize the main view with the graphical view
		showGraphicalView();

		commandPanelWrapper.add(commandPanel.getCommandPanel());

		// get the size of the browserwindow and set the elements to the right size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		this.resizeWithCP(windowWidth, windowHeight);

		// resize the application elements if the size of the window changes
		Window.addResizeHandler(new ResizeHandler() {
			public void onResize(ResizeEvent event) {
				int windowWidth = event.getWidth();
				int windowHeight = event.getHeight();

				if (cpTurnedOn && textTurnedOn) {
					resizeWithTextAndCP(windowWidth, windowHeight);
				}
				if (cpTurnedOn && !textTurnedOn) {
					resizeWithCP(windowWidth, windowHeight);
				}

				if (!cpTurnedOn && textTurnedOn) {
					resizeWithTextPanel(windowWidth, windowHeight);
				}
				if (!cpTurnedOn && !textTurnedOn) {
					resizeToFullScreen(windowWidth, windowHeight);
				}
			}
		});

/* ******************************EventHandler for the Sidebar*************************************************/

		/* Adds an event handler to the terminal button to show the terminal */
		this.sidebar.getShowTerminalButton().addClickHandler(
				new ClickHandler() {
					public void onClick(ClickEvent event) {
						showCommandPanel();
					}
				});

		/* Adds an event handler to the terminal button to hide the terminal */
		this.sidebar.getHideTerminalButton().addClickHandler(
				new ClickHandler() {
					public void onClick(ClickEvent event) {
						hideCommandPanel();
					}
				});

		/* Adds an event handler to the show raw data button to show the raw data */
		this.sidebar.getShowRawdataButton().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				showRawDataView();
			}
		});

		/*Adds an event handler to the hide raw data button to hide the raw data*/
		this.sidebar.getHideRawdataButton().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				hideRawDataView();
			}
		});

		/*Adds an event handler to the show text button to show the text in the view */
		this.sidebar.getShowTextButton().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				showTextView();
			}
		});

		/* Adds an event handler to the show text button to show the text in the view */
		this.sidebar.getHideTextButton().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				hideTextView();
			}
		});

		/* Adds an event handler to the map button to show the map view */
		this.sidebar.getShowMapButton().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				showMapView();
			}
		});

		/* Adds an event handler to the geometry button to show the graphical view */
		this.sidebar.getHideMapButton().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				showGraphicalView(); // resizing
				getGraphicalView().initDataTypes(); // drawing
			}
		});

/* ****************************** EventHandler for the ToolBox ************************************************* */

		/**Adds an event handler on the resetGraphicButton of the toolbar to clear the graphical view */
		this.toolbox.getResetGraphicLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				graphicalView.resetData();
				graphicalView.getMpointController().stopAllAnimations();
				resetMapView();
				toolbox.resetData();
			}
			
		});

		/** Adds an event handler on the resetTextButton of the toolbar to clear the textview*/
		this.toolbox.getResetTextLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				getTextView().resetData();
				getRawDataView().resetData();
			}
		});

		/**Adds an event handler on the playButton of the toolbar to animate the moving point */
		this.toolbox.getPlayLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				if (isMapTurnedOn()) {
					mapView.getMpointController().animateMovingPoints(toolbox, mapView.getMap());
				} else {
					getGraphicalView().getMpointController().animateMovingPoints(toolbox);
				}
			}
		});
		
		this.optionsTabPanel.getSelectOptionsForDisplayMode().addClickHandler(new ClickHandler(){

			@Override
			public void onClick(ClickEvent event) {
				modeForSymTraj=optionsTabPanel.getSelectOptionsForDisplayMode().getSelectedIndex();
				getMapView().setModeForSymTraj(modeForSymTraj);
			}
			
		});
		
		/**Adds an event handler on the playButton of the optionsTabPanel to animate the moving point */
		this.optionsTabPanel.getPlayLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				
		    	if(modeForSymTraj!=0){
		    		
		    		getMapView().getMpointController().animateMovingPoints(optionsTabPanel, mapView.getMap(),modeForSymTraj);		
		    	}
		    	else{
		    		Window.alert("Please select display mode");
		    	}
			}
		});
		
		

		/**Adds an event handler on the forwardButton of the optionsTabPanel to animate the moving point */
		this.optionsTabPanel.getForwardLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

					getMapView().getMpointController().speedUpMovingPoint();
				
			}
		});
		
		/**Adds an event handler on the forwardButton of the toolbar to animate the moving point */
		this.toolbox.getForwardLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				if (isMapTurnedOn()) {
					getMapView().getMpointController().speedUpMovingPoint();
				} else {
					getGraphicalView().getMpointController()
							.speedUpMovingPoint();
				}
			}
		});

		/** Adds an event handler on the rewindButton of the toolbar to animate the moving point */
		this.toolbox.getRewindLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				if (isMapTurnedOn()) {
					getMapView().getMpointController()
							.reduceSpeedOfMovingPoint();
				} else {
					getGraphicalView().getMpointController()
							.reduceSpeedOfMovingPoint();
				}
			}
		});

		/** Adds an event handler on the rewindButton of the optionsTabPanel to animate the moving point */
		this.optionsTabPanel.getRewindLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				
					getMapView().getMpointController()
							.reduceSpeedOfMovingPoint();
				
			}
		});

		
		/** Adds an event handler on the pauseButton of the toolbar to pause the animation of the the moving point */
		this.toolbox.getPauseLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				toolbox.getAnimationPanel().remove(0);
				toolbox.getAnimationPanel().insert(toolbox.getPlaypanel(), 0);

				if (isMapTurnedOn()) {
					getMapView().getMpointController().pauseMovingPoint();
				} else {
					getGraphicalView().getMpointController().pauseMovingPoint();
				}
			}
		});
		
		/** Adds an event handler on the pauseButton of the optionsTabPanel to pause the animation of the the moving point */
		this.optionsTabPanel.getPauseLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				optionsTabPanel.getAnimationPanel().remove(0);
				optionsTabPanel.getAnimationPanel().insert(optionsTabPanel.getPanelForPlay(), 0);

				
					getMapView().getMpointController().pauseMovingPoint();
				
			}
		});

		/** Adds an event handler to the checkbox to show markers on the map */
		this.toolbox.getCheckBoxPoints().addValueChangeHandler(
				new ValueChangeHandler<Boolean>() {
					@Override
					public void onValueChange(ValueChangeEvent<Boolean> event) {

						// checkbox is checked
						if (toolbox.getCheckBoxPoints().getValue() == true) { 
							mapView.getPointController().showPoints();
							graphicalView.getPointController().showPointArray();
						} else {
							mapView.getPointController().hidePoints();
							graphicalView.getPointController().removeCircles();
						}
					}
				});

		/** Adds an event handler to the checkbox to show lines on the map */
		this.toolbox.getCheckBoxLines().addValueChangeHandler(
				new ValueChangeHandler<Boolean>() {
					@Override
					public void onValueChange(ValueChangeEvent<Boolean> event) {

						// checkbox is checked
						if (toolbox.getCheckBoxLines().getValue() == true) { 
							mapView.getPolylineController().showPolylines();
							//graphicalView.getPolylineController().drawAllPolylines();
							graphicalView.updateView();
						} else {
							mapView.getPolylineController().hidePolylines();
							graphicalView.getPolylineController().removeLines();
						}
					}
				});

		/** Adds an event handler to the checkbox to show polygonson the map */
		this.toolbox.getCheckBoxPolygons().addValueChangeHandler(
				new ValueChangeHandler<Boolean>() {
					@Override
					public void onValueChange(ValueChangeEvent<Boolean> event) {

						if (toolbox.getCheckBoxPolygons().getValue() == true) {
							mapView.getPolygonController().showPolygons();
							graphicalView.getPolygonController()
									.showPolygonArray();
						} else {
							mapView.getPolygonController().hidePolygons();
							graphicalView.getPolygonController()
									.removePolygons();
						}
					}
				});

		/** Adds an event handler to the save button of the colorchooser to give the selected elements a new color */
		this.toolbox.getColorChooserDialog().getSaveButton().addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {

						String color = "";
						int colorIndex = 0;
						for (RadioButton colorButton : toolbox.getColorChooserDialog().getButtonList()) {
							if (colorButton.getValue() == true) {
								colorIndex = toolbox.getColorChooserDialog().getButtonList().indexOf(colorButton);
								color = getColorForIndex(colorIndex);
							}
						}
						int queryIndex = toolbox.getColorChooserDialog().getQueryBox().getSelectedIndex();
						// change all datatypes in selected query to the chosen color
						for (DataType datatype : toolbox.getResultList().get(queryIndex)) {

							System.out.println("####change color of datatype: " + color + datatype.getId());
							//elements in map view
							if (mapTurnedOn) {

								if (datatype.getType().equals("Point")) {
									datatype.setColor(color);
									mapView.getPointController().changePointColor(datatype.getId(), color);
								}
								if (datatype.getType().equals("Polygon")) {
									datatype.setColor(color);
									mapView.getPolygonController().changePolygonColor(datatype.getId(), color);
								}
								if (datatype.getType().equals("Polyline")) {
									datatype.setColor(color);
									mapView.getPolylineController().changePolylineColor(datatype.getId(), color);
								}
								if (datatype.getType().equals("MPoint")) {
									datatype.setColor(color);
									mapView.getMpointController().changeMPointColor(datatype.getId(), color);
								}
							// elements in graphical view
							} else { 
								if (datatype.getType().equals("Point")) {
									datatype.setColor(color);
									graphicalView.getPointController().removePoint(datatype.getId());
									graphicalView.getPointController().showPointObject((Point)datatype);
								}
								if (datatype.getType().equals("Polygon")) {
									datatype.setColor(color);
									graphicalView.getPolygonController().changePolygonColor(datatype.getId(), color); 
								}
								if (datatype.getType().equals("Polyline")) {
									datatype.setColor(color);
									graphicalView.getPolylineController().removePolyline(datatype.getId());
									graphicalView.getPolylineController().showPolylineObject((Polyline)datatype);
								}
								if (datatype.getType().equals("MPoint")) {
									datatype.setColor(color);
									//graphicalView.getMpointController().changeMPointColor(datatype.getId(), color);
									graphicalView.getMpointController().stopAllAnimations();
									graphicalView.getMpointController().drawFirstMovingPoint();
								}
							}
						}
						mapView.updateView();
						//polygon in graphical view has to be handled differently because all polygons have to be drawn in one function in d3
						graphicalView.getPolygonController().redrawAllPolygons();
						toolbox.getColorChooserDialog().getDialogBox().hide();
					}
				});
		
		
		
		/** Adds an event handler to the save button of the zoom level dialog to change the zoom level */
		this.toolbox.getZoomLevelDialog().getSaveButton().addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {

						if (toolbox.getZoomLevelDialog().getZoomAll().getValue() == true) {

							mapView.setZoomToAll(true);
						}
						else{
							mapView.setZoomToAll(false);
						}
						
						mapView.updateView();
						toolbox.getZoomLevelDialog().getDialogBox().hide();
					}
				});

/* ********************************* EventHandler for the Menubar of the CommandPanel ************************************* */

		/** Adds a ClickHandler to the selection box of the commandpanel to show the chosen query */
		this.commandPanel.getMenubarCP().getCommandHistoryBox()
				.addChangeHandler(new ChangeHandler() {
					public void onChange(ChangeEvent event) {

						int selectedCommandIndex = commandPanel.getMenubarCP()
								.getCommandHistoryBox().getSelectedIndex();

						if (commandPanel.getMenubarCP().getCommandHistoryBox()
								.getItemText(selectedCommandIndex)
								.equals("Command History...")) {
							// do nothing
						} else {
							commandPanel.getTextArea().setText(
									commandPanel.getMenubarCP()
											.getCommandHistoryBox()
											.getItemText(selectedCommandIndex));
						}
					}
				});
		
		/**Adds an event handler on the zoomallButton of the commandpanel menubar to zoom to all queries */
		this.commandPanel.getMenubarCP().getZoomAllButton()
				.addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {
						mapView.setZoomToAll(true);
						mapView.updateView();
						commandPanel.getMenubarCP().getZoomPanel().clear();
						commandPanel.getMenubarCP().getZoomPanel().add(commandPanel.getMenubarCP().getZoomLastButton());
					}
				});
		
		/**Adds an event handler on the zoomlastButton of the commandpanel menubar to zoom to the last query */
		this.commandPanel.getMenubarCP().getZoomLastButton()
				.addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {
						mapView.setZoomToAll(false);
						mapView.updateView();
						commandPanel.getMenubarCP().getZoomPanel().clear();
						commandPanel.getMenubarCP().getZoomPanel().add(commandPanel.getMenubarCP().getZoomAllButton());
					}
				});

		/**Adds an event handler on the hideTerminalButton of the commandpanel menubar to hide the commandpanel */
		this.commandPanel.getMenubarCP().getHideTerminalButton()
				.addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {
						hideCommandPanel();
					}
				});

		/** Creates a command to execute when optimizer on is selected in themenubar of the commandpanel */
		this.optimizerOn = new Command() {

			public void execute() {

				commandPanel.getMenubarCP().setOptimizerTurnedOn(true);
				commandPanel.getMenubarCP().getOptimizerMenu()
						.removeItem(commandPanel.getMenubarCP().getOptimizerItemOn());
				commandPanel.getMenubarCP().getOptimizerMenu()
						.insertItem(commandPanel.getMenubarCP().getOptimizerItemOff(), 1);
				statusBar.getLabelBox().remove(statusBar.getOffIcon());
				statusBar.getLabelBox().insert(statusBar.getOnIcon(), 9);
			}
		};
		this.commandPanel.getMenubarCP().getOptimizerItemOn().setScheduledCommand(optimizerOn);

		/** Creates a command to execute when optimizer off is selected in the menubar of the commandpanel*/
		this.optimizerOff = new Command() {

			public void execute() {

				commandPanel.getMenubarCP().setOptimizerTurnedOn(false);
				commandPanel.getMenubarCP().getOptimizerMenu()
						.removeItem(commandPanel.getMenubarCP().getOptimizerItemOff());
				commandPanel.getMenubarCP().getOptimizerMenu()
						.insertItem(commandPanel.getMenubarCP().getOptimizerItemOn(), 1);
				statusBar.getLabelBox().remove(statusBar.getOnIcon());
				statusBar.getLabelBox().insert(statusBar.getOffIcon(), 9);
			}
		};
		this.commandPanel.getMenubarCP().getOptimizerItemOff().setScheduledCommand(optimizerOff);
		
		this.header.getPlainTraj().setScheduledCommand(new Command() {
			
			@Override
			public void execute() {				
				header.getTextViewOfTrajInDialog().getPlainTrajDialogBox().center();
				header.getTextViewOfTrajInDialog().getPlainTrajDialogBox().show();
				
			}
		});
	}

	/** On resizing of the browser window the elements of the main view are readjusted with the commandpanel displayed
	 * 
	 * @param width The new width of the visible elements
	 * @param height The new height of the visible elements
	 * */
	public void resizeWithCP(int width, int height) {

		header.resizeWidth(width);
		rawDataView.resizeWithCP(width, height);

		if (textTurnedOn) {
			textView.resizeWithCP(height);
		}
		if (mapTurnedOn) {
			mapView.resizeWithCP(width, height);
			mapView.updateView();
		}
		else {
			graphicalView.resizeWithCP(width, height);
			graphicalView.updateView();
		}

		toolbox.resizeHeightWithCP(height);
		sidebar.resizeHeight(height);
		commandPanel.resizeWidth(width);
		commandPanel.getMenubarCP().resizeWidth(width);
		statusBar.resizeWidth(width);
	}

	/**On resizing of the browser window the elements of the main view are readjusted with the textpanel displayed
	 * 
	 * @param width The new width of all visible elements
	 * @param height The new height of all visible elements
	 * */
	public void resizeWithTextPanel(int width, int height) {

		header.resizeWidth(width);
		rawDataView.resizeToFullScreen(width, height);

		if (textTurnedOn) {
			textView.resizeToFullScreen(height);
		}
		if (mapTurnedOn) {
			mapView.resizeWithTextPanel(width, height);
			mapView.updateView();
		}
		else {
			graphicalView.resizeWithTextPanel(width, height);
			graphicalView.updateView();
		}
		toolbox.resizeHeightToFullScreen(height);
		sidebar.resizeHeight(height);
		commandPanel.resizeWidth(width);
		commandPanel.getMenubarCP().resizeWidth(width);
		statusBar.resizeWidth(width);
	}

	/**On resizing of the browser window the elements of the main view are readjusted with the textpanel and commandpanel displayed
	 * 
	 * @param width The new width of all visible elements
	 * @param height The new height of all visible elements
	 * */
	public void resizeWithTextAndCP(int width, int height) {

		header.resizeWidth(width);
		rawDataView.resizeWithCP(width, height);

		if (textTurnedOn) {
			textView.resizeWithCP(height);
		}
		if (mapTurnedOn) {
			mapView.resizeWithTextAndCP(width, height);
			mapView.updateView();
		} else {
			graphicalView.resizeWithTextAndCP(width, height);
			graphicalView.updateView();
		}
		toolbox.resizeHeightWithCP(height);
		sidebar.resizeHeight(height);
		commandPanel.resizeWidth(width);
		commandPanel.getMenubarCP().resizeWidth(width);
		statusBar.resizeWidth(width);
	}

	/**On resizing of the browser window the elements of the main view are readjusted with to fullscreen
	 * 
	 * @param width The new width of all visible elements
	 * @param height The new height of all visible elements
	 * */
	public void resizeToFullScreen(int width, int height) {

		header.resizeWidth(width);
		rawDataView.resizeToFullScreen(width, height);

		if (textTurnedOn) {
			textView.resizeToFullScreen(height);
		}
		if (mapTurnedOn) {
			mapView.resizeToFullScreen(width, height);
			mapView.updateView();
		} else {
			graphicalView.resizeToFullScreen(width, height);
			graphicalView.updateView();
		}
		toolbox.resizeHeightToFullScreen(height);
		sidebar.resizeHeight(height);
		commandPanel.resizeWidth(width);
		commandPanel.getMenubarCP().resizeWidth(width);
		statusBar.resizeWidth(width);
	}

	/**Shows the commandpanel and resizes all visible elements*/
	public void showCommandPanel() {

		cpTurnedOn = true;
		commandPanelWrapper.add(commandPanel.getCommandPanel());

		sidebar.getSidebar().remove(sidebar.getShowTerminalButton());
		sidebar.getSidebar().insert(sidebar.getHideTerminalButton(), 0);
		sidebar.getSidebar().remove(sidebar.getHideRawdataButton());
		sidebar.getSidebar().insert(sidebar.getShowRawdataButton(), 3); 

		// get the size of the browserwindow 
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		if (textTurnedOn) {
			this.resizeWithTextAndCP(windowWidth, windowHeight);

		} else {
			this.resizeWithCP(windowWidth, windowHeight);
		}
	}

	/**Hides the commandpanel and resizes all visible elements*/
	public void hideCommandPanel() {

		cpTurnedOn = false;
		commandPanelWrapper.clear();

		sidebar.getSidebar().remove(sidebar.getHideTerminalButton());
		sidebar.getSidebar().insert(sidebar.getShowTerminalButton(), 0);
		sidebar.getSidebar().remove(sidebar.getHideRawdataButton());
		sidebar.getSidebar().insert(sidebar.getShowRawdataButton(), 3); 

		// get the size of the browserwindow 
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		if (textTurnedOn) {
			this.resizeWithTextPanel(windowWidth, windowHeight);

		} else {
			this.resizeToFullScreen(windowWidth, windowHeight);
		}
	}

	/**Shows the textpanel and resizes all visible elements*/
	public void showTextView() {

		textTurnedOn = true;
		view.clear();

		sidebar.getSidebar().remove(sidebar.getShowTextButton());
		sidebar.getSidebar().insert(sidebar.getHideTextButton(), 1); 
		sidebar.getSidebar().remove(sidebar.getHideRawdataButton());
		sidebar.getSidebar().insert(sidebar.getShowRawdataButton(), 3);

		view.add(textView.getContentPanel());

		if (mapTurnedOn) {
			view.add(mapView.getContentPanel());
			view.add(toolbox.getFpanel());
		} else {
			view.add(graphicalView.getContentPanel());
			view.add(toolbox.getFpanel());
		}

		// get the size of the browserwindow 
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		if (cpTurnedOn) {
			this.resizeWithTextAndCP(windowWidth, windowHeight);

		} else {
			this.resizeWithTextPanel(windowWidth, windowHeight);
		}

	}

	/**Hides the textpanel and resizes all visible elements*/
	public void hideTextView() {

		textTurnedOn = false;
		view.clear();

		sidebar.getSidebar().remove(sidebar.getHideTextButton());
		sidebar.getSidebar().insert(sidebar.getShowTextButton(), 1);
		sidebar.getSidebar().remove(sidebar.getHideRawdataButton());
		sidebar.getSidebar().insert(sidebar.getShowRawdataButton(), 3);

		if (mapTurnedOn) {
			view.add(mapView.getContentPanel());
			view.add(toolbox.getFpanel());
		} else {
			view.add(graphicalView.getContentPanel());
			view.add(toolbox.getFpanel());
		}

		// get the size of the browserwindow
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		if (cpTurnedOn) {
			this.resizeWithCP(windowWidth, windowHeight);

		} else {
			this.resizeToFullScreen(windowWidth, windowHeight);
		}
	}

	/**Shows the graphical view and resizes all visible elements*/
	public void showGraphicalView() {

		//reset all data
		mapTurnedOn = false;
		mapView.getMpointController().stopAllAnimations();
		toolbox.resetAnimationPanel();

		sidebar.getSidebar().remove(sidebar.getHideMapButton());
		sidebar.getSidebar().insert(sidebar.getShowMapButton(), 2);
		sidebar.getSidebar().remove(sidebar.getHideRawdataButton());
		sidebar.getSidebar().insert(sidebar.getShowRawdataButton(), 3);

		view.clear();

		// get the size of the browserwindow 
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		if (cpTurnedOn && textTurnedOn) {
			view.add(textView.getContentPanel());
			view.add(graphicalView.getContentPanel());
			view.add(toolbox.getFpanel());
			resizeWithTextAndCP(windowWidth, windowHeight);	
		}
		if (cpTurnedOn && !textTurnedOn) {
			view.add(graphicalView.getContentPanel());
			view.add(toolbox.getFpanel());
			resizeWithCP(windowWidth, windowHeight);
		}

		if (!cpTurnedOn && textTurnedOn) {
			view.add(textView.getContentPanel());
			view.add(graphicalView.getContentPanel());
			view.add(toolbox.getFpanel());
			resizeWithTextPanel(windowWidth, windowHeight);
		}
		if (!cpTurnedOn && !textTurnedOn) {
			view.add(graphicalView.getContentPanel());
			view.add(toolbox.getFpanel());
			resizeToFullScreen(windowWidth, windowHeight);
		}
	}

	/**Shows the map view and resizes all visible elements*/
	
	public void showMapView() {

		//reset all data
		mapTurnedOn = true;
		graphicalView.getMpointController().stopAllAnimations();
		toolbox.resetAnimationPanel();

		sidebar.getSidebar().remove(sidebar.getShowMapButton());
		sidebar.getSidebar().insert(sidebar.getHideMapButton(), 2);
		sidebar.getSidebar().remove(sidebar.getHideRawdataButton());
		sidebar.getSidebar().insert(sidebar.getShowRawdataButton(), 3);

		view.clear();

		// get the size of the browserwindow
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		if (textTurnedOn) {
			view.add(textView.getContentPanel());
			view.add(mapView.getContentPanel());
			view.add(toolbox.getFpanel());

			if (cpTurnedOn) {
				this.resizeWithTextAndCP(windowWidth, windowHeight);

			} else {
				this.resizeWithTextPanel(windowWidth, windowHeight);
			}
		}
		if (!textTurnedOn) {
			view.add(mapView.getContentPanel());
			view.add(toolbox.getFpanel());

			if (cpTurnedOn) {
				resizeWithCP(windowWidth, windowHeight);

			} else {
				resizeToFullScreen(windowWidth, windowHeight);
			}
		}else{
		mapView.updateView();
		}
		
		updateLegendInfoForMenuItem();
		header.getTextViewOfTrajInDialog().setTextViewInPlainTrajDialog(textView.getTextOutput());
	    
	    this.optionsTabPanel.setAttributeNameOfMLabelInRelation(this.mapView.getAttributeNameOfMLabel());
		
	}

	/**
	 * Sets a legend information to the menu item "legend"
	 */
	private void updateLegendInfoForMenuItem() {
		final LegendDialog legend = mapView.getLegend();		
	    Command legendInfo = new Command() {
	      public void execute() {
	    	  legend.getHelpDialogBox().center();
	    	  legend.getHelpDialogBox().show();
	      }
	    };
	    header.getLegendMenuItem().setScheduledCommand(legendInfo);
	}

	/**Shows the raw data view in the view panel*/
	public void showRawDataView() {

		sidebar.getSidebar().remove(sidebar.getShowRawdataButton());
		sidebar.getSidebar().insert(sidebar.getHideRawdataButton(), 3);

		view.clear();
		view.add(rawDataView.getContentPanel());
	}

	/** Hides the raw data view and shows the former screen */
	public void hideRawDataView() {

		sidebar.getSidebar().remove(sidebar.getHideRawdataButton());
		sidebar.getSidebar().insert(sidebar.getShowRawdataButton(), 3);

		view.clear();

		// get the size of the browserwindow
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		if (mapTurnedOn) {

			if (textTurnedOn) {
				view.add(textView.getContentPanel());
				view.add(mapView.getContentPanel());
				view.add(toolbox.getFpanel());

				if (cpTurnedOn) {
					this.resizeWithTextAndCP(windowWidth, windowHeight);

				} else {
					this.resizeWithTextPanel(windowWidth, windowHeight);
				}
			}
			if (!textTurnedOn) {
				view.add(mapView.getContentPanel());
				view.add(toolbox.getFpanel());

				if (cpTurnedOn) {
					resizeWithCP(windowWidth, windowHeight);

				} else {
					resizeToFullScreen(windowWidth, windowHeight);
				}
			}
		} else {
			if (textTurnedOn) {
				view.add(textView.getContentPanel());
				view.add(graphicalView.getContentPanel());
				view.add(toolbox.getFpanel());

				if (cpTurnedOn) {
					this.resizeWithTextAndCP(windowWidth, windowHeight);

				} else {
					this.resizeWithTextPanel(windowWidth, windowHeight);
				}
			}
			if (!textTurnedOn) {
				view.add(graphicalView.getContentPanel());
				view.add(toolbox.getFpanel());

				if (cpTurnedOn) {
					resizeWithCP(windowWidth, windowHeight);

				} else {
					resizeToFullScreen(windowWidth, windowHeight);
				}
			}
		}
	}

	/**Adds EventChangeHandler to all Checkboxes of Querys in the Object List of the ToolBox */
	public void addQueryCheckBoxChangeHandler() {

		if (!toolbox.getQueryBoxes().isEmpty()) {

			// add handler to all Query checkboxes
			for (Entry<Number, CheckBox> e : toolbox.getQueryBoxes().entrySet()) {

				final int resultIndex = (Integer) e.getKey();
				CheckBox cb = e.getValue();

				// set all checkboxes to selected
				cb.setValue(true);

				cb.addValueChangeHandler(new ValueChangeHandler<Boolean>() {

					@Override
					public void onValueChange(ValueChangeEvent<Boolean> event) {

						// checkbox is checked
						if (event.getValue() == true) {

							// show all datatypes in selected query
							for (DataType datatype : toolbox.getResultList()
									.get(resultIndex)) { // get list with datatypes from the query that equals the counter

								// Map View
								if (mapTurnedOn) {

									if (datatype.getType().equals("Polygon")) {
										mapView.getPolygonController().showPolygonObject((Polygon) datatype);
									}
									if (datatype.getType().equals("Polyline")) {
										mapView.getPolylineController().showPolylineObject((Polyline) datatype);
									}
									if (datatype.getType().equals("Point")) {
										mapView.getPointController().showPointObject((Point) datatype);
									}
									if (datatype.getType().equals("MPoint")) {
										// mapView.getMpointController().showMPointObject((MPoint)datatype);
									}
								// Graphical View	
								} else { 
									if (datatype.getType().equals("Point")) {
										graphicalView.getPointController().showPoint(datatype.getId(), datatype.getColor());
									}
									if (datatype.getType().equals("Polygon")) {
										graphicalView.getPolygonController().showPolygon(datatype.getId(), datatype.getColor());
									}
									if (datatype.getType().equals("Polyline")) {
										graphicalView.getPolylineController().showPolylineObject((Polyline) datatype);
									}
									if (datatype.getType().equals("MPoint")) {
										// graphicalView.getMpointController().showMovingPoint((MPoint)datatype);
									}
								}
								// set object checkbox to selected, if objects has attributes
								if (!datatype.getAttributeList().isEmpty()) {
									toolbox.getObjectBoxes().get(datatype.getId()).setValue(true);
								}
							}
						} else {
							for (DataType datatype : toolbox.getResultList().get(resultIndex)) {

								if (mapTurnedOn) {
									if (datatype.getType().equals("Polygon")) {
										mapView.getPolygonController().hidePolygonObject((Polygon) datatype);
									}
									if (datatype.getType().equals("Polyline")) {
										mapView.getPolylineController().hidePolylineObject((Polyline) datatype);
									}
									if (datatype.getType().equals("Point")) {
										mapView.getPointController().hidePointObject((Point) datatype);
									}
									if (datatype.getType().equals("MPoint")) {
										// mapView.getMpointController().hideMPointObject((MPoint)datatype);
									}
								} else {
									if (datatype.getType().equals("Point")) {
										graphicalView.getPointController().hidePoint(datatype.getId());
									}
									if (datatype.getType().equals("Polygon")) {
										graphicalView.getPolygonController().hidePolygon(datatype.getId());
									}
									if (datatype.getType().equals("Polyline")) {
										graphicalView.getPolylineController().removePolyline(datatype.getId());
									}
									if (datatype.getType().equals("MPoint")) {
										// graphicalView.getMpointController().removeMovingPoint(datatype.getId());
									}
								}
								// set object checkbox to unselected
								if (!datatype.getAttributeList().isEmpty()) {
									toolbox.getObjectBoxes().get(datatype.getId()).setValue(false);
								}
							}
						}
					}
				});
			}
		}
	}

	/** Adds EventChangeHandler to all Checkboxes of Objects in the Object List of the ToolBox */
	public void addObjectCheckboxChangeHandler() {

		if (!toolbox.getObjectBoxes().isEmpty()) {

			// add handler to all Query checkboxes
			for (Entry<Number, CheckBox> e : toolbox.getObjectBoxes().entrySet()) {

				final int objectId = (Integer) e.getKey();
				CheckBox cb = e.getValue();

				// set all checkboxes to selected
				cb.setValue(true);

				cb.addValueChangeHandler(new ValueChangeHandler<Boolean>() {

					@Override
					public void onValueChange(ValueChangeEvent<Boolean> event) {

						if (event.getValue() == true) { // checkbox is checked

							// search for the object with the given id
							for (ArrayList<DataType> queryList : toolbox.getResultList()) {
								for (DataType datatype : queryList) {
									if (datatype.getId() == objectId) {
										if (mapTurnedOn) {
											if (datatype.getType().equals("Polygon")) {
												mapView.getPolygonController().showPolygonObject((Polygon) datatype);
											}
											if (datatype.getType().equals("Polyline")) {
												mapView.getPolylineController().showPolylineObject((Polyline) datatype);
											}
											if (datatype.getType().equals("Point")) {
												mapView.getPointController().showPointObject((Point) datatype);
											}
											if (datatype.getType().equals("MPoint")) {
												// mapView.getMpointController().showMPointObject((MPoint)datatype);
											}
										} else {
											if (datatype.getType().equals("Point")) {
												graphicalView.getPointController().showPoint(datatype.getId(), datatype.getColor());
											}
											if (datatype.getType().equals("Polygon")) {
												graphicalView.getPolygonController().showPolygon(datatype.getId(), datatype.getColor());
											}
											if (datatype.getType().equals("Polyline")) {
												graphicalView.getPolylineController().showPolylineObject((Polyline) datatype);
											}
											if (datatype.getType().equals("MPoint")) {
												// graphicalView.getMpointController().showMovingPoint((MPoint)datatype);
											}
										}
									}
								}
							}

						}

						else {
							// search for the object with the given id
							for (ArrayList<DataType> queryList : toolbox.getResultList()) {
								for (DataType datatype : queryList) {
									if (datatype.getId() == objectId) {
										if (mapTurnedOn) {
											if (datatype.getType().equals("Polygon")) {
												mapView.getPolygonController().hidePolygonObject((Polygon) datatype);
											}
											if (datatype.getType().equals("Polyline")) {
												mapView.getPolylineController().hidePolylineObject((Polyline) datatype);
											}
											if (datatype.getType().equals("Point")) {
												mapView.getPointController().hidePointObject((Point) datatype);
											}
											if (datatype.getType().equals("MPoint")) {
												// mapView.getMpointController().hideMPointObject((MPoint)datatype);
											}
										} else {
											if (datatype.getType().equals("Point")) {
												graphicalView.getPointController().hidePoint(datatype.getId());
											}
											if (datatype.getType().equals("Polygon")) {
												graphicalView.getPolygonController().hidePolygon(datatype.getId());
											}
											if (datatype.getType().equals("Polyline")) {
												graphicalView.getPolylineController().removePolyline(datatype.getId());
											}
											if (datatype.getType().equals("MPoint")) {
												// graphicalView.getMpointController().removeMovingPoint(datatype.getId());
											}
										}
									}
								}
							}
						}
					}
				});
			}
		}
	}

	/**Returns for the given index of the radiobutton in the list the corresponding color
	 * 
	 *  @param index The index of the radiobutton
	 *  @return The corresponding color to the radiobutton
	 *  */
	public String getColorForIndex(int index) {
		if (index == 0) {
			return "yellow";
		}
		if (index == 1) {
			return "orange";
		}
		if (index == 2) {
			return "#FF0000";
		}
		if (index == 3) {
			return "#660066";
		}
		if (index == 4) {
			return "#0000FF";
		}
		if (index == 5) {
			return "green";
		}
		if (index == 6) {
			return "#660000";
		}
		else {
			return "black";
		}
	}

	/**Returns the main panel of the main view
	 * 
	 * @return The main panel of the main view
	 * */
	public HorizontalPanel getMainPanel() {
		return mainPanel;
	}
	
	/**Returns the header of the main view
	 * 
	 * @return The header of the main view
	 * */
	public Header getMainheader() {
		return header;
	}

	/**Returns the sidebar object
	 * 
	 * @return The sidebar object
	 * */
	public SideBar getSidebar() {
		return sidebar;
	}

	/**Returns the commandpanel object
	 * 
	 * @return The commandpanel object
	 * */
	public CommandPanel getCommandPanel() {
		return commandPanel;
	}

	/**Returns the raw data view object
	 * 
	 * @return The raw data view object
	 * */
	public RawDataView getRawDataView() {
		return rawDataView;
	}

	/**Returns the textpanel object
	 * 
	 * @return The textpanel object
	 * */
	public TextPanel getTextView() {
		return textView;
	}

	/**Returns the graphical view object
	 * 
	 * @return The graphical view object
	 * */
	public GraphicalView getGraphicalView() {
		return graphicalView;
	}

	/**Returns the map view object
	 * 
	 * @return The map view object
	 * */
	public MapView getMapView() {
		return mapView;
	}

	/**Returns the statusbar object
	 * 
	 * @return The statusbar object
	 * */
	public StatusBar getStatusBar() {
		return statusBar;
	}

	/**Returns the toolbox object
	 * 
	 * @return The toolbox object
	 * */
	public ToolBox getToolbox() {
		return toolbox;
	}

	/**Returns true if the commandpanel is visible
	 * 
	 * @return True if the commandpanel is visible
	 * */
	public boolean isCpTurnedOn() {
		return cpTurnedOn;
	}

	/**Returns true if the textpanel is visible
	 * 
	 * @return True if the textpanel is visible
	 * */
	public boolean isTextTurnedOn() {
		return textTurnedOn;
	}

	/**Returns true if the map background is visible
	 * 
	 * @return True if the map background is visible
	 * */
	public boolean isMapTurnedOn() {
		return mapTurnedOn;
	}

	/**
	 * @return optionsTabPanel
	 */
	public OptionsTabPanel getOptionsTabPanel() {
		return optionsTabPanel;
	}
	
	/**
	 * 
	 */
	public void resetMapView() {
		mapView.resetData();
		mapView.getMpointController().stopAllAnimations();
	}

	
}
