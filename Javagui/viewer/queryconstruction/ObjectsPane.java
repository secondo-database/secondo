package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.Iterator;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/* ObjectsPane.java requires no other files. */
public class ObjectsPane extends MainPane {
    
    private ArrayList<ObjectView> elements = new ArrayList<ObjectView>();
    private QueryconstructionViewer viewer;
    private ListExpr objects;

    public ObjectsPane(QueryconstructionViewer viewer, ListExpr obj) {
        this.objects = obj;
        this.viewer = viewer;
        this.addMouseListener(this);
        
        ObjectView Trains = new ObjectView(RELATION, "Trains");
        addObject(Trains);
        ObjectView strassen = new ObjectView(RELATION, "strassen");
        //addObject(strassen);
        ObjectView Kinos = new ObjectView(RELATION, "Kinos");
        //addObject(Kinos);
        
        ObjectView train7 = new ObjectView(MPOINT, "train7");
        //addObject(train7);
        ObjectView mehringdamm = new ObjectView(POINT, "mehringdamm");
        //addObject(mehringdamm);
        ObjectView tiergarten = new ObjectView(REGION, "tiergarten");
        //addObject(tiergarten);
    }
    
    /** paints a Secondo Object into the ObjectsPane */
    public void paintComponent(Graphics g) {        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.paintComponent( g, x, y, null );
            x++;
        }
    }
    
    //    adds an operation or an object to the main panel
    public void addObject(ObjectView object){
        elements.add(object);
    }
    
    // updates the operations panel, only allowed operations shoul be viewed black
    public void update() {
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.setUnactive();
            if (object.getName() == "Trains") {
                object.setActive();
            }
        }
        this.repaint();
    }

    //double click adds the selected object to the main panel
    public void mouseClicked ( MouseEvent arg0 ) {
        if (arg0.getClickCount () == 2) {
            int x = 0;
            if (10 < arg0.getY() && arg0.getY() < 80) {
                while (arg0.getX() > (10 + x*120)) { x++; }
                if (arg0.getX() < (10 + x*120)) {
                    if (x <= elements.size()) {
                        if (elements.get(x-1).isActive())
                            viewer.addObject(elements.get(x-1));
                    }
                }
            }
        }
    }
}
