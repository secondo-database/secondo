// $Header$
import java.applet.*;
import java.awt.*;
import java.net.*;
import java.io.*;
import java.util.StringTokenizer;

import uk.ac.leeds.ccg.geotools.*;
import uk.ac.leeds.ccg.raster.*;
import uk.ac.leeds.ccg.widgets.ToolBar;

/** A simple applet demonstrating the use of imageLayer.
 * Note that this example need to be run through a web server (like Apache),
 * and the image needs to be on the same server as the applet, otherwise you
 * will get java security problems.
 * JM: Not sure this is true, works fine in a browser run localy for me...
 *
 * @author Cameron Shorter <a href="mailto:cameron@shorter.net">cameron@shorter.net</a>
 */
public class ImageExample extends java.applet.Applet {
    Viewer view = new Viewer();
    Theme currentTheme;
    ImageLayer iLayer;
    
    public void init(){
        setLayout(new BorderLayout());
        add(view,"Center");
        ToolBar tools = new ToolBar(view);
        add(tools,"South");
    }
    
    public void start(){
        //get the name of the image to display
        String imageString = this.getParameter("image");
        //and its bounds (in the form x,y,width,height)
        String boundsString = this.getParameter("bounds");
        //get each part of the bounds.
        StringTokenizer tok = new StringTokenizer(boundsString,",");
        double x = new Double(tok.nextToken()).doubleValue();
        double y = new Double(tok.nextToken()).doubleValue();
        double w =  new Double(tok.nextToken()).doubleValue();
        double h = new Double(tok.nextToken()).doubleValue();
        //construct a GeoRectangle
        GeoRectangle extent = new GeoRectangle(x,y,w,h);
        
        try{
            //create a URL that points to the image
            URL imageURL = new URL(getCodeBase(),imageString);
            //construct an image layer from the image and the bounds
            iLayer=new ImageLayer(imageURL,extent);
            //create a theme using the image layer
            currentTheme=new Theme(iLayer);
            //add the image to the viewer.
            view.addTheme(this.currentTheme);
        }
        catch(IOException e){
            this.showStatus("Error loading file "+ imageString + "\n" + e);
        }
    }
    
    /**
     * A Standard Applet method
     * @return String description of applet parameters.
     */
    public String getAppletInfo(){
        String info = "An image viewing applet.  Author - Cameron Shorter";
        return info;
    }
}
