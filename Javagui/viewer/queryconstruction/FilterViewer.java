/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import viewer.QueryconstructionViewer;
import java.util.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import sj.lang.ListExpr;

/**
 *
 * @author lrentergent
 */
public class FilterViewer extends QueryconstructionViewer {
    
    private OperationsDialog dialog;
    private String result;
    private JFrame f = new JFrame();
    private JButton ok = new JButton("ok");
    
    ArrayList<ObjectView> objects;
    ArrayList<Operation> operations;
    
    public FilterViewer(OperationsDialog dialog) {
        super();
        this.dialog = dialog;
        this.result = result;
        
        f.addWindowListener( new WindowAdapter() {
            public void windowClosing ( WindowEvent e) {
                reset();
            }
        } );
        
        JPanel buttonPanel = new JPanel();
        
        ok.setEnabled(false);
        buttonPanel.add(ok);
        buttonPanel.add(back);
        
        this.add(buttonPanel, BorderLayout.SOUTH);
        
        ActionListener okl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                addString();
            }
        };
        ok.addActionListener(okl);
    }
    
    protected void setResult(String result) {
        this.result = result;
    }
    
    protected void showViewer() {
        f.add(this);
        f.setSize(new Dimension(500, 400));
        f.setLocation(100, 100);
        f.setVisible(true);
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
     * add the generated string to the query
     */
    private void addString() {
        dialog.addResult(mainPane.getStrings());
        f.setVisible(false);
    }
    
    /**
     * reset all actions in the filter window
     */
    private void reset() {
        dialog.back();
    }
    
    public void update(){
        super.update();
        if (result != null) {
            if (mainPane.getType().equals(result)) {
                ok.setEnabled(true);
            }
            if (result.equals("new") && !mainPane.getType().startsWith("(stream")){
                ok.setEnabled(true);
            }
            if (result.equals("newstream") && mainPane.getType().startsWith("(stream")) {
                ok.setEnabled(true);
            }
        }
    }
}
