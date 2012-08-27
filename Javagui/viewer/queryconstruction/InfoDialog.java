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
    
    public InfoDialog(int x, int y) {
        //this.setPreferredSize(new Dimension(100, 100));
        textArea.setEditable(false);
        textArea.setBorder(BorderFactory.createEmptyBorder(5,5,5,15));
        this.add(textArea);
        this.setAlwaysOnTop(true);
        this.setLocation(x, y);
        this.setMinimumSize(new Dimension(100,100));
        setLayout(new GridLayout(0, 1));
    }
    
    public void addInfo(String name, String str) {
        str = name + "\n" + str.replace("(", "\n  ").replace(")", "");
        textArea.append("\n" + str + "\n");
    }
    
    public void removeText() {
        textArea.setText("");
    }
    
    public void view() {
        //this.setAlwaysOnTop(true);
        if (textArea.getText() == null) {
            textArea.setText("Keine Informationen verfügbar.");
        }
        pack();
        setVisible(true);
    }
    
    public void viewInfo(String name, String str) {
        
        this.setTitle(name);
        str = str.replace("(", "  ").replace(")", "");
        textArea.setText(str);
        
    }
}
