package PSEditor;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.FlowLayout;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.SwingWorker;
import javax.swing.border.EmptyBorder;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JLabel;
import java.awt.Font;
import javax.swing.JScrollPane;
import javax.swing.LayoutStyle.ComponentPlacement;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.concurrent.ExecutionException;

import javax.swing.JTextArea;

public class CheckMsgDlg extends JDialog {

	static private final String newline = "\n";
	private final JPanel contentPanel = new JPanel();
	private JTextArea msgText; 
	private JButton okButton;
	
	private List<DataServer> cluster;
	private String userName;
	private boolean ns4m, selfClose;
	private boolean available;
	private boolean chkHadoop = false;
	private List<Integer> hdpPorts;
	

	/**
	 * Create the dialog.
	 */
	public CheckMsgDlg() {
				
		available = false;
		setTitle("Cluster Checking");
		setBounds(100, 100, 450, 300);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		
		JLabel lblNewLabel = new JLabel("Checking the defined cluster now: ");
		lblNewLabel.setFont(new Font("Lucida Grande", Font.PLAIN, 15));
		
		JScrollPane scrollPane = new JScrollPane();
		scrollPane.setVerticalScrollBarPolicy(
				JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED); 
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(scrollPane, GroupLayout.PREFERRED_SIZE, 429, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblNewLabel))
					.addContainerGap(GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
		);
		gl_contentPanel.setVerticalGroup(
			gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
					.addContainerGap()
					.addComponent(lblNewLabel)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 192, Short.MAX_VALUE)
					.addContainerGap())
		);
		
		msgText = new JTextArea();
		msgText.setEditable(false);
		msgText.setLineWrap(true);
		msgText.setForeground(Color.BLACK);
		
		scrollPane.setViewportView(msgText);
		
		
		contentPanel.setLayout(gl_contentPanel);
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				okButton = new JButton("OK");
				okButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						setVisible(false);
						dispose();
					}
				});
				okButton.setActionCommand("OK");
				buttonPane.add(okButton);
				getRootPane().setDefaultButton(okButton);
			}
		}
	}
	
	public boolean CheckCluster(String uname, 
			ArrayList<DataServer> c, 
			SCReader hdPara, // Hadoop parameters
			boolean n,	//ns4m 
			boolean sc)	//self close the window
	{
		setVisible(true);
		
		if (c == null)
			return false;
		
		if (c.isEmpty()){
			msgText.append("The cluster is empty!");
			return false;
		}
		
		cluster = new ArrayList<DataServer>(c.size());
		for (DataServer ds : c){
			cluster.add(ds);
		}
		userName = uname;
		ns4m = n;
		selfClose = sc;
		
		if (hdPara != null)
		{
			ArrayList<String> hdpTitles = hdPara.getTitles("Hadoop");
			
			if (!hdpTitles.isEmpty())
			{
				chkHadoop = true;
				hdpPorts = new ArrayList<Integer>();
				for (String t : hdpTitles)
				{
					String value = hdPara.get("Hadoop", t);
					if (value.contains(":"))
					{
						String sport = value.substring(value.lastIndexOf(":") + 1);
						try
						{
							int port = Integer.parseInt(sport);
							hdpPorts.add(port);
						} catch (NumberFormatException e){
							System.err.println("Warning!! Invalid port number " + sport);
						}
					}
				}
				if (hdpPorts.isEmpty())
					chkHadoop = false;
			}
		}
		
		okButton.setEnabled(false);
		
		CheckWorker worker = new CheckWorker();
		worker.execute();

    return isAvailable();
	}

	public boolean isAvailable()
	{
		return available;
	}

	public boolean willCreateFile()
	{
		return selfClose;
	}

	
	class CheckWorker extends SwingWorker<Boolean, String>
	{
		private boolean running = true;
		boolean chkStatus = true;
		
		
		@Override
		protected Boolean doInBackground() throws Exception{
			running = true;
			chkStatus = true;
			
			ArrayList<String> nodeIPs = new ArrayList<String>();
			for (DataServer ds : cluster)
			{
				String ip = ds.getIp().trim();
				if (!nodeIPs.contains(ip))
					nodeIPs.add(ip);
			}
			
			String cmd;
			boolean rtn;
			try{
				
				boolean mfound = false; //find the master
				int slaveCnt = 0;				//slave counter
				for (DataServer ds: cluster)
				{
					if (ds.getRole() == DataServer.ROLES.SLAVE){
						slaveCnt++;
					}
					else
					{
						if (ds.getRole() == DataServer.ROLES.MASLAVE)
							slaveCnt++;
						
						if (!mfound)
							mfound = true;
						else
						{
							publish("ERROR! Multiple master Data Server is set.");
							chkStatus = false;
							break;
						}
					}
				}
				if (!mfound){
					publish("ERROR! No Master Data Server is defined. ");
					chkStatus = false;
				}
				if (slaveCnt == 0){
					publish("ERROR! There is no one slave Data Server defined.");
					chkStatus = false;
				}
				if (!chkStatus){
					throw new Exception("Check failed");
				}
				
				publish("------------------------------------------------");
				
				//Check the accessibility of all cluster nodes
				for (String ip : nodeIPs)
				{
					cmd = "ssh -q " +
							"-o BatchMode=yes " +
							"-o ConnectTimeout=3 " +
							userName + "@" + ip + " echo 1>/dev/null";
					
					rtn = runCommand(cmd);
					if (rtn){
						publish("Node " + ip + " is accessible.");
					}
					else{
						publish("ERROR! Node " + ip + 
						" is NOT accessible without passphrase.");
						chkStatus = false;
						break;
					}
				}
				if (!chkStatus){
					throw new Exception("Check failed");
				}

				publish("------------------------------------------------");
				
				//Check screen installed on every machine
				for (String ip : nodeIPs)
				{
					cmd = "ssh -q " +
					"-o BatchMode=yes " +
					"-o ConnectTimeout=3 " +
					userName + "@" + ip + 
					" which screen >/dev/null";

					rtn = runCommand(cmd);
					if (rtn){
						publish("Node " + ip + " has the program 'screen'.");
					}
					else{
						publish("ERROR! Node " + ip + 
						" NOT have the program 'screen'.");
						chkStatus = false;
						break;
					}
					if (!chkStatus)
						return chkStatus;
				}
					
				publish("------------------------------------------------");

				// Check paths exist on all nodes
				// TODO also check the R/W grant
				for (DataServer ds : cluster) 
				{
					cmd = "ssh -q " + "-o BatchMode=yes " + "-o ConnectTimeout=3 "
							+ userName + "@" + ds.getIp() + " ls " + ds.getPath()
							+ " 2>&1 1>/dev/null";

					rtn = runCommand(cmd);

					if (rtn) {
						publish("Check Data Server " + ds.getIp() + ":" + ds.getPath()
								+ " SUCCESS.");
					} else {
						publish("Check Data Server " + ds.getIp() + ":" + ds.getPath()
								+ " FAILS. ");
						chkStatus = false;
						break;
					}
				}
				if (!chkStatus){
					throw new Exception("Check failed");
				}
					
				
				publish("------------------------------------------------");
				
				//Check the ports are not used by any running nodes
				for (DataServer ds : cluster)
				{
					cmd = "ssh -q " +
					"-o BatchMode=yes " +
					"-o ConnectTimeout=3 " +
					userName + "@" + ds.getIp() + 
					" lsof -i:" + ds.getPort() + " 2>&1 1>/dev/null";
					
					//Here if the port is taken then return TRUE, or else return FALSE
					rtn = runCommand(cmd);
					
					if (!rtn){
						publish("Port " + ds.getPort() + " is available on node " +
								ds.getIp());
					}
					else{
						publish("Port " + ds.getPort() + " is NOT available on node " +
								ds.getIp());
						chkStatus = false;
						break;
					}
				}
				if (!chkStatus){
					throw new Exception("Check failed");
				}
				
				// Check Hadoop ports
				if (chkHadoop)
				{
					publish("------------------------------------------------");
					Set<Integer> set = new HashSet<Integer>(hdpPorts);
					if (set.size() < hdpPorts.size()){
						publish("There exist duplicated Hadoop port setting.");
						chkStatus = false;
					}
					if (!chkStatus){
						throw new Exception("Check failed");
					}
					
					for (String ip : nodeIPs)
					{
						for ( int port : hdpPorts )
						{
							cmd = "ssh -q " +
							"-o BatchMode=yes " +
							"-o ConnectTimeout=3 " +
							userName + "@" + ip + 
							" lsof -i:" + port + " 2>&1 1>/dev/null";
							
							rtn = runCommand(cmd);
							if (!rtn){
								publish("Port " + port + " is available on node " + ip);
							}
							else{
								publish("Port " + port + " is NOT available on node " + ip);
								chkStatus = false;
								break;
							}
						}

						if (!chkStatus){
							throw new Exception("Check failed");
						}
					}
				}
				
				publish("------------------------------------------------");
				publish("All Checks are done, " +
						"and the cluster is available to install Parallel SECONDO on it.");
				
			} catch (Exception e){
				publish(e.toString());
			}
			finally {
				running = false;
				return chkStatus;
			}
		}
		
		public boolean isRunning()
		{
			return running;
		}
		
		public boolean checkResult()
		{
			return chkStatus;
		}
		
		protected boolean runCommand (String cmd) throws InterruptedException, IOException
		{
			Runtime run = Runtime.getRuntime();
			Process p;
			BufferedInputStream in;
			BufferedReader inBr;
			String rtnLine;
			boolean result;
			
			p = run.exec(cmd);
			in = new BufferedInputStream(p.getInputStream());
			inBr = new BufferedReader(new InputStreamReader(in));
			
			while ((rtnLine = inBr.readLine()) != null){
				publish(rtnLine);
			}
			
			if (p.waitFor() != 0){
				result = false;
			}
			else {
				result = true;
			}
			
			inBr.close();
			in.close();
			return result;
		}
		
		@Override
		protected void process(List<String> chunks){
			for (String chunk : chunks){ 
				msgText.append(chunk + newline);
				msgText.setCaretPosition(msgText.getText().length() - 1);
			}
		}

		/* (non-Javadoc)
		 * @see javax.swing.SwingWorker#done()
		 */
		@Override
		protected void done() {
			// TODO Auto-generated method stub
			try {
				available = get();
				okButton.setEnabled(true);
				if (selfClose && isAvailable()){
					setVisible(false);
					dispose();
				}

				
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (ExecutionException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		
	}
	
	
}
