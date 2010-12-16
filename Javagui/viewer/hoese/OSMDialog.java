

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
import javax.swing.JTextField;
import javax.swing.WindowConstants;


public class OSMDialog extends JDialog{


  public OSMDialog(JFrame parent){
     super(parent,true); // create a modal dialog

     accepted = true;

     licenseDialog = new LicenseDialog(null);
    
     // create components for server settings
     selectionCB    = new JComboBox();
     protocolTF     = new JTextField(8);
     serverTF       = new JTextField(30);
     portTF         = new JTextField(3);
     directoryTF    = new JTextField(30);
     minZoomLevelTF = new JTextField(3);
     maxZoomLevelTF = new JTextField(3);
     maxDownloadsTF = new JTextField(3);
     tileSizeXTF    = new JTextField(4);
     tileSizeYTF    = new JTextField(4);
     nameTF         = new JTextField(12);
     licenseUrlTF   = new JTextField(12);
     warningTF      = new JTextField(30);
      

     showFrames = new JCheckBox("show frames");
     showNames = new JCheckBox("show tile names");
     backgroundColor = new JLabel("   ");
     backgroundColor.setOpaque(true);
     setBgColorBtn = new JButton("set background color");
     foregroundColor = new JLabel("   ");
     foregroundColor.setBackground(Color.RED);
     foregroundColor.setOpaque(true);
     setFgColorBtn = new JButton("set frame/name color");
     showLicenseBtn = new JButton("show License");


     acceptBtn = new JButton("accept");
     resetBtn = new JButton("reset");
     cancelBtn = new JButton("cancel");


    layoutComponents(getContentPane());








     setDefaultCloseOperation( WindowConstants.DO_NOTHING_ON_CLOSE ); // don't allow closing by pressing "X"

     addKnownServers();
     selectionCB.setSelectedIndex(0);

     origSettings = new Properties();

     readSettings(origSettings);

     addBtnListener();
     
  }

  /** puts all the contained components at their places. **/
  private void layoutComponents(Container root){
     root.setLayout(new BorderLayout());

     // create and add the command panel (south)
     JPanel commandPanel = new JPanel();
     commandPanel.add(acceptBtn);
     commandPanel.add(resetBtn);
     commandPanel.add(cancelBtn);
     root.add(commandPanel,BorderLayout.SOUTH);

     // north ( preset type and name )
     JPanel categoryPanel = new JPanel(new GridLayout(1,2));
     JPanel selectionPanel = new JPanel();
     selectionPanel.add(new JLabel("Choose preset type:"));
     selectionPanel.add(selectionCB);
     categoryPanel.add(selectionPanel);
     JPanel namePanel = new JPanel();
     namePanel.add(new JLabel("Name:"));
     namePanel.add(nameTF);
     categoryPanel.add(namePanel);
     root.add(categoryPanel, BorderLayout.NORTH);

     // center
     JPanel centerPanel = new JPanel(new BorderLayout());
     JTabbedPane serverSettingsTab = new JTabbedPane();
     JPanel serverSettingsPanel = new JPanel(new GridLayout(8,1));
     serverSettingsTab.add("Server Settings", serverSettingsPanel);

     serverSettingsPanel.add(new JLabel("Base Settings"));
     JPanel baseSettings1 = new JPanel(new GridLayout(1,2));
     JPanel baseSettings2 = new JPanel(new GridLayout(1,2));
     JPanel baseSettings3 = new JPanel(new GridLayout(1,2));
     serverSettingsPanel.add(baseSettings1);
     serverSettingsPanel.add(baseSettings2);
     serverSettingsPanel.add(baseSettings3);
     serverSettingsPanel.add(new JLabel("Zoom Levels"));
     JPanel zoomPanel = new JPanel(new GridLayout(1,2));
     serverSettingsPanel.add(zoomPanel);
     serverSettingsPanel.add(new JLabel("Tile Dimensions"));
     JPanel tileDimensionsPanel = new JPanel(new GridLayout(1,2));
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
     JPanel displayOptions = new JPanel(new GridLayout(2,2));
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

     centerPanel.add(serverSettingsTab, BorderLayout.NORTH);
     centerPanel.add(displayOptionsTab,BorderLayout.CENTER);

     // laber panel
     JTabbedPane infoTab = new JTabbedPane();
     JPanel infoPanel = new JPanel();
     infoTab.add("Information", infoPanel);
     infoPanel.add(new JLabel("Info:")); 
     infoPanel.add(warningTF);
     infoPanel.add(new JLabel("  "));
     infoPanel.add(showLicenseBtn);
     

     centerPanel.add(infoTab,BorderLayout.SOUTH);

     root.add(centerPanel, BorderLayout.CENTER);
  }




