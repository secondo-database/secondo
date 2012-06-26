/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.GridLayout;
import javax.swing.JDialog;
import javax.swing.JTable;
import javax.swing.JLabel;

/**
 *
 * @author lrentergent
 */
public class InfoDialog extends JDialog {
    
    private ObjectType object;
    private String[] attributes;
    private String[] attrtypes;
    
    public InfoDialog(ObjectType object) {
        this.object = object;
        this.attributes = object.getAttributes();
        this.attrtypes = object.getAttrTypes();
        //setPreferredSize( new Dimension(150, 150) );
        setLocation(100, 100);
        setLayout(new GridLayout(0,2));
    }
    
    public void view() {
        JLabel title = new JLabel( object.getName() );
        for (String att: attributes) {
            JLabel label = new JLabel( att );
            add(label);
        }
        for (String type: attrtypes) {
            JLabel label = new JLabel( type );
            add(label);
        }
        pack();
        setVisible(true);
    }
    
}
