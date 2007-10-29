

package movingregion;
import java.util.*;
import java.awt.*;
import java.io.*;
public class Region implements RegionTreeNode
{
    Vector Faces;

    public Region()
    {
        Faces=new Vector();
    }
    public void addFace(Face newFace)
    {
        Faces.add(newFace);        
    }
    public int getNrOfFaces()
    {
        return(Faces.size());
    }
    public Face getFace(int index)
    {
        return((Face)Faces.elementAt(index));
    }
    
    public void paintRegion(Graphics g, boolean isActive)
    {
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            this.getFace(i).paintFace(g,isActive);
        }
    }
    
    
    public void writeRegionToVRML( OutputStreamWriter fs,
            double time, String color,String shininess, String transparency) throws IOException
    {
        for(int i=0;i<getNrOfFaces();i++)
        {
            getFace(i).writeFaceToVRML(fs,time,color,shininess,transparency);
        }            
    }
}
