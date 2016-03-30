package appGuiUtil;

import java.awt.Color;
import java.awt.Container;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import secondoPostgisUtil.IGlobalParameters;

public class Warning
  implements ActionListener, IGlobalParameters
{
  JButton warningText_ok;
  JFrame warningText_frame;
  
  public Warning(Exception warnung)
  {
    JTextArea warningText = new JTextArea(3, 22);
    warningText.setText("\n" + warnung + "\n");
    warningText.setEditable(false);
    this.warningText_ok = new JButton("Ok");
    this.warningText_ok.setBackground(Color.white);
    this.warningText_ok.addActionListener(this);
    
    GridBagConstraints warningDisplayText = new GridBagConstraints();
    GridBagLayout warningLayoutText = new GridBagLayout();
    warningDisplayText.gridx = 0;
    warningDisplayText.gridy = 0;
    warningDisplayText.gridwidth = 0;
    warningDisplayText.fill = 10;
    warningDisplayText.insets = new Insets(10, 10, 10, 10);
    warningLayoutText.setConstraints(warningText, warningDisplayText);
    warningDisplayText.gridx = 0;
    warningDisplayText.gridy = 1;
    warningDisplayText.gridwidth = 0;
    warningDisplayText.fill = 10;
    warningDisplayText.insets = new Insets(10, 10, 10, 10);
    warningLayoutText.setConstraints(this.warningText_ok, warningDisplayText);
    JPanel warningPanelText = new JPanel();
    warningPanelText.setLayout(warningLayoutText);
    warningPanelText.setBackground(Color.white);
    warningPanelText.add(warningText);
    warningPanelText.add(this.warningText_ok);
    this.warningText_frame = new JFrame();
    this.warningText_frame.setTitle("Warning");
    this.warningText_frame.getContentPane().add(warningPanelText);
    this.warningText_frame.pack();
    this.warningText_frame.setLocationByPlatform(true);
    this.warningText_frame.setDefaultCloseOperation(2);
    this.warningText_frame.setIconImage(gimp_S2P);
    this.warningText_frame.setVisible(true);
  }
  
  public Warning(String warnings)
  {
    JTextArea warningText = new JTextArea(3, 22);
    warningText.setText("\n" + warnings + "\n");
    warningText.setEditable(false);
    this.warningText_ok = new JButton("Ok");
    this.warningText_ok.setBackground(Color.white);
    this.warningText_ok.addActionListener(this);
    GridBagConstraints warningDisplayText = new GridBagConstraints();
    GridBagLayout warningTextLayout = new GridBagLayout();
    warningDisplayText.gridx = 0;
    warningDisplayText.gridy = 0;
    warningDisplayText.gridwidth = 0;
    warningDisplayText.fill = 10;
    warningDisplayText.insets = new Insets(10, 10, 10, 10);
    warningTextLayout.setConstraints(warningText, warningDisplayText);
    warningDisplayText.gridx = 0;
    warningDisplayText.gridy = 1;
    warningDisplayText.gridwidth = 0;
    warningDisplayText.fill = 10;
    warningDisplayText.insets = new Insets(10, 10, 10, 10);
    warningTextLayout.setConstraints(this.warningText_ok, warningDisplayText);
    JPanel warningTextPanel = new JPanel();
    warningTextPanel.setLayout(warningTextLayout);
    warningTextPanel.setBackground(Color.white);
    warningTextPanel.add(warningText);
    warningTextPanel.add(this.warningText_ok);
    this.warningText_frame = new JFrame();
    this.warningText_frame.setTitle("Warning");
    this.warningText_frame.getContentPane().add(warningTextPanel);
    this.warningText_frame.pack();
    this.warningText_frame.setLocationByPlatform(true);
    this.warningText_frame.setDefaultCloseOperation(2);
    this.warningText_frame.setIconImage(gimp_S2P);
    this.warningText_frame.setVisible(true);
  }
  
  public void actionPerformed(ActionEvent menuereignis)
  {
    if (menuereignis.getSource() == this.warningText_ok) {
      this.warningText_frame.dispose();
    }
  }
}
