package appGuiUtil;


/*
 *This class will be used to give a 
 *message information when required in all the application 
 */
import javax.swing.JOptionPane;

public class Message
{
  public Message(String strMessage)
  {
    new SetUI().setUIAndLanguage();
    JOptionPane.showMessageDialog(null, strMessage, "Message Info", 1);
  }
}
