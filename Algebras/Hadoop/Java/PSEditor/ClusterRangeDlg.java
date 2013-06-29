package PSEditor;

import java.awt.BorderLayout;
import java.awt.FlowLayout;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.border.LineBorder;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JLabel;
import java.awt.Font;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.SwingConstants;
import java.awt.Component;
import java.awt.Color;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

public class ClusterRangeDlg extends JDialog {

	private final JPanel contentPanel = new JPanel();
	private JTextField sIP_Text;
	private JTextField dIP_Text;
	private JTable locTable;
	private JScrollPane scrollPane;
	
	private int sIPLast, dIPLast;
	private ArrayList<String> dss;
	private boolean prepared;
	
	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		try {
			ClusterRangeDlg dialog = new ClusterRangeDlg();
			dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * Create the dialog.
	 */
	public ClusterRangeDlg() {
		prepared = false;
		dss = new ArrayList<String>();
		
		setTitle("A Set of Data Servers");
		setBounds(100, 100, 368, 329);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		JLabel lblNewLabel = new JLabel("IP Range: ");
		lblNewLabel.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		sIP_Text = new JFormattedTextField(new IPAddressFormatter());
		sIP_Text.setColumns(10);
		sIP_Text.setText(Functions.get_localIP());
		dIP_Text = new JFormattedTextField(new IPAddressFormatter());
		dIP_Text.setColumns(10);
		dIP_Text.setText(Functions.get_localIP());
		JLabel lblTo = new JLabel("TO");
		lblTo.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		
		
		JLabel lblNewLabel_1 = new JLabel("<html>NOTE!! Here the IP address increases only on the last byte within the given range. </html>");
		lblNewLabel_1.setForeground(Color.RED);
		
		JButton btnAdd = new JButton("add");
		btnAdd.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				DefaultTableModel model = (DefaultTableModel)(locTable.getModel());
				model.addRow(new Object[]{""});
			}
		});
		
		JButton btnDelete = new JButton("delete");
		btnDelete.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				int drs[] = locTable.getSelectedRows();
				
				if (drs.length > 0)
				{
					DefaultTableModel model = (DefaultTableModel)locTable.getModel();
					Integer[] odrs = new Integer[drs.length];
					int i = 0;
					for (int v : drs){
						odrs[i++] = Integer.valueOf(v);
					}
					Arrays.sort(odrs, Collections.reverseOrder());
					
					for (int r : odrs){
						model.removeRow(r);
					}
				}
			}
		});
		
		JLabel lblDataServerLocations = new JLabel("Data Server Locations on each node: ");
		lblDataServerLocations.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		scrollPane = new JScrollPane();
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPanel.createSequentialGroup()
							.addComponent(scrollPane, GroupLayout.PREFERRED_SIZE, 341, GroupLayout.PREFERRED_SIZE)
							.addContainerGap())
						.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
							.addGroup(gl_contentPanel.createSequentialGroup()
								.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
									.addGroup(gl_contentPanel.createSequentialGroup()
										.addGap(1)
										.addComponent(sIP_Text, GroupLayout.PREFERRED_SIZE, 154, GroupLayout.PREFERRED_SIZE)
										.addPreferredGap(ComponentPlacement.RELATED)
										.addComponent(lblTo, GroupLayout.PREFERRED_SIZE, 25, GroupLayout.PREFERRED_SIZE)
										.addGap(6)
										.addComponent(dIP_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
									.addComponent(lblNewLabel)
									.addGroup(gl_contentPanel.createSequentialGroup()
										.addComponent(btnAdd)
										.addPreferredGap(ComponentPlacement.RELATED)
										.addComponent(btnDelete, GroupLayout.PREFERRED_SIZE, 75, GroupLayout.PREFERRED_SIZE))
									.addComponent(lblNewLabel_1))
								.addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
							.addGroup(gl_contentPanel.createSequentialGroup()
								.addComponent(lblDataServerLocations, GroupLayout.DEFAULT_SIZE, 343, Short.MAX_VALUE)
								.addGap(9)))))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addComponent(lblNewLabel)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(sIP_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblTo, GroupLayout.PREFERRED_SIZE, 19, GroupLayout.PREFERRED_SIZE)
						.addComponent(dIP_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(lblDataServerLocations, GroupLayout.PREFERRED_SIZE, 19, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 90, Short.MAX_VALUE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnAdd)
						.addComponent(btnDelete))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(lblNewLabel_1)
					.addGap(11))
		);
		gl_contentPanel.linkSize(SwingConstants.HORIZONTAL, new Component[] {btnAdd, btnDelete});
		gl_contentPanel.linkSize(SwingConstants.HORIZONTAL, new Component[] {sIP_Text, dIP_Text});
		
		Object[] header = {"Data Server Location", "Port"};
		Object[][] data = {{"/tmp", 11234}};
		DefaultTableModel model = new DefaultTableModel(data, header);
		
		locTable = new JTable(model){
			private static final long serialVersionUID = 1L;

			@Override
      public Class getColumnClass(int column) {
          switch (column) {
              case 0:
                  return String.class;
              case 1: 
              		return Integer.class;
              default:
                  return String.class;
          }
      }
		};
		locTable.setFont(new Font("Lucida Grande", Font.PLAIN, 13));
		locTable.setBorder(new LineBorder(new Color(0, 0, 0)));
		locTable.setGridColor(Color.BLACK);
		
		
		
		scrollPane.setViewportView(locTable);
		contentPanel.setLayout(gl_contentPanel);
		
		locTable.getModel().addTableModelListener(new TableModelListener(){
			public void tableChanged(TableModelEvent e){
				if (e.getType() == TableModelEvent.UPDATE){
					int row = e.getLastRow();
					int column = e.getColumn();
					
					if (locTable.getValueAt(row, column) == null) return;
					String value = locTable.getValueAt(row, column).toString();
					String errInfo = "";
					
					if (column == 1)
					{
						//Check the port setting
						try {
							int port = Integer.parseInt(value);
							if (port <= 1024 || port > 65535){
								errInfo = "A SECONDO monitor port number should be set " +
										"between (1024, 65535)";
								JOptionPane.showMessageDialog(ClusterRangeDlg.this, 
										errInfo, "ERROR", JOptionPane.ERROR_MESSAGE);
								locTable.setValueAt("", row, column);
							}
						}
						catch(NumberFormatException nue){
							errInfo = "The last input\"" + value + "\" in cell (" + row + "," + column+ ") " 
								+ "is not valid as a SECONDO monitor port number. ";
							JOptionPane.showMessageDialog(ClusterRangeDlg.this, 
									errInfo, "ERROR", JOptionPane.ERROR_MESSAGE);
							locTable.setValueAt("", row, column);
						}
					}
				}
			}
		});
		
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				JButton okButton = new JButton("OK");
				okButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						//Check whether the IP range set correctly.
						
						boolean ok = true;
						if ( !(IPAddressFormatter.isValidIP(sIP_Text.getText()) 
								&& IPAddressFormatter.isValidIP(dIP_Text.getText()))){
							ok = false;
						}
						
						String[] sIP = sIP_Text.getText().trim().split("[.]");
						String[] dIP = dIP_Text.getText().trim().split("[.]");
						
						if (ok){
							for (int i = 0; i < 3; i++){
								if (sIP[i].compareTo(dIP[i]) != 0){
									ok = false;
									break;
								}
							}
						}
						
						sIPLast = Integer.parseInt(sIP[3]);
						dIPLast = Integer.parseInt(dIP[3]);
						
						if (sIPLast > dIPLast){
							ok = false;
						}
						
						if (!ok){
							JOptionPane.showMessageDialog(ClusterRangeDlg.this, 
									"The IP range is not correctly set. ", 
									"ERROR", JOptionPane.ERROR_MESSAGE);
							return;
						}
						
						ArrayList<String> paths = new ArrayList<String>();
						ArrayList<Integer> ports = new ArrayList<Integer>();
						dss.clear();
						for (int i = 0; i < locTable.getRowCount(); i++){
							String path = (String)locTable.getValueAt(i, 0);
							if (!path.isEmpty() && locTable.getValueAt(i,1) != null){
								int port = (Integer)locTable.getValueAt(i, 1);
								
								if (paths.contains(path) || ports.contains(port)){
									JOptionPane.showMessageDialog(ClusterRangeDlg.this, 
											"Repeated Data Servers are set", 
											"ERROR", JOptionPane.ERROR_MESSAGE);
									return;
								}
								else{
									paths.add(path);
									ports.add(port);
									dss.add(path + ":" + port);
								}
							}
						}
						
						prepared = true;
						setVisible(false);
					}
				});
				okButton.setActionCommand("OK");
				buttonPane.add(okButton);
				getRootPane().setDefaultButton(okButton);
			}
			{
				JButton cancelButton = new JButton("Cancel");
				cancelButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						setVisible(false);
					}
				});
				cancelButton.setActionCommand("Cancel");
				buttonPane.add(cancelButton);
			}
		}
	}
	
	public ArrayList<String> getIPRange()
	{
		ArrayList<String> range = new ArrayList<String>();
		if (!prepared)
			return range;
		
		String ipPrefix = sIP_Text.getText().substring(0, sIP_Text.getText().lastIndexOf("."));
		
		for (int ip = sIPLast; ip <= dIPLast; ip++){
			range.add(ipPrefix + "." + ip);
		}
		
		return range;
	}
	
	public ArrayList<String> getDSList()
	{
		return dss;
	}
}
