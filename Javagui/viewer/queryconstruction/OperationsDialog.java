/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.*;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import javax.swing.*;

/**
 *
 * @author lrentergent
 */
public class OperationsDialog extends JDialog {
    
    private MainPane main;
    private FilterViewer viewer;
    private Operation operation;
    private String[] parameters;
    private ArrayList<ObjectView> allObjects;
    
    private JDialog objectDialog;
    private JPanel buttonPanel;
            
    private JCheckBox[] cbs;
    private ArrayList<ButtonGroup> radiogroup = new ArrayList<ButtonGroup>();
    private ButtonGroup objectButtons = new ButtonGroup();
    private boolean hasButtons = false;
    
    private int hasParameter = 0;
    private int charAt = 0;
    private char[] signature;
    private String result = "";
    
    private final static char opChar = '#';
    private final static char pChar = 'p';
    
    public OperationsDialog(MainPane main, Operation operation, ArrayList<ObjectView> objects) {
        this.main = main;
        this.operation = operation;
        this.signature = operation.getSignature();
        this.allObjects = objects;
        this.viewer = new FilterViewer(this, objects);
        this.resetObjectDialog();
        setLayout(new GridLayout(0,1));
        
        parameters = operation.getParameter().split(";");
    }
    
    public void activate(){
        this.setResult();
    }
    
    private void showDialog() {
        this.radiogroup.remove(objectButtons);
        
        String parameter = parameters[hasParameter];
        
        if (parameter.equals("bool") || parameter.equals("new")){
            nestedQuery();
        }
        if (!parameter.equals("bool")) {
            if (parameter.equals("spatial")) {
                addObjectButtons("point,mpoint,points,line,sline,rect");
            }
            else {
                addObjectButtons(parameter);
            }
        }
        if (hasButtons)
            showRadioButtons();
        if (parameter.equals("attrlist")) {
            project();
        }
        if (parameter.equals("int") || parameter.toLowerCase().equals("string")) {
            text();
        }
        
    }
    
    private void show(JDialog dialog, ActionListener al) {
        JButton ok = new JButton("ok");
        ok.addActionListener(al);
        dialog.getRootPane().setDefaultButton(ok);
        dialog.add(ok);
        
        dialog.setLocation(100, 100);
        dialog.pack();
        dialog.setVisible(true);
    }
    
