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
import java.util.Iterator;
import javax.swing.*;

/**
 *
 * @author lrentergent
 */
public class OperationsDialog extends JDialog {
    
    private MainPane main;
    private ArrayList<ObjectView> allObjects;
    private Operation operation;
    private String[] attributes;
    private JCheckBox[] cbs;
    private JRadioButton[] rbs;
    private ArrayList<ButtonGroup> radiogroup = new ArrayList<ButtonGroup>();
    private FilterViewer viewer;
    
    public OperationsDialog(MainPane main, Operation operation, ArrayList<ObjectView> objects) {
        this.main = main;
        this.operation = operation;
        this.allObjects = objects;
        this.viewer = new FilterViewer(this.main, this.operation, allObjects);
        setLayout(new GridLayout(0,1));
        
        if (operation.getParameter().equals("int") || operation.getParameter().toLowerCase().equals("string")) {
            text();
        }
        if (operation.getParameter().split(",").length > 1) {
            
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
    
    public void addCheckboxes(String objectName, String[] atts) {
        this.attributes = atts;
        JLabel name = new JLabel(objectName);
        this.add(name);
        this.cbs = new JCheckBox[attributes.length];
        int i = 0;
        for (String att: attributes) {
            cbs[i] = new JCheckBox( att, false );
            add(cbs[i]);
            i++;
        }
    }
    
    public void addRadiobuttons(String objectName, String[] atts) {
        this.attributes = atts;
        JLabel name = new JLabel(objectName);
        this.add(name);
        ButtonGroup rGroup = new ButtonGroup();
        JRadioButton[] rbs1 = new JRadioButton[attributes.length];
        int i = 0;
        for (String att: attributes) {
            rbs1[i] = new JRadioButton( att );
            rbs1[i].setActionCommand(att);
            this.add(rbs1[i]);
            rGroup.add(rbs1[i]);
            i++;
        }
        
        this.radiogroup.add(rGroup);
    }
    
    public void project() {
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
        viewer.show();
    }
    
    public void addAttributes(ObjectView[] atts) {
        viewer.addObjects(atts);
    }
    
    public void joinAttributes(){
        setLayout(new GridLayout(0,1));
        
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendObject(e);
            }
        };
        show(al);
    }
    
    //shows a radiobutton list of objects of the type "type"
    public void getObjects(String type) {
        int i = 0;
        this.rbs = new JRadioButton[allObjects.size()];
        ButtonGroup buttons = new ButtonGroup();
        for (ObjectView o: allObjects) {
            if (o.getType().equals(type)) {
                rbs[i] = new JRadioButton(o.getName());
                rbs[i].setActionCommand(o.getName());
                this.add(rbs[i]);
                buttons.add(rbs[i]);
                i++;
            }
        }
        radiogroup.add(buttons);
    }
    
    //adds the name of the object to the operation
    public void sendObject(ActionEvent e) {
        String[] attr = new String[radiogroup.size()];
        if (radiogroup.size() > 1) {
            int i = 0;
            for ( Iterator iter = radiogroup.iterator(); iter.hasNext(); ) {
                ButtonGroup group = (ButtonGroup)iter.next();
                
                if (group.getButtonCount() > 0) {
                    attr[i] = group.getSelection().getActionCommand();
                    i++;
                }
            }
            main.addArray(attr);
        }
        else if (radiogroup.get(0).getButtonCount() > 0)
            main.addString(radiogroup.get(0).getSelection().getActionCommand(), this.operation.getBrackets());
        
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
        String brackets = operation.getBrackets();
        String result = text.getText();
        if (operation.getParameter().equals("string")) {
            result = "'"+result+"'";
        }
        String resultString = result;
        if (brackets.toCharArray().length == 2) {
            resultString = brackets.toCharArray()[0] + result + brackets.toCharArray()[1];
        }
        
        if (operation.getName().equals("rename")) {
            operation.setName(resultString);
        }
        else {
            operation.setName(operation.getName() + " " + resultString);
        }
        main.update();
        setVisible(false);
    }
    
}
