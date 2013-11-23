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

package viewer.queryconstruction2;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.*;
import viewer.QueryconstructionViewer2;

/**
 * Window for the nested query construction.
 */
public class FilterViewer extends QueryconstructionViewer2 {
    
    private OperationsDialog dialog;
    private String resultType;
    private String resultString = "";
    
    private JFrame f = new JFrame();
    private JPanel buttonPanel = new JPanel();
    private JButton paste = new JButton("ok");
    private JButton add = new JButton("add");
    private JTextField attribute = new JTextField(15);
    private JLabel textLabel = new JLabel();
    
    /**
     * Construct a new queryconstruction window.
     * @param dialog receives the result query
     */
    public FilterViewer(OperationsDialog dialog) {
        super();
        this.dialog = dialog;
        
        f.addWindowListener( new WindowAdapter() {
            public void windowClosing ( WindowEvent e) {
                exit();
            }
        } );
        
        /* add the buttons for the nested viewer */
        paste.setEnabled(false);
        buttonPanel.add(paste);
        add.setEnabled(false);
        buttonPanel.add(add);
        buttonPanel.add(back);
        buttonPanel.add(attribute);
        buttonPanel.add(textLabel);
        
        this.add(buttonPanel, BorderLayout.SOUTH);
        
        ActionListener okl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                addString("");
                close();
            }
        };
        paste.addActionListener(okl);
        
        ActionListener addl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                addString(", ");
                reset();
            }
        };
        add.addActionListener(addl);
        
        /* update the window, if the a text is added */
        ActionListener textL = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                update();
            }
        };
        attribute.addActionListener(textL);
    }    
    
    /**
     * add the attribute objects to the object panel
     * @param objects attribute objects
     */
    protected void addAttributes(ArrayList<ObjectView> objects) {
        for ( Iterator iter = objects.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            objectPane.addAttributeObject(object);
        }
        update();
    }
    
    /**
     * add the attribute objects to the object panel
     * @param objects attribute objects
     */
    protected void addObjects(ObjectView[] objects) {
        for (ObjectView o: objects) {
            objectPane.addAttributeObject(o);
        }
        objectPane.showAllObjects();
        update();
    }
    
    /**
     * Add the generated string to the result string.
     */
    private void addString(String tail) {
        if  (attribute.getText().length() > 0) {
            if (resultType.startsWith("new"))
                resultString += attribute.getText()+": ";
            if (resultType.equals("fun")) {
                resultString += "fun(" + attribute.getText()
                        + ": TUPLE) ";
            }
            
        }
        
        resultString += mainPane.getStrings() + tail;
        /* replace the automatically generated tuple name */
        if (resultType.equals("fun")) {
            resultString = resultString.replace("(t,", "("+attribute.getText()+",");
        }
        
    }
    
    /**
     * Add the result to the query and close the frame.
     */
    private void close(){
        dialog.addResult(resultString, null);
        f.setVisible(false);
    }
    
    /**
     * Remove the changes, that are made by the last operation.
     */
    private void exit() {
        dialog.back();
    }
    
    protected void rename(String tuple){
        objectPane.renameAttributes(tuple);
    }
    
    /**
     * reset all actions in the filter window
     */
    private void reset(){
        attribute.setText(null);
        mainPane.removeAllObjects();
        this.update();
    }
    
    /**
     * Set the label of the buttonPanel.
     * @param text
     * @param tooltip Tooltip text of the textfield.
     */
    protected void setLabel(String text, String tooltip) {
        textLabel.setText(text);
        attribute.setToolTipText(tooltip);
        buttonPanel.revalidate();
    }
    
    /**
     * Set the expected result type.
     * @param result 
     */
    protected void setResult(String result) {
        this.resultType = result;
    }
    
    /**
     * Set the viewer frame visible.
     */
    protected void showViewer() {
        f.add(this);
        f.setSize(new Dimension(500, 400));
        f.setLocation(100, 100);
        f.setVisible(true);
    }
    
    /**
     * Update the FilterViewer and all buttons.
     */
    public void update(){
        super.update();
        if (resultType != null) {
            //parameter function is expected
            if (resultType.equals("fun") && 
                    (attribute.getText().length() > 0)) {
                paste.setEnabled(true);
            }
            for (String type: resultType.split(",")) {
                //result type equals the active type
                if (mainPane.getType().equals(type.trim())) {
                    paste.setEnabled(true);
                }
            }
            //new stream or new object is expected
            if (resultType.startsWith("new") && 
                    (attribute.getText().length() > 0)){
                if (!resultType.contains("stream") &&
                        !mainPane.getType().startsWith("(stream"))
                    paste.setEnabled(true);
                //only stream of data is allowed, instead of stream of tuple
                if (resultType.contains("stream") && 
                        mainPane.getType().startsWith("(stream") && 
                        !mainPane.getType().startsWith("(stream (tuple"))
                    paste.setEnabled(true);
            }
            //a list of objects is expected
            if (paste.isEnabled() && resultType.endsWith("list") && 
                    (attribute.getText().length() > 0)) {
                add.setEnabled(true);
            }
        }
    }
}
