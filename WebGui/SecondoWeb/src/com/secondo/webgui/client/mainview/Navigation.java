package com.secondo.webgui.client.mainview;

import com.google.gwt.safehtml.shared.SafeHtml;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.MenuBar;
import com.google.gwt.user.client.ui.MenuItem;

public class Navigation extends Composite{
	
	private HorizontalPanel hPanel = new HorizontalPanel();
	private String phrase = "Thanks for selecting this menu item!";
	private MenuBar menu = new MenuBar();
    private MenuBar homeMenu = new MenuBar(true);
    private MenuBar subMenu = new MenuBar(true);
    private MenuBar settingsMenu = new MenuBar(true);
    private MenuBar userMenu = new MenuBar(true);
    private MenuItem closeDatabase;
    private Command closeDatabaseCommand;
    private MenuItem logout;
    private Command logoutCommand;
    private MenuItem textView;
    private Command textViewCommand;
    private MenuItem mapView;
    private Command mapViewCommand;
    private MenuItem graphicalView;
    private Command graphicalViewCommand;
    private MenuItem rawDataView;
    private Command rawDataViewCommand;

	public Navigation() {

		//hPanel.setHeight("60px");
		//hPanel.setWidth("150px");
		//hPanel.setSpacing(15);
		//hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
		hPanel.getElement().setClassName("navigationpanel");
		hPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);
	    
	    
	 // Create a command that will execute on menu item selection
	    Command menuCommand = new Command() {

	      public void execute() {
	        Window.alert(phrase);
	      }
	    };

	    // Configure the menu bar
	    menu.setAutoOpen(true);
	    menu.setAnimationEnabled(true);
	    menu.ensureDebugId("MenuBar");
	    menu.getElement().setClassName("navigation");
	    
        
	    // Create a sub menu
	    subMenu.addItem("Example Submenu", menuCommand);
	    
	    // Create the home menu 
	   // homeMenu.setAnimationEnabled(true);

	    //create an image with safehtml to use in navigation
	   final String home = "<img src='resources/images/house.png' hspace='20px' height='20px' width='20px'/>";
       SafeHtml addActivityImagePath1 = new SafeHtml() {
            @Override
            public String asString() {
                return home;
            }
        };
        
        menu.addItem(new MenuItem(addActivityImagePath1,homeMenu));
        homeMenu.addItem("Example", menuCommand);

	    
        //create an image with safehtml to use in navigation
 	   final String settings = "<img src='resources/images/preferences.png' height='20px' width='20px'/>";
       SafeHtml addActivityImagePath2 = new SafeHtml() {
             @Override
             public String asString() {
                 return settings;
             }
         };
         
         //add items to the settings menu
        textView = new MenuItem("Show Text only", textViewCommand);
        settingsMenu.addItem(textView);
 	    
 	    graphicalView = new MenuItem("Show Graphical View", graphicalViewCommand);
 	    settingsMenu.addItem(graphicalView);
 	    
 	    mapView = new MenuItem("Show Map", mapViewCommand);
 	    settingsMenu.addItem(mapView);
 	    
 	    rawDataView = new MenuItem("Show Raw Data", rawDataViewCommand);
	    settingsMenu.addItem(rawDataView);

	    // Create the settings menu
	    menu.addItem(new MenuItem(addActivityImagePath2, settingsMenu));
	    settingsMenu.addItem("Settings1", menuCommand);
	    settingsMenu.addSeparator();
	    settingsMenu.addItem("Settings2", subMenu);
        settingsMenu.addSeparator();
        settingsMenu.addItem("Settings3", menuCommand);
	    
	  //create an image with safehtml to use in navigation
	  final String user = "<img src='resources/images/user.png' hspace='20px' height='20px' width='20px'/>";
	       SafeHtml addActivityImagePath3 = new SafeHtml() {
	             @Override
	             public String asString() {
	                 return user;
	             }
	         };

	    // Create the user menu
	    closeDatabase = new MenuItem("Close Database", closeDatabaseCommand);
	    userMenu.addItem(closeDatabase);
	    
	    logout = new MenuItem("Logout", logoutCommand);
	    userMenu.addItem(logout);
	    
	    menu.addItem(new MenuItem(addActivityImagePath3, userMenu));
	    
	   //add the menu to the panel for navigation
	   hPanel.add(menu);

	}

	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}

	public MenuBar getHomeMenu() {
		return homeMenu;
	}

	public void setHomeMenu(MenuBar homeMenu) {
		this.homeMenu = homeMenu;
	}

	public MenuBar getSettingsMenu() {
		return settingsMenu;
	}

	public void setSettingsMenu(MenuBar settingsMenu) {
		this.settingsMenu = settingsMenu;
	}
	

	public MenuItem getCloseDatabase() {
		return closeDatabase;
	}

	public void setCloseDatabase(MenuItem closeDatabase) {
		this.closeDatabase = closeDatabase;
	}

	public MenuItem getLogout() {
		return logout;
	}

	public void setLogout(MenuItem logout) {
		this.logout = logout;
	}

	public MenuItem getTextView() {
		return textView;
	}

	public void setTextView(MenuItem textView) {
		this.textView = textView;
	}

	public MenuItem getMapView() {
		return mapView;
	}

	public void setMapView(MenuItem mapView) {
		this.mapView = mapView;
	}

	public MenuItem getGraphicalView() {
		return graphicalView;
	}

	public void setGraphicalView(MenuItem graphicalView) {
		this.graphicalView = graphicalView;
	}

	public MenuItem getRawDataView() {
		return rawDataView;
	}

	public void setRawDataView(MenuItem rawDataView) {
		this.rawDataView = rawDataView;
	}

	
}
