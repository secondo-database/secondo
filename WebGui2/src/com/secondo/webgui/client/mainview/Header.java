package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style;
import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Anchor;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.MenuBar;
import com.google.gwt.user.client.ui.MenuItem;
import com.google.gwt.user.client.ui.VerticalPanel;

/**
 * This class represents the header of the main view.
 * 
 * @author Irina Russkaya
 * 
 **/
public class Header extends Composite {

	/** The main panel of the header */
	private HorizontalPanel hPanel = new HorizontalPanel();

	/** The main grid of the header */
	private FlexTable mainGrid = new FlexTable();

	/** The Secondo logo image */
	private Image logo = new Image("resources/images/secondo-logo.gif");

	/** The panel for buttons */
	private HorizontalPanel buttonPanel = new HorizontalPanel();

	/** The label to show a name of actual database */
	private Label labelWithDatabaseName = new Label();

	/** Width of the header */
	private int width = Window.getClientWidth() - 17;

	private MenuBar mainMenuBar = new MenuBar();
	private MenuItem legend = new MenuItem("Legend", mainMenuBar);
	private MenuItem export = new MenuItem("Export", mainMenuBar);
	private MenuItem plainTraj = new MenuItem("Plain trajectory", mainMenuBar);

	private PlainTrajDialog textViewOfTrajInDialog = new PlainTrajDialog();
	private DBSettingsDialog databaseInfo = new DBSettingsDialog();
	private LocationDialog locationDialog = new LocationDialog();
	private SupportDialog supportDialog = new SupportDialog();
	private PatternDialog patternDialog = new PatternDialog();

	public Header() {

		logo.setSize("190px", "28px");
		mainGrid.setWidget(0, 0, logo);
		mainGrid.setWidth(width + "px");
		mainGrid.setHeight("28px");
		mainGrid.getElement().setClassName("mainheader");

		Image infoImage = new Image("resources/images/info-icon.png");
		infoImage.getElement().setAttribute("align", "right");
		mainGrid.setWidget(0, 1, infoImage);

		VerticalPanel notePanel = new VerticalPanel();
		HTML htmlNote = new HTML(
				"<div class=\"please_note_text\"> <div id=\"please_note_title\">Please note</div>"
						+ "<div><span style=\"color: grey;\">To zoom in and out use your mouse wheel.</span></p>"
						+ "<span style=\"color: grey;\">To pan the map drag your mouse holding left mouse button.</span></p>"
						+ "</div></div>", true);
		notePanel.add(htmlNote);
		mainGrid.setWidget(0, 2, notePanel);
		mainGrid.getFlexCellFormatter().setRowSpan(0, 2, 3);
		mainGrid.getColumnFormatter().setWidth(2, "350px");

		MenuItem homeMenu = new MenuItem("Secondo", new Command() {
			@Override
			public void execute() {
				doClickOnLink();
			}
		});

		homeMenu.ensureDebugId("HomeItem");
		homeMenu.setTitle("link to the secondo web page");
		homeMenu.getElement().getStyle().setColor("white");

		Anchor home = new Anchor("Home",
				"http://dna.fernuni-hagen.de/Secondo.html/index.html");
		home.setVisible(false);
		home.ensureDebugId("LinkToHome");
		mainGrid.setWidget(2, 0, home);

		MenuItem symbTrajMenu = new MenuItem("About symbolic trajectory",
				new Command() {
					@Override
					public void execute() {
						SymTrajDialog infoAboutSymTraj = new SymTrajDialog();
						infoAboutSymTraj.getHelpDialogBox().center();
						infoAboutSymTraj.getHelpDialogBox().show();

					}
				});
		symbTrajMenu.getElement().getStyle().setColor("white");

		MenuItem database = new MenuItem("Database", new Command() {
			@Override
			public void execute() {
				databaseInfo.getDbDialogBox().center();
				databaseInfo.getDbDialogBox().show();

			}
		});
		database.getElement().getStyle().setColor("white");

		MenuItem patternMenu = new MenuItem("About pattern matching",
				new Command() {
					@Override
					public void execute() {
						patternDialog.getHelpDialogBox().center();
						patternDialog.getHelpDialogBox().show();

					}
				});
		patternMenu.getElement().getStyle().setColor("white");

		MenuItem supportMenu = new MenuItem("Support", new Command() {
			@Override
			public void execute() {
				supportDialog.getSupportDialogBox().center();
				supportDialog.getSupportDialogBox().show();

			}
		});
		patternMenu.getElement().getStyle().setColor("white");

		MenuBar menu = new MenuBar();
		menu.addItem(homeMenu);
		menu.addItem(symbTrajMenu);
		menu.addItem(patternMenu);
		menu.addItem(database);
		menu.addItem(supportMenu);
		menu.getElement().getStyle().setColor("white");
		menu.getElement().getStyle().setBorderStyle(Style.BorderStyle.NONE);
		mainGrid.setWidget(1, 0, menu);
		mainGrid.getFlexCellFormatter().setColSpan(1, 0, 3);
		mainGrid.getCellFormatter().setStyleName(1, 0, "GreyMenubar");

		MenuItem homeItem = new MenuItem("My location", new Command() {
			@Override
			public void execute() {
				locationDialog.getLocationDialogBox().center();
				locationDialog.getLocationDialogBox().show();
			}
		});
		homeItem.setTitle("Define your location");
		homeItem.setStyleName("transparent1");
		mainMenuBar.addItem(homeItem);

		legend.setStyleName("transparent2");
		legend.setTitle("Legend to the symbolic trajectory");
		mainMenuBar.addItem(legend);

		MenuItem print = new MenuItem("Print", new Command() {
			@Override
			public void execute() {
				Window.print();
			}
		});
		print.setStyleName("transparent4");
		mainMenuBar.addItem(print);

		export.setStyleName("transparent5");
		export.setTitle("Export raw data");
		mainMenuBar.addItem(export);

		plainTraj.setStyleName("transparent6");
		plainTraj.setTitle("Symbolic trajectory\n in plain text");
		mainMenuBar.addItem(plainTraj);

		mainGrid.setWidget(3, 0, mainMenuBar);
		mainGrid.getFlexCellFormatter().setColSpan(3, 0, 3);
		mainMenuBar.setStyleName("toolbar_menu");
	}

