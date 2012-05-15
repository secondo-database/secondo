package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

/* ObjectsPane.java requires no other files. */
public class ObjectsPane extends JComponent implements MouseListener {
    
    private Collection<ObjectComponent> elements = new ArrayList<ObjectComponent>();
    private int count = 3; //Objects counter

    public ObjectsPane() {
        
    }
    
    /** paints a Secondo Object into the ObjectsPane */
    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        
        
        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectComponent object = (ObjectComponent)iter.next();
            object.paintComponent( g, x, y );
            x++;
        }
        
    }
    
    //    adds an operation or an object to the main panel
    public void addObject(ObjectComponent object){
        object.addMouseListener(this);
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
    public void mousePressed(MouseEvent e){}

    
}
