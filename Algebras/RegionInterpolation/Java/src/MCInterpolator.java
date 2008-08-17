import java.awt.*;
import javax.swing.*;
import movingregion.MCIContents;

public class MCInterpolator {
  public static void main(String[] args) {
    JFrame dispwindow;
    MCIContents mcicont;
    dispwindow = new JFrame("Moving cycle interpolator");
    dispwindow.addWindowListener(new WCloser());
    mcicont = new MCIContents();
    mcicont.init();
    dispwindow.getContentPane().add(mcicont,BorderLayout.CENTER);
    dispwindow.setSize(800,600);    
    dispwindow.setVisible(true);
  }
}
