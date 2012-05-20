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
public class FilterDialog extends JDialog {
    
    private MainPane main;
    private String[] attributes;
    private JCheckBox[] cbs;
    private JCheckBox cb1 = new JCheckBox( "ID", false );
    private JCheckBox cb2 = new JCheckBox( "Line", false );
    
    public FilterDialog(MainPane main, String[] atts) {
        this.main = main;
        this.attributes = atts;
        this.cbs = new JCheckBox[atts.length];
        setPreferredSize( new Dimension(200, 200) );
        setLocation(100, 100);
        setLayout(new FlowLayout());
        
        JButton ok = new JButton("ok");
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                filter(e);
            }
        };
        ok.addActionListener(al);
        
        int i = 0;
        for (String att: atts) {
            cbs[i] = new JCheckBox( att, false );
            add(cbs[i]);
            i++;
        }
        add(ok);
        
        pack();
        setVisible(true);
    }
    
    public void filter(ActionEvent e) {
        int i = 0;
        for (JCheckBox cb: this.cbs) {
            if (!cb.isSelected()) {
                attributes[i] = cb.getName();
            }
            i++;
        }
        main.filter(attributes);
        this.setVisible(false);
    }
}
