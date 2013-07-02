package PSEditor;

import java.awt.BorderLayout;
import java.awt.Dialog;
import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.TableModelEvent;
import javax.swing.table.DefaultTableModel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JTabbedPane;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JButton;
import javax.swing.JTextArea;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;


import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.ArrayList;
import javax.swing.JComboBox;
import javax.swing.DefaultComboBoxModel;
import javax.swing.SwingConstants;
import java.awt.Component;
import javax.swing.JLabel;
import java.awt.Font;
import javax.swing.JTextField;
import javax.swing.JCheckBox;
import java.awt.Color;
import javax.swing.border.LineBorder;

import javax.swing.event.TableModelListener;

public class AdSetting extends JFrame {

	private JPanel contentPane;
	private ParaTable secTable, hdTable;
	private SCReader secPara, hdPara;					 //Parameters set in this editor. 
	private SCReader secCurrPara, psCurrPara;  //Parameters read from the current setting file
	private String masterIP = "";
	private JTextField unText;
	private JTable dsTable;
	private JCheckBox ns4mCheck;
	private JTabbedPane tabbedPane;
	private JTextArea msgText;
	private JComboBox exportCombo;
	private boolean allAvailable;
	
	private String secConfFilePath, psConfFilePath;
	private ArrayList<DataServer> dataServers = new ArrayList<DataServer>();
	
	
	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					AdSetting frame = new AdSetting();
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the frame.
	 */
	public AdSetting() {
		allAvailable = false;
		setResizable(false);
		setTitle("Advanced Setting");
		
		setBounds(100, 100, 665, 711);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		
		tabbedPane = new JTabbedPane(JTabbedPane.TOP);
		
		exportCombo = new JComboBox();
		exportCombo.setModel(new DefaultComboBoxModel(
				new String[] {"Both", "Secondo", "ParallelSecondo"}));
		
		JButton createButton = new JButton("Create");
		createButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if (!allAvailable)
				{
					CheckMsgDlg cmd = new CheckMsgDlg();
					cmd.CheckCluster(unText.getText().trim(), dataServers, 
							hdPara, ns4mCheck.isSelected(), true);
					cmd.addWindowListener(new WindowAdapter(){

						/* (non-Javadoc)
						 * @see java.awt.event.WindowAdapter#windowClosed(java.awt.event.WindowEvent)
						 */
						@Override
						public void windowClosed(WindowEvent arg0) {
							// TODO Auto-generated method stub
							super.windowClosed(arg0);
							
							CheckMsgDlg dlg = (CheckMsgDlg)arg0.getSource();
							
							if (!allAvailable)
							{
								allAvailable = dlg.isAvailable();
								if (allAvailable && dlg.willCreateFile())
								{
									createResultFiles();
								}
							}
						}
								
					});
				}
				else
				{
					createResultFiles();
				}
			}
		});
		
		JButton closeButton = new JButton("Leave");
		closeButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				setVisible(false);
				dispose();
			}
		});
		
		JButton checkButton = new JButton("Check All");
		checkButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				CheckMsgDlg cmd = new CheckMsgDlg();
				cmd.CheckCluster(unText.getText().trim(), dataServers, hdPara, 
						ns4mCheck.isSelected(), false);
				cmd.addWindowListener(new WindowAdapter(){

					/* (non-Javadoc)
					 * @see java.awt.event.WindowAdapter#windowClosed(java.awt.event.WindowEvent)
					 */
					@Override
					public void windowClosed(WindowEvent arg0) {
						// TODO Auto-generated method stub
						super.windowClosed(arg0);
						
						CheckMsgDlg dlg = (CheckMsgDlg)arg0.getSource();
						
						if (!allAvailable)
						{
							allAvailable = dlg.isAvailable();
						}
					}
							
				});

			}
		});
		
		JScrollPane scrollPane_3 = new JScrollPane();
		scrollPane_3.setVerticalScrollBarPolicy(
				JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED); 
		GroupLayout gl_contentPane = new GroupLayout(contentPane);
		gl_contentPane.setHorizontalGroup(
			gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addContainerGap()
					.addComponent(scrollPane_3, GroupLayout.PREFERRED_SIZE, 377, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPane.createParallelGroup(Alignment.TRAILING)
						.addGroup(gl_contentPane.createSequentialGroup()
							.addGap(10)
							.addComponent(exportCombo, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPane.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(checkButton, 0, 0, Short.MAX_VALUE)))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPane.createParallelGroup(Alignment.LEADING)
						.addComponent(createButton, GroupLayout.PREFERRED_SIZE, 94, GroupLayout.PREFERRED_SIZE)
						.addComponent(closeButton))
					.addGap(8))
				.addGroup(gl_contentPane.createSequentialGroup()
					.addComponent(tabbedPane, GroupLayout.DEFAULT_SIZE, 654, Short.MAX_VALUE)
					.addGap(365))
		);
		gl_contentPane.setVerticalGroup(
			gl_contentPane.createParallelGroup(Alignment.TRAILING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addContainerGap()
					.addComponent(tabbedPane, GroupLayout.PREFERRED_SIZE, 587, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPane.createParallelGroup(Alignment.LEADING)
						.addComponent(exportCombo, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addGroup(gl_contentPane.createSequentialGroup()
							.addComponent(createButton)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_contentPane.createParallelGroup(Alignment.BASELINE)
								.addComponent(closeButton)
								.addComponent(checkButton)))
						.addComponent(scrollPane_3, GroupLayout.PREFERRED_SIZE, 78, GroupLayout.PREFERRED_SIZE))
					.addGap(26))
		);
		gl_contentPane.linkSize(SwingConstants.HORIZONTAL, new Component[] {createButton, closeButton});
		gl_contentPane.linkSize(SwingConstants.HORIZONTAL, new Component[] {exportCombo, checkButton});
		
		msgText = new JTextArea();
		scrollPane_3.setViewportView(msgText);
		msgText.setWrapStyleWord(true);
		msgText.setLineWrap(true);
		msgText.setBorder(new LineBorder(new Color(0, 0, 0)));
		msgText.setBackground(new Color(220, 220, 220));
		msgText.setEditable(false);
		
		Object[][] data={};
		String[] header={"Name","Value","Default", "Type"};
		
		JPanel secPanel = new JPanel();
		tabbedPane.addTab("SECONDO", null, secPanel, null);
		
		JScrollPane scrollPane = new JScrollPane();
		
		GroupLayout gl_secPanel = new GroupLayout(secPanel);
		gl_secPanel.setHorizontalGroup(
			gl_secPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_secPanel.createSequentialGroup()
					.addContainerGap()
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 621, Short.MAX_VALUE)
					.addContainerGap())
		);
		gl_secPanel.setVerticalGroup(
			gl_secPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_secPanel.createSequentialGroup()
					.addContainerGap()
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 529, Short.MAX_VALUE)
					.addContainerGap())
		);
		
		secTable = new ParaTable();
		secTable.getModel().addTableModelListener(new ParaTableAdaptor());
		scrollPane.setViewportView(secTable);
		secPanel.setLayout(gl_secPanel);
		
		JPanel hdPanel = new JPanel();
		tabbedPane.addTab("Hadoop", null, hdPanel, null);
		
		JScrollPane scrollPane_1 = new JScrollPane();
		
		JButton imPSButton = new JButton("Import");
		imPSButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				importPSCFile();
			}
		});
		GroupLayout gl_hdPanel = new GroupLayout(hdPanel);
		gl_hdPanel.setHorizontalGroup(
			gl_hdPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_hdPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_hdPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(imPSButton, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 87, GroupLayout.PREFERRED_SIZE)
						.addGroup(gl_hdPanel.createSequentialGroup()
							.addComponent(scrollPane_1, GroupLayout.PREFERRED_SIZE, 621, GroupLayout.PREFERRED_SIZE)
							.addContainerGap(6, Short.MAX_VALUE))))
		);
		gl_hdPanel.setVerticalGroup(
			gl_hdPanel.createParallelGroup(Alignment.TRAILING)
				.addGroup(gl_hdPanel.createSequentialGroup()
					.addContainerGap()
					.addComponent(scrollPane_1, GroupLayout.DEFAULT_SIZE, 482, Short.MAX_VALUE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(imPSButton)
					.addContainerGap())
		);
		
		hdTable = new ParaTable();
		hdTable.getModel().addTableModelListener(new ParaTableAdaptor());
		scrollPane_1.setViewportView(hdTable);
		hdPanel.setLayout(gl_hdPanel);
		
		if (masterIP.isEmpty())
			masterIP = Functions.get_localIP();
		
		JPanel panel_2 = new JPanel();
		tabbedPane.addTab("Cluster", null, panel_2, null);
		
		JLabel lblUserName = new JLabel("User Name: ");
		lblUserName.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		unText = new JTextField();
		unText.setColumns(10);
		unText.setText(System.getenv("USER"));
		
		JScrollPane scrollPane_2 = new JScrollPane();
		
		JButton btnChange = new JButton("Edit");
		btnChange.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				ClusterSetting csf = new ClusterSetting(AdSetting.this, true);
				csf.setTable(dataServers, ns4mCheck.isSelected());
				csf.setLocationRelativeTo(AdSetting.this);
				csf.setVisible(true);
				allAvailable = false;
			}
		});
		
		ns4mCheck = new JCheckBox("<html>Set the existing SECONDO database as the master database</html>");
		
		JButton psImpButton2 = new JButton("Import");
		psImpButton2.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				importPSCFile();
			}
		});
		
		JLabel imPSCLabel2 = new JLabel("<html>The $SECONDO_BUILD_DIR/bin/ParallelSecondoConfig.ini is imported.</html>");
		imPSCLabel2.setVisible(false);
		imPSCLabel2.setFont(new Font("Lucida Grande", Font.BOLD, 13));
		GroupLayout gl_panel_2 = new GroupLayout(panel_2);
		gl_panel_2.setHorizontalGroup(
			gl_panel_2.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_panel_2.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_panel_2.createParallelGroup(Alignment.TRAILING)
						.addComponent(scrollPane_2, GroupLayout.DEFAULT_SIZE, 621, Short.MAX_VALUE)
						.addGroup(gl_panel_2.createSequentialGroup()
							.addComponent(lblUserName)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(unText, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
						.addComponent(imPSCLabel2, Alignment.LEADING, GroupLayout.PREFERRED_SIZE, 492, GroupLayout.PREFERRED_SIZE)
						.addGroup(gl_panel_2.createSequentialGroup()
							.addComponent(ns4mCheck)
							.addGap(32)
							.addComponent(psImpButton2, GroupLayout.PREFERRED_SIZE, 91, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(btnChange, GroupLayout.PREFERRED_SIZE, 85, GroupLayout.PREFERRED_SIZE)))
					.addContainerGap())
		);
		gl_panel_2.setVerticalGroup(
			gl_panel_2.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_panel_2.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_panel_2.createParallelGroup(Alignment.BASELINE)
						.addComponent(unText, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblUserName))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(scrollPane_2, GroupLayout.PREFERRED_SIZE, 422, GroupLayout.PREFERRED_SIZE)
					.addGroup(gl_panel_2.createParallelGroup(Alignment.TRAILING)
						.addGroup(gl_panel_2.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(ns4mCheck)
							.addPreferredGap(ComponentPlacement.RELATED, 28, Short.MAX_VALUE))
						.addGroup(gl_panel_2.createSequentialGroup()
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(gl_panel_2.createParallelGroup(Alignment.BASELINE)
								.addComponent(btnChange)
								.addComponent(psImpButton2))
							.addPreferredGap(ComponentPlacement.RELATED)))
					.addComponent(imPSCLabel2)
					.addContainerGap())
		);
		
		String[] dsHeaders = {"IP Address", "Server Path", "Port", "As"};
		DefaultTableModel dsModel = new DefaultTableModel(data, dsHeaders){

			/* (non-Javadoc)
			 * @see javax.swing.table.DefaultTableModel#isCellEditable(int, int)
			 */
			@Override
			public boolean isCellEditable(int arg0, int arg1) {
				// TODO Auto-generated method stub
				//return super.isCellEditable(arg0, arg1);
				return false;
			}
			
		};

		dsTable = new JTable(dsModel);
		scrollPane_2.setViewportView(dsTable);
		panel_2.setLayout(gl_panel_2);
		contentPane.setLayout(gl_contentPane);
		
		try {
			//Initialize the parameters and tables. 

			secConfFilePath = System.getenv("SECONDO_BUILD_DIR") 
				+ "/bin/SecondoConfig.ini";
			secCurrPara = new SCReader(secConfFilePath);
			if (!secCurrPara.isEmpty()){
				msgText.append("Import " + secConfFilePath + "\n");
			} else {
				msgText.append("Cannot find the default SECONDO configuration " 
						+ secConfFilePath + "\n");
			}
			
			psConfFilePath = System.getenv("SECONDO_BUILD_DIR") 
			+ "/bin/ParallelSecondoConfig.ini";
			psCurrPara = new SCReader(psConfFilePath);
			if (!psCurrPara.isEmpty())
			{
				msgText.append("Import " + psConfFilePath + "\n");
			} else {
				msgText.append("Cannot find the default Parallel SECONDO configuration " 
						+ psConfFilePath + "\n");
			}
			
			showSecTab();
			showHdTab();
			showCsTab();
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
/*		tabbedPane.addChangeListener(new ChangeListener(){
		   public void stateChanged(ChangeEvent e){
		    JTabbedPane tabbedPane = (JTabbedPane)e.getSource();
		    int selectedIndex = tabbedPane.getSelectedIndex();
		    
		    switch (selectedIndex){
		    	case 0:{
		    		showSecTab();
		    		break;
		    	}
		    	case 1:{
		    		showHdTab();
		    		break;
		    	}
		    	case 2:{
		    		showCsTab();
		    		break;
		    	}    		
		    }
		   }
		});
*/	
	}
	
	private void createResultFiles()
	{
		if (!allAvailable)
			return;
		
		boolean[] files = {true, true};	//Create SecConf and PSConf
		if (exportCombo.getSelectedIndex() == 1)
			files[1] = false;
		else if (exportCombo.getSelectedIndex() == 2)
			files[0] = false;
		String SEC_BIN_PATH = System.getenv("SECONDO_BUILD_DIR") + "/bin";
		JFileChooser fc = new JFileChooser();
		
		try
		{
			if (files[0])
			{
				//Create SecondoConfig.ini
				fc.setSelectedFile(new File(SEC_BIN_PATH + "/SecondoConfig.ini"));
				
				int rtn = fc.showSaveDialog(AdSetting.this);
				if (rtn == JFileChooser.APPROVE_OPTION){
					//Create SecondoConfig.ini
					String secFilePath = fc.getSelectedFile().getAbsolutePath();
					boolean create = true;
					if (new File(secFilePath).exists())
					{
						create = JOptionPane.showConfirmDialog(AdSetting.this, 
								"The file " + secFilePath + " exists, overlapped it ? ",
								"Warning", JOptionPane.YES_NO_OPTION, 
													 JOptionPane.QUESTION_MESSAGE) == 0;
					}
					if (create){
						secPara.list(secFilePath);
						JOptionPane.showMessageDialog(AdSetting.this, 
								"The SECONDO configuration file " + secFilePath + " is created. ",
								"Info", JOptionPane.INFORMATION_MESSAGE);
					}
				}
			}	

			if (files[1])
			{
				//Create ParallelSecondoConfig.ini
				fc.setSelectedFile(new File(SEC_BIN_PATH + "/ParallelSecondoConfig.ini"));
				
				int rtn = fc.showSaveDialog(AdSetting.this);
				if (rtn == JFileChooser.APPROVE_OPTION){
					//Create SecondoConfig.ini
					String psFilePath = fc.getSelectedFile().getAbsolutePath();
					boolean create = true;
					if (new File(psFilePath).exists())
					{
						create = JOptionPane.showConfirmDialog(AdSetting.this, 
								"The file " + psFilePath + " exists, overlapped it ? ",
								"Warning", JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE) == 0;
					}
					if (create){
						
						
						//Temporarily add Data Server list to Parallel Secondo Config
						String sectionName = "Cluster";
						for (DataServer ds : dataServers)
						{
							if (ds.isMaster()){
								hdPara.set(sectionName, "Master", 
										ds.toString(), SCReader.MODE.SINGLE);
							}
							
							if (ds.isSlave()){
								hdPara.set(sectionName, "Slaves", 
										ds.toString(), SCReader.MODE.MULTI);
							}
						}
						if (ns4mCheck.isSelected()){
							hdPara.set("Options", "NS4Master", "true", SCReader.MODE.SINGLE);
						}
						
						hdPara.list(psFilePath);
						JOptionPane.showMessageDialog(AdSetting.this, 
								"The Parallel SECONDO configuration file " + psFilePath + " is created. ",
								"Info", JOptionPane.INFORMATION_MESSAGE);
						hdPara.delSection(sectionName);
						hdPara.delSinglePara("Options", "NS4Master");
					}
				}

			}
		} catch (IOException ioe){
			System.err.println(ioe.getMessage());
		}
	}
	
	private void showSecTab()
	{
		secTable.clearTable();
		
/*
 * -------------Read SECONDO configuration------------------

The reason that I put parameters into a xml file, 
instead of reading current setting from the SecondoConfig.ini
is that some parameters are commented out in the default configuration file, 
and it is difficult to distinguish which is really a comment, 
while which is a disabled parameter. 
However, it is clear in the xml file. 

Besides, I can set the default value for parameters, 
which is also nowhere to indicate in the configuration file. 

*/
		try {
			String path = "SEConf.xml";
			InputStream in = getClass().getClassLoader().getResourceAsStream(path);
			secPara = new SCReader();
			
			DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder dBuilder;
			dBuilder = dbFactory.newDocumentBuilder();
			Document doc = dBuilder.parse(new InputSource(in));

/*
Also compare the default setting with the current SECONDO setting, 
adjust if the values change.  
 
*/
			
			NodeList sections = doc.getElementsByTagName("section");
			for (int sc = 0; sc <sections.getLength(); sc++)
			{
				Element section = (Element)sections.item(sc);
				
				String sectionName = section.getAttribute("name");
				
				secTable.addSection(sectionName);
				int itemNum = section.getElementsByTagName("key").getLength();
				for (int ii = 0; ii < itemNum; ii++)
				{
					String k = section.getElementsByTagName("key").item(ii).getTextContent();
					String t = section.getElementsByTagName("type").item(ii).getTextContent();
					String v = section.getElementsByTagName("value").item(ii).getTextContent();
					String dv = section.getElementsByTagName("default").item(ii).getTextContent();
					
					//Add the parameter to the SCReader
					if (SCReader.MODE.valueOf(t) == SCReader.MODE.MULTI)
					{
						String title = k.substring(0, k.indexOf("::"));
						String value = k.substring(k.indexOf("::") + 2);
						Boolean bv = Boolean.parseBoolean(v);
						
						if (!secCurrPara.isEmpty())
						{
							ArrayList<String> cvs = secCurrPara.getMultiValues(sectionName, title);
							Boolean cv = cvs.contains(value);
							
							if (bv != cv)
								v = cv.toString().toLowerCase();
						}
						
						if (Boolean.parseBoolean(v))
							secPara.set(sectionName, title, value, SCReader.MODE.MULTI);
					}
					else{
						if (!secCurrPara.isEmpty())
						{
							String cv = secCurrPara.get(sectionName, k);
							if (v.compareTo(cv) != 0 )
								v = cv;
						}
						
						if (!v.isEmpty())
							secPara.set(sectionName, k, v, SCReader.MODE.SINGLE);
					}

					//Add the parameter to the table
					//All SECONDO parameters are editable. 
					secTable.addParameter(k, v, dv, t, "true");
				}
			}
			
		} catch (ParserConfigurationException err) {
			// TODO Auto-generated catch block
			err.printStackTrace();
		}
		//Document doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(scConf);
		catch (SAXException err) {
			// TODO Auto-generated catch block
			err.printStackTrace();
		} catch (IOException err) {
			// TODO Auto-generated catch block
			err.printStackTrace();
		}
	}

	private void showHdTab()
	{
		hdTable.clearTable();
		
		//Read the Hadoop setting
		try{
			hdPara = new SCReader();
			
			String sectionName = "Hadoop";
			String path = "HdConf.xml";
			InputStream in = getClass().getClassLoader().getResourceAsStream(path);
			
			DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder dBuilder;
			dBuilder = dbFactory.newDocumentBuilder();
			Document doc = dBuilder.parse(new InputSource(in));

			NodeList files = doc.getElementsByTagName("File");
			for (int fi = 0; fi < files.getLength(); fi++)
			{
				Element file = (Element)files.item(fi);
				
				String fileName = file.getAttribute("name");
				hdTable.addSection(fileName);
				int itemNum = file.getElementsByTagName("key").getLength();
				for (int ii = 0; ii < itemNum; ii++)
				{
					String k = file.getElementsByTagName("key").item(ii).getTextContent();
					String t = file.getElementsByTagName("type").item(ii).getTextContent();
					String v = file.getElementsByTagName("value").item(ii).getTextContent();
					String dv = file.getElementsByTagName("default").item(ii).getTextContent();
					String ed = file.getElementsByTagName("editable").item(ii).getTextContent();
					
/*
* TYPES: 
* 
* SINGLE: a single value
* SINGLE-PORT: a single port value, should export with the prefix of 0.0.0.0:
* SINGLE-HOSTPORT: a single port value, export by adding the prefix of the master node's IP address
* SINGLE-HDFSPORT: a single port value, export by adding the prefix of the master IP, and also a hdfs:// stamp.
*   
*/
					String type = t; 
					if (type.contains("-"))
					  type = t.substring(0, t.indexOf("-"));
					
					String title = "", value = "";
					if (SCReader.MODE.valueOf(type) == SCReader.MODE.MULTI)
					{
						title = k.substring(0, k.indexOf("::"));
						value = k.substring(k.indexOf("::")+2);
						Boolean bv = Boolean.parseBoolean(v);
						//Compare with the current setting
						
						title = fileName + ":" + title;
						if (!psCurrPara.isEmpty())
						{
							ArrayList<String> cvs = psCurrPara.getMultiValues(sectionName, title);
							Boolean cv = cvs.contains(value);
							
							if (bv != cv )
								v = cv.toString().toLowerCase();
						}

						if (Boolean.parseBoolean(v))
							hdPara.set(sectionName, title, value, SCReader.MODE.valueOf(type));
					}
					else
					{
						//Here, ~v~ is the string in the table, only the port number
						//~value~ is the string in the parameter list, with the prefixes. 
						
						title = k;
						//For single mode 
						if (t.compareTo("SINGLE-PORT") == 0)
						{
							value = "0.0.0.0:" + v;
						}
						else if (t.compareTo("SINGLE-HDFSPORT") == 0)
						{
							value = "hdfs://" + masterIP + ":" + v;
						}
						else if (t.compareTo("SINGLE-HOSTPORT") == 0)
						{
							value = masterIP + ":" + v;
						}
						else if (t.compareTo("SINGLE") == 0)
						{
							value = v;
						}
						else
						{
							String errInfo = "Unknown item types : " + t ;
							JOptionPane.showMessageDialog(AdSetting.this, errInfo,
									"ERROR", JOptionPane.ERROR_MESSAGE);
						}
						
						title = fileName + ":" + title;
						if (!psCurrPara.isEmpty())
						{
							String cv = psCurrPara.get(sectionName, title);
							
							if (value.compareTo(cv) != 0)
							{
								value = cv;
								if (value.isEmpty()){
									v = "";
								}
								else if (t.contains("-")){
									v = value.substring(value.lastIndexOf(":") + 1);
								}
							}
						}
						
						if (!v.isEmpty())
							hdPara.set(sectionName, title, value, SCReader.MODE.valueOf(type));
					}
					hdTable.addParameter(k, v, dv, type, ed);
				}
			}
		} catch (ParserConfigurationException err) {
			// TODO Auto-generated catch block
			err.printStackTrace();
		}
		//Document doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(scConf);
		catch (SAXException err) {
			// TODO Auto-generated catch block
			err.printStackTrace();
		} catch (IOException err) {
			// TODO Auto-generated catch block
			err.printStackTrace();
		}
	}

	private void showCsTab()
	{
		DefaultTableModel model = (DefaultTableModel) dsTable.getModel();
		model.getDataVector().removeAllElements();
		
		if (dataServers.isEmpty()) 
			dataServers = createDataServers(psCurrPara);
		
		for (DataServer ds : dataServers)
		{
			model.addRow(new Object[]{ds.getIp(), ds.getPath(), 
					ds.getPort(), ds.getRole().toString()});
		}
		
		Boolean ns4m = Boolean.parseBoolean(psCurrPara.get("Options", "NS4Master"));
		ns4mCheck.setSelected(ns4m);
	}
	
	public void setupCluster(SCReader parameters)
	{
		ArrayList<DataServer> dss = createDataServers(parameters);
		if (dss.isEmpty())
		{
			JOptionPane.showMessageDialog(AdSetting.this, 
					"The Data Servers are not correctly described.",
					"Warning",
					JOptionPane.ERROR_MESSAGE);
		}
		else
		{
			dataServers = dss;
		}
		showCsTab();
	}
	
	private ArrayList<DataServer> createDataServers(SCReader parameters)
	{
		String mstr = parameters.get("Cluster", "Master");
		ArrayList<String> sstrs = parameters.getMultiValues("Cluster", "Slaves");
		
		ArrayList<DataServer> dss = new ArrayList<DataServer>();
		boolean findMaster = false;
		for (String s : sstrs)
		{
			String[] elem = s.split(":");
			DataServer ds;
			if (!findMaster)
			{
				if (s.compareTo(mstr) == 0){
					findMaster = true;
					ds = new DataServer(elem[0], elem[1], 
							Integer.parseInt(elem[2]), DataServer.ROLES.MASLAVE);
					dss.add(ds);
					continue;
				}
			}
			ds = new DataServer(elem[0], elem[1], 
					Integer.parseInt(elem[2]), DataServer.ROLES.SLAVE);
			dss.add(ds);
		}
		
		if (!findMaster && !mstr.isEmpty()){
			String[] elem = mstr.split(":");
			dss.add(new DataServer(elem[0], elem[1], 
					Integer.parseInt(elem[2]), DataServer.ROLES.MASTER));
		}
		
		return dss;
	}
	
	public int importPSCFile()
	{
		String SEC_BinPath = System.getenv("SECONDO_BUILD_DIR") + "/bin";
		
		JFileChooser fc = new JFileChooser(SEC_BinPath);
		fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
		
		int rtn = fc.showOpenDialog(AdSetting.this);
		if (rtn == JFileChooser.APPROVE_OPTION){
			try {
				String path = fc.getSelectedFile().getAbsolutePath();
				SCReader para = new SCReader(path);
				if (para.getTitles("Hadoop").isEmpty()){
					JOptionPane.showMessageDialog(AdSetting.this, 
							"The selected file doesn't match Parallel SECONDO configuration format.",
							"ERROR", JOptionPane.ERROR_MESSAGE);
					return -1;
				}
				else {
					psConfFilePath = path;
				  psCurrPara = para;
				  dataServers = createDataServers(para);
				  msgText.append("Import " + psConfFilePath + "\n");
				}
	
				//Refresh the tables
				showHdTab();
				showCsTab();
				allAvailable = false;
			} catch (IOException e) {
				// TODO Auto-generated catch block
				//e.printStackTrace();
				JOptionPane.showMessageDialog(AdSetting.this, 
						"The selected file doesn't match Parallel SECONDO configuration format.",
						"ERROR", JOptionPane.ERROR_MESSAGE);
				return -1;
			}
		}
		return 0;
	}

