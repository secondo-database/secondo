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
    private MainPane main;
    private Operation operation;
    private JFrame f = new JFrame();
    
    public FilterViewer(MainPane main, Operation operation, ArrayList<ObjectView> objects) {
        this.main = main;
        this.operation = operation;
        
        this.setLayout(new BorderLayout());
        MainPane = new MainPane(this);
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
    
    public void addObjects(ObjectView[] objects) {
        for (ObjectView object : objects) {
            ObjectPane.addObject(object);
        }
        ObjectPane.update();
    }
    
    //adds an object to the main panel
    public void addOperation(Operation operation){
        super.addOperation(operation);
        if (!operation.getParameter().equals("")) {
            ObjectPane.updateObjects(operation.getParameter());
        }
        else {
            ObjectPane.showAllObjects();
        }
        update();
    }
    
    public void addString() {
        String brackets = operation.getBrackets();
        String resultString = brackets.toCharArray()[0] + MainPane.getStrings() + brackets.toCharArray()[1];
        operation.setName(operation.getName() + resultString);
        main.update();
        //TODO Objekt bleibt bestehen
        f.setVisible(false);
    }
}
