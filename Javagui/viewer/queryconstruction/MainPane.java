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
    private ObjectType relation;
    
    protected static ObjectView lastComponent = new ObjectView(ObjectType.OPERATION, "query");
    protected static int state;
    
    protected static final int TUPEL = 0;
    protected static final int STREAM = 1;
    protected static final int TWOSTREAMS = 2;
    
    public MainPane() {
        state = TUPEL;
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
        
        //speichert die letzte Tabelle, die dem Hauptbereich hinzugefügt wurde in der Variable relation
        if (object.getType().equals(ObjectType.RELATION) || object.getType().equals(ObjectType.TRELATION)) {
            relation = object.getOType();
        }
        //wenn eine Tabelle existiert, werden ihre Attribute für die Dialoge ausgewählt
        if (relation != null) {
            OperationsDialog dialog = new OperationsDialog(this, relation.getAttributes());
            if (object.getName().startsWith("project")){
                dialog.project();
            }
            if (object.getName().startsWith("head")){
                dialog.integer();
            }
        }
        if (object.getName().equals("feed")) {
            state = STREAM;
        }
        lastComponent = object;
    }
    
    public void removeLastObject(){
        if (elements.contains(lastComponent)) {
            elements.remove(lastComponent);
            lastComponent = elements.get(elements.size()-1);
        }
    }
    
    public Component getLast () {
        return lastComponent;
    }
    
    public int getState() {
        return state;
    }
    
    public String getStrings(){
        String query = "query ";
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            query += object.getName()+" ";
        }
        
        return query;
    }
    
    //adds an array of strings to the active operation
    public void addArray(String[] attributes) {
        String result = lastComponent.getName();
        result+="[";
        
        for (String att: attributes) {
            
            if (att != null)
                result+=att+", ";
        }
        lastComponent.setName(result.substring(0, result.length()-2) +"]");
        System.out.println(result.substring(0, result.length()-2) +"]");
        repaint();
    }
    
    public void addString(String s) {
        String result = lastComponent.getName();
        result+="[" + s + "]";
        lastComponent.setName(result);
        System.out.println(result);
        repaint();
    }
    
    //Handle mouse events.
    public void mouseReleased(MouseEvent e) {
    }
    public void mouseClicked ( MouseEvent e ) {
        //rechts auf das Objekt geklickt
        if (e.getButton() == 3) {
            
        }
    }
    public void mouseEntered(MouseEvent e){
    }
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {
    }
}
