package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.*;
import javax.swing.*;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/* ObjectsPane.java requires no other files. */
public class ObjectPane  extends MainPane {
    
    private ArrayList<ObjectView> elements = new ArrayList<ObjectView>();
    private QueryconstructionViewer viewer;
    private ListExpr objects;
    private JTextField textfield = new JTextField();
    private InfoDialog dialog = new InfoDialog();

    public ObjectPane (QueryconstructionViewer viewer) {
        super(viewer);
        this.viewer = viewer;
        this.setLayout(new GridLayout(1, 0));
        
        ActionListener textL = new ActionListener() {
            @Override
            public void actionPerformed( ActionEvent e ) {
                String objName = textfield.getText();
                updateObjects(objName);
            }
        };
        
        textfield.addActionListener(textL);
    }
    
    //add all relations and objects of berlintest database to the viewer
    public void addObjects(ListExpr list){
        this.removeAll();
        elements.clear();
        this.add(textfield);
        
        // the length must be two and the object element must be a symbol atom with content "inquiry"
        if(list.listLength()==2 && list.first().symbolValue().equals("inquiry")) { 
            objects = list.second().second();
            
            while (objects.listLength() > 1) {
                ListExpr object = objects.second();
                
                ObjectView objectView = new ObjectView(object);
                if (!objectView.getName().startsWith("SEC")) {
                    addObject(objectView);
                }
                objects = objects.rest();
            }
        }
        this.update();
    }
    
    public void addObjects(ArrayList<ObjectView> objects) {
        elements = objects;
    }
    
    public ArrayList<ObjectView> getObjects(){
        return elements;
    }
    
    //adds an operation or an object to the main panel
    @Override
    public void addObject(ObjectView object){
        object.addMouseListener(this);
        object.setActive(true);
        add(object);
        elements.add(object);
    }
    
    // updates the panel, only active objects are shown
    @Override
    public void update() {
        setPreferredSize(new Dimension (this.getComponentCount()*120, 70));
        this.revalidate();
    }
    
    public void showAllObjects() {
        this.removeAll();
        this.add(textfield);
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            this.add(object);
            object.setActive(true);
        }
    }
    
    // updates the panel, only active that fit to the input of the textfield are shown
    public void updateObjects(String s) {
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            //TODO alle Objekte sind aktiv
            if (object.getName().toLowerCase().startsWith(s.toLowerCase()) || object.getType().equals(s)) {
                if (!object.isActive()) {
                    this.add(object);
                    object.setActive(true);
                }
            }
            else {
                this.remove(object);
                object.setActive(false);
            }
        }
    }

    @Override
    public void mouseClicked ( MouseEvent arg0 ) {
        if (arg0.getComponent() != null) {
            ObjectView element = (ObjectView)arg0.getComponent();
            
            if (arg0.getButton() == 3) {
                //generating the info dialog of the clicked object
                if (element.getOType() == null) {
                    dialog.viewInfo(element.getName(), element.getType());
                }
                else {
                    dialog.viewInfo(element.getName(), element.getOType().getViewString());
                }
                dialog.view();
            }
            else {
                //a copy of the object is added to the main panel
                ObjectView new_object = new ObjectView(element.getType(), element.getName());
                new_object.setOType(element.getOType());
                viewer.addObject(new_object);
            }
        }
    }

}
