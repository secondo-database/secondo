package viewer;

import javax.swing.*;

public class MessageBox{

private MessageBox(){}; // we need no constructor


public static void showMessage(String Text){
  OptionPane.showMessageDialog(null,Text);
}

public static int showQuestion(String ASK){
  int res = OptionPane.showConfirmDialog(null,ASK,null,
                      JOptionPane.YES_NO_OPTION,JOptionPane.QUESTION_MESSAGE);

  if(res==JOptionPane.YES_OPTION)
      return YES;
  if(res==JOptionPane.NO_OPTION)
      return NO;
  return ERROR;
}

private static JOptionPane OptionPane = new JOptionPane();
public static final int YES = 0;
public static final int NO = 1;
public static final int ERROR = -1;

}
