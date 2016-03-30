package appGuiUtil;


import javax.swing.JOptionPane;

public class ConvertingFinishedGUI
{
  public ConvertingFinishedGUI()
  {
    new SetUI().setUIAndLanguage();
    JOptionPane.showMessageDialog(null, "Your copy process is finished.", "Finish", 1);
  }
}
