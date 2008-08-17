

package movingregion;
import java.util.*;
import java.awt.*;
import java.io.*;
public class Region implements RegionTreeNode, Cloneable, Serializable
{
    Vector Faces;
    Vector OriginalVertices;
    double OffsetX;
    double OffsetY;
    double scaleX;
    double scaleY;
    static final long serialVersionUID =-7777879906774960304l;
    int sourceOrTarget;
    public Region()
    {
        Faces=new Vector();
    }
    public void addFace(Face newFace, int sourceOrTarget)
    {
        this.sourceOrTarget = sourceOrTarget;
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
    
    public Face[] getFaces()
    {
        Face[] res=new Face[getNrOfFaces()];
        for(int i=0;i< getNrOfFaces();i++)
        {
            res[i]=this.getFace(i);
        }
        return(res);
    }
    
    public void setSourceOrTarget(int sourceOrTarget)
    {
        this.sourceOrTarget=sourceOrTarget;
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            this.getFace(i).setSourceOrTarget(sourceOrTarget);
        }
    }
    
    public Face[] splitOnLine(LineWA[] splitLine)
    {
        Vector resv=new Vector();
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            Face trythis=this.getFace(i);
            Face tmp=trythis.splitOnLine(splitLine);
            if(tmp!=null)
            {
                this.addFace(tmp, this.sourceOrTarget);
                resv.add(tmp);
                resv.add(trythis);
            }
        }
        Face[] res=new Face[resv.size()];
        for(int i=0;i<resv.size();i++)
        {
            res[i]=(Face)resv.elementAt(i);
        }
        return(res);        
    }
    
    public void paintRegion(Graphics g, boolean isActive)
    {
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            this.getFace(i).paintFace(g,isActive);
        }
    }
    
    public String toString()
    {
        String res="";
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            res=res+"Facenr. "+i+'\n';
            res=res+this.getFace(i);
        }
        return(res);
    }
    
    
    public void removeFace(int index)
    {
        this.Faces.remove(index);
    }
    
    public void writeRegionToVRML( OutputStreamWriter fs,
            double time, String color,String shininess, String transparency) throws IOException
    {
        for(int i=0;i<getNrOfFaces();i++)
        {
            getFace(i).writeFaceToVRML(fs,time,color,shininess,transparency);
        }
    }
    
    private double[] readTwoDoubles(Reader f)throws IOException
    {
        double[] res=new double[2];
        char ch;
        String[] number=new String[2];
        number[0]="";
        number[1]="";
        int active=0;
        while ((ch=(char)f.read())!=')')
        {
            if(ch=='\n'||ch=='\r'||ch=='\b'||ch=='\f')
                continue;
            if (ch=='t')
                ch=' ';
            if(ch==' '&&number[active].equals(""))
                continue;
            if(ch==' '&&!(number[active].equals("")))
                active++;
            if(ch=='0'||ch=='1'||ch=='2'||ch=='3'||ch=='4'||ch=='5'||ch=='6'||ch=='7'||ch=='8'||ch=='9'||ch=='.'||ch=='-')
                number[active]=number[active]+ch;
        }
        
        res[0]=Double.parseDouble(number[0]);
        res[1]=Double.parseDouble(number[1]);
        return res;
    }
    
    protected Object clone()
    {
        Region res=new Region();
        for(int i=0;i<this.getNrOfFaces();i++)
        {
            Face tmp=(Face)this.getFace(i).clone();
            tmp.setParent(res);
            res.addFace(tmp,this.sourceOrTarget);
        }
        return(res);
    }
}
