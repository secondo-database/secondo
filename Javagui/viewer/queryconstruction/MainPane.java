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
    private ObjectView lastComponent;
    private int lastY = 0;
    private int lastX = 0;
    
    private static int state;
    private InfoDialog infoDialog = new InfoDialog();
    
    public MainPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        state = StreamView.EMPTY;
        //fullStream.add(activeStream);
        
        this.addMouseListener(this);
    }
    
    public void paintComponent(Graphics g) {
        
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            stream.paintComponent(g);
        }
        
    }    
    
    //adds an operation or an object to the main panel
    public void addObject(ObjectView object){
        activeStream = new StreamView(object.getName(), 0, lastY);
        fullStream.add(activeStream);
        lastY++;
        activeStream.addObject(object);
        lastComponent = object;
    }
    
    public void addOperation(Operation operation) {
        
        if (operation.getParameter() != null) {
            String parameter = operation.getParameter();
            if (operation.getParameter().equals("typ")) {
                parameter = lastComponent.getType();
            }
            dialog = new OperationsDialog(this, operation, viewer.getObjects());
            dialog.getObjects(parameter);
            if (operation.getObjects().length > 0) {
                String dot = ".";
                for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
                    StreamView stream = (StreamView)iter.next();
                    if (stream.getAttributes() != null) {
                        dialog.addAttributes(stream.getAttrObjects(dot));
                        if (operation.getParameter().equals("attr,attr")) {
                            dialog.addRadiobuttons(stream.getName(), stream.getAttributes());
                        }
                        if (operation.getParameter().equals("attrlist")) {
                            dialog.addCheckboxes(stream.getName(), stream.getAttributes());
                        }
                        dot += ".";
                    }
                }
                if (operation.getParameter().equals("attr,attr")) {
                    dialog.joinAttributes();
                }
                if (operation.getParameter().equals("attrlist")) {
                    dialog.project();
                }
            }
            
        }
        
        lastComponent = operation.getView();
        
        if (operation.countObjects() > 1) {
            lastX = getLastX();
            
            activeStream = new StreamView(operation.getName(), lastX, activeStream.getY());
            for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
                StreamView stream = (StreamView)iter.next();
                activeStream.addInputStream(stream);
                stream.setNext(activeStream);
            }
            fullStream.add(activeStream);
            
        } 
        if (operation.getObjects() == new String[]{null}) {
            addObject(new ObjectView(operation.getName(), operation.getResultType()));
        }
        else {
            activeStream.addObject(operation.getView());
        }
        
        if (operation.getParameter().equals("bool")){
            dialog.filter();
        }
        
    }
    
    /**
     * update the panel and the stream information
     */
    public void update() {
        setActiveStreams();
        this.updateStream(activeStream);
        ListExpr type = viewer.getType(getStringsQuery());
        if (type != null) {
            String text = type.second().textValue();
            this.setToolTipText(text);
        }
        else {
            this.setToolTipText(getStringsQuery());
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
            fullStream.remove(lastStream);
            if (fullStream.size() > 0) {
                activeStream = fullStream.get(fullStream.size()-1);
                if (activeStreams.size() > 0) {
                    for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
                        StreamView stream = (StreamView)iter.next();
                        stream.removeNext();
                    }
                }
            }
            else {
                activeStream = new StreamView("new", 0, 0);
            }
        }
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
    
    /**
     * check for attributes with the same name
     * @return true, if no double attribute names exist
     */
    public boolean checkAttributes() {
        String[][] attributes = new String[activeStreams.size()][];
        int i = 0;
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            attributes[i] = stream.getAttributes();
            i++;
        }
        int j = 0;
        while (j < attributes.length-1) {
            for (String attr1 : attributes[j]) {
                System.out.println(attr1);
                for (String attr2 : attributes[j+1]) {
                    if (attr1.equals(attr2)) {
                        return false;
                    }
                }
            }
            j++;
        }
        for (String[] sarray : attributes) {
            for (String attr : sarray) {
                System.out.println(attr);
            }
        }
        return true;
    }
    
    public void setActiveStreams(){
        activeStreams = (ArrayList<StreamView>)fullStream.clone();
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (!stream.isActive()) {
                activeStreams.remove(stream);
            }
            else {
                activeStream = stream;
            }
        }
    }
    
    public int getLastX(){
        int length = 0;
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (stream.getX() + stream.getLength() > length) {
                length = stream.getX() + stream.getLength();
            }
        }
        return length;
    }
    
    public void updateStream(StreamView stream) {
        if (stream != null) {
            ListExpr obj = viewer.getType("query " + stream.toString());
        
            if (obj != null) {
                String result = obj.second().textValue();
                if (result != null) {
                    stream.setState(result);
                }
            }
        }
        
    }
    
    public String[] getParameters() {
        String parameters[] = new String[activeStreams.size()];
        int i = 0;
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            parameters[i] = stream.getType();
            
            if (parameters[i] == null && stream.getObjects().size() > 0) {
                parameters[i] = stream.getObjects().get(0).getType();
            }
            i++;
        }
        return parameters;
    }
    
    public void getInfo() {
        infoDialog.removeText();
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            String result = stream.getState();
            String name = stream.getName();
            String countL = viewer.getCount(stream.toString());
            if (countL != null) {
                name += countL;
            }
            infoDialog.addInfo(name, result);
        }
        infoDialog.view();
    }
    
    public String getStrings(){
        String query = "";
        
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            query += stream.toString();
        }
        
        return query;
    }
    
    public String getStringsQuery(){
        return "query "+getStrings();
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
    
    public void addString(String s, String brackets) {
        String result = lastComponent.getName();
        result += brackets.toCharArray()[0] + s + brackets.toCharArray()[1];
        
        lastComponent.setName(result);
        viewer.update();
    }
    
    //Handle mouse events.
    
    public void mouseClicked ( MouseEvent e ) {
        
        //get the position of the click
        int y = 0;
        while (e.getY() > (10 + (y+1)*80)) { y++; }
        int x = 0;
        while (e.getX() > (10 + x*120)) { x++; }
        
        //right click on an object shows more information about it
        if (e.getButton() == 3) {
            if (e.getComponent() instanceof ObjectView) {
                ObjectView object = (ObjectView)e.getComponent();
            }
            else {
                getInfo();
            }
        }
        
        //double click sets the last object of the selected stream active
        if ((e.getClickCount () == 1) && (fullStream.size() > y) && (e.getButton() == 1)) {
            if (getStream(x,y) != null) {
                getStream(x,y).change();
                viewer.update();
            }
                
            if ((activeStream != null) && !activeStream.equals(fullStream.get(y))) {
                //lastComponent = activeStream.getLastComponent();
                infoDialog.setTitle(fullStream.get(y).getName());
            }
            
            
        }
    }
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {}
}
