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
    private String result;
    private QueryconstructionViewer viewer;
    private MainPane main;
    private Operation operation;
    
    public FilterViewer(MainPane main, Operation operation, String result) {
        super();
//        this.main = main;
//        this.operation = operation;
//        this.result = result;
//        
//        this.setLayout(new BorderLayout());
//        MainPane = new MainPane(this);
//        MainPane.setPreferredSize(new Dimension (500, 400));
//        ObjectPane = new ObjectPane(this);
//        OperationsPane.update();
//        
//        JScrollPane MainScrollPane = new JScrollPane(MainPane);
//        JScrollPane ObjectsScrollPane = new JScrollPane(ObjectPane);
//        ObjectsScrollPane.setPreferredSize(new Dimension (600, 90));
//        JScrollPane OperationsScrollPane = new JScrollPane(OperationsPane);
//        OperationsScrollPane.setPreferredSize(new Dimension (120, 400));
//        
//        this.add(ObjectsScrollPane, BorderLayout.NORTH);
//        this.add(OperationsScrollPane, BorderLayout.EAST);
//        this.add(MainScrollPane, BorderLayout.CENTER);
//        
//        JPanel buttonPanel = new JPanel();
//        JButton run = new JButton("ok");
//        buttonPanel.add(run);
//        JButton back = new JButton("back");
//        buttonPanel.add(back);
//        
//        this.add(buttonPanel, BorderLayout.SOUTH);
//        
//        ActionListener runl = new ActionListener() {
//            public void actionPerformed( ActionEvent e ) {
//                addString();
//            }
//        };
//        run.addActionListener(runl);
//        
//        ActionListener backl = new ActionListener() {
//            public void actionPerformed( ActionEvent e ) {
//                back();
//            }
//        };
//        back.addActionListener(backl);
        
        JFrame f = new JFrame();
        f.add(this);
        this.setPreferredSize(new Dimension(500, 400));
        f.setPreferredSize(new Dimension(500, 400));
        f.setLocation(100, 100);
        f.setVisible(true);
        
        this.OperationsPane.setResult(result);
    }
    
    public void addObjects(ObjectView[] objects) {
        for (ObjectView object : objects) {
            ObjectPane.addObject(object);
        }
        ObjectPane.update();
    }
    
//    public String[] getParameters() {
//        
//    }
    
    @Override
    public void addObject(ObjectView object) {
        super.addObject(object);
        if (super.getParameters() != null)
            System.out.println(super.getParameters()[0] + super.getParameters().length);
    }
    
    public void addOperation(Operation op) {
        if (op.getResultType().equals(this.result)) {
            super.addOperation(op);
        }
    }
    
    public void addString() {
        String resultString = operation.getBrackets()[0] + MainPane.getStrings() + operation.getBrackets()[1];
        operation.setName(operation.getName() + resultString);
        main.update();
    }
}
