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
    private JFrame f = new JFrame();
    
    ArrayList<ObjectView> objects;
    ArrayList<Operation> operations;
    
    public FilterViewer(OperationsDialog dialog) {
        super();
        this.dialog = dialog;
        f.addWindowListener( new WindowAdapter() {
            public void windowClosing ( WindowEvent e) {
                back();
            }
        } );
        
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
    
    protected void showViewer() {
        f.add(this);
        f.setSize(new Dimension(500, 400));
        f.setLocation(100, 100);
        f.setVisible(true);
    }
    
    public void back() {
        dialog.back();
    }
    
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
}
