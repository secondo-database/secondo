package com.secondo.webgui.client;

import java.util.ArrayList;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.event.dom.client.ChangeEvent;
import com.google.gwt.event.dom.client.ChangeHandler;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.rpc.AsyncCallback;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.PopupPanel;
import com.google.gwt.user.client.ui.RootPanel;
import com.secondo.webgui.client.controller.RPCConnector;
import com.secondo.webgui.client.mainview.MainView;
import com.secondo.webgui.client.rpc.SecondoService;
import com.secondo.webgui.client.rpc.SecondoServiceAsync;
import com.secondo.webgui.utils.config.Resources;
import com.secondo.webgui.utils.config.SecondoConstants;

/**
 * This is the Entry point class which defines the onModuleLoad()-Method to
 * start the application, creates important objects and sets the content of the
 * HTML-Page with the method setContent().
 * 
 * @author Irina Russkaya
 */
public class SecondoWeb implements EntryPoint {

	/** Main Elements of the Website which represent the divs in the HTML-Page */
	private FlowPanel header = new FlowPanel();
	private FlowPanel content = new FlowPanel();
	private FlowPanel footer = new FlowPanel();	
	
	/** Element of the main view */
	private MainView mainView = new MainView();

	/** Object to execute RPC-Calls to the server */
	private RPCConnector rpcConnector = new RPCConnector();

	/** Service for RPC-Calls to the server in this class */
	private SecondoServiceAsync secondoService = (SecondoServiceAsync) GWT
			.create(SecondoService.class);

	/** Popup showing an animation while loading data from the database */
	private PopupPanel loadingPopup = new PopupPanel(true);
	private Image loadingImage = new Image("resources/images/loader1.gif");

	/**
	 * The message displayed to the user when the server cannot be reached or
	 * returns an error.
	 */
	private static final String SERVER_ERROR = "An error occurred while "
			+ "attempting to contact the server. Please check your network "
			+ "connection and try again.";

