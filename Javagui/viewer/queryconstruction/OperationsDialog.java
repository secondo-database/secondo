/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.ButtonGroup;
import javax.swing.JRadioButton;
import javax.swing.JDialog;
import javax.swing.JTextField;

/**
 *
 * @author lrentergent
 */
public class OperationsDialog extends JDialog {
    
    private MainPane main;
    private ArrayList<ObjectView> objects;
    private String[] attributes;
    private JCheckBox[] cbs;
    private JRadioButton[] rbs;
    private ButtonGroup radiogroup = new ButtonGroup();
    
    public OperationsDialog(MainPane main, String[] atts) {
        this.main = main;
        this.attributes = atts;
        this.cbs = new JCheckBox[atts.length];
        setLayout(new FlowLayout(FlowLayout.LEADING));
    }
    
    public OperationsDialog(MainPane main, ArrayList<ObjectView> objects) {
        this.main = main;
        this.objects = objects;
        this.rbs = new JRadioButton[objects.size()];
        setLayout(new GridLayout(0,1));
    }
    
    public OperationsDialog(MainPane main) {
        this.main = main;
        setLayout(new GridLayout(0,1));
    }
    
    public void show(ActionListener al) {
        JButton ok = new JButton("ok");
        ok.addActionListener(al);
        add(ok);
        
        setLocation(100, 100);
        pack();
        setVisible(true);
    }
    
    public void project(String[] atts) {
        int i = 0;
        for (String att: atts) {
            cbs[i] = new JCheckBox( att, false );
            add(cbs[i]);
            i++;
        }
        
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendProject(e);
            }
        };
        show(al);
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
    
    public void filter(){
        int i = 0;
        for (JCheckBox cb: this.cbs) {
            if (!cb.isSelected()) {
                attributes[i] = cb.getName();
            }
            i++;
        }
    }
    
    public void joinAttributes(ObjectType r1, ObjectType r2){
        setLayout(new GridLayout(0,1));
        this.attributes = r1.getAttributes();
        this.cbs = new JCheckBox[attributes.length];
        int i = 0;
        for (JCheckBox cb: this.cbs) {
            attributes[i] = cb.getName();
            i++;
        }
    }
    
    //shows a radiobutton list of objects of the type "type"
    public void getObjects(String type, ArrayList<ObjectView> objects) {
        int i = 0;
        for (ObjectView o: objects) {
            if (o.getType().equals(type)) {
                rbs[i] = new JRadioButton(o.getName());
                rbs[i].setActionCommand(o.getName());
                this.add(rbs[i]);
                radiogroup.add(rbs[i]);
                i++;
            }
        }
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendObject(e);
            }
        };
        show(al);
    }
    
    //adds the name of the object to the operation
    public void sendObject(ActionEvent e) {
        main.addString(radiogroup.getSelection().getActionCommand());
        this.setVisible(false);
    }
    
    //opens a popup window with a textfield
    public void text(){
        final JTextField textfield = new JTextField(20);
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendText(e, textfield);
            }
        };
        
        add(textfield);
        show(al);
    }
    
    public void sendText(ActionEvent e, JTextField text){
        main.addString(text.getText());
        setVisible(false);
    }
    
}