class ParaTableAdaptor implements TableModelListener
{

	@Override
	public void tableChanged(TableModelEvent e) {
		// TODO Auto-generated method stub
		if (e.getType() == TableModelEvent.UPDATE){
			
			int row = e.getLastRow();
			int column = e.getColumn();
			
			DefaultTableModel model = (DefaultTableModel)e.getSource();
			
			String value = (String)model.getValueAt(row, column);
			String dvalue = (String)model.getValueAt(row, 2);
			
			if (value.compareTo(dvalue) != 0)
			{
				//Do the format check. 
				//Basically there are only integer, boolean and string values
				//It is not necessary to check boolean, since it is set by a combobox
				
				if (isInteger(dvalue) && !isInteger(value))
				{
					String errInfo = "This value is invalid, since it is not an integer.";
					JOptionPane.showMessageDialog(AdSetting.this, errInfo,
							"ERROR", JOptionPane.ERROR_MESSAGE);
					model.setValueAt(dvalue, row, column);
					return;
				}
			}
			
			String secName = "";
			for (int ri = (row - 1); ri >= 0; ri--)
			{
				String v_c0 = (String)model.getValueAt(ri, 0);
				String v_c1 = (String)model.getValueAt(ri, 1);
				String v_c2 = (String)model.getValueAt(ri, 2);
				
				if (!v_c0.isEmpty() && v_c1.isEmpty() && v_c2.isEmpty()){
					secName = v_c0;
					break;
				}
			}
			
			if (!secName.isEmpty())
			{
				SCReader.MODE mode = SCReader.MODE.valueOf((String)model.getValueAt(row, 3));
				
				int currTab = tabbedPane.getSelectedIndex();
				if (currTab == 0)
				{
					String title = (String)model.getValueAt(row, 0);
					if (mode == SCReader.MODE.MULTI)
					{
						boolean isSet = Boolean.parseBoolean(value);
						
						String t = title.substring(0, title.indexOf("::"));
						String v = title.substring(title.indexOf("::")+2);
						//MAY add or remove one multi value
						if (!isSet) {
							secPara.delMultiPara(secName, t, v);
						} else {
							secPara.set(secName, t, v, mode);
						}
					}
					else
					{
						if (value.trim().isEmpty()){
							secPara.delSinglePara(secName, title);
						}else{
							secPara.set(secName, title, value, mode);
						}
					}
				}
				else if (currTab == 1)
				{
					String fileName = secName;
					secName = "Hadoop";
					String paraName = (String)model.getValueAt(row, 0);
					String title = fileName + ":" + paraName;
					if (mode == SCReader.MODE.MULTI)
					{
						boolean isSet = Boolean.parseBoolean(value);
						String t = title.substring(0, title.indexOf("::"));
						String v = title.substring(title.indexOf("::") + 2);
						if (!isSet){
							hdPara.delMultiPara(secName, t, v);
						} else {
							hdPara.set(secName, t, v, mode);
						}
					}
					else
					{
						String t = title;
						String type = getHdpParaInfo(fileName, paraName, "type");
						String v = "";
						
						if (type.compareTo("SINGLE-PORT") == 0)
						{
							v = "0.0.0.0:" + value;
						}
						else if (type.compareTo("SINGLE-HDFSPORT") == 0)
						{
							v = "hdfs://" + masterIP + ":" + value;
						}
						else if (type.compareTo("SINGLE-HOSTPORT") == 0)
						{
							v = masterIP + ":" + value;
						}
						else if (type.compareTo("SINGLE") == 0)
						{
							v = value;
						}
						
						if (value.isEmpty())
						{
							hdPara.delSinglePara(secName, t);
						}
						else 
						{
							hdPara.set(secName, t, v, mode);
						}
					}
				}
				
				System.err.println("Something changes in the parameter table");
				allAvailable = false;
			}
		}
	}
	
