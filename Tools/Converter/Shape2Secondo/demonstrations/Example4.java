// first import standard java packages that we will need
import java.applet.*;
import java.awt.*;
import java.net.*;
import java.io.*;

//now import the main geotools package
import uk.ac.leeds.ccg.geotools.*;
//finaly import the toolbar from the widgets package
import uk.ac.leeds.ccg.widgets.*;

/**
 * Example 4 is based almost entierly on Example 2, this time with the addition of a shader and key.
 * <p>
 * Applet parameters
 *
 * The relative location of a zip file from a single peramiter in the applet tag for the applet:
 * param name="shapefile" value="nameOfShapefileWithoutExtension"
 * param name="tooltip" value="nameOfColumn"
 * param name="shadeby" value="nameOfColumn"
 * 
 * The applet demostrates how to set up a theme with a classification shader
 * 
 * If you would like to use this applet as it is then edit Example3.html so that the parametiers points to the shapefile and columname that you want to display.
 * <br> For best performance put the shp and dbf files into a zip file with the same name
 * <br> e.g.  map.shp, map.dbf -> map.zip
 *
 * If you have any questions or need help then contact the author at<br>
 * j.macgill@geog.leeds.ac.uk
 * <p>
 * Note this applet was built and tested against GeoTools 0.7.9dev1 
 * 
 * @author James Macgill
 * @version 1.0
 * @since 0.7.9dev1
 */

public class Example4 extends Applet
{
    //A Viewer is the component in which any maps are actualy displayed.
    Viewer view = new Viewer(); // Note it is declared and initalized here
    Panel keys = new Panel();
    
    //Initalise the applets comonents and sort out the layout
    public void init(){
        //Switch to BorderLayout
        setLayout(new BorderLayout());
        add(view,"Center"); //Add the view to the center so that it will expand to fill available space
        
        //The toolbar widget object is a small panel with buttons to control a viewer.
        ToolBar tools = new ToolBar(view);//Constucted with the Viewer to control
        //Add a zoom level picker to control the zoom level by %
        tools.add(new ZoomLevelPicker(view));//The picker is constructed using the view that it is to control
        add(tools,"South");//add the toolbar to the south of the applet
        
        //Add the key panel (which will have a key added to it latter) to the east of the applet.
        add(keys,"East");
        
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
        //setup the shapefile reader as in past examples.
        String shapefile = this.getParameter("shapefile"); //this should be a relative address.
        URL url = new URL(getCodeBase(),shapefile);
        ShapefileReader sfr = new ShapefileReader(url);
        
        
           
        //Pull the column to shade by from applet param tag
        String shadeby = this.getParameter("shadeby"); //this should be the name of a column in the dbf file
        
        //use shapefile reader to construct a geodata from that column
        GeoData shadeData = sfr.readData(shadeby);
        
        //Build a classification shader with 6 classes, spit into quantiles and shaded from green to orange (yuck)
        //N.B. the shadeData is used ONLY to set the range of values to be used by the shader.
        Shader shade = new ClassificationShader(shadeData, 6,ClassificationShader.QUANTILE,Color.green,Color.orange); 
        
        //Grab the theme as normal
        Theme t = sfr.getTheme();
        
        //set the themes shader to the one constructed above
        t.setShader(shade);
        //set the data to be used when shading.  This is a very important step, and seperate from passing the shade data to the shader above.
        t.setGeoData(shadeData);
        
        //Get the key object from the shader and add it to the key panal that was setup in the init phase.
        keys.add(shade.getKey());
        
        //setup tooltips as in example2
        String tooltip = this.getParameter("tooltip"); //this should be the name of a column in the dbf file
        GeoData tips = sfr.readData(tooltip);
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
                {"shadeby",    "string",    "name of column in shapefiles dbf shade map by"},
	        {"tooltip",    "string",    "name of column in shapefiles dbf to pull tooltips from"}
        };
        return pinfo;
    }

    
}