package appGuiUtil;

import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.HierarchyEvent;
import java.awt.event.HierarchyListener;
import javax.swing.JSplitPane;


import javax.swing.JFrame;

import javax.swing.JLabel;


import javax.swing.SwingUtilities;


public class JSplitPainInTheAss
{
  public static JSplitPane setDividerLocation(JSplitPane splitter, final double proportion)
  {
    if (splitter.isShowing())
    {
      if ((splitter.getWidth() > 0) && (splitter.getHeight() > 0)) {
        splitter.setDividerLocation(proportion);
      } else {
        splitter.addComponentListener(new ComponentAdapter()
        {
          public void componentResized(ComponentEvent ce)
          {
           // JSplitPainInTheAss.this.removeComponentListener(this);
        	  splitter.removeComponentListener(this);
            //JSplitPainInTheAss.setDividerLocation(JSplitPainInTheAss.this, proportion);
        	setDividerLocation(splitter, proportion);
          }
        });
      }
    }
    else {
      splitter.addHierarchyListener(new HierarchyListener()
      {
        public void hierarchyChanged(HierarchyEvent e)
        {
          if (((e.getChangeFlags() & 0x4) != 0L) &&  (splitter.isShowing()))
           // (JSplitPainInTheAss.this.isShowing()))
          {
           // JSplitPainInTheAss.this.removeHierarchyListener(this);
            splitter.removeHierarchyListener(this);
            
          //  JSplitPainInTheAss.setDividerLocation(JSplitPainInTheAss.this, proportion);
            setDividerLocation(splitter, proportion);
          }
        }
      });
    }
    return splitter;
  }
}



