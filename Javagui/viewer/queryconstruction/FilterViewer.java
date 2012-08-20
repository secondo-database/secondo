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

/**
 *
 * @author lrentergent
 */
public class FilterViewer extends QueryconstructionViewer {
    private OperationsDialog dialog;
    private JFrame f = new JFrame();
    
    public FilterViewer(OperationsDialog dialog, ArrayList<ObjectView> objects) {
        this.dialog = dialog;
        
        this.setLayout(new BorderLayout());
        MainPane = new MainPane(this);
        ObjectPane = new ObjectPane(this);
        OperationsPane = new OperationsPane(this);
        MainPane.setPreferredSize(new Dimension (500, 400));
        ObjectPane = new ObjectPane(this);
        ObjectPane.addObjects(objects);
        OperationsPane.update();
        
        JScrollPane MainScrollPane = new JScrollPane(MainPane);
        JScrollPane ObjectsScrollPane = new JScrollPane(ObjectPane);
        ObjectsScrollPane.setPreferredSize(new Dimension (600, 90));
        JScrollPane OperationsScrollPane = new JScrollPane(OperationsPane);
        OperationsScrollPane.setPreferredSize(new Dimension (120, 400));
        
        this.add(ObjectsScrollPane, BorderLayout.NORTH);
        this.add(OperationsScrollPane, BorderLayout.EAST);
        this.add(MainScrollPane, BorderLayout.CENTER);
        
        JPanel buttonPanel = new JPanel();
        JButton ok = new JButton("ok");
        buttonPanel.add(ok);
        JButton back = new JButton("back");
        buttonPanel.add(back);
        
        this.add(buttonPanel, BorderLayout.SOUTH);
        
        ActionListener okl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                addString();
            }
        };
        ok.addActionListener(okl);
        
        ActionListener backl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                back();
            }
        };
        back.addActionListener(backl);
        
    }
    
    public void show() {
        f.add(this);
        f.setSize(new Dimension(500, 400));
        f.setLocation(100, 100);
        f.setVisible(true);
    }
    
    protected void addObjects(ObjectView[] objects) {
        ObjectPane.clear();
        for (ObjectView o: objects) {
            ObjectPane.addAttributeObject(o);
        }
        update();
    }
    
    public void updateObjects(String type) {
        this.ObjectPane.updateObjects(type);
    }
    
    //
    /**
     * adds an operation to the dialog main panel
     * @param operation new operation
     */
    public void addOperation(Operation operation){
        this.MainPane.addOperation(operation);
        if (!operation.getParameter().equals("")) {
            this.ObjectPane.updateObjects(operation.getParameter());
        }
        else {
            this.ObjectPane.showAllObjects();
        }
        update();
    }
    
    /**
     * add the generated string to the query
     */
    private void addString() {
        dialog.addResult(MainPane.getStrings());
        f.setVisible(false);
    }
}
