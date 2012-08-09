package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Iterator;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/* ObjectsPane.java requires no other files. */
public class ObjectPane extends MainPane {
    
    private ArrayList<ObjectView> elements = new ArrayList<ObjectView>();
    private QueryconstructionViewer viewer;
    private ListExpr objects;

    public ObjectPane (QueryconstructionViewer viewer) {
        super(viewer);
        this.viewer = viewer;
        this.setLayout(new GridLayout(1, 0));
    }
    
    //add all relations and objects of berlintest database to the viewer
    public ArrayList<ObjectView> addObjects(ListExpr list){
        this.removeAll();
        
        // the length must be two and the object element must be an symbol atom with content "inquiry"
        if(list.listLength()==2 && list.first().symbolValue().equals("inquiry")) { 
            objects = list.second().second();
            setPreferredSize(new Dimension (objects.listLength()*120, 70));
            while (objects.listLength() > 1) {
                ListExpr object = objects.second();
                ObjectView objectView = new ObjectView(object);
                addObject(objectView);
                objects = objects.rest();
            }
        }
        this.update();
        return elements;
    }
    
    //    adds an operation or an object to the main panel
    @Override
    public void addObject(ObjectView object){
        object.addMouseListener(this);
        add(object);
        elements.add(object);
    }
    
    public void showObjectType(String type) {
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            if (!object.getType().equals(type)) {
                object.setActive(true);
            }
        }
        this.revalidate();
    }
    
    // updates the panel, only active objects are shown
    public void update() {
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            //TODO alle Objekte sind aktiv
            object.setActive(true);
        }
        this.revalidate();
    }

    //double click adds a copy of the selected object to the main panel
    @Override
    public void mouseClicked ( MouseEvent arg0 ) {
        if (arg0.getComponent() != null) {
            ObjectView element = (ObjectView)arg0.getComponent();
            
            if (arg0.getButton() == 3) {
                InfoDialog dialog = new InfoDialog(element.getOType());
                dialog.viewInfo(element.getOType().getViewString());
                dialog.view();
            }
            else {
                ObjectView new_object1 = new ObjectView(element.getType(), element.getName());
                new_object1.setOType(element.getOType());
                viewer.addObject(new_object1);
            }
        }
    }
    public void mousePressed(MouseEvent e) {}

    public void mouseReleased(MouseEvent e) {}

    public void mouseEntered(MouseEvent e) {}

    public void mouseExited(MouseEvent e) {}

}
