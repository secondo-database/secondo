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
    
    private Collection<ObjectComponent> elements = new ArrayList<ObjectComponent>();
    
    public MainPane() {
        this.addMouseListener(this);
    }
    
    public void paintComponent(Graphics g) {
        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectComponent object = (ObjectComponent)iter.next();
            object.paintComponent( g, x, y);
            if (!iter.hasNext()){
                g.setColor(Color.red);
            }
            g.drawLine(10 + x*120 + 90, 10 + y*70 + 25, 10 + x*120 + 120, 10 + y*70 + 25);
            x++;
        }
        
    }    
    
//    adds an operation or an object to the main panel
    public void addObject(ObjectComponent object){
        elements.add(object);
    }
    
    //Handle mouse events.
    public void mouseReleased(MouseEvent e) {
    }
    public void mouseClicked ( final MouseEvent arg0 ) {
        if (arg0.getClickCount () == 2) {
            System.out.println("es wurde doppelgeklickt");
        }
    }
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {
        //System.out.println("es wurde doppelgeklickt");
    }
    
//    public void run() {
//        
//        while (true) {
//            ObjectComponent neu2 = new ObjectComponent("operation", "query");
//            elements.add(neu2);
//            for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
//                ObjectComponent object = (ObjectComponent)iter.next();
//                
//            }
//
//            this.repaint();
//
//            try {
//                runner.sleep(500);
//            }
//            catch (InterruptedException e) {
//                e.printStackTrace();
//            }
//
//        }
//
//     }
}
