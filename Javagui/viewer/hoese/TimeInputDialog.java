/* file viwer/hoese/TimeInputDialog.jave
   last change: 8-2003, Thomas Behr
*/

package viewer.hoese;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import viewer.*;

public class TimeInputDialog extends JDialog{

  /* creates a new TimeInputDialog*/
  public TimeInputDialog(Frame parent){
     super(parent,true); // show Modal
     getContentPane().setLayout(new BorderLayout());
     setSize(300,100);
     setTitle("input time difference");
     CB = new JComboBox();
     for(int i=0; i<measure_unit.length;i++)
        CB.addItem(measure_unit[i]);
     Text = new JTextField(15);
     Text.setText("1");
     OkBtn = new JButton("ok");
     CancelBtn = new JButton("cancel");
     JPanel P1 = new JPanel();
     P1.add(Text);
     P1.add(CB);
     JPanel P2 = new JPanel();
     P2.add(OkBtn);
     P2.add(CancelBtn);
     getContentPane().add(P1,BorderLayout.CENTER);
     getContentPane().add(P2,BorderLayout.SOUTH);
     ActionListener ButtonControl = new ActionListener(){
        public void actionPerformed(ActionEvent evt){
           Object O = evt.getSource();
	   if(O.equals(CancelBtn)){
	      Result = CANCELED;
	      hide(); //setVisible(false);
	   }
	   if(O.equals(OkBtn)){
	      try{
                 long tmp = Long.parseLong(Text.getText().trim());
		 if(tmp<=0){
		   MessageBox.showMessage("the time must be greater then zero!");
		   return;
		 }
                 int i = CB.getSelectedIndex();
		 if(i<0){
		    MessageBox.showMessage("internal error"); // should never be reached
		    return;
		 }
                 time = tmp*factors[i];
		 TimeString = ""+tmp+" "+measure_unit[i];
		 Result = OK;
	         hide(); //setVisible(false);
	      } catch(Exception e){
  	          MessageBox.showMessage("the input is not correct");
		  return;
	      }

	   }//if
	 }//actionPerformed
     };
     CancelBtn.addActionListener(ButtonControl);
     OkBtn.addActionListener(ButtonControl);

  }

  /* returns the last accepted Time */
  public long getTime(){
     return time;
  }

  /* returns the Time as String */
  public String getTimeString(){
      return TimeString;
  }

  /** returns the result (OK,CANCELED) of this dialog */
  public int getResult(){
     return Result;
  }

  /* shows this Component, after input the result is
     OK or CANCELED depending of pushed button */
  public int inputTime(){
      //setVisible(true);
      show();
      return Result;
  }

  // the input Field
  private JTextField Text;
  // checkbox for measure unit
  private JComboBox CB;

  private JButton CancelBtn;
  private JButton OkBtn;
  private String[] measure_unit ={"msecs", "secs", "mins",
                                  "hours" , "days"};
  private long[] factors = {1, 1000, 60000, 3600000, 86400000};
  
  private int Result = 0;
  private long time=1; // the entered time in MilliSec
  private String TimeString="";


  public  final static int CANCELED=0;
  public  final static int OK = 1;


}
