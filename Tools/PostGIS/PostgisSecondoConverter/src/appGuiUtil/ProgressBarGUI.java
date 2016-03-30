package appGuiUtil;



import java.awt.Container;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.util.logging.Logger;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JProgressBar;
import javax.swing.border.Border;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;

public class ProgressBarGUI
  implements ActionListener, WindowListener, IGlobalParameters
{
  public JFrame jframeProg;
  public JButton jbutCancel;
  JProgressBar jProgBar;
  boolean mbCanceld;
  
  public ProgressBarGUI(String _strTitel, String _strBorderTitle)
  {
    this.mbCanceld = false;
    


    this.jframeProg = new JFrame(_strTitel);
    Container content = this.jframeProg.getContentPane();
    
    Border border = BorderFactory.createTitledBorder(_strBorderTitle);
    
    this.jProgBar = new JProgressBar(0, 100);
    this.jProgBar.setIndeterminate(true);
    this.jProgBar.setBorder(border);
    
    this.jbutCancel = new JButton("Cancel");
    this.jbutCancel.addActionListener(this);
    
    content.add(this.jProgBar, "North");
    content.add(this.jbutCancel, "South");
    
    this.jframeProg.setSize(150, 100);
    this.jframeProg.setUndecorated(false);
    this.jframeProg.setDefaultCloseOperation(2);
    this.jframeProg.addWindowListener(this);
  }
  
  public void showProgbar()
  {
    this.mbCanceld = false;
    this.jframeProg.setIconImage(gimp_S2P);
    this.jframeProg.pack();
    this.jframeProg.setVisible(true);
    LogFileHandler.mlogger.info("show Progbar");
  }
  
  public void closeProgbar()
  {
    this.mbCanceld = true;
    this.jframeProg.setVisible(false);
    LogFileHandler.mlogger.info("close Progbar");
  }
  
  public boolean isCanceld()
  {
    return this.mbCanceld;
  }
  
  public JFrame getFrame()
  {
    return this.jframeProg;
  }
  
  public void setActionListener2CancelButton(ActionListener _actionlistener)
  {
    this.jbutCancel.addActionListener(_actionlistener);
  }
  
  public void actionPerformed(ActionEvent e)
  {
    if (e.getSource() == this.jbutCancel)
    {
      this.mbCanceld = true;
      this.jframeProg.setVisible(false);
    }
  }
  
  public void windowOpened(WindowEvent e) {}
  
  public void windowClosing(WindowEvent e)
  {
    this.mbCanceld = true;
    this.jframeProg.setVisible(false);
  }
  
  public void windowClosed(WindowEvent e) {}
  
  public void windowIconified(WindowEvent e) {}
  
  public void windowDeiconified(WindowEvent e) {}
  
  public void windowActivated(WindowEvent e) {}
  
  public void windowDeactivated(WindowEvent e) {}
}
