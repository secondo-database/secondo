package viewer.hoese;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.URL;
import java.util.Properties;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JColorChooser;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.WindowConstants;
import javax.swing.BoxLayout;

public class OSMDialog extends JDialog {

	/** UID for serialization */
	private static final long serialVersionUID = 8201724968404205556L;

	public OSMDialog(JFrame parent) {
		super(parent, true); // create a modal dialog

		accepted = true;

		licenseDialog = new LicenseDialog(null);

		// create components for server settings
		selectionCB = new JComboBox<String>();
		protocolTF = new JTextField(8);
		serverTF = new JTextField(24);
		portTF = new JTextField(3);
		directoryTF = new JTextField(30);
		prefixTF = new JTextField(10);
		minZoomLevelTF = new JTextField(3);
		maxZoomLevelTF = new JTextField(3);
		maxDownloadsTF = new JTextField(3);
		tileSizeXTF = new JTextField(4);
		tileSizeYTF = new JTextField(4);
		nameTF = new JTextField(12);
		licenseUrlTF = new JTextField(12);
		warningTF = new JTextField(24);
		mapperClassTF = new JTextField(24);

		showFrames = new JCheckBox("Show tile frames");
		fixedZoom = new JCheckBox("Use fixed zoom levels");
		fixedZoom.setSelected(true);
		showNames = new JCheckBox("Show tile names");
		showCompleteMap = new JCheckBox("Show complete map");
		backgroundColor = new JLabel("   ");
		backgroundColor.setOpaque(true);
		setBgColorBtn = new JButton("Set background color");
		foregroundColor = new JLabel("   ");
		foregroundColor.setBackground(Color.RED);
		foregroundColor.setOpaque(true);
		setFgColorBtn = new JButton("Set frame/name color");
		showLicenseBtn = new JButton("Show License");

		acceptBtn = new JButton("accept");
		resetBtn = new JButton("reset");
		cancelBtn = new JButton("cancel");

		layoutComponents(getContentPane());

		setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE); // don't
																		// allow
																		// closing
																		// by
																		// pressing
																		// "X"

		addKnownServers();
		selectionCB.setSelectedIndex(0);

		origSettings = new Properties();

		storeSettingsToProperties(origSettings);

