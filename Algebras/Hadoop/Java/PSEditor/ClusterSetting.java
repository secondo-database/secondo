package PSEditor;

import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.DefaultCellEditor;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.LayoutStyle.ComponentPlacement;
import java.awt.Font;
import javax.swing.JTable;
import java.awt.Color;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableColumn;
import javax.swing.JScrollPane;
import javax.swing.JButton;
import javax.swing.SwingConstants;
import java.awt.Component;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

import javax.swing.border.LineBorder;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.JTextField;

public class ClusterSetting extends JFrame {
	private JTable dsTable;
	private JScrollPane scrollPane;
	private ArrayList<DataServer> dataServers;
	private JTextField un_Text;
	private JCheckBox ns4m_Check;
	private boolean isClusterAvailable = false;
	private JButton createButton, checkButton;
	
	private JFrame fatherWindow = null;
	private boolean onlyCluster = false; 
	//True if this frame creates the cluster information only.  
	 
	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					ClusterSetting frame = new ClusterSetting();
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}


	//Make this frame can be modal
	public ClusterSetting(JFrame pframe, boolean modal)
	{
		this();
		
		if (pframe != null && modal)
		{
			fatherWindow = pframe;
			fatherWindow.setEnabled(false);
			onlyCluster = true;
			changeToClusterEditor();
		}
	}
	
	private void changeToClusterEditor()
	{
		if (onlyCluster)
		{
			//Adjust the Interface to edit only Cluster information
			createButton.setText("OK");
			checkButton.setEnabled(false);
		}
	}
	

	/* (non-Javadoc)
	 * @see java.awt.Window#setVisible(boolean)
	 */
	@Override
	public void setVisible(boolean b) {
		// TODO Auto-generated method stub
		if (!b && fatherWindow != null){
			fatherWindow.setEnabled(true);
		}
		super.setVisible(b);
	}


	/**
	 * Create the frame.
	 */
	public ClusterSetting() {
		
		dataServers = new ArrayList<DataServer>();
		
		setResizable(false);
		setTitle("Simple Cluster");
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 627, 408);
		
		JLabel lblNewLabel = new JLabel("Select: ");
		lblNewLabel.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		String selOptions[] = {"", "All", "None", "Reverse"};
		JComboBox comboBox = new JComboBox();
		comboBox.setModel(new DefaultComboBoxModel(selOptions));
		comboBox.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		comboBox.setSelectedIndex(1);
		comboBox.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				JComboBox box = (JComboBox)arg0.getSource();
				
				switch (box.getSelectedIndex()) {
					case 0: 
					  //Do nothing
						break;
					case 1:
						//all
						for (int r = 0; r < dsTable.getRowCount(); r++){
							dsTable.setValueAt(true, r, 0);
						}
						break;
					case 2:
						//none
						for (int r = 0; r < dsTable.getRowCount(); r++){
							dsTable.setValueAt(false, r, 0);
						}
						break;
					case 3:
						//reverse
						for (int r = 0; r < dsTable.getRowCount(); r++){
							dsTable.setValueAt(!((Boolean)dsTable.getValueAt(r, 0)), r, 0);
						}
						break;
					default: 
						//should never happen.
						JOptionPane.showMessageDialog(ClusterSetting.this, 
								"Wrong option for the comboBox", "ERROR", JOptionPane.ERROR_MESSAGE);
				}
				
				box.setSelectedIndex(0);
			}
		});
		
		scrollPane = new JScrollPane();
		
		JButton addButton = new JButton("Add");
		addButton.setToolTipText("Add a new Data Server");
		addButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				DefaultTableModel model = (DefaultTableModel)dsTable.getModel();
				model.addRow(new Object[]{true, "", "", null, "Slave"});
				
				//remove existing rows
				
				isClusterAvailable = false;
			}
		});
		
		JButton importButton = new JButton("Import");
		importButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				//Import a set of Data Servers from the given file
				JFileChooser fc = new JFileChooser();
				fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
				
				int rtn = fc.showOpenDialog(ClusterSetting.this);
				if (rtn == JFileChooser.APPROVE_OPTION){
					try {
						DefaultTableModel model = (DefaultTableModel)dsTable.getModel();
						
						if (model.getRowCount() > 0){
							String info = "Clean the current cluster setting? ";
							int rtn2 = JOptionPane.showConfirmDialog(ClusterSetting.this, info, 
									"", JOptionPane.YES_NO_OPTION);
							
							if (rtn2 == 0){
								model.getDataVector().removeAllElements();
								dataServers.clear();
							}
						}
						
						BufferedReader br = new BufferedReader(
								new FileReader(fc.getSelectedFile().getAbsolutePath()));
						while (br.ready()){
							String ds = br.readLine().trim();
							if (ds.startsWith("#"))
								continue;
							
							String elem[] = ds.split(":");
							if (elem.length != 3)
								throw new IOException("Invalid Data Server format.");
							
							if (!IPAddressFormatter.isValidIP(elem[0]))
								throw new IOException("Invalid Data Server IP address.");
							
							int port = Integer.parseInt(elem[2]);
							if ( port <= 1024 || port > 65535)
								throw new IOException("Invalid SECONDO monitor port.");
							
							model.addRow(new Object[]{true, elem[0], elem[1], elem[2],"Slave"});
						}
						br.close();
					} catch (IOException e) {
						// TODO Auto-generated catch block
						JOptionPane.showMessageDialog(ClusterSetting.this, 
								e.getMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
					} 
					
					//remove existing rows
					
					isClusterAvailable = false;
					
				}
			}
		});
		importButton.setToolTipText("<html>Import a set of Data Servers from a text file. <br/>\nData Servers in the file is represented by lines, each line <br/>\nindicates a Data Server with three elements separated by colons. <br/>\nFor example: <br/>\n192.168.1.1:/tmp:11234\n</html>");
		
		JButton descButton = new JButton("Describe");
		descButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				ClusterRangeDlg crDlg = new ClusterRangeDlg();

				crDlg.setLocationRelativeTo(ClusterSetting.this);
				crDlg.setModal(true);
				crDlg.setVisible(true);
				ArrayList<String> ipRange = crDlg.getIPRange();
				ArrayList<String> dss = crDlg.getDSList();
				crDlg.dispose();
				
				DefaultTableModel model = (DefaultTableModel)dsTable.getModel();
				
				if (model.getRowCount() > 0){
					String info = "Clean the current cluster setting? ";
					int rtn = JOptionPane.showConfirmDialog(ClusterSetting.this, info, 
							"", JOptionPane.YES_NO_OPTION);
					
					if (rtn == 0){
						model.getDataVector().removeAllElements();
						dataServers.clear();
					}
				}
				
				for (String dsv : dss)
				{
					for (String ip : ipRange)
					{
						String elem[] = dsv.split(":");
						
						DataServer ds = new DataServer(ip, elem[0], Integer.parseInt(elem[1]), DataServer.ROLES.SLAVE);
						if (!dataServers.contains(ds)){
							dataServers.add(ds);
							model.addRow(new Object[]{true, ip, elem[0], elem[1], "Slave"});
						}
					}
				}
				
				isClusterAvailable = false;
			}
		});
		descButton.setToolTipText("Describe a cluster of Data Servers when the computers are organized in a uniform way. ");
		
		ns4m_Check = new JCheckBox("<html>Set the existing SECONDO database as the master database</html>");
		
		JButton cancelButton = new JButton("Leave");
		cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				setVisible(false);
				dispose();
			}
		});
		
		createButton = new JButton("Create");
		createButton.addActionListener(new ActionListener() {
			
			public void actionPerformed(ActionEvent e) {
				if (!isClusterAvailable)
				{
					//should check the cluster first 
					CheckMsgDlg cmd = new CheckMsgDlg();
					cmd.CheckCluster(un_Text.getText().trim(), dataServers, null, ns4m_Check.isSelected(), true);
					cmd.addWindowListener(new WindowAdapter(){

						/* (non-Javadoc)
						 * @see java.awt.event.WindowAdapter#windowClosed(java.awt.event.WindowEvent)
						 */
						@Override
						public void windowClosed(WindowEvent arg0) {
							// TODO Auto-generated method stub
							super.windowClosed(arg0);
							
							CheckMsgDlg dlg = (CheckMsgDlg)arg0.getSource();
							
							if (!isClusterAvailable)
							{
								isClusterAvailable = dlg.isAvailable();
								if ( isClusterAvailable && dlg.willCreateFile())
								{
									createResultFile();
								}
							}
						}
					});
				}
				else
				{
					createResultFile();
				}
			}
		});
		
		checkButton = new JButton("Check");
		checkButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				CheckMsgDlg cmd = new CheckMsgDlg();
				cmd.CheckCluster(un_Text.getText().trim(), dataServers, null, ns4m_Check.isSelected(), false);
				cmd.addWindowListener(new WindowAdapter(){

					/* (non-Javadoc)
					 * @see java.awt.event.WindowAdapter#windowClosed(java.awt.event.WindowEvent)
					 */
					@Override
					public void windowClosed(WindowEvent arg0) {
						// TODO Auto-generated method stub
						super.windowClosed(arg0);
						
						CheckMsgDlg dlg = (CheckMsgDlg)arg0.getSource();
						
						if (!isClusterAvailable)
						{
							isClusterAvailable = dlg.isAvailable();
						}
					}
				});
			}
		});
		
		JButton delButton = new JButton("Delete");
		delButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				
				//Get rows which are selected, i.e. the checkbox in the first column is selected.
				
				DefaultTableModel model = (DefaultTableModel)dsTable.getModel();
				int rnum = dsTable.getRowCount();
				for (int ri = (rnum - 1); ri >= 0; ri--){
					if ((Boolean)dsTable.getValueAt(ri, 0))
					{
						model.removeRow(ri);
						dataServers.remove(ri);
					}
				}
			}
		});
		delButton.setToolTipText("Delete all selected Data Servers");
		
		JLabel lblUserName = new JLabel("User Name: ");
		lblUserName.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		un_Text = new JTextField();
		un_Text.setColumns(10);
		un_Text.setText(System.getenv("USER"));
		GroupLayout groupLayout = new GroupLayout(getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGap(6)
					.addComponent(lblNewLabel)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(comboBox, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addComponent(lblUserName)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(un_Text, 204, 204, 204)
					.addContainerGap(142, Short.MAX_VALUE))
				.addGroup(groupLayout.createSequentialGroup()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(ns4m_Check, GroupLayout.DEFAULT_SIZE, 479, Short.MAX_VALUE)
							.addGap(48))
						.addGroup(Alignment.TRAILING, groupLayout.createSequentialGroup()
							.addContainerGap()
							.addComponent(scrollPane, GroupLayout.PREFERRED_SIZE, 479, GroupLayout.PREFERRED_SIZE)
							.addPreferredGap(ComponentPlacement.RELATED)))
					.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
						.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
							.addComponent(addButton)
							.addComponent(importButton, GroupLayout.PREFERRED_SIZE, 75, GroupLayout.PREFERRED_SIZE)
							.addComponent(descButton, GroupLayout.PREFERRED_SIZE, 114, GroupLayout.PREFERRED_SIZE)
							.addComponent(delButton, GroupLayout.PREFERRED_SIZE, 90, GroupLayout.PREFERRED_SIZE))
						.addComponent(createButton)
						.addComponent(cancelButton)
						.addComponent(checkButton))
					.addGap(22))
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(Alignment.TRAILING, groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
						.addGroup(groupLayout.createSequentialGroup()
							.addComponent(checkButton)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(createButton)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(cancelButton))
						.addGroup(groupLayout.createSequentialGroup()
							.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
								.addComponent(lblNewLabel)
								.addComponent(comboBox, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblUserName, GroupLayout.PREFERRED_SIZE, 19, GroupLayout.PREFERRED_SIZE)
								.addComponent(un_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
							.addPreferredGap(ComponentPlacement.RELATED)
							.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
								.addGroup(groupLayout.createSequentialGroup()
									.addComponent(addButton)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(importButton)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(descButton)
									.addPreferredGap(ComponentPlacement.RELATED)
									.addComponent(delButton))
								.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 297, Short.MAX_VALUE))
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(ns4m_Check)))
					.addGap(14))
		);
		groupLayout.linkSize(SwingConstants.HORIZONTAL, new Component[] {cancelButton, createButton, checkButton});
		groupLayout.linkSize(SwingConstants.HORIZONTAL, new Component[] {delButton, addButton, importButton, descButton});

		Object[] cHeaders = {"","IP Address","Server Path", "Port","As"};
		Object[][] cData = {};
		
    DefaultTableModel model = new DefaultTableModel(cData, cHeaders);
		dsTable = new JTable(model){
			private static final long serialVersionUID = 1L;

			@Override
      public Class getColumnClass(int column) {
          switch (column) {
              case 0:
                  return Boolean.class;
              case 1:
                  return String.class;
              case 2:
                  return String.class;
              case 3:
                  return String.class;
              case 4:
                	return String.class;
              default:
                  return String.class;
          }
      }
		};
		dsTable.setFont(new Font("Lucida Grande", Font.PLAIN, 13));
		dsTable.setBorder(new LineBorder(new Color(0, 0, 0)));
		
		dsTable.setGridColor(Color.BLACK);
		dsTable.setRowHeight(25);
		dsTable.setRowSelectionAllowed(true);
		
		TableColumn roleColumn = dsTable.getColumnModel().getColumn(4);
		JComboBox roleBox = new JComboBox();
		roleBox.addItem("Slave");
		roleBox.addItem("Master");
		roleBox.addItem("Master & Slave");
		roleColumn.setCellEditor(new DefaultCellEditor(roleBox));
		
		dsTable.setDefaultRenderer(Object.class, new AlterRowColor());
	
		
		dsTable.getColumnModel().getColumn(0).setMaxWidth(20);
		dsTable.getColumnModel().getColumn(1).setMaxWidth(120);
		dsTable.getColumnModel().getColumn(2).setMaxWidth(300);
		dsTable.getColumnModel().getColumn(3).setMaxWidth(50);
		dsTable.getColumnModel().getColumn(4).setMaxWidth(50);
		dsTable.getTableHeader().setReorderingAllowed(false);
				
		scrollPane.setViewportView(dsTable);
		getContentPane().setLayout(groupLayout);
		
		dsTable.getModel().addTableModelListener( new TableModelListener(){
			
				public void tableChanged(TableModelEvent e){
					if (e.getType() == TableModelEvent.UPDATE){
						int row = e.getLastRow();
						int column = e.getColumn();
						
						if (dsTable.getValueAt(row, column) == null) return;
						String value = dsTable.getValueAt(row, column).toString();
						String errInfo = "";

						boolean ok = true;
						if (column == 1)
						{
							ok = IPAddressFormatter.isValidIP(value);
				      if (!ok){
				      	errInfo = "The last input \"" + value + "\" in cell (" + row + "," + column+ ") " 
				      		+ "is not valid as an IP address";
				      	JOptionPane.showMessageDialog(ClusterSetting.this, 
										errInfo, "ERROR", JOptionPane.ERROR_MESSAGE);
				      	dsTable.setValueAt("", row, column);
				      }
						}
						else if (column == 3)
						{
							Integer port = 0;
							try{
								port = Integer.parseInt(value);
								if (port <= 1024 || port > 65535){
									errInfo = "A SECONDO monitor port number should be set " +
											"between (1024, 65535)";
									JOptionPane.showMessageDialog(ClusterSetting.this, 
											errInfo, "ERROR", JOptionPane.ERROR_MESSAGE);
									dsTable.setValueAt("", row, column);
									ok = false;
								}
							}
							catch(NumberFormatException nue){
								errInfo = "The last input\"" + value + "\" in cell (" + row + "," + column+ ") " 
									+ "is not valid as a SECONDO monitor port number. ";
								JOptionPane.showMessageDialog(ClusterSetting.this, 
										errInfo, "ERROR", JOptionPane.ERROR_MESSAGE);
								dsTable.setValueAt("", row, column);
								ok = false;
							}
						}
						
						if (ok){
							if (row < dataServers.size())
								dataServers.remove(row);
							
							String ip = dsTable.getValueAt(row, 1).toString();
							String path = dsTable.getValueAt(row, 2).toString();
							
							int port = 0;
							if (dsTable.getValueAt(row, 3) != null)
								port = Integer.parseInt(dsTable.getValueAt(row, 3).toString());
							
							DataServer.ROLES role = getDSRole(dsTable.getValueAt(row, 4).toString());
							DataServer ds = new DataServer(ip, path, port, role);
							
							if (!dataServers.contains(ds))
								dataServers.add(row, ds);
							else{
								errInfo = "The current row defines a duplicated Data Server, and will be deleted.";
								JOptionPane.showMessageDialog(ClusterSetting.this, errInfo,
										"Warning", JOptionPane.WARNING_MESSAGE);
								DefaultTableModel model = (DefaultTableModel)dsTable.getModel();
								model.removeRow(row);
							}
								
						}
						
						isClusterAvailable = false;
					}
				}
			}
		);
				
	}
	
	private void createResultFile()
	{
		if (!isClusterAvailable)
			return;
		
		boolean createFile = checkButton.isEnabled();
		boolean created = false;
		//If not create the file, then just return the dataServers to the parent window.
		
		try {
			SCReader parameters; 
			
			if (createFile)
			{
				//Read from the example file, which must exist
				String examplePath = System.getenv("SECONDO_BUILD_DIR")
				+ "/Algebras/Hadoop/clusterManagement/ParallelSecondoConfig.ini";
				parameters = new SCReader(examplePath);
			}
			else
				parameters = new SCReader();
			
			// 1. Set the cluster setting
			DataServer mds = null;  // Find the master Data Server
			for (DataServer ds : dataServers) {
				if (ds.isMaster()) {
					mds = ds;
					parameters.set("Cluster", "Master", ds.toString(),
							SCReader.MODE.SINGLE);
				}

				if (ds.isSlave()) {
					parameters.set("Cluster", "Slaves", ds.toString(),
							SCReader.MODE.MULTI);
				}
			}

			if (createFile)
			{
				// 2. Set the master IP in Hadoop section
				ArrayList<String> titles = parameters.getTitles("Hadoop");
				if (titles == null) {
					throw new IOException(
					"The Hadoop section is not defined in the example file");
				} else {
					for (String t : titles) {
						String value = parameters.get("Hadoop", t);
						if (value.contains("localhost")) {
							value = value.replaceAll("localhost", mds.getIp());
							parameters.set("Hadoop", t, value, SCReader.MODE.SINGLE);
						}
					}
				}
			}

			// 3. Set the ns4m
			if (ns4m_Check.isSelected()) {
				parameters.set("Options", "NS4Master", "true",
						SCReader.MODE.SINGLE);
			}

			if (checkButton.isEnabled()) {
				// ouput the file
				String SEC_BIN_PATH = System.getenv("SECONDO_BUILD_DIR") + "/bin";
				JFileChooser fc = new JFileChooser();
				fc.setSelectedFile(new File(SEC_BIN_PATH + "/ParallelSecondoConfig.ini"));
				int rtn = fc.showSaveDialog(ClusterSetting.this);
				if (rtn == JFileChooser.APPROVE_OPTION) {
					String outputPath = fc.getSelectedFile().getAbsolutePath();
					parameters.list(outputPath);
					JOptionPane.showMessageDialog(ClusterSetting.this,
							"The Parallel SECONDO configuration is created.", "INFO",
							JOptionPane.INFORMATION_MESSAGE);
					created = true;
				}
			} else {
				// return the value to the father window
				AdSetting pframe = (AdSetting) fatherWindow;
				pframe.setupCluster(parameters);
				created = true;
			}

			if (created){
				setVisible(false);
				dispose();
			}
		} catch (IOException exc) {
			JOptionPane.showMessageDialog(ClusterSetting.this,
					exc.getMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
		}

	}
	
	public void setTable(ArrayList<DataServer> dss, boolean ns4m)
	{
		DefaultTableModel model = (DefaultTableModel)dsTable.getModel();
		model.getDataVector().removeAllElements();
		
		for (DataServer ds : dss)
		{
			model.addRow(new Object[]{true, ds.getIp(), ds.getPath(), 
					ds.getPort(), ds.getRole().toString()});
			dataServers.add(ds);
		}
		
		ns4m_Check.setSelected(ns4m);
	}
	
	private DataServer.ROLES getDSRole(String value)
	{
		if (value.compareTo("Master") == 0)
			return DataServer.ROLES.MASTER;
		else if (value.compareTo("Master & Slave") == 0)
			return DataServer.ROLES.MASLAVE;
		
		return DataServer.ROLES.SLAVE;
	}
}