	/*
	 * Search HdConf.xml file, and return one elment's specific value  
	 */
	private String getHdpParaInfo(String fileName, String key, String tag)
	{
		String value = "";
		
		String path = "HdConf.xml";
		InputStream in = getClass().getClassLoader().getResourceAsStream(path);
		
		try {
			DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder dBuilder;
			dBuilder = dbFactory.newDocumentBuilder();
			Document doc = dBuilder.parse(new InputSource(in));
			NodeList files = doc.getElementsByTagName("File");
			boolean found = false;
			
			for (int fi = 0; fi < files.getLength(); fi++)
			{
				Element file = (Element)files.item(fi);
				
				if (file.getAttribute("name").compareTo(fileName) == 0)
				{
					int itemNum = file.getElementsByTagName("key").getLength();
					for (int ii = 0; ii < itemNum; ii++)
					{
						if (file.getElementsByTagName("key")
								.item(ii).getTextContent().compareTo(key) == 0)
						{
							value = file.getElementsByTagName(tag).item(ii).getTextContent();
							found = true;
							break;
						}
					}
				}
				if (found)
					break;
			}
			
			return value;
		} catch (ParserConfigurationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SAXException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}


		
		return value;
	}
	
	private boolean isInteger(String value)
	{
		if (value.isEmpty())
			return true;
		
		try
		{
			Integer.parseInt(value);
			return true;
		} catch (NumberFormatException e){
			return false;
		}
	}
	
}
}