	/**
	 * On resizing of the browser window the width of the main panel and button
	 * panel is readjusted
	 * 
	 * @param width
	 *            The new width of the main panel and the button panel
	 * */
	public void resizeWidth(int width) {

		if (width > 950) {
			mainGrid.setWidth(width + "px");
			mainMenuBar.getElement().getStyle().setMarginLeft(300, Unit.PX);
		} else {
			mainGrid.setWidth("950px");
			if (width < 680) {
				mainMenuBar.getElement().getStyle().setMarginLeft(0, Unit.PX);
			} else {
				mainMenuBar.getElement().getStyle()
						.setMarginLeft(width - 680, Unit.PX);
			}
		}
	}

	/**
	 * Returns the main panel of the header
	 * 
	 * @return The main panel of the header
	 * */
	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	/**
	 * Returns the label with the currently selected database
	 * 
	 * @return The label
	 * */
	public Label getLabelWithDatabaseName() {
		return labelWithDatabaseName;
	}

	/**
	 * Returns the main grid of the header
	 * 
	 * @return The main grid
	 */
	public FlexTable getGrid() {
		return mainGrid;
	}

	/**
	 * Returns the menu item "legend"
	 * 
	 * @return The legend
	 */
	public MenuItem getLegendMenuItem() {
		return legend;
	}

	/**
	 * Returns the menu item "export"
	 * 
	 * @return the export
	 */
	public MenuItem getExport() {
		return export;
	}

	/**
	 * Returns the menu item "Symbolic trajectory in plain text"
	 * 
	 * @return the plainTraj
	 */
	public MenuItem getPlainTraj() {
		return plainTraj;
	}

	/**
	 * Returns the dialog with plain trajectory
	 * 
	 * @return The textViewOfTrajInDialog
	 */
	public PlainTrajDialog getTextViewOfTrajInDialog() {
		return textViewOfTrajInDialog;
	}

	/**
	 * Does redirect to the secondo web-page
	 */
	public static native void doClickOnLink() /*-{
		$doc.getElementById('gwt-debug-LinkToHome').click();
	}-*/;

	/**
	 * Returns the dialog with db info
	 * 
	 * @return The databaseInfo
	 */
	public DBSettingsDialog getDatabaseInfo() {
		return databaseInfo;
	}

	/**
	 * Returns the dialog to determine location
	 * 
	 * @return The infoToLocation
	 */
	public LocationDialog getLocationDialog() {
		return locationDialog;
	}

	/**
	 * Returns the command to determine a location
	 * 
	 * @return The command to be sent to secondo
	 */
	public String getCommandForGeocode() {
		String command = "query geocode (\"";
		command += getLocationDialog().getStreet().getText() + "\",";
		command += getLocationDialog().getBuilding().getText() + ",";
		command += getLocationDialog().getZip().getText() + ", \"";
		command += getLocationDialog().getCity().getText() + "\")";
		return command;

	}

	/**
	 * Returns the support dialog
	 * 
	 * @return The support dialog
	 */
	public SupportDialog getSupportDialog() {
		return supportDialog;
	}
}
