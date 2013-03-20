package utilgui;

import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextArea;
public class Warnung implements ActionListener
{
 JButton warnungtext_ok;
 JFrame warnungtext_frame;
 public Warnung(Exception warnung)
 {
  JTextArea warnungtext=new JTextArea(3,22);
  warnungtext.setText("\n"+warnung+"\n");
  warnungtext.setEditable(false);
  warnungtext_ok=new JButton("Ok");
  warnungtext_ok.setBackground(Color.white);
  warnungtext_ok.addActionListener(this);
  GridBagConstraints warnungtext_anzeige=new GridBagConstraints();
  GridBagLayout warnungtext_layout=new GridBagLayout();
  warnungtext_anzeige.gridx=0;
  warnungtext_anzeige.gridy=0;  
  warnungtext_anzeige.gridwidth=GridBagConstraints.REMAINDER;
  warnungtext_anzeige.fill=GridBagConstraints.CENTER;
  warnungtext_anzeige.insets=new Insets(10,10,10,10);
  warnungtext_layout.setConstraints(warnungtext,warnungtext_anzeige);
  warnungtext_anzeige.gridx=0;
  warnungtext_anzeige.gridy=1;  
  warnungtext_anzeige.gridwidth=GridBagConstraints.REMAINDER;
  warnungtext_anzeige.fill=GridBagConstraints.CENTER;
  warnungtext_anzeige.insets=new Insets(10,10,10,10);
  warnungtext_layout.setConstraints(warnungtext_ok,warnungtext_anzeige);
  JPanel warnungtext_panel=new JPanel();
  warnungtext_panel.setLayout(warnungtext_layout);
  warnungtext_panel.setBackground(Color.white);
  warnungtext_panel.add(warnungtext);
  warnungtext_panel.add(warnungtext_ok);  
  warnungtext_frame=new JFrame();
  warnungtext_frame.setTitle("Warnung");
  warnungtext_frame.getContentPane().add(warnungtext_panel);    
  warnungtext_frame.pack();
  warnungtext_frame.setLocationByPlatform(true);
  warnungtext_frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
  warnungtext_frame.setVisible(true); 
 }
 
 public Warnung(String warnung)
 {
  JTextArea warnungtext=new JTextArea(3,22);
  warnungtext.setText("\n"+warnung+"\n");
  warnungtext.setEditable(false);
  warnungtext_ok=new JButton("Ok");
  warnungtext_ok.setBackground(Color.white);
  warnungtext_ok.addActionListener(this);
  GridBagConstraints warnungtext_anzeige=new GridBagConstraints();
  GridBagLayout warnungtext_layout=new GridBagLayout();
  warnungtext_anzeige.gridx=0;
  warnungtext_anzeige.gridy=0;  
  warnungtext_anzeige.gridwidth=GridBagConstraints.REMAINDER;
  warnungtext_anzeige.fill=GridBagConstraints.CENTER;
  warnungtext_anzeige.insets=new Insets(10,10,10,10);
  warnungtext_layout.setConstraints(warnungtext,warnungtext_anzeige);
  warnungtext_anzeige.gridx=0;
  warnungtext_anzeige.gridy=1;  
  warnungtext_anzeige.gridwidth=GridBagConstraints.REMAINDER;
  warnungtext_anzeige.fill=GridBagConstraints.CENTER;
  warnungtext_anzeige.insets=new Insets(10,10,10,10);
  warnungtext_layout.setConstraints(warnungtext_ok,warnungtext_anzeige);
  JPanel warnungtext_panel=new JPanel();
  warnungtext_panel.setLayout(warnungtext_layout);
  warnungtext_panel.setBackground(Color.white);
  warnungtext_panel.add(warnungtext);
  warnungtext_panel.add(warnungtext_ok);  
  warnungtext_frame=new JFrame();
  warnungtext_frame.setTitle("Warning");
  warnungtext_frame.getContentPane().add(warnungtext_panel);    
  warnungtext_frame.pack();
  warnungtext_frame.setLocationByPlatform(true);
  warnungtext_frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
  warnungtext_frame.setVisible(true); 
 }
 
 public void actionPerformed(ActionEvent menuereignis )
 {
  if(menuereignis.getSource()==warnungtext_ok)
  {
   warnungtext_frame.dispose();   	  
  }
 }
}