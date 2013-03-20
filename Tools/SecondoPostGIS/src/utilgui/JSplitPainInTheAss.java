/**
 * 
 */
package utilgui;

/**
 * @author Bill
 * https://gist.github.com/daveray/1021984/raw/628700a51ad3c8876bbb67b0613bcd18474fc532/JSplitPainInTheAss.java
 */


import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.HierarchyEvent;
import java.awt.event.HierarchyListener;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JSplitPane;
import javax.swing.SwingUtilities;

public class JSplitPainInTheAss {
  
  public static JSplitPane setDividerLocation(final JSplitPane splitter,
      final double proportion) {
    if (splitter.isShowing()) {
      if(splitter.getWidth() > 0 && splitter.getHeight() > 0) {
        splitter.setDividerLocation(proportion);
      }
      else {
        splitter.addComponentListener(new ComponentAdapter() {
          @Override
          public void componentResized(ComponentEvent ce) {
            splitter.removeComponentListener(this);
            setDividerLocation(splitter, proportion);
          }
        });
      }
    }
    else {
      splitter.addHierarchyListener(new HierarchyListener() {
        @Override
        public void hierarchyChanged(HierarchyEvent e) {
          if((e.getChangeFlags() & HierarchyEvent.SHOWING_CHANGED) != 0 &&
              splitter.isShowing()) {
            splitter.removeHierarchyListener(this);
            setDividerLocation(splitter, proportion);
          }
        }
      });
    }
    return splitter;
  }
  
 /* public static void main(String[] args) {
    SwingUtilities.invokeLater(new Runnable() {

      @Override
      public void run() {
        final JFrame frame = new JFrame("JSplitPainInTheAss");
        final JSplitPane splitter = new JSplitPane(JSplitPane.VERTICAL_SPLIT, new JLabel("TOP"), new JLabel("BOTTOM"));
        setDividerLocation(splitter, 0.75);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setContentPane(splitter);
        frame.setSize(800, 600);
        frame.setVisible(true);     
      }});
  }
  */
}

