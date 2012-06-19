/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JTextField;

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
    
    //opens a popup window with a textfield
    public void text(){
        final JTextField textfield = new JTextField(20);
        JButton ok = new JButton("ok");
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendText(e, textfield);
            }
        };
        
        add(textfield);
        ok.addActionListener(al);
        add(ok);
        pack();
        setVisible(true);
    }
    
    public void sendText(ActionEvent e, JTextField text){
        main.addString(text.getText());
        setVisible(false);
    }
    
}
