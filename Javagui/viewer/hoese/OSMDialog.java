

package viewer.hoese;


import javax.swing.*;
import java.util.Properties;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;


public class OSMDialog extends JDialog{


  public OSMDialog(JFrame parent){
     super(parent,true); // create a modal dialog

     accepted = true;
    
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
      
     JPanel serverSettings1 = new JPanel();
     serverSettings1.add(new Label("Protocol"));
     serverSettings1.add(protocolTF);
     serverSettings1.add(new JLabel("  Server"));
     serverSettings1.add(serverTF);
     serverSettings1.add(new JLabel("  Port"));
     serverSettings1.add(portTF);
     serverSettings1.add(new JLabel("  Directory"));
     serverSettings1.add(directoryTF);

     JPanel serverSettings2 = new JPanel();
     serverSettings2.add(new JLabel("min Zoom"));
     serverSettings2.add(minZoomLevelTF);
     serverSettings2.add(new JLabel("  max Zoom"));
     serverSettings2.add(maxZoomLevelTF);
     serverSettings2.add(new JLabel(" max Downloads"));
     serverSettings2.add(maxDownloadsTF);

     JPanel serverSettings3 = new JPanel();
     serverSettings3.add(new JLabel("tile size X"));
     serverSettings3.add(tileSizeXTF);
     serverSettings3.add(new JLabel("  tile size Y"));
     serverSettings3.add(tileSizeYTF);
     serverSettings3.add(new JLabel("  name"));
     serverSettings3.add(nameTF);

     JPanel serverSettings4 = new JPanel(new GridLayout(3,1));
     serverSettings4.add(serverSettings1);
     serverSettings4.add(serverSettings2);
     serverSettings4.add(serverSettings3);

     JPanel serverSettings = new JPanel(new BorderLayout());
     serverSettings.add(selectionCB, BorderLayout.NORTH);
     serverSettings.add(serverSettings4, BorderLayout.CENTER);


     // create components for common setting      

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

     JPanel commonSettings1 = new JPanel();
     commonSettings1.add(showFrames);
     commonSettings1.add(showNames);
     
     JPanel commonSettings2 = new JPanel();
     commonSettings2.add(backgroundColor);
     commonSettings2.add(setBgColorBtn);
     commonSettings2.add(foregroundColor);
     commonSettings2.add(setFgColorBtn);

     JPanel commonSettings3 = new JPanel();
     commonSettings3.add(showLicenseBtn);

     JPanel commonSettings = new JPanel(new GridLayout(3,1));
     commonSettings.add(commonSettings1);
     commonSettings.add(commonSettings2);
     commonSettings.add(commonSettings3);


     acceptBtn = new JButton("accept");
     resetBtn = new JButton("reset");
     cancelBtn = new JButton("cancel");
     JPanel commandPanel = new JPanel();
     commandPanel.add(acceptBtn);
     commandPanel.add(resetBtn);
     commandPanel.add(cancelBtn);


     getContentPane().setLayout(new BorderLayout());
     getContentPane().add(serverSettings, BorderLayout.NORTH);
     getContentPane().add(commonSettings, BorderLayout.CENTER);  
     getContentPane().add(commandPanel, BorderLayout.SOUTH);

     setDefaultCloseOperation( WindowConstants.DO_NOTHING_ON_CLOSE ); // don't allow closing by pressing "X"

     addKnownServers();
     selectionCB.setSelectedIndex(0);

     origSettings = new Properties();

     readSettings(origSettings);

     addBtnListener();
     
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
          } else if(s.equals("customized")){
             OSMDialog.this.enableServerSettings(true);
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
		  settings.setProperty(OSMBackground.KEY_DIRECTORY,directoryTF.getText());

		  settings.setProperty(OSMBackground.KEY_MINZOOMLEVEL,minZoomLevelTF.getText());
		  settings.setProperty(OSMBackground.KEY_MAXZOOMLEVEL,maxZoomLevelTF.getText());
		  settings.setProperty(OSMBackground.KEY_MAXDOWNLOADS,maxDownloadsTF.getText());

		  settings.setProperty(OSMBackground.KEY_TILESIZEX,tileSizeXTF.getText());
		  settings.setProperty(OSMBackground.KEY_TILESIZEY,tileSizeYTF.getText());
		  settings.setProperty(OSMBackground.KEY_NAME,nameTF.getText());

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
       URL url = new URL(protocolTF.getText(), serverTF.getText(), port, directoryTF.getText());
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

}
