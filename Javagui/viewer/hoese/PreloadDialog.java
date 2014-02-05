

package viewer.hoese;


import javax.swing.*;
import java.awt.*;
import java.awt.event.*;


public class PreloadDialog extends JDialog implements PreloadObserver {

   JProgressBar bar;
   JButton cancelBtn;
   CancelListener cancelListener; 
   JLabel targetLabel;
   JLabel successLabel;
   JLabel failedLabel;
   private static final String ALL     ="all      : ";
   private static final String FINISHED="finished : ";
   private static final String FAILED   ="failed  : ";
   private int success;
   private int failed;

   public PreloadDialog( Frame parent){
      super(parent,true);
      bar = new JProgressBar(SwingConstants.HORIZONTAL);
      cancelBtn = new JButton("Cancel");
      setLayout(new BorderLayout());
      JPanel p1 = new JPanel();
      p1.setLayout(new GridLayout(4,1));
      p1.add(bar);

      targetLabel = new JLabel(ALL);
      successLabel = new JLabel(FINISHED);
       failedLabel = new JLabel(FAILED);
      p1.add(targetLabel);
      p1.add(successLabel);
      p1.add(failedLabel);

      add(p1, BorderLayout.CENTER);
      JPanel p2 = new JPanel();
      p2.add(cancelBtn);
      add(p2,BorderLayout.SOUTH);
      cancelBtn.addActionListener(new ActionListener(){
         public void actionPerformed(ActionEvent evt){
             if(cancelListener!=null){
                 cancelListener.cancel();
             }
         }
      });
      setSize(400,200);
   }


   public void finish(boolean complete){
      setVisible(false);
   }

   public void step(boolean ok){
       if(ok){
          success++;
          successLabel.setText(FINISHED+success);
       } else {
          failed++;
          failedLabel.setText(FAILED+failed);
       }
       bar.setValue(success+failed);
   }

   public void setTarget(int targetNumber){
       bar.setMinimum(0);
       bar.setMaximum(targetNumber);
       bar.setValue(0);
       targetLabel.setText(ALL+targetNumber);
       success=0;
       failed=0;
       successLabel.setText(FINISHED+success);
       failedLabel.setText(FAILED+failed);
   }



   public void setCancelListener(CancelListener cl){
      cancelListener = cl;
   }


}
