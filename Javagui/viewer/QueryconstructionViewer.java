/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer;

import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.Collection;
import gui.SecondoObject;
import viewer.queryconstruction.*;

/**
 *
 * @author lrentergent
 */
public class QueryconstructionViewer extends SecondoViewer {
    
    private ObjectsPane ObjectsPane = new ObjectsPane();
    private OperationsPane OperationsPane = new OperationsPane();
    private MainPane MainPane;
    
    private MenuVector MV = new MenuVector();
    
    public QueryconstructionViewer(){
        this.setLayout(new BorderLayout());
        
        MainPane = new MainPane();
               
        ObjectsPane.setPreferredSize(new Dimension (500, 100));
        OperationsPane.setPreferredSize(new Dimension (110, 500));
        MainPane.setPreferredSize(new Dimension (600, 400));
        
        ObjectComponent query = new ObjectComponent("operation", "query");
        MainPane.addObject(query);
        ObjectComponent neu1 = new ObjectComponent("rel", "train");
        MainPane.addObject(neu1);
        
        ObjectComponent neu2 = new ObjectComponent("rel", "train");
        ObjectsPane.addObject(neu2);
        
        ObjectComponent neu3 = new ObjectComponent("operation", "feed");
        OperationsPane.addObject(neu3);
        
        JScrollPane ScrollPane = new JScrollPane(MainPane);
       
        this.add(ObjectsPane, BorderLayout.NORTH);
        this.add(OperationsPane, BorderLayout.EAST);
        this.add(ScrollPane, BorderLayout.CENTER);
    }
    
    
    public boolean addObject(SecondoObject o){
        return true;
    }
    
    public void removeObject(SecondoObject o){
        
    }
    
    public boolean selectObject (SecondoObject o) {
        return true;
    }
    
    /** remove all containing objects */
    public void removeAll(){
    
    }
    
    /** returns InquiryViewer */
    public String getName(){
        return "QueryconstructionViewer";
    }
    
    public boolean canDisplay(SecondoObject o) {
        return true;
    }

    /** check if o displayed in the moment **/
    public boolean isDisplayed(SecondoObject o) {
        return true;
    }
    
    public MenuVector getMenuVector() {
        return MV;
    }
    
    public double getDisplayQuality(SecondoObject SO) {
        return 1.0;
    }

    public void enableTestmode (boolean on) {
        on = true;
    }
}
