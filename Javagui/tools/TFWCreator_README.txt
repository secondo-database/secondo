The TFW Creator Tool
====================

What is a tfw file
------------------
For using bitmap images as background image for geographical objects,
the position of this picture must be known. A tfw file contains 
information about the left buttom corner, the size of a pixel of the
image in the 'real world', and rotation parameters. 

In the HoeseViewer of Javagui, such a tfw file can be evaluated for 
setting the bounding box of an image in the "Settings->Background->Set Image"
dialog. 

Note: In the current implementation, the rotation information is not evaluated 
      in the HoeseViewer. 

Requirements for using the TFWCreator
-------------------------------------
For creating the information for a tfw file, the map must be in a Java readable
format (jpg, gif, or png). Because the rotation is not computed (always set to
0.0000), the map must also be stricly vertical. The map must be projected via
the Gauss-Krueger projection. This will be the case in the most standard maps
of germany.
Furthermore, the positions of two points in the map must be known. The coordinates
can be given as geographical coordinates(wgs84 or bessel) or directly as
coordinates projected by the Gauss-Krueger projection. You can got such positions
using a PDA with GPS recveiver aided by the tool 'kompass' (downloadable at 
http://www.bb-eng.com/Downloads.htm ). via the "Save Position" function.
The best quality will be reached, when the points are at the upper-left (upper-right)
and the bottom-right (bottom-left) corners of the map.

The TFWCreator
--------------
The first thing to do, is the compile the Javagui of SECONDO - the TFWCreator will
compiled together with Javagui. After that, you can start the tool in the Javagui main
directory. According to the size of the map, you should choose a fitting value for the
memory which can use this tool. In this example, 250 MB can be used. Call the tool 
from a console by typing in:
	java -Xmx250M tools.TFWCreator

A new window will be opened. At the left side, an empty area can be shown and at the 
right side are a lot of labels, buttons, and text fields. 
First, you should load the map using the top load button. The left area will be 
filled with this image. 

After that, the points must be set in the map. To do that, click on a set button.
The buttun will be red colored to indicate to be active. Scroll the map to the 
known position and click on it. The position will be marked by a cross.
Repeat this for the second point using the second set-button. 

Now, the same positions in the world must be fixed. The simplest way is to load 
a *.pos file for each point. The load functionality includes also point objects in
SECONDO's nested list format. If the positions are not stored in a file, you can 
also type in them directly. 

Select the kind of the coordinates in the world depending on the used GPS-receiver.
Normally, GPS-receiver in germany provide the position information in the wgs84
ellipsoid (default). Additionally, enter the used meridian. 
The meridian can be computed as
    round(x/3.0) 
In the case that the two points use different meridians, you have to select one of them.

Finally, press the Create TFW button at the bottom corner in the TFWCreator. Select a
filename and store the file. 



