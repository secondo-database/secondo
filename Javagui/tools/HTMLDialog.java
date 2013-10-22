

package tools;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class HTMLDialog extends JDialog{
  private JEditorPane edit;
  
  public HTMLDialog(String title){
     super((JDialog)null,title,true);
     edit = new JEditorPane();
     JScrollPane sp = new JScrollPane(edit);
     setLayout(new BorderLayout());
     add(sp,BorderLayout.CENTER);
     JPanel p = new JPanel();
     add(p, BorderLayout.SOUTH);
     JButton closeBtn = new JButton("close");
     p.add(closeBtn);
     closeBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
           setVisible(false);
        }
     });
     setSize(800,600);
     edit.setContentType("text/html");
  }

  public void setText(String text){
      edit.setText(text);
  }

}


