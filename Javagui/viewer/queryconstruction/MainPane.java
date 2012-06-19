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
    
    private ArrayList<ObjectView> activeStream = new ArrayList<ObjectView>();
    private ArrayList<ObjectView> firstStream = new ArrayList<ObjectView>();
    private ArrayList<ObjectView> secondStream = new ArrayList<ObjectView>();
    private ArrayList<ObjectView> joinStream = new ArrayList<ObjectView>();
    private ObjectType relation;
    
    protected static ObjectView lastComponent = new ObjectView(ObjectType.OPERATION, "query");
    
    private static int state;
    protected static final int EMPTY = 0;
    protected static final int TUPEL = 1;
    protected static final int STREAM = 2;
    protected static final int TWOSTREAMS = 3;
    
    public MainPane() {
        state = EMPTY;
        activeStream = firstStream;
    }
    
    @Override
    public void paintComponent(Graphics g) {
        int x = 0;
        int y = 0;
        
        //paints all objects and operations of the first stream
        for ( Iterator iter = firstStream.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.paintComponent( g, x, y, Color.black);
            if (object.equals(lastComponent)){
                g.setColor(Color.red);
            }
            g.drawLine(10 + x*120 + 90, 10 + y*40 + 25, 10 + x*120 + 120, 10 + y*40 + 25);
            x++;
        }
        
        x = 0;
        y = 2;
        //paints all objects and operations of the second stream
        for ( Iterator iter = secondStream.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.paintComponent( g, x, y, Color.black);
            if (object.equals(lastComponent)){
                g.setColor(Color.red);
            }
            g.drawLine(10 + x*120 + 90, 10 + y*40 + 25, 10 + x*120 + 120, 10 + y*40 + 25);
            x++;
        }
        if (joinStream.size() > 0) {
            y = 1;
            //paints a connecting line between the two streams
            g.drawLine(10 + x*120, 35, 10 + x*120, 115);
            
            //paints all objects after the join operator
            for ( Iterator iter = joinStream.iterator(); iter.hasNext(); ) {
                ObjectView object = (ObjectView)iter.next();
                object.paintComponent( g, x, y, Color.black);
                if (object.equals(lastComponent)){
                    g.setColor(Color.red);
                }
                g.drawLine(10 + x*120 + 90, 10 + y*40 + 25, 10 + x*120 + 120, 10 + y*40 + 25);
                x++;
            }
        }
    }    
    
    //adds an operation or an object to the main panel
    public void addObject(ObjectView object){
        
        //speichert die letzte Tabelle, die dem Hauptbereich hinzugefügt wurde in der Variable relation
        if (object.getType().equals(ObjectType.RELATION) || object.getType().equals(ObjectType.TRELATION)) {
            relation = object.getOType();
            if (state > TUPEL) {
                activeStream = secondStream;
            }
            else {
                state = TUPEL;
            }
        }
        if (object.getName().endsWith("join")) {
            activeStream = joinStream;
        }
        activeStream.add(object);
        //wenn eine Tabelle existiert, werden ihre Attribute für die Dialoge ausgewählt
        if (relation != null) {
            OperationsDialog dialog = new OperationsDialog(this, relation.getAttributes());
            if (object.getName().startsWith("project")){
                dialog.project();
            }
            if (object.getName().startsWith("head") || object.getName().startsWith("rename") || object.getName().startsWith("symmjoin")){
                dialog.text();
            }
        }
        if (object.getName().equals("feed")) {
            if (state == STREAM) {
                state = TWOSTREAMS;
            }
            if (state == TUPEL) {
                state = STREAM;
            }
        }
        lastComponent = object;
    }
    
    //removes the last object of the query and sets the object before to the last and active component
    public void removeLastObject(){
        if (activeStream.contains(lastComponent)) {
            activeStream.remove(lastComponent);
            if (activeStream.size() > 1) {
                lastComponent = activeStream.get(activeStream.size()-1);
            }
            else if (activeStream.equals(joinStream)) {
                lastComponent = secondStream.get(activeStream.size()-1);
            }
            else if (activeStream.equals(secondStream)) {
                lastComponent = firstStream.get(activeStream.size()-1);
            }
        }
    }
    
    public int getState() {
        return state;
    }
    
    public String getStrings(){
        String query = "query ";
        
        for ( Iterator iter = firstStream.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            query += object.getName()+" ";
        }
        for ( Iterator iter = secondStream.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            query += object.getName()+" ";
        }
        for ( Iterator iter = joinStream.iterator(); iter.hasNext(); ) {
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
        repaint();
    }
    
    public void addString(String s) {
        String result = lastComponent.getName();
        if (lastComponent.getName().startsWith("head") || lastComponent.getName().startsWith("symmjoin")) {
            result+="[" + s + "]";
            lastComponent.setName(result);
            
        }
        if (lastComponent.getName().startsWith("rename")) {
            lastComponent.setName("{" + s + "}");
        }
        repaint();
    }
    
    //Handle mouse events.
    public void mouseReleased(MouseEvent e) {
    }
    public void mouseClicked ( MouseEvent e ) {
        //rechts auf das Objekt geklickt
        if (e.getButton() == 3) {
            System.out.println(e.getX());
        }
    }
    public void mouseEntered(MouseEvent e){
    }
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {
    }
}
