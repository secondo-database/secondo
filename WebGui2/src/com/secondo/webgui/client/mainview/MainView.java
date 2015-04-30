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

import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.logical.shared.ResizeEvent;
import com.google.gwt.event.logical.shared.ResizeHandler;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.AbsolutePanel;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FormPanel;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.FormPanel.SubmitCompleteEvent;
import com.google.gwt.user.client.ui.FormPanel.SubmitEvent;

/**
 * This class represents the main view of the application and includes all
 * elements that are displayed in the main view.
 * 
 * @author Irina Russkaya
 */
public class MainView extends Composite {

	/** The main panel of the main view */
	private HorizontalPanel mainPanel = new HorizontalPanel();
	private AbsolutePanel contentPanel = new AbsolutePanel();

	// main elements of the application
	private Header header = new Header();
	private OptionsTabPanel optionsTabPanel = new OptionsTabPanel();

	// different views that can be displayed in the viewpanel
	private HorizontalPanel view = new HorizontalPanel();	
	private MapView mapView = new MapView();

	// boolean values to show if panels are visible or not
	private boolean cpTurnedOn = true;
	private boolean mapTurnedOn = false;

	/**
	 * default value=0, means "doesn't show symtraj"
	 */
	private int modeForSymTraj = 0;

	public MainView() {

		contentPanel.add(view);
		contentPanel.add(optionsTabPanel.getOptionsTabPanel(), 10, 0);
		mainPanel.add(contentPanel);

		// initialize the main view with the map view
		showMapView();

		// get the size of the browserwindow and set the elements to the right
		// size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();
		this.resizeToFullScreen(windowWidth, windowHeight);

		// resize the application elements if the size of the window changes
		Window.addResizeHandler(new ResizeHandler() {
			public void onResize(ResizeEvent event) {
				int windowWidth = event.getWidth();
				int windowHeight = event.getHeight();

				resizeToFullScreen(windowWidth, windowHeight);

			}
		});

		/*
		 * ****************************** EventHandler for the TabPanel
		 * *************************************************
		 */
		/**
		 * Adds an event handler to the list box with options for displaying a
		 * sym trajectory
		 */
		this.optionsTabPanel.getSelectOptionsForDisplayMode().addClickHandler(
				new ClickHandler() {
					@Override
					public void onClick(ClickEvent event) {
						modeForSymTraj = optionsTabPanel
								.getSelectOptionsForDisplayMode()
								.getSelectedIndex();
						getMapView().setModeForSymTraj(modeForSymTraj);
					}

				});

		/**
		 * Adds an event handler on the playButton of the optionsTabPanel to
		 * animate the moving point
		 */
		this.optionsTabPanel.getPlayLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {

				if (modeForSymTraj != 0) {
					getMapView().getMpointController().animateMovingPoints(
							optionsTabPanel, mapView.getMap(), modeForSymTraj);
				} else {
					Window.alert("Please select display mode");
				}
			}
		});

		/**
		 * Adds an event handler on the forwardButton of the optionsTabPanel to
		 * animate the moving point
		 */
		this.optionsTabPanel.getForwardLink().addClickHandler(
				new ClickHandler() {
					public void onClick(ClickEvent event) {
						getMapView().getMpointController().speedUpMovingPoint();

					}
				});

		/**
		 * Adds an event handler on the rewindButton of the optionsTabPanel to
		 * animate the moving point
		 */
		this.optionsTabPanel.getRewindLink().addClickHandler(
				new ClickHandler() {
					public void onClick(ClickEvent event) {
						getMapView().getMpointController()
								.reduceSpeedOfMovingPoint();
					}
				});

		/**
		 * Adds an event handler on the pauseButton of the optionsTabPanel to
		 * pause the animation of the the moving point
		 */
		this.optionsTabPanel.getPauseLink().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				optionsTabPanel.getAnimationPanel().remove(0);
				optionsTabPanel.getAnimationPanel().insert(
						optionsTabPanel.getPanelForPlay(), 0);
				getMapView().getMpointController().pauseMovingPoint();
			}
		});

		/**
		 * Adds an event handler to the menu item "Plain trajectory" and shows
		 * an appropriate dialog
		 */
		this.header.getPlainTraj().setScheduledCommand(new Command() {
			@Override
			public void execute() {
				header.getTextViewOfTrajInDialog().getPlainTrajDialogBox()
						.center();
				header.getTextViewOfTrajInDialog().getPlainTrajDialogBox()
						.show();

			}
		});

		/**
		 * This event is fired just before the form is submitted. It performs
		 * validation
		 */
		this.optionsTabPanel.getUploadWidget().addSubmitHandler(
				new FormPanel.SubmitHandler() {
					@Override
					public void onSubmit(SubmitEvent event) {
						String filename = optionsTabPanel.getUploadWidget()
								.getFilenameFromFileUpload();
						if (filename.length() == 0) {
							Window.alert("Please select a file");
							event.cancel();
						} else {
							if (!filename.endsWith(".gpx")) {
								Window.alert("You can upload only gpx file");
								event.cancel();
							}
						}
					}
				});

		/**
		 * This event is fired just after the form is submitted. It informs
		 * about result and makes "create sym. traj." options visible
		 */
		this.optionsTabPanel.getUploadWidget().addSubmitCompleteHandler(
				new FormPanel.SubmitCompleteHandler() {
					@Override
					public void onSubmitComplete(SubmitCompleteEvent event) {
						if (event.getResults().contains("File name:")) {
							System.out.println("Result " + event.getResults());
							String result = event.getResults().replace(
									"</pre>", "");
							String uploadedFilename = result.substring(result
									.lastIndexOf(":") + 1);
							optionsTabPanel.getUploadWidget()
									.setNameOfUploadedFile(uploadedFilename);
							uploadedFilename = uploadedFilename
									.substring(uploadedFilename
											.lastIndexOf("/") + 1);
							Window.alert("File " + uploadedFilename
									+ " uploaded successfully!");
						}
						optionsTabPanel.getGridWithOptionsForCreatingSymTraj()
								.setVisible(true);
					}
				});

		/**
		 * This event is fired on click on the SimpleQueriesStackPanel. If it is
		 * "pass through" panel draw layer should be added to the main layer
		 */
		final SimpleQueriesStackPanel stackpanel = this.getOptionsTabPanel()
				.getSimpleQueriesStackPanel();
		stackpanel.addHandler(new ClickHandler() {
			@Override
			public void onClick(ClickEvent event) {

				if (stackpanel.getSelectedIndex() == 1) {
					mapView.initDrawLayer();
				} else {
					mapView.removeDrawLayer();
				}
				stackpanel
						.cleanResultInfoLabelsAndPanelWithNumberOfTrajectoriesToBeShown();

			}
		}, ClickEvent.getType());
	}

	/**
	 * On resizing of the browser window the elements of the main view are
	 * readjusted with to fullscreen
	 * 
	 * @param width
	 *            The new width of all visible elements
	 * @param height
	 *            The new height of all visible elements
	 * */
	public void resizeToFullScreen(int width, int height) {
		width = width - 30;
		height = height - 134;
		header.resizeWidth(width);

		if (mapTurnedOn) {
			mapView.resizeToFullScreen(width, height, optionsTabPanel
					.getOptionsTabPanel().getElement().getClientHeight());
			mapView.updateView();
		}
	}

	/** Shows the map view and resizes all visible elements */
	public void showMapView() {
		// reset all data
		mapTurnedOn = true;
		view.clear();

		// get the size of the browserwindow
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();

		view.add(mapView.getContentPanel());
		resizeToFullScreen(windowWidth, windowHeight);
		mapView.updateView();
		updateLegendInfoForMenuItem();

		this.optionsTabPanel.setAttributeNameOfMLabelInRelation(this.mapView
				.getAttributeNameOfMLabel());
		this.optionsTabPanel.setAttributeNameOfMPointInRelation(this.mapView
				.getAttributeNameOfMPoint());
	}

	/** Sets a legend information to the menu item "legend" */
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

	/**
	 * Returns the main panel of the main view
	 * 
	 * @return The main panel of the main view
	 * */
	public HorizontalPanel getMainPanel() {
		return mainPanel;
	}

	/**
	 * Returns the header of the main view
	 * 
	 * @return The header of the main view
	 * */
	public Header getMainheader() {
		return header;
	}
	
	/**
	 * Returns the map view object
	 * 
	 * @return The map view object
	 * */
	public MapView getMapView() {
		return mapView;
	}

	/**
	 * Returns true if the commandpanel is visible
	 * 
	 * @return True if the commandpanel is visible
	 * */
	public boolean isCpTurnedOn() {
		return cpTurnedOn;
	}

	/**
	 * Returns true if the map background is visible
	 * 
	 * @return True if the map background is visible
	 * */
	public boolean isMapTurnedOn() {
		return mapTurnedOn;
	}

	/**
	 * Returns OptionsTabPanel object
	 * 
	 * @return optionsTabPanel
	 */
	public OptionsTabPanel getOptionsTabPanel() {
		return optionsTabPanel;
	}

	/** Resets the map view and all data */
	public void resetMapView() {
		mapView.resetData();
		mapView.getMpointController().stopAllAnimations();
	}
}