    /**
     * show the dialog with buttons
     */
    private void showRadioButtons(){
        if (objectButtons.getButtonCount() > 0)
            radiogroup.add(objectButtons);
        
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendButtons();
            }
        };
        show(objectDialog, al);
    }
    
    /**
     * add object attributes to the FilterViewer
     * @param atts object attributes
     */
    public void addAttributes(ObjectView[] atts) {
        this.viewer.addObjects(atts);
    }
    
    /**
     * adds CheckBoxes for attributes of an object to the dialog window
     * @param objectName
     * @param atts attributes of the object
     */
    public void addCheckboxes(String objectName, String[] atts) {
        JLabel name = new JLabel(objectName);
        this.add(name);
        this.cbs = new JCheckBox[atts.length];
        int i = 0;
        for (String att: atts) {
            cbs[i] = new JCheckBox( att, false );
            add(cbs[i]);
            i++;
        }
    }
    
    /**
     * shows a RadioButton list of objects of the type "type"
     * @param type 
     */
    private void addObjectButtons(String type) {
        int i = 0;
        //resetObjectDialog();
        //hasButtons = false;
        String[] types = type.split(",");
        for (ObjectView o: allObjects) {
            for (String t: types) {
                if (o.getType().equals(t)) {
                    JRadioButton rb = new JRadioButton(o.getType() + " " + o.getName());
                    rb.setActionCommand(o.getName());
                    buttonPanel.add(rb);
                    objectButtons.add(rb);
                    i++;
                }
            }
        }
        
        if (i > 0) {
            hasButtons = true;
        }  
    }
    
    /**
     * adds a group of RadioButtons for attributes of an object to the dialog window
     * @param objectName
     * @param atts attributes of the object
     */
    public void addRadiobuttons(String objectName, String[] atts) {
        JLabel name = new JLabel(objectName);
        buttonPanel.add(name);
        ButtonGroup buttons = new ButtonGroup();
        JRadioButton rb = new JRadioButton();
        int i = 0;
        for (String att: atts) {
            rb = new JRadioButton( att );
            rb.setActionCommand(att);
            buttonPanel.add(rb);
            buttons.add(rb);
            i++;
        }
        if (i > 0){
            hasButtons = true;
            this.radiogroup.add(buttons);
        }
    }
    
    public void addResult(String s){
        hasParameter++;
        result += s;
        check();
    }
    
    /**
     * check if the input parameters are complete
     */
    private void check() {
        if (hasParameter == parameters.length) {
            send();
        }
        else {
            setResult();
        }
    }
    
    /**
     * add the next character or string to the result and start the operation showDialog()
     * if the parameter is 'p'
     */
    private void setResult() {
        if (charAt < signature.length) {
            char c = signature[charAt];
            while (c != pChar && charAt < signature.length) {
                c = signature[charAt++];
                switch(c) {
                    case opChar: 
                        result += operation.getName();
                        break;
                    case pChar: 
                        showDialog();
                        break;
                    default: 
                        result += c;
                        break;
                }
            }
        }
    }
    
    private void project() {
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendCheckboxes();
            }
        };
        show(this, al);
    }
    
    private void resetObjectDialog() {
        objectDialog = new JDialog();
        objectDialog.setLayout(new BorderLayout());
        
        JPanel objectPane = new JPanel();
        objectPane.setLayout(new GridLayout(0,1));
        JScrollPane objectScrollPane = new JScrollPane(objectPane);
        objectScrollPane.setPreferredSize(new Dimension (250, 300));
        
        objectDialog.add(objectScrollPane, BorderLayout.NORTH);
        objectButtons = new ButtonGroup();
        buttonPanel = objectPane;
    }
    
    /**
     * activates the FilterViewer for a boolean parameter
     */
    private void nestedQuery(){
        viewer.show();
    }
    
    public void setMessage(String message) {
        JLabel label = new JLabel(message);
        add(label);
    }
    
    //opens a popup window with a textfield
    private void text(){
        
        final JTextField textfield = new JTextField(20);
        ActionListener al = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                sendText(textfield);
            }
        };
        
        add(textfield);
        show(this, al);
    }
    
    
    /**
    * adds the name of the chosen object to the operation
    */
    private void sendButtons() {
        objectDialog.setVisible(false);
        String labels = "";
        if (radiogroup.size() > 1) {
            int i = 0;
            for ( Iterator iter = radiogroup.iterator(); iter.hasNext(); ) {
                ButtonGroup group = (ButtonGroup)iter.next();
                
                if (group.getButtonCount() > 0) {
                    labels += group.getSelection().getActionCommand() + ", ";
                    i++;
                }
            }
            if(i > 0){
                labels = labels.substring(0, labels.length()-2);
            }
        }
        else if (radiogroup.get(0).getButtonCount() > 0)
            labels += radiogroup.get(0).getSelection().getActionCommand();
        
        addResult(labels);
    }
    
    private void sendCheckboxes() {
        int i = 0;
        String labels = "";
        for (JCheckBox cb: this.cbs) {
            if (cb.isSelected()) {
                labels += cb.getActionCommand() + ", ";
                i++;
            }
            
        }
        if(i > 0){
            labels = labels.substring(0, labels.length()-2);
        }
        
        addResult(labels);
    }
    
    //adds the textfield to the operation name
    private void sendText(JTextField textfield){
        String text = textfield.getText();
        if (parameters[hasParameter].equals("string")) {
            text = "'"+text+"'";
        }
        this.addResult(text);
    }
    
    private void send() {
        setResult();
        main.updateOperation(result);
        setVisible(false);
    }
    
}