  /** Shows this dialog.
    * @return true if the new selection was accepted, false if canceled.
    **/
  public boolean showDialog(){
      setSize(900,300);
      setVisible(true);
      return accepted;
  }



  /** adds known servers to the selectionCb.
    * Additionally, a listener is registered to set the contents of the
    * server setting text fields
    **/
  private void addKnownServers(){
     selectionCB.addItem("open street map mapnik");
     selectionCB.addItem("open street map osmarender");
     selectionCB.addItem("open street map cycle");
     selectionCB.addItem("customized");

     selectionCB.addActionListener( new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          String s = OSMDialog.this.selectionCB.getSelectedItem().toString();
          if(s==null){
             return;
          }
          if(s.equals("open street map mapnik")){
             OSMDialog.this.enableServerSettings(false);
             protocolTF.setText("http");
             serverTF.setText("tile.openstreetmap.org");
             portTF.setText("80");
             directoryTF.setText("");

             minZoomLevelTF.setText("1");
             maxZoomLevelTF.setText("18");
             maxDownloadsTF.setText("2");

             tileSizeXTF.setText("256");
             tileSizeYTF.setText("256");
             nameTF.setText("Mapnik");
             licenseUrlTF.setText("http://dna.fernuni-hagen.de/secondo");
          } else if(s.equals("open street map osmarender")){
             OSMDialog.this.enableServerSettings(false);
             protocolTF.setText("http");
             serverTF.setText("tah.openstreetmap.org");
             portTF.setText("80");
             directoryTF.setText("Tiles/tile/");

             minZoomLevelTF.setText("1");
             maxZoomLevelTF.setText("17");
             maxDownloadsTF.setText("2");

             tileSizeXTF.setText("256");
             tileSizeYTF.setText("256");
             nameTF.setText("Osmarender");
             licenseUrlTF.setText("http://dna.fernuni-hagen.de/christian.html");
          } else if(s.equals("open street map cycle")){
             OSMDialog.this.enableServerSettings(false);
             protocolTF.setText("http");
             serverTF.setText("andy.sandbox.cloudmade.com");;
             portTF.setText("80");
             directoryTF.setText("tiles/cycle");

             minZoomLevelTF.setText("1");
             maxZoomLevelTF.setText("18");
             maxDownloadsTF.setText("2");

             tileSizeXTF.setText("256");
             tileSizeYTF.setText("256");
             nameTF.setText("Cycle");
             licenseUrlTF.setText("http://filmclub-bali.bplaced.net/bali.php");
          } else if(s.equals("customized")){
             OSMDialog.this.enableServerSettings(true);
             licenseUrlTF.setText("");
          } else {
             System.err.println("Fatal System Error, "+
                                "Please turn off your computer and never switch it on again!\n"+
                                "After that burn down your computer to avoid infections from computer viruses.");
          }
       }

     });
  }

   /** enables edititing of servers setting depening on the argument **/
   private void enableServerSettings(boolean on){
     protocolTF.setEditable(on);
     serverTF.setEditable(on);
     portTF.setEditable(on);
     directoryTF.setEditable(on);
     licenseUrlTF.setEditable(on);

     minZoomLevelTF.setEditable(on);
     maxZoomLevelTF.setEditable(on);
     maxDownloadsTF.setEditable(on);

     tileSizeXTF.setEditable(on);
     tileSizeYTF.setEditable(on);
     nameTF.setEditable(on);
   }

   
  /** reads the current values into origSettings **/
  public void readSettings(Properties settings){
      trimValues();
      settings.setProperty(OSMBackground.KEY_SELECTION, selectionCB.getSelectedItem().toString());

      settings.setProperty(OSMBackground.KEY_PROTOCOL, protocolTF.getText() );
      settings.setProperty(OSMBackground.KEY_SERVER, serverTF.getText() );
      settings.setProperty(OSMBackground.KEY_PORT,portTF.getText());
		settings.setProperty(OSMBackground.KEY_DIRECTORY, directoryTF.getText());

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

      settings.setProperty(OSMBackground.KEY_SHOWFRAMES,(showFrames.isSelected()?"TRUE":"FALSE"));;
      settings.setProperty(OSMBackground.KEY_SHOWNAMES,(showNames.isSelected()?"TRUE":"FALSE"));
      Color c;
      if(( c = backgroundColor.getBackground())!=null){
         settings.setProperty(OSMBackground.KEY_BACKGROUNDCOLOR, ("" + c.getRGB()));
      }
      if( (c = foregroundColor.getBackground() ) !=null){
        settings.setProperty(OSMBackground.KEY_FOREGROUNDCOLOR, ("" + c.getRGB()));
      }
  }  


  /** puts the values from origSettings into the textFields **/
  public void reset(Properties properties){

      selectionCB.setSelectedItem(properties.getProperty(OSMBackground.KEY_SELECTION)); 

      protocolTF.setText(properties.getProperty(OSMBackground.KEY_PROTOCOL));
      serverTF.setText(properties.getProperty(OSMBackground.KEY_SERVER));
 
      portTF.setText(properties.getProperty(OSMBackground.KEY_PORT));
      directoryTF.setText(properties.getProperty(OSMBackground.KEY_DIRECTORY));

      minZoomLevelTF.setText(properties.getProperty(OSMBackground.KEY_MINZOOMLEVEL));
      maxZoomLevelTF.setText(properties.getProperty(OSMBackground.KEY_MAXZOOMLEVEL));
      maxDownloadsTF.setText(properties.getProperty(OSMBackground.KEY_MAXDOWNLOADS));

      tileSizeXTF.setText(properties.getProperty(OSMBackground.KEY_TILESIZEX));
      tileSizeYTF.setText(properties.getProperty(OSMBackground.KEY_TILESIZEY));
      nameTF.setText(properties.getProperty(OSMBackground.KEY_NAME));


      showFrames.setSelected( properties.getProperty(OSMBackground.KEY_SHOWFRAMES).equals("TRUE"));
      showNames.setSelected( properties.getProperty(OSMBackground.KEY_SHOWNAMES).equals("TRUE"));

      String colorstr = properties.getProperty(OSMBackground.KEY_BACKGROUNDCOLOR);    
      if(colorstr==null){
        backgroundColor.setBackground(null);
      } else {
         try{
            int rgb = Integer.parseInt(colorstr);
            backgroundColor.setBackground(new Color(rgb));
         } catch(Exception e){
           backgroundColor.setBackground(Color.PINK);
         }
      }

      colorstr = properties.getProperty(OSMBackground.KEY_FOREGROUNDCOLOR);    
      if(colorstr==null){
        foregroundColor.setBackground(null);
      } else {
         try{
            int rgb = Integer.parseInt(colorstr);
            foregroundColor.setBackground(new Color(rgb));
         } catch(Exception e){
            foregroundColor.setBackground(Color.PINK);
         }
      }
      if(!checkContents()){
          setToDefault();
      }
  }

  /** Adds listener to all contained buttons. **/
  private void addBtnListener(){
    acceptBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
         if(OSMDialog.this.checkContents()){
            OSMDialog.this.accepted = true;
            setVisible(false);
         }
       }
    });
    cancelBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
         OSMDialog.this.reset(origSettings);
         OSMDialog.this.accepted = false;
         setVisible(false);
       }
    });

    setBgColorBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
          Color c = JColorChooser.showDialog(OSMDialog.this,"Choose a background color", backgroundColor.getBackground());
          if(c!=null){
             backgroundColor.setBackground(c);
          }
        }
    });
    
    setFgColorBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
          Color c = JColorChooser.showDialog(OSMDialog.this,"Choose a frame/name color", foregroundColor.getBackground());
          if(c!=null){
             foregroundColor.setBackground(c);
          }
        }
    });

    resetBtn.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         OSMDialog.this.reset(origSettings);
      }
    });

    showLicenseBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          String lurl = licenseUrlTF.getText().trim();
          if(lurl.length()==0){
               JOptionPane.showMessageDialog(null, "License unknown");
          } else {
             try{
                URL licenseUrl = new URL(lurl);
                if(licenseDialog.setSource(licenseUrl)){
                   licenseDialog.setVisible(true);
                } else {
                   JOptionPane.showMessageDialog(null, "Cannot load license from " + licenseUrl);
                }
             } catch(Exception e){
                JOptionPane.showMessageDialog(null, lurl +" is not a valid url.");
             }
         }
       }
    });


  }


  public boolean checkContents(){
    trimValues();
 
    int port;
    try{
      port = Integer.parseInt(portTF.getText());
      if(port<=0){
         JOptionPane.showMessageDialog(this, "Port number must be greater than -1.");
         portTF.requestFocus();
         return false;
      }
    } catch(Exception e){
         JOptionPane.showMessageDialog(this, "Port number has to be a number..");
         portTF.requestFocus();
         return false;
    }   

    try{
			URL url = new URL(protocolTF.getText(), serverTF.getText(), port,
					directoryTF.getText());
    } catch(Exception e){
        JOptionPane.showMessageDialog(this, "Cannot build a URL from protocol, server, port, and directory.");
        protocolTF.requestFocus();
        return false;
    }

    int minZoomLevel;
    try{
       minZoomLevel = Integer.parseInt(minZoomLevelTF.getText());
       if(minZoomLevel < 1){
          JOptionPane.showMessageDialog(this, "Minimum zoom must be at least 1.");
          minZoomLevelTF.requestFocus();
          return false;
       }
    } catch(Exception e){
       JOptionPane.showMessageDialog(this, "Minimum zoom must be a number.");
       minZoomLevelTF.requestFocus();
       return false;
    }


    try{
       int maxZoomLevel = Integer.parseInt(minZoomLevelTF.getText());
       if(maxZoomLevel < minZoomLevel){
          JOptionPane.showMessageDialog(this, "Maximum zoom must be at least equal to minimum zoom.");
          maxZoomLevelTF.requestFocus();
          return false;
       }
    } catch(Exception e){
       JOptionPane.showMessageDialog(this, "Maximum zoom must be a number.");
       maxZoomLevelTF.requestFocus();
       return false;
    }


   int tileSizeX;
   try{
      tileSizeX = Integer.parseInt(tileSizeXTF.getText());
      if(tileSizeX < 1){
        JOptionPane.showMessageDialog(this, "Tile size must be greater than zero.");
        tileSizeXTF.requestFocus();
        return false;
      }
   } catch(Exception e){
			JOptionPane.showMessageDialog(this, "Tile size must be a number.");
			tileSizeXTF.requestFocus();
			return false;
   }

   int tileSizeY;
   try{
      tileSizeY = Integer.parseInt(tileSizeYTF.getText());
      if(tileSizeY < 1){
        JOptionPane.showMessageDialog(this, "Tile size must be greater than zero.");
        tileSizeYTF.requestFocus();
        return false;
      }
   } catch(Exception e){
			JOptionPane.showMessageDialog(this, "Tile size must be a number.");
			tileSizeYTF.requestFocus();
			return false;
   }

   if(nameTF.getText().length()==0){
			JOptionPane.showMessageDialog(this, "An empty name is not allowed.");
			nameTF.requestFocus();
			return false;
   }
   return true;
 } 

 public void setToDefault(){
    selectionCB.setSelectedIndex(0);
 }


 private void trimValues(){
    protocolTF.setText(protocolTF.getText().trim());
    serverTF.setText(serverTF.getText().trim());
    portTF.setText(portTF.getText().trim());


    String dir = directoryTF.getText().trim();
    if(!dir.startsWith("/")){
       dir = "/"+dir;
    }
    

    if(!dir.endsWith("/")){
      dir +="/";
    }

    directoryTF.setText(dir.trim());
    
    


    minZoomLevelTF.setText(minZoomLevelTF.getText().trim());
    maxZoomLevelTF.setText(maxZoomLevelTF.getText().trim());
    maxDownloadsTF.setText(maxDownloadsTF.getText().trim());

    tileSizeXTF.setText(tileSizeXTF.getText().trim());
    tileSizeYTF.setText(tileSizeYTF.getText().trim());
    nameTF.setText(nameTF.getText().trim());
 } 


  public Properties getSettings(){
    Properties res = new Properties();
    readSettings(res);
    return res;
  }



  // fields for server settings
  private JComboBox selectionCB;  // selection of predefined server settings + customize

  private JTextField protocolTF;
  private JTextField serverTF;
  private JTextField portTF;
  private JTextField directoryTF;

  private JTextField minZoomLevelTF;
  private JTextField maxZoomLevelTF;
  private JTextField maxDownloadsTF;

  private JTextField tileSizeXTF;
  private JTextField tileSizeYTF;
  private JTextField nameTF;
  private JTextField licenseUrlTF;

  private JTextField warningTF;

  // display and cache settings
  private JCheckBox showFrames;
  private JCheckBox showNames;
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
