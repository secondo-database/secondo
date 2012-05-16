/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.*;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import javax.swing.*;
import java.util.*;

/**
 *
 * @author lrentergent
 */
public class MainPane extends JComponent implements MouseListener {
    
    private ArrayList<ObjectView> elements = new ArrayList<ObjectView>();
    static protected Component lastComponent;
    
    public MainPane() {
        //this.addMouseListener(this);
    }
    
    public void paintComponent(Graphics g) {
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.paintComponent( g, x, y, Color.black);
            if (object.equals(lastComponent)){
                g.setColor(Color.red);
            }
            g.drawLine(10 + x*120 + 90, 10 + y*70 + 25, 10 + x*120 + 120, 10 + y*70 + 25);
            x++;
        }
        
    }    
    
    //adds an operation or an object to the main panel
    public void addObject(ObjectView object){
        elements.add(object);
        lastComponent = object;
        System.out.println(getStrings());
    }
    
    public Component getLast () {
        return lastComponent;
    }
    
    public String getStrings(){
        String query = new String();
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            query += object.getName()+" ";
        }
        
        return query;
    }
    
    //Handle mouse events.
    public void mouseReleased(MouseEvent e) {
    }
    public void mouseClicked ( MouseEvent arg0 ) {
    }
    public void mouseEntered(MouseEvent e){
        
    }
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {
    }
}
