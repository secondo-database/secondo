/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.*;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.*;
import java.util.Iterator;
import javax.swing.JComponent;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class MainPane extends JComponent implements MouseListener {
    
    private StreamView activeStream = new StreamView("", 0, 0);
    private ArrayList<StreamView> fullStream = new ArrayList<StreamView>();
    private ArrayList<StreamView> activeStreams = new ArrayList<StreamView>();
    
    private QueryconstructionViewer viewer;
    private OperationsDialog dialog;
    protected static ObjectView lastComponent = new ObjectView(ObjectType.OPERATION, "query");
    private ObjectType relation1;
    private ObjectType relation2;
    private ArrayList<ObjectType> relations = new ArrayList<ObjectType>();
    private StreamView relation1View;
    private StreamView relation2View;
    private int lastX = 0;
    private int lastY = 0;
    
    private static int state;
    private InfoDialog infoDialog = new InfoDialog();
    
    public MainPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        state = StreamView.EMPTY;
        fullStream.add(activeStream);
        
        this.addMouseListener(this);
    }
    
    @Override
    public void paintComponent(Graphics g) {
        
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            stream.paintComponent(g);
        }
        
    }    
    
    //adds an operation or an object to the main panel
    public void addObject(ObjectView object){
        if (object.isSecondoObject()) {
            activeStream = new StreamView(object.getName(), 0,lastY);
            fullStream.add(activeStream);
            relations.add(object.getOType());
            lastX = 0;
            lastY++;
        }
        else {
            lastX++;
        }
        //the last relation added to the panel is saved
        if (object.getType().equals(ObjectType.RELATION) || object.getType().equals(ObjectType.TRELATION)) {
            if (state > StreamView.EMPTY) {
                state = StreamView.TWORELATIONS;
                relation2 = object.getOType();
            }
            else {
                state = StreamView.TUPEL;
                relation1 = object.getOType();
            }
        }
        activeStream.addObject(object);
        lastComponent = object;
        
        this.update();
    }
    
    public void addOperation(Operation operation) {
        dialog = new OperationsDialog(this, operation);
        lastComponent = operation.getView();
        
        if (operation.countObjects() > 1) {
            StreamView lStream = getLongestStream();
            
            activeStream = new StreamView(operation.getName(), lStream.getX() + lStream.getLength(), lStream.getY());
            for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
                StreamView stream = (StreamView)iter.next();
                stream.setNext(activeStream);
            }
            fullStream.add(activeStream);
            lastX++;
            
            
        } 
        
        if (operation.getName().endsWith("join")) {
            //dialog.joinAttributes(relation1View, relation2View);
        }
        
        if (operation.getParameter().equals("mpoint")){
            //opens a list of all objects of the type mpoint
            dialog.getObjects(operation.getParameter(), viewer.getObjects());
        }
        activeStream.addObject(operation.getView());
        if (operation.getName().startsWith("project")){
            if (state < StreamView.TWORELATIONS) {
                dialog.project(relation1.getAttributes());
            }
            else {
                dialog.project(relation2.getAttributes());
            }
        }
        if (operation.getParameter().equals("bool")){
            dialog.filter(activeStream.getAttrObjects());
        }
        
        if (operation.getName().equals("feed")) {
            if (state == StreamView.TWORELATIONS) {
                state = StreamView.TWOSTREAMS;
            }
            if (state == StreamView.TUPEL) {
                state = StreamView.STREAM;
            }
        }
    }
    
    public void update() {
        getActiveStreams();
        this.updateStream(activeStream);
        ListExpr type = viewer.getType(getStrings());
        if (type != null) {
            String text = type.second().textValue();
            this.setToolTipText(text);
            infoDialog.viewInfo(text);
        }
        else {
            this.setToolTipText(getStrings());
        }
        this.repaint();
        this.revalidate();
    }
    
    //removes the last object of the query and sets the object before to the last and active component
    public void removeLastObject(){
        
        StreamView lastStream = fullStream.get(fullStream.size()-1);
        lastStream.getObjects().remove(lastStream.getLastComponent());
        if (lastStream.getLength() < 1) {
            if (lastStream.getY() > 0) {
                lastY--;
            }
            else if (lastStream.getX() > 0) {
                lastX--;
            }
            fullStream.remove(lastStream);
            activeStream = fullStream.get(fullStream.size()-1);
            activeStream.removeNext();
        }
        
        this.update();
    }
    
    public StreamView getStream(int x , int y){
        StreamView returnStream = null;
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (stream.getY() == y) {
                if ((stream.getX()-1 < x ) && (x < stream.getX() + stream.getLength() + 1))
                    returnStream = stream;
            }
        }
        return returnStream;
    }
    
    public void getActiveStreams(){
        activeStreams = (ArrayList<StreamView>)fullStream.clone();
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (!stream.isActive()) {
                activeStreams.remove(stream);
            }
        }
    }
    
    public StreamView getLongestStream(){
        int length = 0;
        int index = 0;
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (stream.getLength() > length) {
                length = stream.getLength();
                index = activeStreams.indexOf(stream);
            }
        }
        return activeStreams.get(index);
    }
    
    public String getType(StreamView stream) {
        return stream.getState();
    }
    
    public QueryconstructionViewer getViewer() {
        return viewer;
    }
    
    public void updateStream(StreamView stream) {
        if (stream != null) {
            ListExpr obj = viewer.getType("query " + streamToString(stream.getObjects()));
        
            if (obj != null) {
                String result = obj.second().textValue();
                if (result != null) {
                    stream.setState(result);
                }
                
            }
        }
        
    }
    
    public int getState() {
        return state;
    }
    
    public String[] getParameters() {
        String parameters[] = new String[activeStreams.size()];
        int i = 0;
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            parameters[i] = stream.getState();
            i++;
        }
        return parameters;
    }
    
    public void getInfo(ArrayList<ObjectView> active) {
        int i = 0;
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            ListExpr obj = viewer.getType("query " + streamToString(stream.getObjects()));
            if (obj != null) {
                String result = obj.second().textValue();
                stream.setState(result);
                infoDialog.addStream(stream);
                i++;
            }
        }
        infoDialog.view();
    }
    
    public String getStrings(){
        String query = "query ";
        
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            query += streamToString(stream.getObjects());
        }
        
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
    
    public void addString(String s, char[] brackets) {
        String result = lastComponent.getName();
        result += brackets[0] + s + brackets[1];
        
        lastComponent.setName(result);
        viewer.update();
    }
    
    //Handle mouse events.
    public void mouseReleased(MouseEvent e) {
    }
    public void mouseClicked ( MouseEvent e ) {
        
        //get the position of the click
        int y = 0;
        while (e.getY() > (10 + y*80)) { y++; }
        int x = 0;
        while (e.getX() > (10 + x*120)) { x++; }
        
        //right click on an object shows more information about it
        if (e.getButton() == 3) {
            infoDialog.view();
        }
        
        //double click sets the last object of the selected stream active
        if ((e.getClickCount () == 1) && (fullStream.size() > y) && (e.getButton() == 1)) {
            if (getStream(x,y-1) != null) {
                getStream(x,y-1).change();
                viewer.update();
                activeStream = this.getLongestStream();
            }
                
            if (!activeStream.equals(fullStream.get(y))) {
                //lastComponent = activeStream.getLastComponent();
                infoDialog.setTitle(fullStream.get(y).getName());
            }
            
            
        }
    }
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {}
}
