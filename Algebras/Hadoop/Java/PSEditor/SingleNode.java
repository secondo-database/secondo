package PSEditor;

import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JLabel;
import javax.swing.JFormattedTextField;
import java.awt.Font;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.JTextField;
import java.text.ParseException;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JOptionPane;
import javax.swing.SwingConstants;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.IOException;

public class SingleNode extends JFrame {

	private JPanel contentPane;
	private JTextField ip_Text;
	private JButton create_Button;
	private JButton cancel_Button;
	private JCheckBox ns4m_Check;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					SingleNode frame = new SingleNode();
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
	public SingleNode() throws ParseException{
		setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
		setBounds(100, 100, 296, 150);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		
		JLabel lblNewLabel = new JLabel("Target IP Address: ");
		lblNewLabel.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		ip_Text = new JFormattedTextField(new IPAddressFormatter());
		ip_Text.setText(Functions.get_localIP());
		
		create_Button = new JButton("Create");
		create_Button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
/*
Here the ParallelSecondoConfig.ini file is prepared. 
It reads the example file that is set in the Hadoop algebra, 
  1. adds the cluster setting
  2. changes "localhost" related setting in the hadoop setting to the given IP address
  3. set up the NS4Master if necessary 
  
*/
				String examplePath = System.getenv("SECONDO_BUILD_DIR") 
				  + "/Algebras/Hadoop/clusterManagement/ParallelSecondoConfig.ini";
				String outputPath = System.getenv("SECONDO_BUILD_DIR")
					+ "/bin/ParallelSecondoConfig.ini";
				try {
					SCReader parameters = new SCReader(examplePath);
					
					//1. 
					//Single-computer Data Server, it is set on the directory /tmp by default,
					//also the monitor port is set to be the default value 11234
					String CurrDS = ip_Text.getText() + ":/tmp:11234"; 
					parameters.set("Cluster", "Master", CurrDS, SCReader.MODE.SINGLE);
					parameters.set("Cluster", "Slaves", CurrDS, SCReader.MODE.MULTI);
					
					//2. 
					ArrayList<String> titles = parameters.getTitles("Hadoop");
					if (titles == null){
						throw new IOException("The Hadoop section is not defined in the example file");
					}
					else
					{
						for (String t : titles)
						{
							String value = parameters.get("Hadoop", t);
							if (value.contains("localhost"))
							{
								value = value.replaceAll("localhost", ip_Text.getText());
								parameters.set("Hadoop", t, value, SCReader.MODE.SINGLE);
							}
						}
					}
					
					//3. 
					if (ns4m_Check.isSelected()){
						parameters.set("Options", "NS4Master", "true", SCReader.MODE.SINGLE);
					}

					//Output the file
					parameters.list(outputPath);
					
				} catch (IOException e) {
					// TODO Auto-generated catch block
					JOptionPane.showMessageDialog(SingleNode.this, 
							e.getMessage(), "ERROR", JOptionPane.ERROR_MESSAGE);
					e.printStackTrace();
				}
				
				JOptionPane.showMessageDialog(SingleNode.this,
		    "<html>The configuration file for the single computer" + ip_Text.getText()
		    + "<br/>is prepared at " + outputPath + "</html>");
				setVisible(false);
				dispose();
			}
		});
		
		cancel_Button = new JButton("Cancel");
		cancel_Button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				setVisible(false);
				dispose();
			}
		});
		
		ns4m_Check = new JCheckBox("<html>Set the existing SECONDO database as the master database</html>");
		ns4m_Check.setVerticalAlignment(SwingConstants.TOP);
		
		GroupLayout gl_contentPane = new GroupLayout(contentPane);
		gl_contentPane.setHorizontalGroup(
			gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addGroup(gl_contentPane.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_contentPane.createSequentialGroup()
							.addGap(97)
							.addComponent(create_Button)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(cancel_Button, GroupLayout.PREFERRED_SIZE, 84, GroupLayout.PREFERRED_SIZE))
						.addGroup(gl_contentPane.createSequentialGroup()
							.addContainerGap()
							.addComponent(lblNewLabel)
							.addGap(15)
							.addComponent(ip_Text, GroupLayout.PREFERRED_SIZE, 123, GroupLayout.PREFERRED_SIZE))
						.addComponent(ns4m_Check, GroupLayout.DEFAULT_SIZE, 280, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_contentPane.setVerticalGroup(
			gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPane.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblNewLabel)
						.addComponent(ip_Text, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(2)
					.addComponent(ns4m_Check, GroupLayout.PREFERRED_SIZE, 35, Short.MAX_VALUE)
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_contentPane.createParallelGroup(Alignment.BASELINE)
						.addComponent(create_Button)
						.addComponent(cancel_Button))
					.addGap(23))
		);
		contentPane.setLayout(gl_contentPane);
	}
}
