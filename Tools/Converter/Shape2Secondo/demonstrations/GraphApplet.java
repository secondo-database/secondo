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
 * GraphApplet is based for the main part on example 4, this time with the addition some charting options.
 * <p>
 * Applet parameters
 *
 * The relative location of a zip file from a single peramiter in the applet tag for the applet:
 * param name="shapefile" value="nameOfShapefileWithoutExtension"
 * param name="tooltip" value="nameOfColumn"
 * param name="groups" value="numberOfGroups"
 * param name="groupXcol" value="nameOfColumn" // for each group
 * param name="groupXcolor" value="#rrggbb hex colour reference"
 * param name="groupXname" value="Description of group"
 * 
 * The applet demostrates how to set up a theme with a pie chart.
 * 
 * If you would like to use this applet as it is then edit BarGraph.html so that the parametiers points to the shapefile and columname that you want to display.
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
 * @since 0.7.8.1
 */

public class GraphApplet extends Applet implements java.awt.event.ItemListener
{
    //A Viewer is the component in which any maps are actualy displayed.
    Viewer view = new Viewer(); // Note it is declared and initalized here
    Panel pieKeys = new Panel(); //Panel to hold the pie chart key
    Panel mapKeys = new Panel(); //Panel to hold the map keys
    Panel pickShader = new Panel(); //Panel to hold radio button for each group
    
    PieChart graph; // the Pie chart
    Checkbox[] radios; // radio button for each group
    Panel sideBar; // to hold all of the keys, chart and radios.
    
    FirstPastPostShader fpps; //only used here as a quick way to build the key for the pie chart.
    //Future versions of charts should be able to produce their own keys.
    
    GeoData[] groupData; // The values for each group
    Key[] groupKeys; // Key for each groups shader
    ClassificationShader[]  groupShader; //Shader for each group
    Theme t;
    GeoLabel place = new GeoLabel(); //Linked label to display current state name
    boolean loaded = false; //Have the maps been loaded yet?
    
    //Initalise the applets comonents and sort out the layout
    public void init(){
        System.out.println("Graph Applet Demo v1.1.1");
        //Switch to BorderLayout
        this.setBackground(Color.white);
        setLayout(new BorderLayout(5,5)); // Small amount of white space between each section.
        
        //setup panels.
        Panel graphPanel = new Panel();
        sideBar = new Panel();
        pickShader.setLayout(new GridLayout(0,1));
        sideBar.setLayout(new GridLayout(2,2));
        
        add(view,"Center"); //Add the view to the center so that it will expand to fill available space
        
        //The toolbar widget object is a small panel with buttons to control a viewer.
        ToolBar tools = new ToolBar(view);//Constucted with the Viewer to control
        tools.add(new ZoomLevelPicker(view)); //Constructed with the Viewer to control
        add(tools,"South");//add the toolbar to the south of the applet
        
        //Add the key panel (which will have a key added to it latter) to the east of the applet.
        sideBar.add(mapKeys);
       
        graph = new PieChart(); 
        graph.setSize(50,50);
        
        add(sideBar,"East");
        
      
        sideBar.add(pickShader);
        
        graphPanel.setLayout(new BorderLayout());
        graphPanel.add(graph,"Center");
        
        //Set up place label
        Font f = new Font("Arial",Font.BOLD,14);
        place.setFont(f);
        graphPanel.add(place,"South");
        
        sideBar.add(graphPanel);
        sideBar.add(pieKeys);
       
        
        //that's it for initalization, the rest will come when start() is called.
}
    
    public void start(){
        //load maps
        try{
            if(!loaded){
                loaded=true;
                loadMaps();
            }
        }
        catch(IOException e){
            this.showStatus("Error loading map file "+e);
        }
    }
    
    //Keep a note of which group was displayed last.
    public int last =0;
    
    
    
