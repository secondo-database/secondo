package appGui;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;

import secondo.MySecondoObject;
import sj.lang.ListExpr;

public class SecStandardGUI
  extends HelpGUI
{
  public void init(MySecondoObject mysecObj)
  {
    this.textpane.setEditable(false);
    this.textpane.setText(mysecObj.toListExpr().toString());
    
    this.jscrollpane = new JScrollPane(this.textpane);
    
    this.mTableFrame.setTitle("SECONDO Standard-Viewer");
    
    this.panelMainFrame.add(new JLabel(), "First");
    this.panelMainFrame.add(new JLabel(), "West");
    this.panelMainFrame.add(new JLabel(), "East");
    this.panelMainFrame.add(new JLabel(), "Last");
    this.panelMainFrame.add(this.jscrollpane, "Center");
    
    this.mTableFrame.add(this.panelMainFrame);
    this.mTableFrame.pack();
    if (this.mTableFrame.getWidth() < 300) {
      this.mTableFrame.setSize(300, this.mTableFrame.getHeight());
    }
    if (this.mTableFrame.getHeight() < 300) {
      this.mTableFrame.setSize(this.mTableFrame.getWidth(), 300);
    }
    this.mTableFrame.setIconImage(gimp_S2P);
    this.mTableFrame.setVisible(true);
  }
}
