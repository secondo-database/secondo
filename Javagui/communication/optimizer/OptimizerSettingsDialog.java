package communication.optimizer;

import sj.lang.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;


public class OptimizerSettingsDialog extends JDialog{


/** creates a new Dialog */
public OptimizerSettingsDialog(JFrame parent){
  super(parent,true);
  setSize(400,400);
  setTitle("Optimizer-Settings");
  getContentPane().setLayout(new BorderLayout());
  JPanel ControlPanel = new JPanel();
  ControlPanel.add(OKBtn);
  ControlPanel.add(CANCELBtn);
  ActionListener BtnListener = new ActionListener(){
     public void actionPerformed(ActionEvent evt){
         Object source = evt.getSource();
	 if(source.equals(OKBtn)){
	      accept();
	 }
	 if(source.equals(CANCELBtn)){
	    abort();
	 }
     }
  };
  OKBtn.addActionListener(BtnListener);
  CANCELBtn.addActionListener(BtnListener);
  getContentPane().add(ControlPanel,BorderLayout.SOUTH);
  JPanel InputPanel = new JPanel(new GridLayout(4,4));
  InputPanel.add(new JLabel("Host"));
  InputPanel.add(HostName);
  InputPanel.add(new JLabel("Port"));
  InputPanel.add(PortNumber);
  JPanel P3 = new JPanel();
  P3.add(InputPanel);
  getContentPane().add(P3);
  Host="localhost";
  Port=1235;
}


/** accept the changes */
private void accept(){
 String H = HostName.getText().trim();
 if(H.equals("")){
    JOptionPane.showMessageDialog(this,"the hostname must not be empty");
    return;
 }
 try{
    int P = Integer.parseInt(PortNumber.getText());
    if(P<=0){
        JOptionPane.showMessageDialog(this,"the portnumber must be greater then zero");
	return;
    }
    Host = H;
    Port = P;
    accepted = true;
    setVisible(false);
 }catch(Exception e){
     JOptionPane.showMessageDialog(this,"the portnumber is not a valid integer");
     return;
 }
}


/** gets the changes back */
private void abort(){
   HostName.setText(Host);
   PortNumber.setText(""+Port);
   accepted=false;
   setVisible(false);
}


/** makes this dialog visible
  * if the OK button is pressed true is returned
  * otherwise the result will be false
  */
public boolean showDialog(){
   accepted = false;
   PortNumber.setText(""+Port);
   HostName.setText(Host);
   setVisible(true);
   return accepted;
}


/** returns the current HostName */
public String getHost(){
   return Host;
}


/** returns the current PortNumber */
public int getPort(){
   return Port;
}


/** sets the values in this dialog */
public void setConnection(String Host, int Port){
  this.Host = Host;
  this.Port = Port;
}

private JButton OKBtn= new JButton("Ok");
private JButton CANCELBtn = new JButton("Cancel");
private JTextField HostName = new JTextField(20);
private JTextField PortNumber = new JTextField(5);
private String Host;
private int Port;
private boolean accepted;


}