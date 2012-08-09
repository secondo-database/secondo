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
    
    JTextArea textArea = new JTextArea();
    
    public InfoDialog() {
        //this.setPreferredSize(new Dimension(100, 100));
        textArea.setEditable(false);
        textArea.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
        
    }
        
    public InfoDialog(StreamView stream) {
        
        super();
        setLayout(new GridLayout(0, 1));
    }
    
    public InfoDialog(ObjectType object) {
        
        super();
        setLayout(new GridLayout(0, 1));
        
    }
    
    public void addStream(StreamView stream) {
        
    }
    
    public void view() {
        //this.setAlwaysOnTop(true);
        pack();
        setVisible(true);
    }
    
    public void viewInfo(String str) {
        
        str = str.replace("(", "  ").replace(")", "");
        textArea.setText(str);
        this.add(textArea);
        
    }
}