		addBtnListener();
	}

	/** puts all the contained components at their places. **/
	private void layoutComponents(Container root) {


    JPanel content = new JPanel();
		content.setLayout(new BoxLayout(content,BoxLayout.Y_AXIS));

    JScrollPane sp =  new JScrollPane(content);

    root.add(sp);


		// north ( preset type and name )
		JPanel categoryPanel = new JPanel();
    categoryPanel.setLayout(new BoxLayout(categoryPanel,BoxLayout.X_AXIS));
		categoryPanel.add(new JLabel("Choose preset type:"));
		categoryPanel.add(selectionCB);
		JPanel namePanel = new JPanel();
		namePanel.add(new JLabel("Name:"));
		namePanel.add(nameTF);
		categoryPanel.add(namePanel);
		content.add(categoryPanel);

		// center
		JPanel centerPanel = new JPanel(new BorderLayout());
		JTabbedPane serverSettingsTab = new JTabbedPane();
		JPanel serverSettingsPanel = new JPanel(new GridLayout(9, 1));
		serverSettingsTab.add("Server Settings", serverSettingsPanel);

		serverSettingsPanel.add(new JLabel("Base Settings"));
		JPanel baseSettings1 = new JPanel(new GridLayout(1, 2));
		JPanel baseSettings2 = new JPanel(new GridLayout(1, 2));
		JPanel baseSettings3 = new JPanel(new GridLayout(1, 2));
		JPanel baseSettings4 = new JPanel(new GridLayout(1, 2));
		serverSettingsPanel.add(baseSettings1);
		serverSettingsPanel.add(baseSettings2);
		serverSettingsPanel.add(baseSettings3);
		serverSettingsPanel.add(baseSettings4);
		serverSettingsPanel.add(new JLabel("Zoom Levels"));
		JPanel zoomPanel = new JPanel(new GridLayout(1, 2));
		serverSettingsPanel.add(zoomPanel);
		serverSettingsPanel.add(new JLabel("Tile Dimensions"));
		JPanel tileDimensionsPanel = new JPanel(new GridLayout(1, 2));
		serverSettingsPanel.add(tileDimensionsPanel);

		JPanel protocolPanel = new JPanel();
		protocolPanel.add(new JLabel("Protocol:"));
		protocolPanel.add(protocolTF);
		JPanel serverPanel = new JPanel();
		serverPanel.add(new JLabel("Server:"));
		serverPanel.add(serverTF);
		baseSettings1.add(protocolPanel);
		baseSettings1.add(serverPanel);

		JPanel portPanel = new JPanel();
		portPanel.add(new JLabel("Port:"));
		portPanel.add(portTF);
		JPanel directoryPanel = new JPanel();
		directoryPanel.add(new JLabel("Directory:"));
		directoryPanel.add(directoryTF);
		baseSettings2.add(portPanel);
		baseSettings2.add(directoryPanel);

		JPanel maxDownPanel = new JPanel();
		maxDownPanel.add(new JLabel("Max Parallel Downloads:"));
		maxDownPanel.add(maxDownloadsTF);
		JPanel licenseUrlPanel = new JPanel();
		licenseUrlPanel.add(new JLabel("License's URL:"));
		licenseUrlPanel.add(licenseUrlTF);
		baseSettings3.add(maxDownPanel);
		baseSettings3.add(licenseUrlPanel);

		JPanel mapperPanel = new JPanel();
		mapperPanel.add(new JLabel("Mapper Class:"));
		mapperPanel.add(mapperClassTF);
		baseSettings4.add(mapperPanel);
		JPanel prefixPanel = new JPanel();
		prefixPanel.add(new JLabel("Prefix:"));
		prefixPanel.add(prefixTF);
		baseSettings4.add(prefixPanel);

		JPanel minZoomPanel = new JPanel();
		minZoomPanel.add(new JLabel("Minimum:"));
		minZoomPanel.add(minZoomLevelTF);
		JPanel maxZoomPanel = new JPanel();
		maxZoomPanel.add(new JLabel("Maximum:"));
		maxZoomPanel.add(maxZoomLevelTF);
		zoomPanel.add(minZoomPanel);
		zoomPanel.add(maxZoomPanel);

		JPanel tileXPanel = new JPanel();
		tileXPanel.add(new JLabel("Width (X):"));
		tileXPanel.add(tileSizeXTF);
		tileXPanel.add(new JLabel("pixels"));
		JPanel tileYPanel = new JPanel();
		tileYPanel.add(new JLabel("Height (Y):"));
		tileYPanel.add(tileSizeYTF);
		tileYPanel.add(new JLabel("pixels"));
		tileDimensionsPanel.add(tileXPanel);
		tileDimensionsPanel.add(tileYPanel);

		JTabbedPane displayOptionsTab = new JTabbedPane();
		JPanel displayOptions = new JPanel(new GridLayout(3, 2));
		displayOptionsTab.add("Display Options", displayOptions);
		JPanel showFramesPanel = new JPanel();
		showFramesPanel.add(showFrames);
		displayOptions.add(showFramesPanel);
		JPanel backgroundPanel = new JPanel();
		backgroundPanel.add(backgroundColor);
		backgroundPanel.add(setBgColorBtn);
		displayOptions.add(backgroundPanel);
		JPanel showNamesPanel = new JPanel();
		showNamesPanel.add(showNames);
		displayOptions.add(showNamesPanel);
		JPanel foregroundPanel = new JPanel();
		foregroundPanel.add(foregroundColor);
		foregroundPanel.add(setFgColorBtn);
		displayOptions.add(foregroundPanel);
		JPanel showCompleteMapPanel = new JPanel();
		showCompleteMapPanel.add(showCompleteMap);
    showCompleteMapPanel.add(fixedZoom);
		displayOptions.add(showCompleteMapPanel);

		centerPanel.add(serverSettingsTab, BorderLayout.NORTH);
		centerPanel.add(displayOptionsTab, BorderLayout.CENTER);

		// laber panel
		JTabbedPane infoTab = new JTabbedPane();
		JPanel infoPanel = new JPanel();
		infoTab.add("Information", infoPanel);
		infoPanel.add(new JLabel("Info:"));
		infoPanel.add(warningTF);
		infoPanel.add(new JLabel("  "));
		infoPanel.add(showLicenseBtn);

		centerPanel.add(infoTab, BorderLayout.SOUTH);

		content.add(centerPanel);
		

    // create and add the command panel (south)
		JPanel commandPanel = new JPanel();
		commandPanel.add(acceptBtn);
		commandPanel.add(resetBtn);
		commandPanel.add(cancelBtn);
		content.add(commandPanel);
	}

	/**
	 * Shows this dialog.
	 * 
	 * @return true if the new selection was accepted, false if canceled.
	 **/
	public boolean showDialog() {
		setSize(900,700);
		setVisible(true);
		return accepted;
	}

	/**
	 * adds known servers to the selectionCb. Additionally, a listener is
	 * registered to set the contents of the server setting text fields
	 **/
	private void addKnownServers() {
		selectionCB.addItem("Open Street Map - Mapnik Style");
		selectionCB.addItem("Open Street Map - Osmarender");
		selectionCB.addItem("Open Street Map - Cycle Style");
		selectionCB.addItem("Open Street Map - Maplint Style");
		selectionCB.addItem("Google Maps - Roadmap Style");
		selectionCB.addItem("Google Maps - Satellite Style");
		selectionCB.addItem("Google Maps - Hybrid Style");
		selectionCB.addItem("Google Maps - Terrain Style");
		selectionCB.addItem("Google Maps - Terrain, Streets & Water Style");
		selectionCB.addItem("OutdoorActive.com (Germany only)");
		selectionCB.addItem("OutdoorActive.com (Austria only)");
		selectionCB.addItem("OutdoorActive.com (Southern Tyrolia only)");
		selectionCB.addItem("Eniro maps - Roadmap (Sweden only)");
		selectionCB.addItem("Eniro maps - Topographic (Sweden only)");
		selectionCB.addItem("Eniro maps - Aerial (Sweden only)");
		selectionCB.addItem("MyTopo - Toporaphic (USA only)");
		selectionCB.addItem("Google RoadMap");

		selectionCB.addItem("customized");

		selectionCB.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				String s = OSMDialog.this.selectionCB.getSelectedItem()
						.toString();
				if (s == null) {
					return;
				}
				if (s.equals("Open Street Map - Mapnik Style")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("tile.openstreetmap.org");
					portTF.setText("80");
					directoryTF.setText("");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("18");
					maxDownloadsTF.setText("2");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("OSM Mapnik");
					licenseUrlTF
							.setText("http://wiki.openstreetmap.org/wiki/Tile_usage_policy");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("Open Street Map - Osmarender")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("tah.openstreetmap.org");
					portTF.setText("80");
					directoryTF.setText("Tiles/tile/");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("17");
					maxDownloadsTF.setText("2");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("OSM Osmarender");
					licenseUrlTF
							.setText("http://wiki.openstreetmap.org/wiki/Tile_usage_policy");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("Open Street Map - Cycle Style")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("andy.sandbox.cloudmade.com");

					portTF.setText("80");
					directoryTF.setText("tiles/cycle");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("18");
					maxDownloadsTF.setText("2");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("OSM Cycle");
					licenseUrlTF
							.setText("http://wiki.openstreetmap.org/wiki/Tile_usage_policy");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("Open Street Map - Maplint Style")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("tah.openstreetmap.org");

					portTF.setText("80");
					directoryTF.setText("Tiles/maplint");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("18");
					maxDownloadsTF.setText("2");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("OSM Maplint");
					licenseUrlTF
							.setText("http://wiki.openstreetmap.org/wiki/Tile_usage_policy");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("Google Maps - Roadmap Style")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("mt0.google.com");
					;
					portTF.setText("80");
					directoryTF.setText("vt/");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("10");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Google RoadMap");
					licenseUrlTF
							.setText("http://code.google.com/intl/de-DE/apis/maps/terms.html");
					mapperClassTF
							.setText("viewer.hoese.StaticGoogleMapsMapper");

				} else if (s.equals("Google Maps - Satellite Style")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("khm1.google.com");
					;
					portTF.setText("80");
					directoryTF.setText("kh");
					prefixTF.setText("v=49&");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("10");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Google SatelliteMap");
					licenseUrlTF
							.setText("http://code.google.com/intl/de-DE/apis/maps/terms.html");
					mapperClassTF
							.setText("viewer.hoese.StaticGoogleMapsMapper");

				} else if (s.equals("Google Maps - Hybrid Style")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("mt1.google.com");
					;
					portTF.setText("80");
					directoryTF.setText("vt");
					prefixTF.setText("lyrs=y&");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("10");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Google HybridMap");
					licenseUrlTF
							.setText("http://code.google.com/intl/de-DE/apis/maps/terms.html");
					mapperClassTF
							.setText("viewer.hoese.StaticGoogleMapsMapper");

				} else if (s.equals("Google Maps - Terrain Style")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("khm.google.com");
					;
					portTF.setText("80");
					directoryTF.setText("vt/lbw");
					prefixTF.setText("lyrs=p&");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("16");
					maxDownloadsTF.setText("10");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Google TerrainMap");
					licenseUrlTF
							.setText("http://code.google.com/intl/de-DE/apis/maps/terms.html");
					mapperClassTF
							.setText("viewer.hoese.StaticGoogleMapsMapper");

				} else if (s
						.equals("Google Maps - Terrain, Streets & Water Style")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("mt1.google.com");
					;
					portTF.setText("80");
					directoryTF.setText("vt");
					prefixTF.setText("lyrs=p&");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("15");
					maxDownloadsTF.setText("10");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Google TerrainStreetsWaterMap");
					licenseUrlTF
							.setText("http://code.google.com/intl/de-DE/apis/maps/terms.html");
					mapperClassTF
							.setText("viewer.hoese.StaticGoogleMapsMapper");

				} else if (s.equals("OutdoorActive.com (Germany only)")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("t0.outdooractive.com");
					;
					portTF.setText("80");
					directoryTF.setText("portal/map");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("20");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("OutdoorActive.com (Deutschland)");
					licenseUrlTF.setText("http://www.outdooractive.com/");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("OutdoorActive.com (Austria only)")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("t0.outdooractive.com");
					;
					portTF.setText("80");
					directoryTF.setText("austria/map");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("20");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("OutdoorActive.com (Österreich)");
					licenseUrlTF.setText("http://www.outdooractive.com/");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s
						.equals("OutdoorActive.com (Southern Tyrolia only)")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("t0.outdooractive.com");
					;
					portTF.setText("80");
					directoryTF.setText("suedtirol/map");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("20");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("OutdoorActive.com (Südtyrol)");
					licenseUrlTF.setText("http://www.outdooractive.com/");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("Eniro maps - Roadmap (Sweden only)")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("kartat.02.fi");
					;
					portTF.setText("80");
					directoryTF.setText("tiles/maps");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("20");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Eniro Roadmap (Sweden)");
					licenseUrlTF.setText("");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("Eniro maps - Topographic (Sweden only)")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("kartat.02.fi");
					;
					portTF.setText("80");
					directoryTF.setText("tiles/maps");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("20");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Eniro Topographic (Sweden)");
					licenseUrlTF.setText("");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("Eniro maps - Aerial (Sweden only)")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("kartat.02.fi");
					;
					portTF.setText("80");
					directoryTF.setText("tiles/topographic");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("20");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Eniro Aerial (Sweden)");
					licenseUrlTF.setText("");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("MyTopo - Toporaphic (USA only)")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("maps.mytopo.com");
					;
					portTF.setText("80");
					directoryTF.setText("groundspeak/tilecache.py/1.0.0/topoG");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("20");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("MyTopo TopographicMap (USA)");
					licenseUrlTF.setText("");
					mapperClassTF.setText("viewer.hoese.StaticOSMMapper");

				} else if (s.equals("Google RoadMap")) {
					OSMDialog.this.enableServerSettings(false);
					protocolTF.setText("http");
					serverTF.setText("maps.google.com");
					;
					portTF.setText("80");
					directoryTF.setText("maps/api");
					prefixTF.setText("");

					minZoomLevelTF.setText("1");
					maxZoomLevelTF.setText("21");
					maxDownloadsTF.setText("2");

					tileSizeXTF.setText("256");
					tileSizeYTF.setText("256");
					nameTF.setText("Google Roadmap)");
					licenseUrlTF.setText("");
					mapperClassTF.setText("viewer.hoese.StaticGoogleMapsMapper2");
				} else if (s.equals("customized")) {
					OSMDialog.this.enableServerSettings(true);
					licenseUrlTF.setText("");
				} else {
					System.err
							.println("Fatal System Error, "
									+ "Please turn off your computer and never switch it on again!\n"
									+ "After that burn down your computer to avoid infections from computer viruses.");
				}
			}

		});
	}

	/** enables edititing of servers setting depening on the argument **/
	private void enableServerSettings(boolean on) {
		protocolTF.setEditable(on);
		serverTF.setEditable(on);
		portTF.setEditable(on);
		directoryTF.setEditable(on);
		prefixTF.setEditable(on);
		licenseUrlTF.setEditable(on);

		minZoomLevelTF.setEditable(on);
		maxZoomLevelTF.setEditable(on);
		maxDownloadsTF.setEditable(on);

		tileSizeXTF.setEditable(on);
		tileSizeYTF.setEditable(on);
		nameTF.setEditable(on);
		mapperClassTF.setEditable(on);
	}

	/** stores the current settings to a Properties object **/
	public void storeSettingsToProperties(Properties settings) {
		trimValues();
		settings.setProperty(OSMBackground.KEY_SELECTION, selectionCB
				.getSelectedItem().toString());
		settings.setProperty(OSMBackground.KEY_MAPPERCLASS,
				mapperClassTF.getText());

		settings.setProperty(OSMBackground.KEY_PROTOCOL, protocolTF.getText());
		settings.setProperty(OSMBackground.KEY_SERVER, serverTF.getText());
		settings.setProperty(OSMBackground.KEY_PORT, portTF.getText());
		settings.setProperty(OSMBackground.KEY_DIRECTORY, directoryTF.getText());
		settings.setProperty(OSMBackground.KEY_PREFIX, prefixTF.getText());

		settings.setProperty(OSMBackground.KEY_MINZOOMLEVEL,
				minZoomLevelTF.getText());
		settings.setProperty(OSMBackground.KEY_MAXZOOMLEVEL,
				maxZoomLevelTF.getText());
		settings.setProperty(OSMBackground.KEY_MAXDOWNLOADS,
				maxDownloadsTF.getText());
		settings.setProperty(OSMBackground.KEY_LICENSEURL,
				licenseUrlTF.getText());

		settings.setProperty(OSMBackground.KEY_TILESIZEX, tileSizeXTF.getText());
		settings.setProperty(OSMBackground.KEY_TILESIZEY, tileSizeYTF.getText());
		settings.setProperty(OSMBackground.KEY_NAME, nameTF.getText());

		settings.setProperty(OSMBackground.KEY_SHOWFRAMES,
				(showFrames.isSelected() ? "TRUE" : "FALSE"));
	
   	settings.setProperty(OSMBackground.KEY_FIXEDZOOM,
				(fixedZoom.isSelected() ? "TRUE" : "FALSE"));
		
		settings.setProperty(OSMBackground.KEY_SHOWNAMES,
				(showNames.isSelected() ? "TRUE" : "FALSE"));
		settings.setProperty(Background.KEY_USEFORBBOX,
				(showCompleteMap.isSelected() ? "TRUE" : "FALSE"));
		Color c;
		if ((c = backgroundColor.getBackground()) != null) {
			settings.setProperty(OSMBackground.KEY_BACKGROUNDCOLOR,
					("" + c.getRGB()));
		}
		if ((c = foregroundColor.getBackground()) != null) {
			settings.setProperty(OSMBackground.KEY_FOREGROUNDCOLOR,
					("" + c.getRGB()));
		}
	}

	/** puts the values from origSettings into the textFields **/
	public void restoreSettingsFromProperties(Properties properties) {

		selectionCB.setSelectedItem(properties
				.getProperty(OSMBackground.KEY_SELECTION));
		mapperClassTF.setText(properties
				.getProperty(OSMBackground.KEY_MAPPERCLASS));

		protocolTF.setText(properties.getProperty(OSMBackground.KEY_PROTOCOL));
		serverTF.setText(properties.getProperty(OSMBackground.KEY_SERVER));

		portTF.setText(properties.getProperty(OSMBackground.KEY_PORT));
		directoryTF
				.setText(properties.getProperty(OSMBackground.KEY_DIRECTORY));
		prefixTF.setText(properties.getProperty(OSMBackground.KEY_PREFIX));
		minZoomLevelTF.setText(properties
				.getProperty(OSMBackground.KEY_MINZOOMLEVEL));
		maxZoomLevelTF.setText(properties
				.getProperty(OSMBackground.KEY_MAXZOOMLEVEL));
		maxDownloadsTF.setText(properties
				.getProperty(OSMBackground.KEY_MAXDOWNLOADS));
		licenseUrlTF.setText(properties
				.getProperty(OSMBackground.KEY_LICENSEURL));

		tileSizeXTF
				.setText(properties.getProperty(OSMBackground.KEY_TILESIZEX));
		tileSizeYTF
				.setText(properties.getProperty(OSMBackground.KEY_TILESIZEY));
		nameTF.setText(properties.getProperty(OSMBackground.KEY_NAME));

    fixedZoom.setSelected("TRUE".equals(properties.getProperty(OSMBackground.KEY_FIXEDZOOM)));

		showFrames.setSelected(properties.getProperty(
				OSMBackground.KEY_SHOWFRAMES).equals("TRUE"));
		showNames.setSelected(properties.getProperty(
				OSMBackground.KEY_SHOWNAMES).equals("TRUE"));
		showCompleteMap.setSelected(properties.getProperty(
				Background.KEY_USEFORBBOX).equals("TRUE"));

		String colorstr = properties
				.getProperty(OSMBackground.KEY_BACKGROUNDCOLOR);
		if (colorstr == null) {
			backgroundColor.setBackground(null);
		} else {
			try {
				int rgb = Integer.parseInt(colorstr);
				backgroundColor.setBackground(new Color(rgb));
			} catch (Exception e) {
				backgroundColor.setBackground(Color.PINK);
			}
		}

		colorstr = properties.getProperty(OSMBackground.KEY_FOREGROUNDCOLOR);
		if (colorstr == null) {
			foregroundColor.setBackground(null);
		} else {
			try {
				int rgb = Integer.parseInt(colorstr);
				foregroundColor.setBackground(new Color(rgb));
			} catch (Exception e) {
				foregroundColor.setBackground(Color.PINK);
			}
		}
		if (!checkContents()) {
			setToDefault();
		}
	}

	/** Adds listener to all contained buttons. **/
	private void addBtnListener() {
		acceptBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				if (OSMDialog.this.checkContents()) {
					OSMDialog.this.accepted = true;
					setVisible(false);
				}
			}
		});
		cancelBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				OSMDialog.this.restoreSettingsFromProperties(origSettings);
				OSMDialog.this.accepted = false;
				setVisible(false);
			}
		});

		setBgColorBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				Color c = JColorChooser.showDialog(OSMDialog.this,
						"Choose a background color",
						backgroundColor.getBackground());
				if (c != null) {
					backgroundColor.setBackground(c);
				}
			}
		});

		setFgColorBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				Color c = JColorChooser.showDialog(OSMDialog.this,
						"Choose a frame/name color",
						foregroundColor.getBackground());
				if (c != null) {
					foregroundColor.setBackground(c);
				}
			}
		});

		resetBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				OSMDialog.this.restoreSettingsFromProperties(origSettings);
			}
		});

		showLicenseBtn.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				String lurl = licenseUrlTF.getText().trim();
				if (lurl.length() == 0) {
					JOptionPane.showMessageDialog(null, "License unknown");
				} else {
					try {
						URL licenseUrl = new URL(lurl);
						if (licenseDialog.setSource(licenseUrl)) {
							licenseDialog.setVisible(true);
						} else {
							JOptionPane.showMessageDialog(null,
									"Cannot load license from " + licenseUrl);
						}
					} catch (Exception e) {
						JOptionPane.showMessageDialog(null, lurl
								+ " is not a valid url.");
					}
				}
			}
		});

	}

	public boolean checkContents() {
		trimValues();

		int port;
		try {
			port = Integer.parseInt(portTF.getText());
			if (port <= 0) {
				JOptionPane.showMessageDialog(this,
						"Port number must be greater than -1.");
				portTF.requestFocus();
				return false;
			}
		} catch (Exception e) {
			JOptionPane.showMessageDialog(this,
					"Port number has to be a number..");
			portTF.requestFocus();
			return false;
		}

		try {
			new URL(protocolTF.getText(), serverTF.getText(), port,
					directoryTF.getText());
		} catch (Exception e) {
			JOptionPane
					.showMessageDialog(this,
							"Cannot build a URL from protocol, server, port, and directory.");
			protocolTF.requestFocus();
			return false;
		}

		int minZoomLevel;
		try {
			minZoomLevel = Integer.parseInt(minZoomLevelTF.getText());
			if (minZoomLevel < 1) {
				JOptionPane.showMessageDialog(this,
						"Minimum zoom must be at least 1.");
				minZoomLevelTF.requestFocus();
				return false;
			}
		} catch (Exception e) {
			JOptionPane.showMessageDialog(this,
					"Minimum zoom must be a number.");
			minZoomLevelTF.requestFocus();
			return false;
		}

		try {
			int maxZoomLevel = Integer.parseInt(minZoomLevelTF.getText());
			if (maxZoomLevel < minZoomLevel) {
				JOptionPane.showMessageDialog(this,
						"Maximum zoom must be at least equal to minimum zoom.");
				maxZoomLevelTF.requestFocus();
				return false;
			}
		} catch (Exception e) {
			JOptionPane.showMessageDialog(this,
					"Maximum zoom must be a number.");
			maxZoomLevelTF.requestFocus();
			return false;
		}

		int tileSizeX;
		try {
			tileSizeX = Integer.parseInt(tileSizeXTF.getText());
			if (tileSizeX < 1) {
				JOptionPane.showMessageDialog(this,
						"Tile size must be greater than zero.");
				tileSizeXTF.requestFocus();
				return false;
			}
		} catch (Exception e) {
			JOptionPane.showMessageDialog(this, "Tile size must be a number.");
			tileSizeXTF.requestFocus();
			return false;
		}

		int tileSizeY;
		try {
			tileSizeY = Integer.parseInt(tileSizeYTF.getText());
			if (tileSizeY < 1) {
				JOptionPane.showMessageDialog(this,
						"Tile size must be greater than zero.");
				tileSizeYTF.requestFocus();
				return false;
			}
		} catch (Exception e) {
			JOptionPane.showMessageDialog(this, "Tile size must be a number.");
			tileSizeYTF.requestFocus();
			return false;
		}

		if (nameTF.getText().length() == 0) {
			JOptionPane
					.showMessageDialog(this, "An empty name is not allowed.");
			nameTF.requestFocus();
			return false;
		}

		try {
			Class c = Class.forName(mapperClassTF.getText());
			Class[] interfaces = c.getInterfaces();
			Class inter = Class.forName("viewer.hoese.Rect2UrlMapper");
			boolean found = false;
			for (int i = 0; i < interfaces.length; i++) {
				if (inter.equals(inter)) {
					found = true;
				}
			}
			if (!found) {
				JOptionPane
						.showMessageDialog(this,
								"Invalid Mapper class (needs to implement interface Rect2UrlMapper)!");
				mapperClassTF.requestFocus();
				return false;
			}
			c.getConstructor(new Class[] { int.class, int.class, int.class,
					int.class, int.class, int.class, URL.class, String.class });
		} catch (Exception e) {
			JOptionPane
					.showMessageDialog(this,
							"Invalid Mapper class (need a constructor(int, int, int, int, int, int, URL, String)!");
			mapperClassTF.requestFocus();
			e.printStackTrace();
			return false;
		}
		return true;
	}

	public void setToDefault() {
		selectionCB.setSelectedIndex(0);
	}

	private void trimValues() {
		protocolTF.setText(protocolTF.getText().trim());
		serverTF.setText(serverTF.getText().trim());
		portTF.setText(portTF.getText().trim());

		String dir = directoryTF.getText().trim();
		if (!dir.startsWith("/")) {
			dir = "/" + dir;
		}

		if (!dir.endsWith("/")) {
			dir += "/";
		}

		directoryTF.setText(dir.trim());

		prefixTF.setText(prefixTF.getText().trim());

		minZoomLevelTF.setText(minZoomLevelTF.getText().trim());
		maxZoomLevelTF.setText(maxZoomLevelTF.getText().trim());
		maxDownloadsTF.setText(maxDownloadsTF.getText().trim());

		tileSizeXTF.setText(tileSizeXTF.getText().trim());
		tileSizeYTF.setText(tileSizeYTF.getText().trim());
		nameTF.setText(nameTF.getText().trim());
	}

	public Properties getSettings() {
		Properties res = new Properties();
		storeSettingsToProperties(res);
		return res;
	}

	// fields for server settings
	private JComboBox<String> selectionCB; // selection of predefined server settings +
									// customize

	private JTextField protocolTF;
	private JTextField serverTF;
	private JTextField portTF;
	private JTextField directoryTF;
	private JTextField prefixTF;

	private JTextField minZoomLevelTF;
	private JTextField maxZoomLevelTF;
	private JTextField maxDownloadsTF;

	private JTextField tileSizeXTF;
	private JTextField tileSizeYTF;
	private JTextField nameTF;
	private JTextField licenseUrlTF;

	private JTextField mapperClassTF;

	private JTextField warningTF;

	// display and cache settings
	private JCheckBox showFrames;
	private JCheckBox fixedZoom;
	private JCheckBox showNames;
	private JCheckBox showCompleteMap;
	private JLabel backgroundColor;
	private JButton setBgColorBtn;
	private JLabel foregroundColor;
	private JButton setFgColorBtn;

	private JButton showLicenseBtn;

	// command buttons
	private JButton acceptBtn;
	private JButton resetBtn;
	private JButton cancelBtn;

	private boolean accepted;
	private Properties origSettings;

	private LicenseDialog licenseDialog;

}
