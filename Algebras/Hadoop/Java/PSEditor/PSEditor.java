package PSEditor;

import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JLabel;
import java.awt.Font;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.SwingConstants;
import java.awt.Color;
import javax.swing.JTextField;
import javax.swing.JButton;
import java.awt.Component;
import javax.swing.JEditorPane;
import javax.swing.UIManager;

import java.io.*;
import java.text.ParseException;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.Dimension;

public class PSEditor {

	private JFrame frmParallelSecondoConfiguration;
	private JTextField ip_Text;
	private JTextField sec_Text;
	private JTextField hadoop_Text;
	private JTextField plat_Text;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					PSEditor window = new PSEditor();
					window.frmParallelSecondoConfiguration.setLocationRelativeTo(null);
					window.frmParallelSecondoConfiguration.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the application.
	 */
	public PSEditor() {
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		frmParallelSecondoConfiguration = new JFrame();
		frmParallelSecondoConfiguration.setResizable(false);
		frmParallelSecondoConfiguration.getContentPane().setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		frmParallelSecondoConfiguration.setTitle("Parallel SECONDO Configuration Editor");
		frmParallelSecondoConfiguration.setBounds(100, 100, 474, 279);
		frmParallelSecondoConfiguration.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		
		JLabel lblNewLabel = new JLabel("Current IP: ");
		lblNewLabel.setHorizontalAlignment(SwingConstants.RIGHT);
		lblNewLabel.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		JLabel lblSecondoInstallation = new JLabel("SECONDO: ");
		lblSecondoInstallation.setHorizontalAlignment(SwingConstants.RIGHT);
		lblSecondoInstallation.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		JLabel lblHadoop = new JLabel("Hadoop: ");
		lblHadoop.setHorizontalAlignment(SwingConstants.RIGHT);
		lblHadoop.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		JLabel lblPlatfrom = new JLabel("Platform: ");
		lblPlatfrom.setHorizontalAlignment(SwingConstants.RIGHT);
		lblPlatfrom.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		boolean ok = true;		//Check the environment
		
		ip_Text = new JTextField();
		ip_Text.setEnabled(false);
		ip_Text.setEditable(false);
		ip_Text.setColumns(10);
		ip_Text.setText(Functions.get_localIP());
		if (ip_Text.getText().isEmpty()){
			ip_Text.setEnabled(true);
			ip_Text.setForeground(Color.red);
			ip_Text.setText("Cannot detect the IP address.");
			ok = false;
		}
		
		String secPath = System.getenv("SECONDO_BUILD_DIR");
		String haaPath = secPath + "/Algebras/Hadoop";
		sec_Text = new JTextField();
		sec_Text.setEnabled(false);
		sec_Text.setEditable(false);
		sec_Text.setColumns(10);
		if (new File(haaPath).isDirectory()){
			sec_Text.setText(secPath);
		}
		else{
			sec_Text.setEnabled(true);
			sec_Text.setText("Cannot find the SECONDO installation.");
			sec_Text.setForeground(Color.red);
			ok = false;
		}
		
		String hadoopPath = secPath + "/bin/hadoop-0.20.2.tar.gz";
		File hadoopArchive = new File(hadoopPath);
		hadoop_Text = new JTextField();
		hadoop_Text.setEnabled(false);
		hadoop_Text.setEditable(false);
		hadoop_Text.setColumns(10);
		if (hadoopArchive.isFile()){
			hadoop_Text.setText(hadoopPath);
		}
		else{
			hadoop_Text.setEnabled(true);
			hadoop_Text.setText("No Hadoop archive in $SECONDO_BUILD_DIR/bin.");
			hadoop_Text.setForeground(Color.red);
			ok = false;
		}
		
		plat_Text = new JTextField();
		plat_Text.setEnabled(false);
		plat_Text.setEditable(false);
		plat_Text.setColumns(10);
		plat_Text.setText(System.getenv("SECONDO_PLATFORM"));
		if (plat_Text.getText().isEmpty()){
			plat_Text.setEnabled(true);
			plat_Text.setForeground(Color.red);
			plat_Text.setText("Cannot detect the current platform.");
			ok = false;
		}
				
		btnImport = new JButton("Import");
		btnImport.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				AdSetting asFrame = new AdSetting();
				int rtn = asFrame.importPSCFile();
				if (rtn == 0)
				{
					asFrame.setLocationRelativeTo(frmParallelSecondoConfiguration);
					asFrame.setVisible(true);
				}
			}
		});
		btnImport.setToolTipText("Import an exisiting Parallel SECONDO configuration file. ");
		btnSingleNode = new JButton("Single Node");
		btnSingleNode.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				try {
					SingleNode sn = new SingleNode();
					sn.setLocationRelativeTo(frmParallelSecondoConfiguration);
					sn.setVisible(true);
				} catch (ParseException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
			}
		});
		
		btnSingleNode.setToolTipText("Install Parallel SECONDO on a single computer. ");
		btnSimpleCluster = new JButton("Simple Cluster");
		btnSimpleCluster.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				ClusterSetting csFrame = new ClusterSetting();
				csFrame.setLocationRelativeTo(frmParallelSecondoConfiguration);
				csFrame.setVisible(true);
			}
		});
		
		btnSimpleCluster.setToolTipText("Install Parallel SECONDO on a simple computer cluster. ");
		btnAdvancedSetting = new JButton("Advanced");
		btnAdvancedSetting.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				AdSetting asFrame = new AdSetting();
				asFrame.setLocationRelativeTo(frmParallelSecondoConfiguration);
				asFrame.setVisible(true);
			}
		});
		btnAdvancedSetting.setToolTipText("Prepare the configuration file for advanced users. ");
		
		warnText = new JEditorPane();
		warnText.setFont(new Font("Lucida Grande", Font.BOLD, 13));
		warnText.setEditable(false);
		warnText.setBackground(UIManager.getColor("Button.background"));
		if (!ok){
			warnText.setForeground(Color.RED);
			warnText.setText("  Cannot create the Parallel SECONDO configuration file.");
		}
		else{
			warnText.setText("  The current environment is OK !");
		}
		
		GroupLayout groupLayout = new GroupLayout(frmParallelSecondoConfiguration.getContentPane());
		groupLayout.setHorizontalGroup(
			groupLayout.createParallelGroup(Alignment.TRAILING)
				.addGroup(groupLayout.createSequentialGroup()
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addGroup(groupLayout.createSequentialGroup()
							.addContainerGap()
							.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
								.addGroup(Alignment.LEADING, groupLayout.createSequentialGroup()
									.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
										.addComponent(lblNewLabel, Alignment.TRAILING)
										.addComponent(lblSecondoInstallation, Alignment.TRAILING)
										.addComponent(lblHadoop, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 82, GroupLayout.PREFERRED_SIZE)
										.addComponent(lblPlatfrom, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 82, GroupLayout.PREFERRED_SIZE))
									.addPreferredGap(ComponentPlacement.RELATED)
									.addGroup(groupLayout.createParallelGroup(Alignment.LEADING, false)
										.addComponent(sec_Text, GroupLayout.DEFAULT_SIZE, 350, Short.MAX_VALUE)
										.addComponent(hadoop_Text, GroupLayout.DEFAULT_SIZE, 350, Short.MAX_VALUE)
										.addComponent(plat_Text, GroupLayout.DEFAULT_SIZE, 350, Short.MAX_VALUE)
										.addComponent(ip_Text)))
								.addGroup(groupLayout.createSequentialGroup()
									.addGroup(groupLayout.createParallelGroup(Alignment.TRAILING)
										.addComponent(btnSingleNode)
										.addComponent(btnAdvancedSetting, GroupLayout.PREFERRED_SIZE, 140, GroupLayout.PREFERRED_SIZE))
									.addPreferredGap(ComponentPlacement.UNRELATED)
									.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
										.addComponent(btnImport, Alignment.TRAILING)
										.addComponent(btnSimpleCluster, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 119, GroupLayout.PREFERRED_SIZE))))
							.addPreferredGap(ComponentPlacement.RELATED, GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
						.addComponent(warnText, GroupLayout.DEFAULT_SIZE, 448, Short.MAX_VALUE))
					.addContainerGap())
		);
		groupLayout.setVerticalGroup(
			groupLayout.createParallelGroup(Alignment.LEADING)
				.addGroup(groupLayout.createSequentialGroup()
					.addContainerGap()
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblNewLabel)
						.addComponent(ip_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblSecondoInstallation, GroupLayout.PREFERRED_SIZE, 19, GroupLayout.PREFERRED_SIZE)
						.addComponent(sec_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblHadoop, GroupLayout.PREFERRED_SIZE, 19, GroupLayout.PREFERRED_SIZE)
						.addComponent(hadoop_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.LEADING)
						.addComponent(lblPlatfrom, GroupLayout.PREFERRED_SIZE, 19, GroupLayout.PREFERRED_SIZE)
						.addComponent(plat_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnSimpleCluster)
						.addComponent(btnSingleNode))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addGroup(groupLayout.createParallelGroup(Alignment.BASELINE)
						.addComponent(btnImport)
						.addComponent(btnAdvancedSetting))
					.addPreferredGap(ComponentPlacement.RELATED, 21, Short.MAX_VALUE)
					.addComponent(warnText, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
		);
		groupLayout.linkSize(SwingConstants.HORIZONTAL, new Component[] {btnImport, btnSingleNode, btnSimpleCluster, btnAdvancedSetting});
		frmParallelSecondoConfiguration.getContentPane().setLayout(groupLayout);
	}
	private JButton btnSingleNode;
	private JButton btnSimpleCluster;
	private JButton btnAdvancedSetting;
	private JButton btnImport;
	private JEditorPane warnText;
}
