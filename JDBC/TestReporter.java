import javax.swing.JOptionPane;


public class TestReporter {
	//Activated for Testreasons
	public static int showQuestion(String ASK){
	  int res = JOptionPane.showConfirmDialog(null,ASK,null,
	            JOptionPane.YES_NO_OPTION,JOptionPane.QUESTION_MESSAGE);
	  if(res==JOptionPane.YES_OPTION)
	     return YES;
	  if(res==JOptionPane.NO_OPTION)
	    return NO;
	  return ERROR;
	 }

	public static final int YES = 0;
	public static final int NO = 1;
	public static final int ERROR = -1;


		public static void showMessage(String Mess) {
			JOptionPane.showMessageDialog(null, Mess, "for Testreasons", 0);
		}
		
		// End for Testreasons
}
