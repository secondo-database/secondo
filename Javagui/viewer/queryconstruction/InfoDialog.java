/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.Dimension;
import java.awt.GridLayout;
import javax.swing.JDialog;
import javax.swing.*;

/**
 *
 * @author lrentergent
 */
public class InfoDialog extends JDialog {
        
    public InfoDialog(StreamView stream) {
        
        setLocation(100, 100);
        setLayout(new GridLayout(0, 2));
        setPreferredSize(new Dimension(600, (stream.getHeight()+1)*50));
        stream.view(this);
        
        view();
    }
    
    public InfoDialog(ObjectView object) {
        object.viewInfo(this);
    }
    
    public void addStream(StreamView stream) {
        stream.view(this);
    }
    
    public void view() {
        this.setAlwaysOnTop(true);
        pack();
        setVisible(true);
    }
    
}
