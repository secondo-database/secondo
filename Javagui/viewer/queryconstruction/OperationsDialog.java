//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
package viewer.queryconstruction;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.*;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class OperationsDialog extends JDialog {
    
    private MainPane main;
    private QueryconstructionViewer mainViewer;
    private FilterViewer filterViewer =  new FilterViewer(this);
    private String[] params;
    private ArrayList<ObjectView> allObjects;
    
    private JDialog objectDialog;
    private JPanel buttonPanel;
            
    private JCheckBox[] cbs;
    private ArrayList<ButtonGroup> radiogroup = new ArrayList<ButtonGroup>();
    private ButtonGroup objectButtons = new ButtonGroup();
    private boolean hasButtons = false;
    
    private int hasParameter = 0;
    
    protected final static char obChar = 'o';
    protected final static char opChar = '#';
    protected final static char pChar = 'p';
    
    public OperationsDialog(MainPane main, QueryconstructionViewer viewer, Operation operation, ArrayList<ObjectView> objects) {
        this.main = main;
        this.mainViewer = viewer;
        
        this.resetObjectDialog();
        setLayout(new GridLayout(0,1));
        if (main.isShowing())
            this.setLocation(main.getLocationOnScreen());
        
        params = operation.getParameter();
        
        this.addWindowListener( new WindowAdapter() {
            public void windowClosing ( WindowEvent e) {
                back();
            }
        } );
        
        filterViewer.setOperators(mainViewer.getOperatorList());
        filterViewer.setObjects(mainViewer.getObjectList());
        allObjects = filterViewer.getObjects();
        filterViewer.setViewerControl(viewer.getViewerControl());
    }
    
    protected void activate(){
        check();
    }
    
    protected void back(){
        mainViewer.back();
    }
    
    private void showDialog() {
        this.radiogroup.remove(objectButtons);
        
        String parameter = params[hasParameter];
        if (parameter.equals("bool") || parameter.startsWith("new")){
            nestedQuery(parameter);
            if (parameter.startsWith("new"))
                filterViewer.setLabel("attribute name", "First letter has to be uppercase.");
        }
        if (!parameter.equals("bool")) {
            for (String param: parameter.split(",")) {
                addObjectButtons(param);
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
        
        dialog.addWindowListener( new WindowAdapter() {
            public void windowClosing ( WindowEvent e) {
                back();
            }
        } );
        
        if (main.isShowing())
            dialog.setLocation(main.getLocationOnScreen());
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
        this.filterViewer.addObjects(atts);
    }
    
    /**
     * adds CheckBoxes for attributes of an object to the dialog window
     * @param objectName
     * @param atts attributes of the object
     */
    protected void addCheckboxes(String objectName, String[] atts) {
        JLabel name = new JLabel(objectName);
        this.add(name);
        this.cbs = new JCheckBox[atts.length];
        int i = 0;
        for (String att: atts) {
            cbs[i] = new JCheckBox( att, false );
            
            i++;
        }
    }
    
    /**
     * shows a RadioButton list of objects of the type "type"
     * @param type 
     */
    private void addObjectButtons(String type) {
        int i = 0;
        String[] types = type.split(",");
        for (ObjectView o: allObjects) {
            for (String t: types) {
                if (o.getType().equals(t)) {
                    JRadioButton rb = new JRadioButton(o.getType() + " " + o.getObjectName());
                    rb.setActionCommand(o.getObjectName());
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
    protected void addRadiobuttons(String objectName, String[] atts) {
        JLabel name = new JLabel(objectName);
        buttonPanel.add(name);
        ButtonGroup buttons = new ButtonGroup();
        int i = 0;
        for (String att: atts) {
            JRadioButton rb = new JRadioButton( att );
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
    
    /**
     * Add the string s to the result string.
     * @param s 
     */
    protected void addResult(String s){
        hasParameter++;
        
        ObjectView new_object = new ObjectView(s, "param");
        StreamView paramStream = new StreamView("OperationStream", "", 0, 0);
        paramStream.addObject(new_object);
        main.addParamStream(paramStream);
        
        check();
    }
    
    /**
     * Adds the tuple as an object to the filterViewer.
     * @param tuple 
     */
    protected void addTuple(ObjectView tuple){
        filterViewer.addObjects(new ObjectView[]{tuple});
    }
    
    /**
     * check if the input params are complete
     */
    private void check() {
        if (hasParameter == params.length) {
            close();
        }
        else {
            showDialog();
        }
    }
    
    private void project() {
        for (JCheckBox cb: this.cbs){
            add(cb);
        }
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
     * Activates the FilterViewer to get a boolean parameter.
     */
    private void nestedQuery(String result){
        filterViewer.setResult(result);
        filterViewer.showViewer();
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
        objectDialog.dispose();
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
        objectDialog.dispose();
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
    
    /**
     * adds the text of the textfield to the result
     * @param textfield 
     */
    private void sendText(JTextField textfield){
        String text = textfield.getText();
        addResult(text);
    }
    
    private void close() {
        this.dispose();
        mainViewer.update();
    }
    
}
