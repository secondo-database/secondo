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
    
    private QueryconstructionViewer viewer;
    private OperationsDialog dialog;
    protected static ObjectView lastComponent = new ObjectView(ObjectType.OPERATION, "query");
    private ObjectType relation1;
    private ObjectType relation2;
    private StreamView relation1View;
    private StreamView relation2View;
    
    private static int state;
    private InfoDialog infoDialog;
    
    public MainPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        state = StreamView.EMPTY;
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
        
        //the last relation added to the panel is saved
        if (object.getType().equals(ObjectType.RELATION) || object.getType().equals(ObjectType.TRELATION)) {
            if (state > StreamView.EMPTY) {
                activeStream = secondStream;
                state = StreamView.TWORELATIONS;
                relation2 = object.getOType();
            }
            else {
                state = StreamView.TUPEL;
                relation1 = object.getOType();
            }
        }
        dialog = new OperationsDialog(this);
        if (object.getName().endsWith("join")) {
            activeStream = joinStream;
            dialog.joinAttributes(relation1View, relation2View);
        }
        if (object.getName().startsWith("units")){
            //opens a list of all objects of the type mpoint
            dialog.getObjects("mpoint", viewer.getObjects());
        }
        activeStream.add(object);
        if (object.getName().startsWith("project")){
            if (state < StreamView.TWORELATIONS) {
                dialog.project(relation1.getAttributes());
            }
            else {
                dialog.project(relation2.getAttributes());
            }
        }
        if (object.getName().startsWith("head") || object.getName().startsWith("rename") || object.getName().endsWith("join")){
            dialog.text();
        }
        
        if (object.getName().equals("feed")) {
            if (state == StreamView.TWORELATIONS) {
                state = StreamView.TWOSTREAMS;
            }
            if (state == StreamView.TUPEL) {
                state = StreamView.STREAM;
            }
        }
        lastComponent = object;
        this.setPreferredSize(new Dimension(firstStream.size()*120 + joinStream.size()*120, 400));
        this.revalidate();
    }
    
    public void update() {
        this.setRelations();
        ListExpr type = viewer.getType(getStrings());
        if (type != null) {
            String text = type.second().textValue();
            this.setToolTipText(text);
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
            //if the operator feed ist removed, the state must be changed
            if (lastComponent.getName().startsWith("feed")) {
                state = state-1;
            }
            //changes the active stream if the last object has been deleted
            if (activeStream.isEmpty()) {
                if (secondStream.size() > 0) {
                    activeStream = secondStream;
                    state = StreamView.TWOSTREAMS;
                }
                else {
                    activeStream = firstStream;
                    state = StreamView.STREAM;
                }
            }
            if (!activeStream.isEmpty()) {
                lastComponent = activeStream.get(activeStream.size()-1);
            } else {
                state = StreamView.EMPTY;
            }
        }
    }
    
    public int getState() {
        return state;
    }
    
    public void getInfo(ArrayList<ObjectView> active) {
        ListExpr obj1 = viewer.getType("query " + streamToString(firstStream));
        
        if (obj1 != null){
            String result = obj1.second().textValue();
            infoDialog = new InfoDialog(new StreamView(relation1.getName(), result));
            
            ListExpr obj2 = viewer.getType("query " + streamToString(secondStream));
            if (obj2 != null) {
                infoDialog.addStream(new StreamView(relation2.getName(), obj2.second().textValue()));
            }
            
            ListExpr obj3 = viewer.getType(getStrings());
            if (obj3 != null) {
                infoDialog.addStream(new StreamView(null, obj3.second().textValue()));
            }
            infoDialog.view();
        }
    }
    
    // TODO doppelt
    public void setRelations() {
        if (firstStream != null && state > StreamView.TUPEL) {
            ListExpr obj = viewer.getType("query " + streamToString(firstStream));
            if (obj != null) {
                relation1View = new StreamView(relation1.getName(), obj.second().textValue());
            }
            if (secondStream != null) {
                ListExpr obj2 = viewer.getType("query " + streamToString(secondStream));
                if (obj2 != null) {
                    relation2View = new StreamView(relation2.getName(), obj2.second().textValue());
                }
            }
        }
        
        if (activeStream.equals(secondStream)) {
            //relation2 = new ObjectType(getRelation(activeStream).getName(), type);
        }
    }
    
    public String getStrings(){
        String query = "query ";
        
        query += streamToString(firstStream);
        
        query += streamToString(secondStream);
        
        query += streamToString(joinStream);
        
        return query;
    }
    
    public String streamToString(ArrayList<ObjectView> stream) {
        String s = "";
        for ( Iterator iter = stream.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            s += object.getName()+" ";
        }
        return s;
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
            getInfo(firstStream);
//            y--;
//            if (y == 0) {
//                if (firstStream.size() > (x-1)) {
//                    if (!firstStream.get(x-1).getInfo()) {
//                        getInfo(firstStream);
//                    }
//                }
//            }
//            if (y == 1) {
//                if (joinStream.size() > (x - 1 - firstStream.size())) {
//                    System.out.println(joinStream.get(x - 1 - firstStream.size()).getName());
//                    joinStream.get(x - 1 - firstStream.size()).getInfo();
//                }
//            }
//            if (y == 2) {
//                if (secondStream.size() > (x-1)) {
//                    //the object is not a relation
//                    if (!secondStream.get(x-1).getInfo()) {
//                        getInfo(secondStream);
//                    }
//                }
//            }
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
