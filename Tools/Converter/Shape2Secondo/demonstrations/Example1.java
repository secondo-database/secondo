// first import standard java packages that we will need
import java.applet.*;
import java.awt.*;
import java.net.*;
import java.io.*;

//now import the main geotools package
import uk.ac.leeds.ccg.geotools.*;
//finaly import the toolbar from the widgets package
import uk.ac.leeds.ccg.widgets.ToolBar;

/**
 * Example 1 is the most simple configurable GeoTools applet possible.
 * The map that is displayed is shaded in dull gray with no attribute or label information used.
 * 
 *
 * It reads the relative location of a zip file from a single peramiter in the applet tag for the applet:
 * param name="shapefile" value="nameOfShapefileWithoutExtension"
 * 
 * The applet demostrates how to set up a Viewer, a ToolBar and how to add a theme to the Viewer.
 * The applet also shows how to setup and use a ShapefileReader to get a simple theme from a url.
 * 
 * If you would like to use this applet as it is then edit Example1.html so that the parametier points to the shapefile that you want to display.
 * <br> For best performance put the shp and dbf files into a zip file with the same name
 * <br> e.g.  map.shp, map.dbf -> map.zip
 *
 * If you have any questions or need help then contact the author at
 * j.macgill@geog.leeds.ac.uk
 * 
 * Note this applet was built and tested against GeoTools 0.7.8.1 but it should work just as well with older and newer versions.
 * 
 * @author James Macgill
 * @version 1.0
 * @since 0.7.8.1
 */

public class Example1 extends Applet
{
    //A Viewer is the component in which any maps are actualy displayed.
    Viewer view = new Viewer(); // Note it is declared and initalized here
    
    //Initalise the applets components and sort out the layout
    public void init(){
        //Switch to BorderLayout
        setLayout(new BorderLayout());
        add(view,"Center"); //Add the view to the center so that it will expand to fill available space
        
        //The toolbar widget object is a small panel with buttons to control a viewer.
        ToolBar tools = new ToolBar(view);//Constucted with the Viewer to control
        add(tools,"South");//add the toolbar to the south of the applet
        
        //that's it for initalization, the rest will come when start() is called.
    }
    
    public void start(){
        //load maps
        try{
            loadMaps();
        }
        catch(IOException e){
            this.showStatus("Error loading map file "+e);
        }
    }
    
    public void loadMaps() throws IOException{
        /* read the value for shapefile from the param tag in the html code.
         * This will be in the form <param name="shapefile" value="anyshapefile"
         * The shapefile should either be two files - anyshapefile.shp anyshapefile.dbf
         * or a single .zip file containing both.
         **/
        String shapefile = this.getParameter("shapefile"); //this should be a relative address.
        
        //build a full URL from the documentBase and the param fetched above. 
        URL url = new URL(getCodeBase(),shapefile);
        
        //Build a ShapefileReader from the above URL.  
        //ShapefileReaders allow access to both the geometry and the attribute data
        //contained within the shapefile.
        ShapefileReader sfr = new ShapefileReader(url);
        
        //Using the shapefileReader, a default theme object is created.
        //Other more advanced versions of getTheme are available which construct more interesting themes
        //by using attribute data
        Theme t = sfr.getTheme();
 
        //Finaly, add the theme created above to the Viewer
        view.addTheme(t);
        
        //Thats it, the rest is automatic.
    }
    
    /**
     * A Standard Applet method
     * @return String description of applet perameters.
     */
    public String getAppletInfo(){
        String info = "A very basic shapefile viewing applet demonstrating the use of GeoTools\nAuthor - James Macgill";
        return info;
    }
    
    /**
     * A Standard Applet method
     * @return String description of applet perameters.
     */
    public String[][] getParameterInfo(){
        String pinfo[][] = {
	        {"shapefile",    "relative url",    "location of shapefile to use"}
        };
        return pinfo;
    }

    
}