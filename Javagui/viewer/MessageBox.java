package viewer;

import javax.swing.*;

public class MessageBox{

private MessageBox(){}; // we need no constructor


public static void showMessage(String Text){
  OptionPane.showMessageDialog(null,Text);
}

private static JOptionPane OptionPane = new JOptionPane();

}