	/* Storage for temporary data */	
	private ArrayList<String> logindata = new ArrayList<String>();
	/**************************************************************
	 * This is the entry point method which starts the application.
	 **************************************************************/
	public void onModuleLoad() {
		Resources.INSTANCE.css().ensureInjected();

		/* set default values in login textfields using properties */
		SecondoConstants constantsToConnect = GWT
				.create(SecondoConstants.class);

		if (!logindata.isEmpty()) {
			logindata.clear();
		}
		// get the content of the login text fields
		logindata.add("");
		logindata.add("");
		logindata.add(constantsToConnect.IP());
		logindata.add(constantsToConnect.port());

		// connect to secondo with logindata
		sendLogin(logindata, constantsToConnect.DB());

		/* initialize the loading popup */
		loadingPopup.setAnimationEnabled(true);
		loadingPopup.ensureDebugId("imagePopup");
		loadingPopup.setWidget(loadingImage);
		loadingPopup.setGlassEnabled(true);
		loadingPopup.getElement().setClassName("loadingpopup");
		

		/* Allows to download a raw data result */
		this.mainView.getMainheader().getExport()
				.setScheduledCommand(new Command() {

					@Override
					public void execute() {
						rpcConnector.saveTextFile(mainView.getMainheader()
								.getTextViewOfTrajInDialog().getTextView()
								.getText(), "secondo-text.txt");
					}
				});
		
		/* Adds an event handler to the button "create symtraj" */
		this.mainView.getOptionsTabPanel().getCreateSymTrajButton()
				.addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {
						if (mainView.getOptionsTabPanel()
								.getOptionsForCreatingSymTraj()
								.getSelectedIndex() == 2
								&& mainView.getMapView().getMyLocation() == null) {
							Window.alert("Please select my location using menu");
						} else {
							rpcConnector
									.doGPXimport(mainView.getOptionsTabPanel()
											.getNameOfUploadedFile(), mainView
											.getOptionsTabPanel()
											.getOptionsForCreatingSymTraj()
											.getSelectedIndex(), mainView,
											loadingPopup);
						}

					}
				});

		/* Adds an event handler to the button "get relation" of the options tab panel */
		this.mainView.getOptionsTabPanel().getAnimateButton()
				.addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {
						String command = mainView.getOptionsTabPanel()
								.getCommandForQuerySampleRelation();
						String commandForCountTuples = mainView
								.getOptionsTabPanel()
								.getCommandForCountTuplesInSampleRelation();
						if (!command.isEmpty()) {
							mainView.resetMapView();
							
							rpcConnector.sendCommandAndUpdateHistory(command,
									commandForCountTuples, mainView,
									loadingPopup);
							
							loadingPopup.center();
							mainView.getOptionsTabPanel()
									.getLabelForInfoAboutOpenedRelation()
									.setVisible(true);

						} else {
							Window.alert("Please select relation");
						}

					}
				});

		/* Adds an event handler to the button "retrieve" to match a defined pattern to the loaded relation */
		this.mainView.getOptionsTabPanel().getNumberOfTrajectoriesToShow()
				.addChangeHandler(new ChangeHandler() {

					@Override
					public void onChange(ChangeEvent event) {
						if (!mainView.getOptionsTabPanel()
								.isUnsuccessfulVerification()) {
							String command = mainView.getOptionsTabPanel()
									.getCommandForPatternMatching();
							String commandForCountResult = mainView
									.getOptionsTabPanel()
									.getCommandForCountPatternMatching();
							mainView.getOptionsTabPanel()
									.setPatternMatchingIsInitiated(true);
							if (!command.isEmpty()) {
								mainView.resetMapView();
								
								rpcConnector.sendCommandAndUpdateHistory(
										command, commandForCountResult,
										mainView, loadingPopup);
								
								loadingPopup.center();
							} else {
								Window.alert("Please select relation and load it");
							}

						} else {
							Window.alert("Please correct your pattern statement!");
						}
					}
				});

		/* Adds an event handler to the button "count" to match a defined pattern to the loaded relation */
		this.mainView.getOptionsTabPanel().getCountButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {
						if (!mainView.getOptionsTabPanel()
								.isUnsuccessfulVerification()) {
							String commandForCountResult = mainView
									.getOptionsTabPanel()
									.getCommandForCountPatternMatching();
							mainView.getOptionsTabPanel()
									.setPatternMatchingIsInitiated(true);
							if (!commandForCountResult.isEmpty()) {
								mainView.resetMapView();

								rpcConnector
										.setNumberOfTuplesInPatternMatchingResult(
												commandForCountResult,
												mainView, loadingPopup);

								loadingPopup.center();
							}
						}

						else {
							Window.alert("Please correct your pattern statement!");
						}
					}
				});

		/* Adds an event handler to the button "get GPX coordinate" to define a location from address */
		this.mainView.getMainheader().getLocationDialog()
				.getGetCoordinateButton().addClickHandler(new ClickHandler() {
					@Override
					public void onClick(ClickEvent event) {
						String command = mainView.getMainheader()
								.getCommandForGeocode();
						
						rpcConnector.getCoordinateFromAddress(command,
								mainView, loadingPopup);

					}
				});

		/* Adds an event handler to the button "retrieve" in the simple queries panel (for operator passes) */
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getPassesPanel().getNumberOfTrajectoriesToBeShown()
				.addChangeHandler(new ChangeHandler() {

					@Override
					public void onChange(ChangeEvent event) {
						String command = mainView.getOptionsTabPanel()
								.getCommandForSimpleQueryPasses();
						String commandForCountResult = mainView
								.getOptionsTabPanel()
								.getCommandForSimpleQueryPassesCount();
						mainView.getOptionsTabPanel()
								.setSimpleQueryForPassesIsInitiated(true);
						if (!command.isEmpty()) {
							mainView.resetMapView();
							
							rpcConnector.sendCommandAndUpdateHistory(command,
									commandForCountResult, mainView,
									loadingPopup);
							
							loadingPopup.center();

						} else {
							Window.alert("Please select relation and load it");
						}
					}

				});

		/* Adds an event handler to the button "count" in the simple queries panel (for operator passes)*/
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getPassesPanel().getCountButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {
						String commandForCountResult = mainView
								.getOptionsTabPanel()
								.getCommandForSimpleQueryPassesCount();
						mainView.getOptionsTabPanel()
								.setSimpleQueryForPassesIsInitiated(true);
						if (!commandForCountResult.isEmpty()) {							
							rpcConnector
									.setNumberOfTuplesInSimpleQueryResultPasses(
											commandForCountResult, mainView,
											loadingPopup);
						} else {
							Window.alert("Please select relation and load it");
						}

					}
				});

		/* Adds an event handler to the button "define" in the simple queries panel (for operator deftime)*/
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getDeftimePanel().getQueryButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {

						String command = mainView.getOptionsTabPanel()
								.getCommandForSimpleQueryDeftime();
						if (!command.isEmpty()) {
							rpcConnector.sendSimpleQuery(command, "deftime",
									mainView);
						} else {
							Window.alert("Please select relation and load it");
						}
					}
				});

		/* Adds an event handler to the button "define" in the simple queries panel (for operator atinstant)*/
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getAtinstantPanel().getQueryButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {

						String command = mainView.getOptionsTabPanel()
								.getCommandForSimpleQueryAtinstant();
						if (!command.isEmpty()) {
							rpcConnector.sendSimpleQuery(command, "atinstant",
									mainView);
						} else {
							Window.alert("Please select relation and load it");
						}
					}
				});

		/* Adds an event handler to the button "retrieve" in the simple queries panel (for operator passes through)*/
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getPassesThroughRegionPanel()
				.getNumberOfTrajectoriesToBeShown()
				.addChangeHandler(new ChangeHandler() {

					@Override
					public void onChange(ChangeEvent event) {
						String command = mainView.getOptionsTabPanel()
								.getCommandForSimpleQueryPassesThroughRegion(
										mainView.getMapView().getDrawLayer(),
										"retrieve");
						String commandForCount = mainView.getOptionsTabPanel()
								.getCommandForSimpleQueryPassesThroughRegion(
										mainView.getMapView().getDrawLayer(),
										"count");
						mainView.getOptionsTabPanel()
								.setSimpleQueryForPassesTrhoughRegionsInitiated(
										true);

						if (!command.isEmpty()) {
							mainView.resetMapView();
							
							rpcConnector.sendCommandAndUpdateHistory(command,
									commandForCount, mainView, loadingPopup);
							
							loadingPopup.center();
						} else {
							Window.alert("Please select relation and load it");
						}

					}
				});

		/* Adds an event handler on the button "count" in the simple queries panel (for operator passes)*/
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getPassesThroughRegionPanel().getCountButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {
						String commandForCountResult = mainView
								.getOptionsTabPanel()
								.getCommandForSimpleQueryPassesThroughRegion(
										mainView.getMapView().getDrawLayer(),
										"count");
						mainView.getOptionsTabPanel()
								.setSimpleQueryForPassesTrhoughRegionsInitiated(
										true);
						if (!commandForCountResult.isEmpty()) {							
							rpcConnector
									.setNumberOfTuplesInSimpleQueryResultPassesThrough(
											commandForCountResult, mainView,
											loadingPopup);

						} else {
							Window.alert("Please select relation and load it");
						}

					}
				});

		/* Adds an event handler to the menu item Support to send a user request to the support email*/
		this.mainView.getMainheader().getSupportDialog().getSendButton()
				.addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {
						rpcConnector.sendMailToSupport(mainView.getMainheader()
								.getSupportDialog().getMessage());

					}
				});

		/*
		 * sets default content after starting the application, which is the
		 * login-page
		 */
		// check for ie8 and display default message
		if (Window.Navigator.getUserAgent().contains("MSIE 8")) {
			System.out.println("###Browser version: "
					+ Window.Navigator.getUserAgent());
			HTML defaultText = new HTML(
					"<p><h3>Your browser does not support Scalable Vector Graphics (SVG).<br>"
							+ " Please upgrade to a modern browser.</h3></p>");
			mainView.getMainPanel().remove(2);
			mainView.getMainPanel().insert(defaultText, 2);
		}		
	}
	

	/**
	 * Starts an RPC call to the server to send the logindata to secondo and
	 * verify it
	 * 
	 * @param userDataList
	 *            List with logindata of the user
	 * */
	public void sendLogin(ArrayList<String> userDataList, final String db) {

		AsyncCallback<Void> callback = new AsyncCallback<Void>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert("An error occurred while "
						+ "attempting to contact the server. Please check your logindata and try again.");
			}

			@Override
			public void onSuccess(Void result) {				
				openDatabase(db);
			}
		};

		// Make the call. Control flow will continue immediately and later
		// 'callback' will be invoked when the RPC completes.
		secondoService.setSecondoConnectionData(userDataList, callback);
	}	

	/**
	 * Starts an RPC call to the server to send the name of the database to
	 * secondo and open the database
	 * 
	 * @param database
	 *            The database to be opened
	 * */
	public void openDatabase(String database) {

		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);
			}

			@Override
			public void onSuccess(String openDatabase) { 
				
				// set database info to info dialog
				mainView.getMainheader().getDatabaseInfo().getHost()
						.setText(logindata.get(2));
				mainView.getMainheader().getDatabaseInfo().getPort()
						.setText(logindata.get(3));
				mainView.getMainheader().getDatabaseInfo().getDb()
						.setText(openDatabase);

				// reset text view
				mainView.getMainheader().getTextViewOfTrajInDialog()
						.resetData();

				// delete data from map
				mainView.getMapView().resetData();
				mainView.getMapView().getMpointController().stopAllAnimations();

				rpcConnector.resetObjectCounter();

				mainView.showMapView();

				setContent(2);
			}
		};
		secondoService.openDatabase(database, callback);
	}
	

	/**
	 * Method to set 3 different contents to the HTML-Page, depending on the
	 * userstatus and the chosen database
	 * 
	 * @param status
	 *            Integer value with the status number
	 * */
	public void setContent(int status) {
		switch (status) {
		case 0:
			System.out.println("User is not logged in");				

			/* Associate the panels with the HTML host page. */
			RootPanel.get("content").add(content);

			RootPanel.get("header").add(header);

			RootPanel.get("footer").add(footer);

			break;
		case 1:
			System.out
					.println("User is logged in but has not chosen a database");
			
			/* Associate the panels with the HTML host page. */
			RootPanel.get("content").add(content);

			RootPanel.get("header").add(header);

			RootPanel.get("footer").add(footer);

			break;
		case 2:
			System.out.println("User is logged in and has chosen a database");

			header.clear();			
			header.add(mainView.getMainheader().getGrid());

			content.clear();
			content.add(mainView.getMainPanel());

			footer.clear();

			/* Associate the panels with the HTML host page. */
			RootPanel.get("content").add(content);

			RootPanel.get("header").add(header);

			break;
		default:
			System.out.println("switch-case-defaulttext");
		}
	}
}
