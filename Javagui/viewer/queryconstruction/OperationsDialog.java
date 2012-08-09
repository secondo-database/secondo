/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import javax.swing.*;

/**
 *
 * @author lrentergent
 */
public class OperationsDialog extends JDialog {
    
    private MainPane main;
    private ArrayList<ObjectView> objects;
    private Operation operation;
    private String[] attributes;
    private JCheckBox[] cbs;
    private JRadioButton[] rbs;
    private ButtonGroup radiogroup = new ButtonGroup();
    private ButtonGroup radiogroupRelation1 = new ButtonGroup();
    private ButtonGroup radiogroupRelation2 = new ButtonGroup();
    
    public OperationsDialog(MainPane main, Operation operation) {
        this.main = main;
        this.operation = operation;
        setLayout(new GridLayout(0,1));
        
        if (operation.getParameter().equals("int") || operation.getParameter().equals("String")) {
            text();
        }
    }
    
    public void show(ActionListener al) {
        JButton ok = new JButton("ok");
        ok.addActionListener(al);
        this.getRootPane().setDefaultButton(ok);
        add(ok);
        
        setLocation(100, 100);
        pack();
        setVisible(true);
    }
    
    public void addCheckboxes(String[] atts) {
        this.attributes = atts;
        this.cbs = new JCheckBox[attributes.length];
        int i = 0;
        for (String att: attributes) {
            cbs[i] = new JCheckBox( att, false );
            add(cbs[i]);
            i++;
        }
    }
    
    public ButtonGroup addRadiobuttons(String[] atts) {
        this.attributes = atts;
        ButtonGroup rGroup = new ButtonGroup();
        this.rbs = new JRadioButton[attributes.length];
        int i = 0;
        for (String att: attributes) {
            rbs[i] = new JRadioButton( att );
            rbs[i].setActionCommand(att);
            this.add(rbs[i]);
            rGroup.add(rbs[i]);
            i++;
        }
        return rGroup;
    }
    
    public void project(String[] atts) {
        addCheckboxes(atts);
        
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
    
    public void filter(ObjectView[] atts){
        FilterViewer viewer = new FilterViewer(this.main, this.operation, "bool");
        viewer.addObjects(atts);
        viewer.setVisible(true);
    }
    
    public void joinAttributes(StreamView r1, StreamView r2){
        setLayout(new GridLayout(0,1));
        this.radiogroupRelation1 = addRadiobuttons(r1.getAttributes());
        this.radiogroupRelation2 = addRadiobuttons(r2.getAttributes());
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendObject(e);
            }
        };
        show(al);
    }
    
    //shows a radiobutton list of objects of the type "type"
    public void getObjects(String type, ArrayList<ObjectView> objects) {
        int i = 0;
        this.rbs = new JRadioButton[objects.size()];
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
        if (this.radiogroupRelation1.getButtonCount() > 0) {
            main.addString(radiogroupRelation1.getSelection().getActionCommand()+ ", " + radiogroupRelation2.getSelection().getActionCommand(), this.operation.getBrackets());
        } else {
            main.addString(radiogroup.getSelection().getActionCommand(), this.operation.getBrackets());
        }
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
    
    //adds the text to the operation name
    public void sendText(ActionEvent e, JTextField text){
        String resultString = operation.getBrackets()[0] + text.getText() + operation.getBrackets()[1];
        if (operation.getName().equals("rename")) {
            operation.setName(resultString);
        }
        else {
            operation.setName(operation.getName() + resultString);
        }
        main.update();
        setVisible(false);
    }
    
}
