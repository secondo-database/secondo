

package viewer;
import gui.SecondoObject;
import sj.lang.MessageListener;
import sj.lang.ListExpr;
import javax.swing.*;
import gui.ViewerControl;
import java.awt.*;

public class MessageViewer extends SecondoViewer implements MessageListener{


public MessageViewer(){
   textArea = new JTextArea(100,100);
   setLayout(new BorderLayout());
   JScrollPane sp = new JScrollPane(textArea);
   add(sp, BorderLayout.CENTER);
}


public boolean canDisplay(SecondoObject o){
  return false;
}

public boolean addObject(SecondoObject o){
   return false;
}

public void removeObject(SecondoObject o){
}

public void removeAll(){
}
 
public boolean isDisplayed(SecondoObject o){
  return false;
}

public MenuVector getMenuVector(){
   return null;
}

public boolean selectObject(SecondoObject o){
  return false;
}

public String getName(){
  return "MessageViewer";
}

public void enableTestmode(boolean on){}


public void processMessage(ListExpr list){
 textArea.setText(""+list);
 textArea.paintImmediately(textArea.getBounds());
}

public void setViewerControl(ViewerControl vc){
   super.setViewerControl(vc);
   if(vc!=null) {
      vc.addMessageListener(this);
   }
}


private JTextArea textArea;

}

