/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JComponent;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class MainPane extends JComponent implements MouseListener {
    
    private ArrayList<ObjectView> activeStream = new ArrayList<ObjectView>();
    private ArrayList<ObjectView> firstStream = new ArrayList<ObjectView>();
    private ArrayList<ObjectView> secondStream = new ArrayList<ObjectView>();
    private ArrayList<ObjectView> joinStream = new ArrayList<ObjectView>();
    private ObjectView relation;
    
    private QueryconstructionViewer viewer;
    private OperationsDialog dialog;
    protected static ObjectView lastComponent = new ObjectView(ObjectType.OPERATION, "query");
    private ObjectType relation1;
    private ObjectType relation2;
    
    private static int state;
    protected static final int EMPTY = 0;
    protected static final int TUPEL = 1;
    protected static final int STREAM = 2;
    protected static final int TWOSTREAMS = 3;
    
    public MainPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        state = EMPTY;
        activeStream = firstStream;
        this.addMouseListener(this);
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
            if (state > TUPEL) {
                activeStream = secondStream;
            }
            else {
                state = TUPEL;
                
            }
        }
        relation = this.getRelation(activeStream);
        dialog = new OperationsDialog(this);
        if (object.getName().endsWith("join")) {
            activeStream = joinStream;
            dialog.joinAttributes(relation1, relation2);
        }
        if (object.getName().startsWith("units")){
            //opens a list of all objects of the type mpoint
            dialog.getObjects("mpoint", viewer.getObjects());
        }
        activeStream.add(object);
            if (object.getName().startsWith("project")){
                dialog.project(relation.getAttributes());
            }
            if (object.getName().startsWith("head") || object.getName().startsWith("rename") || object.getName().endsWith("join")){
                dialog.text();
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
        this.setPreferredSize(new Dimension(firstStream.size()*120 + joinStream.size()*120, 400));
        this.revalidate();
    }
    
    public void update(ListExpr type) {
        if (type != null) {
            this.setToolTipText(type.second().textValue());
            setRelation(type);
        }
        else {
            this.setToolTipText(getStrings());
        }
        
        this.repaint();
    }
    
    //removes the last object of the query and sets the object before to the last and active component
    public void removeLastObject(){
        if (activeStream.contains(lastComponent)) {
            activeStream.remove(lastComponent);
            if (activeStream.size() > 0) {
                lastComponent = activeStream.get(activeStream.size()-1);
            }
            else if (activeStream.equals(joinStream)) {
                activeStream = secondStream;
            }
            else if (activeStream.equals(secondStream)) {
                activeStream = firstStream;
            }
            if (activeStream.size() > 0) {
                lastComponent = activeStream.get(activeStream.size()-1);
            }
        }
    }
    
    public int getState() {
        return state;
    }
    
    public ObjectView getRelation(ArrayList<ObjectView> active) {
        ObjectView rel = null;
        for ( Iterator iter = active.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            if (object.getType().equals("rel")) {
                rel = object;
            }
        }
        return rel;
    }
    
    public void setRelation(ListExpr type) {
        if (activeStream.equals(firstStream)) {
            relation1 = new ObjectType(getRelation(activeStream).getName(), type);
        }
        if (activeStream.equals(secondStream)) {
            relation2 = new ObjectType(getRelation(activeStream).getName(), type);
        }
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
        viewer.update();
    }
    
    public void addString(String s) {
        String result = lastComponent.getName();
        if (lastComponent.getName().startsWith("head") || lastComponent.getName().endsWith("join")) {
            result += "[" + s + "]";
        }
        if (lastComponent.getName().startsWith("rename")) {
            result = "{" + s + "}";
        }
        if (lastComponent.getName().startsWith("units")) {
            result += "(" + s + ")";
        }
        lastComponent.setName(result);
        viewer.update();
    }
    
    //Handle mouse events.
    public void mouseReleased(MouseEvent e) {
    }
    public void mouseClicked ( MouseEvent e ) {
        
        //get the position of the click
        int y = 0;
        while (e.getY() > (10 + y*40)) { y++; }
        int x = 0;
        while (e.getX() > (10 + x*120)) { x++; }
        
        //right click on an object shows more information about it
        if (e.getButton() == 3) {
            y--;
            if (y == 0) {
                if (firstStream.get(x-1) != null) {
                    System.out.println(firstStream.get(x-1).getName());
                    firstStream.get(x-1).getInfo();
                }
            }
            if (y == 1) {
                if (joinStream.get(x - 1 - firstStream.size()) != null) {
                    System.out.println(joinStream.get(x - 1 - firstStream.size()).getName());
                    joinStream.get(x-1).getInfo();
                }
            }
            if (y == 2) {
                if (secondStream.get(x-1) != null) {
                    System.out.println(secondStream.get(x-1).getName());
                    secondStream.get(x-1).getInfo();
                }
            }
        }
        
        //double click sets the last object of the selected stream active
        if ((e.getClickCount () == 2) && (!activeStream.equals(joinStream))) {
            switch (y-1) {
                case 0: activeStream = firstStream;
                    lastComponent = firstStream.get(firstStream.size()-1);
                    break;
                case 2: activeStream = secondStream;
                    lastComponent = secondStream.get(secondStream.size()-1);
                    break;
            }
            viewer.update();
        }
    }
    public void mouseEntered(MouseEvent e){
    }
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {
    }
}
