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
 * Example 2 is based almost entierly on Example 1, this time with the addition of tool tips.
 * The map that is displayed is shaded in dull gray, however, when the pointer rests over
 * one of the features on the map, a small box appears providing information about that feature.
 * <p>
 * Applet parameters
 *
 * The relative location of a zip file from a single peramiter in the applet tag for the applet:
 * param name="shapefile" value="nameOfShapefileWithoutExtension"
 * param name="tooltip" value="nameOfColumn"
 * 
 * The applet demostrates how to set up a theme with tooltip data.
 * 
 * If you would like to use this applet as it is then edit Example2.html so that the parametiers points to the shapefile and columname that you want to display.
 * <br> For best performance put the shp and dbf files into a zip file with the same name
 * <br> e.g.  map.shp, map.dbf -> map.zip
 *
 * If you have any questions or need help then contact the author at<br>
 * j.macgill@geog.leeds.ac.uk
 * <p>
 * Note this applet was built and tested against GeoTools 0.7.8 
 * 
 * @author James Macgill
 * @version 1.0
 * @since 0.7.8.1
 */

public class Example2 extends Applet
{
    //A Viewer is the component in which any maps are actualy displayed.
    Viewer view = new Viewer(); // Note it is declared and initalized here
    
    //Initalise the applets comonents and sort out the layout
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
        String shapefile = this.getParameter("shapefile"); //this should be a relative address.
        URL url = new URL(getCodeBase(),shapefile);
        ShapefileReader sfr = new ShapefileReader(url);
        Theme t = sfr.getTheme();
        
        //read the value for tooltip from the param tag in the html code.
        String tooltip = this.getParameter("tooltip"); //this should be the name of a column in the dbf file
 
        //GeoData objects act as a lookup between an ID and a value of some form for each feature
        //the following line constructs a GeoData that maps each features ID to the column in the dbf file selected above.
        GeoData tips = sfr.readData(tooltip);
        
        //The GeoData created above is now used as the Tip Data for the theme.
        t.setTipData(tips);
       
        view.addTheme(t);
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
	        {"shapefile",    "relative url",    "location of shapefile to use"},
	        {"tooltip",    "string",    "name of column in shapefiles dbf to pull tooltips from"}
        };
        return pinfo;
    }

    
}