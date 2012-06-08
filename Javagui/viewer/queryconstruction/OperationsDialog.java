/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.*;
import java.util.ArrayList;

/**
 *
 * @author lrentergent
 */
public class OperationsDialog extends JDialog {
    
    private MainPane main;
    private String[] attributes;
    private JCheckBox[] cbs;
    
    public OperationsDialog(MainPane main, String[] atts) {
        this.main = main;
        this.attributes = atts;
        this.cbs = new JCheckBox[atts.length];
        //setPreferredSize( new Dimension(150, 150) );
        setLocation(100, 100);
        setLayout(new FlowLayout(FlowLayout.LEADING));
    }
    
    public void project() {
        int i = 0;
        for (String att: attributes) {
            cbs[i] = new JCheckBox( att, false );
            add(cbs[i]);
            i++;
        }
        JButton ok = new JButton("ok");
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendProject(e);
            }
        };
        ok.addActionListener(al);
        add(ok);
        
        pack();
        setVisible(true);
    }
    
    public void sendProject(ActionEvent e) {
        int i = 0;
        for (JCheckBox cb: this.cbs) {
            if (!cb.isSelected()) {
                attributes[i] = cb.getName();
            }
            i++;
        }
        main.addArray(attributes);
        this.setVisible(false);
    }
    
    public void filter(ActionEvent e){
        int i = 0;
        for (JCheckBox cb: this.cbs) {
            if (!cb.isSelected()) {
                attributes[i] = cb.getName();
            }
            i++;
        }
    }
    
    public void integer(){
        final JTextField textfield = new JTextField(20);
        add(textfield);
        
        JButton ok = new JButton("ok");
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendInteger(e, textfield);
            }
        };
        ok.addActionListener(al);
        add(ok);
        
        pack();
        setVisible(true);
    }
    
    public void sendInteger(ActionEvent e, JTextField integer){
        System.out.println(e.paramString());
        main.addString(integer.getText());
        this.setVisible(false);
    }
}
