import java.awt.event.*;

public class WCloser extends WindowAdapter {
  public void windowClosing (WindowEvent e) {
    System.exit(0);
  }
}