    public void loadMaps() throws IOException{
        //setup the shapefile reader as in past examples.
        String shapefile = this.getParameter("shapefile"); //this should be a relative address.
        URL url = new URL(getCodeBase(),shapefile);
        ShapefileReader sfr = new ShapefileReader(url);
        
           
        //Grab the theme as normal
        t = sfr.getTheme();
     
        //setup tooltips as in example2
        String tooltip = this.getParameter("tooltip"); //this should be the name of a column in the dbf file
        GeoData tips = sfr.readData(tooltip);
        
        //Find how many groups there are
        int groups = Integer.parseInt(this.getParameter("groups")); 
        
        fpps = new FirstPastPostShader();//This shader is just being used to build a key.
        groupShader = new ClassificationShader[groups];//An array of shaders, one for each group
        groupData = new GeoData[groups];//An array of geodatas one for each group
        
        CheckboxGroup cbgroup = new CheckboxGroup();//Group for all of the checkboxes
        
        radios = new Checkbox[groups];
        groupKeys = new Key[groups];
        
        //loop through once for each group
        for(int i =1;i<=groups;i++){
            String name = this.getParameter("group"+i+"col"); //The column in the dbf file for this group
            System.out.println("Adding "+name);
           
            GeoData data = sfr.readData(name);//Grab the data
            
            String niceName = this.getParameter("group"+i+"name"); //Optional text name for this group
            
            if(niceName==null) niceName = name; // If no name was given revert to column name
            
            data.setName(niceName); // Set the name for the geodata to the one selected above
            radios[i-1] = new Checkbox(niceName,false,cbgroup);//Add a checkbox for this group
            radios[i-1].addItemListener(this);//Listen to it
            pickShader.add(radios[i-1]);
            
            
            Color color = Color.decode(this.getParameter("group"+i+"color"));//get and decode colour to use in pie chart
            System.out.println("Building shader for "+name);
            //Build a shader, this one has 7 classifications split by equal intervals.
            groupShader[i-1] = new ClassificationShader(data, 7,ClassificationShader.EQUAL_INTERVAL,Color.yellow.brighter().brighter(),Color.red);
            groupData[i-1] = data;
            fpps.addGroup(data,color);
            graph.addGroup(data,color);//add the data to the graph
            
            groupKeys[i-1] = groupShader[i-1].getKey();//get the key for that groups shader
         
        }
        radios[0].setState(true);//Set the first group as active
        
        System.out.println("Setting up highlight manager");
        mapKeys.add(groupKeys[0]);
        
        pieKeys.add(fpps.getKey());//pull the key from the shader, graphs should be able to do this themselves.
        
        //Create a higlight manager
        HighlightManager hm = new HighlightManager();
        //pass it to the theme
        t.setHighlightManager(hm);
        
      
        
        String highlightHex = this.getParameter("highlightColor");//get optional highlight color
        ShadeStyle hs = t.getHighlightShadeStyle();
        Color hiColor;
        if(highlightHex!=null){
            hiColor = Color.decode(highlightHex);
        }
        else{
            hiColor = Color.pink;
        }
        hs.setFillColor(hiColor);
        
        
        
        //add the graph component as a listener to the highlight manager
        hm.addHighlightChangedListener(graph);
        
       
        
        System.out.println("Setting shader");
        t.setShader(groupShader[0]);//set the theme up with the first shader and data pair.
        t.setGeoData(groupData[0]);
        
        //set the tool tip data as the tip data for the theme
        t.setTipData(tips);
        
        place.setGeoData(tips);//Set the data for the label
        hm.addHighlightChangedListener(place);//Add the label as a highlight listener
        
         //is there a starting point?
        String start = this.getParameter("startID"); //get optional starting pont
        
        if(start!=null){
            int startID = Integer.parseInt(start);
            hm.setHighlight(startID);//If set then set highlight (and graph+label) to given id.
        }
        
       //add the theme to the viewer
        view.addTheme(t);
    }
    
    /** 
     * Called when one of the group radio buttons has been selected.
     **/
    public void itemStateChanged(java.awt.event.ItemEvent ie){
        //Which group was chosen.
        Checkbox src = (Checkbox)ie.getSource();
        for(int i=0;i<radios.length;i++){
            if(radios[i] == src){
                //group found
                t.setShader(groupShader[i]);//Switch to the shader for that group
                t.setGeoData(groupData[i]);//Switch to the data for that group

                //Swap the keys over in the key panel
                mapKeys.add(groupKeys[i]);
                mapKeys.remove(groupKeys[last]);
                
                //Small fudge to get the keys to be the right size
                groupKeys[i].setSize(groupKeys[last].getSize());
                groupKeys[i].setLocation(groupKeys[last].getLocation());
                groupKeys[i].addNotify();
                last = i;
                groupKeys[i].doLayout();
                sideBar.doLayout();
                repaint();
                
                return;//finished
            }
        }
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
            { "groups","int","number of groups" },
            { "groupXcol","String","name of column wich holds group X"},
            { "groupXcolor","String","#rrggbb hex colour reference for shading group X"},  
	        {"tooltip","string","name of column in shapefiles dbf to pull tooltips from"},
                {"startID","int","Optional inital id to highlight and show graph for"},
                {"highlightColor","String","Optional highlight color #rrggbb - defaults to pink."}
        };
        return pinfo;
    }

    
}
