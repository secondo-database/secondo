/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import javax.swing.*;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class FilterViewer extends QueryconstructionViewer {
    
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
            resultString += attribute.getText()+": ";
        }
        resultString += mainPane.getStrings() + tail;
    }
    
    /**
     * Add the result to the query and close the frame.
     */
    private void close(){
        dialog.addResult(resultString);
        f.setVisible(false);
    }
    
    /**
     * Remove the changes, that are made by the last operation.
     */
    private void exit() {
        dialog.back();
    }
    
    /**
     * reset all actions in the filter window
     */
    private void reset(){
        attribute.setText(null);
        removeAll();
        update();
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
            if (mainPane.getType().equals(resultType)) {
                paste.setEnabled(true);
            }
            if (resultType.startsWith("new") && !mainPane.getType().startsWith("(stream") && (attribute.getText().length() > 0)){
                paste.setEnabled(true);
            }
            
            if (resultType.startsWith("newstream") && mainPane.getType().startsWith("(stream")) {
                paste.setEnabled(true);
            }
            if (paste.isEnabled() && resultType.endsWith("list") && (attribute.getText().length() > 0)) {
                add.setEnabled(true);
            }
        }
    }
}
